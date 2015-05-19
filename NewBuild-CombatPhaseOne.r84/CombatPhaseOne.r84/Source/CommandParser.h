// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2012 - Michael Lingg
// http://mxoemu.info
//
// ---------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ---------------------------------------------------------------------------
//
// The Command Parser processes a chat string for world building or admin
// commands.
//
// World building commands are prefixed with a $.
// ?Add command list here?
//
// Admin commands are prefixed with &
// ?Add command list here?
//
// ***************************************************************************

#ifndef MXOSIM_COMMANDPARSER_H
#define MXOSIM_COMMANDPARSER_H

#include "Common.h"



class CommandParser
{

	public :

			// The different types of chat commands available including
			// identifying a plain text message that does not include a
			// command.
		typedef enum CommandType_T
		{
			FIRST_TYPE = 0 ,
			BUILD = FIRST_TYPE ,
			ADMIN ,
			NON ,
		} ;

			// The list of prefix characters for each Command Type, strings specified
			// in the cpp file.
		static char CommandChars[NON] ;

			// The list of commands.
			// Note that some values have multiple enums in this list,
			// allowing for pointers to key points in the list.
			//
			// TBD: Planning on making Add_Patrol_Path and Point generic to become either a patrol or transit route.
		typedef enum Command_T
		{
			FIRST_BUILD = 0 ,
			ADD_SPAWN_POINT = FIRST_BUILD ,
			ADD_PATROL_PATH ,
			ADD_PATROL_POINT ,
			UNKNOWN_BUILD ,
			LAST_BUILD = UNKNOWN_BUILD ,
			FIRST_ADMIN = LAST_BUILD + 1 ,
			SPAWN_NPC = FIRST_ADMIN ,
			ADD_NPC_PATROL ,
			ADD_NPC_TRANSIT ,
			SET_NPC_SPEED ,
			RUN_ANIMATION ,
			SPAWN_INTERLOCK ,
			UNKOWN_ADMIN ,
			LAST_ADMIN = UNKOWN_ADMIN ,
			LAST_COMMAND = LAST_ADMIN ,
			NON_COMMAND = LAST_COMMAND + 1 ,
		} ;

			// The list of strings for each Command, strings specified
			// in the cpp file.
			// TBD? Combine CommandStrings and NumArgs into a single array of structs
		static std::string CommandStrings[NON_COMMAND] ;

			// The number of arguments expected for each command.
			// Values specified in the cpp file.
		static unsigned NumArgs[NON_COMMAND] ;

			//
			// Parse the chat string and store key results.
			//
			// Commands are identified by a command character followed
			// immediately by the case sensitive command (may change
			// to case insensitive.  Arguments are identified by one
			// or more space separating each token.
			//
		CommandParser ( std::string TheMessage ) ;
		~CommandParser ( void ) ;

			//
			// Returns the command type identified in the string.
			//
		Command_T GetCommand ( void ) ;

			//
			// Returns the number of arguments identified in the string.
			//
		unsigned GetArgumentCount ( void ) ;

			//
			//	Returns the argument at the specified index.
			//
		std::string GetArgument ( unsigned Index ) ;
		
	private :
	
			// The command parsed from the constructor string.
		Command_T TheCommand ;

			// The list of arguments from the constructor string.
		vector<std::string> ArgumentList ;

			// Is true if the number of arguments found match with the number of
			// arguments expected.
		bool ArgumentsGood ;

			// Parses the arguments after the command string.
		void parseArguments ( std::string Arguments ) ;
} ;

#endif MXOSIM_COMMANDPARSER_H