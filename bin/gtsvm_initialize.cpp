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
	\file gtsvm_initialize.cpp
*/




#include "headers.hpp"




//============================================================================
//    main function
//============================================================================


int main( int argc, char* argv[] ) {

	int resultCode = EXIT_SUCCESS;

	std::string dataset;
	std::string output;
	bool multiclass;
	float regularization = std::numeric_limits< float >::quiet_NaN();
	std::string kernelName;
	float kernelParameter1 = std::numeric_limits< float >::quiet_NaN();
	float kernelParameter2 = std::numeric_limits< float >::quiet_NaN();
	float kernelParameter3 = std::numeric_limits< float >::quiet_NaN();
	bool biased;

	boost::program_options::options_description description( "Allowed options" );
	description.add_options()
		( "help,h", "display this help" )
		( "file,f", boost::program_options::value< std::string >( &dataset ), "dataset file (SVM-Light format)" )
		( "output,o", boost::program_options::value< std::string >( &output ), "output model file" )
		( "multiclass,m", boost::program_options::value< bool >( &multiclass )->default_value( false ), "is this a multiclass problem?" )
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
				"Creates a model file from the given dataset (in SVM-Light format), and" << std::endl <<
				"initializes it to the zero classifier--use gtsvm_optimize to actually find an" << std::endl <<
				"optimal classifier. The regularization parameter is that of the C-formulation," << std::endl <<
				"while the kernel parameter must be one of \"gaussian\", \"polynomial\" or" << std::endl <<
				"\"sigmoid\", for which the kernel functions are:" << std::endl <<
				"\tgaussian    ==>  K( x, y ) = exp( -p1 * || x - y ||^2 )" << std::endl <<
				"\tpolynomial  ==>  K( x, y ) = ( p1 * <x,y> + p2 )^p3" << std::endl <<
				"\tsigmoid     ==>  K( x, y ) = tanh( p1 * <x,y> + p2 )" << std::endl <<
				"Here, \"p1\", \"p2\" and \"p3\" are the values given for parameter1, parameter2 and" << std::endl <<
				"parameter3, respectively. The multiclass and biased parameters select whether" << std::endl <<
				"the optimization problem is a multiclass problem, and whether it should include" << std::endl <<
				"an unregularized bias." << std::endl <<
				std::endl <<
				description << std::endl;
		}
		else {

			if ( ! variables.count( "file" ) )
				throw std::runtime_error( "You must provide a dataset file" );
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

			// load the dataset file
			unsigned int rows    = 0;
			unsigned int columns = 0;
			std::vector< boost::int32_t > labels;
			std::vector< float   > values;
			std::vector< size_t  > indices;
			std::vector< size_t  > offsets;
			{	unsigned int const bufferSize = ( 1u << 24 );
				boost::shared_array< char > buffer( new char[ bufferSize ] );

				const boost::regex spaceRegex( "[[:space:]]+" );
				const boost::regex elementRegex( "^[[:space:]]*([[:digit:]]+):(-?[[:digit:]]+(\\.[[:digit:]]+)?([eE]-?[[:digit:]]+)?)[[:space:]]*$" );

				offsets.push_back( 0 );

				std::ifstream file( dataset.c_str() );
				if ( file.fail() )
					throw std::runtime_error( "Unable to open dataset file" );
				while ( ! file.eof() ) {

					file.getline( buffer.get(), bufferSize );
					if ( file.fail() )
						break;
					std::string lineString( buffer.get() );

					boost::sregex_token_iterator ii(
						lineString.begin(),
						lineString.end(),
						spaceRegex,
						-1
					);
					boost::sregex_token_iterator iiEnd;

					if ( ii != iiEnd ) {    // ignore blank lines

						std::string const labelString = ii->str();
						if ( ii == iiEnd )
							throw std::runtime_error( "Failed to parse first element of dataset line" );
						int label = atoi( labelString.c_str() );

						int lastIndex = -1;
						for ( ++ii; ii != iiEnd; ++ii ) {

							std::string const elementString = ii->str();
							boost::smatch elementMatch;
							if ( ! boost::regex_match( elementString, elementMatch, elementRegex, boost::match_extra ) )
								throw std::runtime_error( "Failed to parse element of dataset line" );

							std::string const indexString = elementMatch[ 1 ].str();
							int index = atoi( indexString.c_str() );
							std::string const valueString = elementMatch[ 2 ].str();
							float const value = static_cast< float >( atof( valueString.c_str() ) );
							if ( index < 0 )
								throw std::runtime_error( "Failed to parse element of dataset line: negative index encountered" );
							if ( index <= lastIndex )
								throw std::runtime_error( "Failed to parse element of dataset line: features must be listed in order of increasing index" );
							lastIndex = index;

							if ( value != 0 ) {

								values.push_back( value );
								indices.push_back( index );

								if ( index + 1 > static_cast< int >( columns ) )
									columns = index + 1;
							}
						}

						BOOST_ASSERT( values.size() == indices.size() );

						labels.push_back( label );
						offsets.push_back( values.size() );

						BOOST_ASSERT( labels.size() + 1 == offsets.size() );
					}
				}
				file.close();

				rows = offsets.size() - 1;
			}

			//std::cout <<  kernelParameter1  << std::endl;

			AutoContext context;

			if (
				GTSVM_InitializeSparse(
					context,
					&values[ 0 ],
					&indices[ 0 ],
					&offsets[ 0 ],
					GTSVM_TYPE_FLOAT,
					&labels[ 0 ],
					GTSVM_TYPE_INT32,
					rows,
					columns,
					false,
					multiclass,
					regularization,
					kernel,
					kernelParameter1,
					kernelParameter2,
					kernelParameter3,
					biased,
					false,
					1
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
