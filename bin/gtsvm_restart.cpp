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
	\file gtsvm_restart.cpp
*/




#include "headers.hpp"




//============================================================================
//    main function
//============================================================================


int main( int argc, char* argv[] ) {

	int resultCode = EXIT_SUCCESS;

	std::string input;
	std::string output;
	float regularization = std::numeric_limits< float >::quiet_NaN();
	std::string kernelName;
	float kernelParameter1 = std::numeric_limits< float >::quiet_NaN();
	float kernelParameter2 = std::numeric_limits< float >::quiet_NaN();
	float kernelParameter3 = std::numeric_limits< float >::quiet_NaN();
	bool biased;

	boost::program_options::options_description description( "Allowed options" );
	description.add_options()
		( "help,h", "display this help" )
		( "input,i", boost::program_options::value< std::string >( &input ), "input model file" )
		( "output,o", boost::program_options::value< std::string >( &output ), "output model file (may be same as input)" )
		( "regularization,C", boost::program_options::value< float >( &regularization ), "regularization parameter" )
		( "kernel,k", boost::program_options::value< std::string >( &kernelName ), "kernel" )
		( "parameter1,1", boost::program_options::value< float >( &kernelParameter1 ), "first kernel parameter" )
		( "parameter2,2", boost::program_options::value< float >( &kernelParameter2 ), "second kernel parameter" )
		( "parameter3,3", boost::program_options::value< float >( &kernelParameter3 ), "third kernel parameter" )
		( "biased,b", boost::program_options::value< bool >( &biased )->default_value( false ), "include an unregularized bias?" )
	;

	try {

		boost::program_options::variables_map variables;
		boost::program_options::store( boost::program_options::command_line_parser( argc, argv ).options( description ).run(), variables );
		boost::program_options::notify( variables );

		if ( variables.count( "help" ) ) {

			std::cout <<
				"Resets a model file to the zero classifier, and changes the values of" << std::endl <<
				"parameters--use gtsvm_optimize to actually find an optimal classifier. The" << std::endl <<
				"regularization parameter is that of the C-formulation, while the kernel" << std::endl <<
				"parameter must be one of \"gaussian\", \"polynomial\" or \"sigmoid\", for which the" << std::endl <<
				"kernel functions are:" << std::endl <<
				"\tgaussian    ==>  K( x, y ) = exp( -p1 * || x - y ||^2 )" << std::endl <<
				"\tpolynomial  ==>  K( x, y ) = ( p1 * <x,y> + p2 )^p3" << std::endl <<
				"\tsigmoid     ==>  K( x, y ) = tanh( p1 * <x,y> + p2 )" << std::endl <<
				"Here, \"p1\", \"p2\" and \"p3\" are the values given for parameter1, parameter2 and" << std::endl <<
				"parameter3, respectively. The biased parameter selects whether the optimization" << std::endl <<
				"problem should include an unregularized bias." << std::endl <<
				std::endl <<
				description << std::endl;
		}
		else {

			if ( ! variables.count( "input" ) )
				throw std::runtime_error( "You must provide an input file" );
			if ( ! variables.count( "output" ) )
				throw std::runtime_error( "You must provide an output file" );
			if ( ! variables.count( "regularization" ) )
				throw std::runtime_error( "You must provide a regularization parameter" );
			if ( ! variables.count( "kernel" ) )
				throw std::runtime_error( "You must provide a kernel parameter" );

			GTSVM_Kernel kernel;
			if ( boost::iequals( kernelName, "gaussian" ) ) {

				kernel = GTSVM_KERNEL_GAUSSIAN;
				if ( ! variables.count( "parameter1" ) )
					throw std::runtime_error( "You must provide parameter1 for the Gaussian kernel" );
				if ( variables.count( "parameter2" ) )
					throw std::runtime_error( "The Gaussian kernel does not require parameter2" );
				if ( variables.count( "parameter3" ) )
					throw std::runtime_error( "The Gaussian kernel does not require parameter3" );
			}
			else if ( boost::iequals( kernelName, "polynomial" ) ) {

				kernel = GTSVM_KERNEL_POLYNOMIAL;
				if ( ! variables.count( "parameter1" ) )
					throw std::runtime_error( "You must provide parameter1 for the polynomial kernel" );
				if ( ! variables.count( "parameter2" ) )
					throw std::runtime_error( "You must provide parameter2 for the polynomial kernel" );
				if ( ! variables.count( "parameter3" ) )
					throw std::runtime_error( "You must provide parameter3 for the polynomial kernel" );
			}
			else if ( boost::iequals( kernelName, "sigmoid" ) ) {

				kernel = GTSVM_KERNEL_SIGMOID;
				if ( ! variables.count( "parameter1" ) )
					throw std::runtime_error( "You must provide parameter1 for the sigmoid kernel" );
				if ( ! variables.count( "parameter2" ) )
					throw std::runtime_error( "You must provide parameter2 for the sigmoid kernel" );
				if ( variables.count( "parameter3" ) )
					throw std::runtime_error( "The sigmoid kernel does not require parameter3" );
			}
			else
				throw std::runtime_error( "The kernel parameter must be one of \"gaussian\", \"polynomial\" and \"sigmoid\"" );

			AutoContext context;

			if (
				GTSVM_Load(
					context,
					input.c_str(),
					false,
					1
				)
			)
			{
				throw std::runtime_error( GTSVM_Error() );
			}

			if (
				GTSVM_Restart(
					context,
					regularization,
					kernel,
					kernelParameter1,
					kernelParameter2,
					kernelParameter3,
					biased
				)
			)
			{
				throw std::runtime_error( GTSVM_Error() );
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
