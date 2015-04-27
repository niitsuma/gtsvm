/*
	Copyright (C) 2011  Andrew Cotter

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/**
	\file gtsvm_optimize.cpp
*/




#include "headers.hpp"




//============================================================================
//    main function
//============================================================================


int main( int argc, char* argv[] ) {

	int resultCode = EXIT_SUCCESS;

	std::string input;
	std::string output;
	double epsilon = std::numeric_limits< double >::quiet_NaN();
	unsigned int iterations;
	bool smallClusters;
	unsigned int activeClusters;

	boost::program_options::options_description description( "Allowed options" );
	description.add_options()
		( "help,h", "display this help" )
		( "input,i", boost::program_options::value< std::string >( &input ), "input model file" )
		( "output,o", boost::program_options::value< std::string >( &output ), "output model file (may be same as input)" )
		( "epsilon,e", boost::program_options::value< double >( &epsilon ), "termination threshold" )
		( "iterations,n", boost::program_options::value< unsigned int >( &iterations ), "maximum number of iterations" )
		( "small_clusters,s", boost::program_options::value< bool >( &smallClusters )->default_value( false ), "use size-16 instead of size-256 clusters?" )
		( "active_clusters,a", boost::program_options::value< unsigned int >( &activeClusters )->default_value( 64 ), "number of \"active\" clusters" )
	;

	try {

		boost::program_options::variables_map variables;
		boost::program_options::store( boost::program_options::command_line_parser( argc, argv ).options( description ).run(), variables );
		boost::program_options::notify( variables );

		if ( variables.count( "help" ) ) {

			std::cout <<
				"Optimizes the SVM problem contained in the input model file, saving the result" << std::endl <<
				"to the output model file. Optimization will continue until either the" << std::endl <<
				"normalized duality gap (2(p-d)/(p-d), where \"p\" is the value of the primal" << std::endl <<
				"objective, and \"d\" the dual) is smaller than epsilon, or the maximum number of" << std::endl <<
				"iterations has been exceeded." << std::endl <<
				std::endl <<
				"This implementation handles sparsity using a greedy clustering approach. The" << std::endl <<
				"small_clusters parameter indicates the size of the clusters: 16 (small) or 256" << std::endl <<
				"(not small). Generally, size-256 clusters will give significantly better" << std::endl <<
				"performance. The active_clusters parameter is the number of clusters which will" << std::endl <<
				"be active at every point in the greedy clustering algorithm. We have found that" << std::endl <<
				"64 works well, but increasing this number will improve the quality of the" << std::endl <<
				"clustering (at the cost of more time being required to find it)." << std::endl <<
				std::endl <<
				description << std::endl;
		}
		else {

			if ( ! variables.count( "input" ) )
				throw std::runtime_error( "You must provide an input file" );
			if ( ! variables.count( "output" ) )
				throw std::runtime_error( "You must provide an output file" );
			if ( ! variables.count( "epsilon" ) )
				throw std::runtime_error( "You must provide a epsilon parameter" );
			if ( boost::math::isinf( epsilon ) )
				throw std::runtime_error( "The epsilon parameter must be finite" );
			if ( boost::math::isnan( epsilon ) )
				throw std::runtime_error( "The epsilon parameter cannot be NaN" );
			if ( epsilon <= 0 )
				throw std::runtime_error( "The epsilon parameter must be positive" );

			AutoContext context;

			if (
				GTSVM_Load(
					context,
					input.c_str(),
					smallClusters,
					activeClusters
				)
			)
			{
				throw std::runtime_error( GTSVM_Error() );
			}

			{	unsigned int const repetitions = 256;    // must be a multiple of 16

				for ( unsigned int ii = 0; ii < iterations; ii += repetitions ) {

					double primal =  std::numeric_limits< double >::infinity();
					double dual   = -std::numeric_limits< double >::infinity();
					if (
						GTSVM_Optimize(
							context,
							&primal,
							&dual,
							repetitions
						)
					)
					{
						throw std::runtime_error( GTSVM_Error() );
					}
					std::cout << "Iteration " << ( ii + 1 ) << '/' << iterations << ", primal = " << primal << ", dual = " << dual << std::endl;
					if ( 2 * ( primal - dual ) < epsilon * ( primal + dual ) )
						break;
				}
			}

			if ( GTSVM_Save( context, output.c_str() ) )
				throw std::runtime_error( GTSVM_Error() );
		}
	}
	catch( std::exception& error ) {

		std::cerr << "Error: " << error.what() << std::endl << std::endl << description << std::endl;
		resultCode = EXIT_FAILURE;
	}

	return resultCode;
}
