#include "CommandParser.h"



char CommandParser::CommandChars[CommandParser::NON]  =
{
	'$' ,
	'&' ,
} ;

std::string CommandParser::CommandStrings[CommandParser::NON_COMMAND] =
{
	"AddSpawnPoint" ,
	"AddPatrolPath" ,
	"AddPatrolPoint" ,
	"" , // This index is for a bad build command.
	"SpawnNPC" ,
	"AddNPCPatrol" ,
	"AddNPCTransit" ,
	"SetNPCSpeed" ,
	"RunAnimation" ,
	"SpawnInterlock" ,
	"" , // This index is for a bad admin command.
} ;

unsigned CommandParser::NumArgs[CommandParser::NON_COMMAND] =
{
	0 , // AddSpawnPoint
	0 , // AddPatrolPath
	1 , // AddPatrolPoint
	0 , // Invalid, bad build.
	1 , // SpawnNPC
	2 , // AddNPCPatrol
	2 , // AddNPCTransit
	2 , // SetNPCSpeed
	2 , // RunAnimationCode
	2 , // SpawnInterlock
	0 , // Invalid, bad command.
} ;


CommandParser::CommandParser ( std::string TheMessage )
{
	unsigned typeIndex ;
	unsigned commandIndex ;

		// Loop through all command type chars and see if any match with the first
		// char of the string.
	for ( typeIndex = BUILD ; typeIndex < NON ; typeIndex++ )
	{
		if ( TheMessage[0] == CommandChars[ typeIndex ] )
		{
			break ;
		}
	}

	switch ( ( CommandType_T ) typeIndex )
	{
			// World build character found, see if any world build commands
			// match the text immediately after the first character in the
			// string.
		case BUILD :
		{
			for ( commandIndex = FIRST_BUILD ;
			      commandIndex < LAST_BUILD ;
			      commandIndex++ )
			{
				if ( TheMessage.substr ( 1 ).compare ( 0 , CommandStrings[commandIndex].length () ,
			                                               CommandStrings[commandIndex] ) == 0 )
				{
					break ;
				}
			}

			TheCommand = ( Command_T ) commandIndex ;

				// Grab all text after the command string and parse for arguments.
			std::string arguments =
			 TheMessage.substr ( TheMessage.find ( CommandStrings[commandIndex] ) +
			                                       CommandStrings[commandIndex].length () ) ;
			if ( arguments.empty () == false )
			{
				parseArguments ( arguments ) ;
			}
		}
		break ;

			// Admin command character found, see if any world build commands
			// match the text immediately after the first character in the
			// string.
		case ADMIN :
		{
			for ( commandIndex = FIRST_ADMIN ;
			      commandIndex < LAST_ADMIN ;
			      commandIndex++ )
			{
				if ( TheMessage.substr ( 1 ).compare ( 0 , CommandStrings[commandIndex].length () ,
				                                           CommandStrings[commandIndex] )  == 0 )
				{
					break ;
				}
			}

			TheCommand = ( Command_T ) commandIndex ;

				// Grab all text after the command string and parse for arguments.
			std::string arguments =
			 TheMessage.substr ( TheMessage.find ( CommandStrings[commandIndex] ) +
			                     CommandStrings[commandIndex].length () ) ;
			if ( arguments.empty () == false )
			{
				parseArguments ( arguments ) ;
			}
		}
		break ;

			// Not a command, no format to process.
		case NON :
			TheCommand = NON_COMMAND ;
		break ;
	}
}



CommandParser::Command_T CommandParser::GetCommand ( void )
{
	return TheCommand ;
}



unsigned CommandParser::GetArgumentCount ( void )
{
	return NumArgs[( unsigned )TheCommand] ;
}



std::string CommandParser::GetArgument ( unsigned Index )
{
		// Prevent out of bounds access.
	if ( Index < NumArgs[ ( unsigned )TheCommand ] )
	{
		return ArgumentList[ Index ] ;
	}
	else
	{
		return "" ;
	}
}



void CommandParser::parseArguments ( std::string Arguments )
{
	ArgumentsGood = false ;

		// Increment start character pointer past initial space(s).
	unsigned firstChar = 0 ;
	while ( Arguments.at ( firstChar ) == ' ' )
	{
		firstChar++ ;
	}

		// Find the first space after the current start pointer.
	unsigned lastChar = Arguments.substr ( firstChar ).find ( ' ' ) ;

	unsigned argumentCount = 0 ;

		// Loop until the whole string has been parsed or all arguments have
		// been found for this command.
	while ( ( lastChar != std::string::npos ) &&
		    ( argumentCount <= this->GetArgumentCount () ) )
	{
		argumentCount++ ;

			// Grab the argument found
		ArgumentList.push_back ( Arguments.substr ( firstChar , lastChar ) ) ;

			// Set the start pointer to first non space after
			// the current argument.
		firstChar = lastChar + 1 ;
		while ( Arguments.at ( firstChar ) == ' ' )
		{
			firstChar++ ;
		}
			// Point the end pointer to the first space after the start.
		lastChar = Arguments.substr ( firstChar ).find ( ' ' ) ;
	}

		// Grab the argument at the end of the string.
	if ( Arguments.substr ( firstChar ).empty () == false )
	{
		ArgumentList.push_back ( Arguments.substr ( firstChar ) ) ;
		argumentCount++ ;
	}

		// Indicate if the right number of characters are found.
	if ( argumentCount == this->GetArgumentCount () )
	{
		ArgumentsGood = true ;
	}
}



CommandParser::~CommandParser ()
{

}
