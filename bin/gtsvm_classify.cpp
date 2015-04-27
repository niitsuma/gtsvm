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
	\file gtsvm_classify.cpp
*/




#include "headers.hpp"




//============================================================================
//    main function
//============================================================================


int main( int argc, char* argv[] ) {

	int resultCode = EXIT_SUCCESS;

	std::string dataset;
	std::string input;
	std::string output;
	bool smallClusters;
	unsigned int activeClusters;

	boost::program_options::options_description description( "Allowed options" );
	description.add_options()
		( "help,h", "display this help" )
		( "file,f", boost::program_options::value< std::string >( &dataset ), "dataset file (SVM-Light format)" )
		( "input,i", boost::program_options::value< std::string >( &input ), "input model file" )
		( "output,o", boost::program_options::value< std::string >( &output ), "output text file" )
		( "small_clusters,s", boost::program_options::value< bool >( &smallClusters )->default_value( false ), "use size-16 instead of size-256 clusters?" )
		( "active_clusters,a", boost::program_options::value< unsigned int >( &activeClusters )->default_value( 64 ), "number of \"active\" clusters" )
	;

	try {

		boost::program_options::variables_map variables;
		boost::program_options::store( boost::program_options::command_line_parser( argc, argv ).options( description ).run(), variables );
		boost::program_options::notify( variables );

		if ( variables.count( "help" ) ) {

			std::cout <<
				"Classifies a dataset (in SVM-Light format). For binary classifiers, the result" << std::endl <<
				"is saved to a text file containing one floating-point number per line: the" << std::endl <<
				"value of the classification function applied the corresponding testing vector" << std::endl <<
				"(the signs of these numbers are the classifications). For multiclass" << std::endl <<
				"classifiers, each line of the output file contains a comma-separated list of" << std::endl <<
				"values of the classification functions of the classes, applied to the" << std::endl <<
				"corresponding testing vector (the index of the largest of these is the" << std::endl <<
				"classification)." << std::endl <<
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

			if ( ! variables.count( "file" ) )
				throw std::runtime_error( "You must provide a dataset file" );
			if ( ! variables.count( "input" ) )
				throw std::runtime_error( "You must provide an input file" );
			if ( ! variables.count( "output" ) )
				throw std::runtime_error( "You must provide an output file" );

			// load the dataset file
			unsigned int rows    = 0;
			unsigned int columns = 0;
			std::vector< float  > values;
			std::vector< size_t > indices;
			std::vector< size_t > offsets;
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

						std::string const signString = ii->str();
						if ( ii == iiEnd )
							throw std::runtime_error( "Failed to parse first element of dataset line" );
						// we don't care about the value of the label, since we're classifying

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

						offsets.push_back( values.size() );
					}
				}
				file.close();

				rows = offsets.size() - 1;
			}

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
				GTSVM_Shrink(
					context,
					smallClusters,
					activeClusters
				)
			)
			{
				throw std::runtime_error( GTSVM_Error() );
			}

			unsigned int classes;
			if (
				GTSVM_GetClasses(
					context,
					&classes
				)
			)
			{
				throw std::runtime_error( GTSVM_Error() );
			}

			boost::shared_array< double > result( new double[ rows * classes ] );
			if (
				GTSVM_ClassifySparse(
					context,
					result.get(),
					GTSVM_TYPE_DOUBLE,
					&values[ 0 ],
					&indices[ 0 ],
					&offsets[ 0 ],
					GTSVM_TYPE_FLOAT,
					rows,
					columns,
					false
				)
			)
			{
				throw std::runtime_error( GTSVM_Error() );
			}

			{	std::ofstream file( output.c_str() );
				if ( file.fail() )
					throw std::runtime_error( "Unable to open output file" );
				for ( unsigned int ii = 0; ii < rows; ++ii ) {

					file << result[ ii * classes + 0 ];
					for ( unsigned int jj = 1; jj < classes; ++jj )
						file << ", " << result[ ii * classes + jj ];
					file << std::endl;
				}
				file.close();
			}
		}
	}
	catch( std::exception& error ) {

		std::cerr << "Error: " << error.what() << std::endl << std::endl << description << std::endl;
		resultCode = EXIT_FAILURE;
	}

	return resultCode;
}
