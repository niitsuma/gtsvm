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
	\file gtsvm_shrink.cpp
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
				"Removes all vectors from the dataset file which have an associated dual" << std::endl <<
				"variable of zero. This is entirely optional--its purpose is only to shrink the" << std::endl <<
				"model file after optimization, but before classification, by removing" << std::endl <<
				"unnecessary information." << std::endl <<
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
