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
	\file gtsvm_recalculate.cpp
*/




#include "headers.hpp"




//============================================================================
//    main function
//============================================================================


int main( int argc, char* argv[] ) {

	int resultCode = EXIT_SUCCESS;

	std::string input;
	std::string output;
	bool smallClusters;
	unsigned int activeClusters;

	boost::program_options::options_description description( "Allowed options" );
	description.add_options()
		( "help,h", "display this help" )
		( "input,i", boost::program_options::value< std::string >( &input ), "input model file" )
		( "output,o", boost::program_options::value< std::string >( &output ), "output model file (may be same as input)" )
		( "small_clusters,s", boost::program_options::value< bool >( &smallClusters )->default_value( false ), "use size-16 instead of size-256 clusters?" )
		( "active_clusters,a", boost::program_options::value< unsigned int >( &activeClusters )->default_value( 64 ), "number of \"active\" clusters" )
	;

	try {

		boost::program_options::variables_map variables;
		boost::program_options::store( boost::program_options::command_line_parser( argc, argv ).options( description ).run(), variables );
		boost::program_options::notify( variables );

		if ( variables.count( "help" ) ) {

			std::cout <<
				"During optimization, certain running sums are computed, which may, over time," << std::endl <<
				"become out-of-sync with their true values. In practice, we have not found this" << std::endl <<
				"to be a problem, but in case it pops up, this program will recalculate all of" << std::endl <<
				"these sums from scratch." << std::endl <<
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

			if ( GTSVM_Recalculate( context ) )
				throw std::runtime_error( GTSVM_Error() );

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
