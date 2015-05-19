// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2006-2010 Rajko Stojadinovic
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
// ***************************************************************************

#include "Common.h"
#include "Singleton.h"
#include "Log.h"
#include "Config.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
using boost::iequals;

#include <time.h>

createFileSingleton( Log );

static const string LogLevel_Strings[Log::LOGLEVEL_LAST] = { "CRITICAL" , "ERROR" , "WARNING" , "INFO" , "DEBUG" } ;

Log::Log()
{
	OpenLogFile("Reality.log");

	ConsoleLogLevel = LOGLEVEL_INFO;
	FileLogLevel = LOGLEVEL_WARNING;

	ConsoleDisplayLogLevel = false;
	FileDisplayLogLevel = true;

	ConsoleLoggingTiming = false;
	FileLoggingTiming = false;
}

Log::~Log()
{
	if (LogFile.is_open() == true)
	{
		LogFile.close();
	}
}


void Log::OpenLogFile( const char *logFileName )
{
	fileMutex.Acquire();

	if (LogFile.is_open() == true)
		LogFile.close();

	LogFile.open(logFileName,ios::out | ios::app);

	fileMutex.Release();
}

string Log::ProcessString( LogLevel level,const string &str,bool displayLogLevel)
{
	ostringstream returnings;

	//Timestamp
	time_t curr=time(0);

	unsigned miliseconds = clock () * ( CLOCKS_PER_SEC / 1000 ) % 1000 ;
	tm local;
	local=*(localtime(&curr));


	ostringstream Day,Month,Year;
	ostringstream Hour,Minute,Second,Milisecond;

	Milisecond.width ( 3 ) ;
	Milisecond.fill ( '0' ) ;
	Milisecond << miliseconds ;

	if(local.tm_mday < 10)
		Day << "0";
	Day << local.tm_mday;

	if ((local.tm_mon + 1) < 10)
		Month << "0";
	Month << (local.tm_mon + 1);

	Year << (local.tm_year + 1900);

	if (local.tm_hour < 10)
		Hour << "0";
	Hour << local.tm_hour;

	if (local.tm_min < 10)
		Minute << "0";
	Minute << local.tm_min;

	if (local.tm_sec < 10)
		Second << "0";
	Second << local.tm_sec;

	string LogLevelStr;

	if ( displayLogLevel == true )
	{
		LogLevelStr = LogLevel_Strings[level] ;
	}
	else
	{
		LogLevelStr = "" ;
	}

	returnings << "[" << Day.str() << "." << Month.str() << "." << Year.str() << " " << Hour.str() << ":" << Minute.str() << ":" << Second.str() << "." << Milisecond.str() << "] " << LogLevelStr << ": " << str;

	return returnings.str();
}

void Log::OutputConsole( LogLevel level,const string &str )
{
	if ((ConsoleLogLevel >= level) ||
	    ((level == LOGLEVEL_TIMING ) &&
	     (ConsoleLoggingTiming == true)))
	{
		string outputMe = ProcessString(level,str,ConsoleDisplayLogLevel);
		printMutex.Acquire();
		cout << outputMe << std::endl;
		printMutex.Release();
	}
}

void Log::OutputFile( LogLevel level,const string &str )
{
	if (LogFile.is_open() == true)
	{
		if ((FileLogLevel >= level) ||
	        ((level == LOGLEVEL_TIMING ) &&
	         (FileLoggingTiming == true)))
		{
			string outputMe = ProcessString(level,str,FileDisplayLogLevel);
			fileMutex.Acquire();
			LogFile << outputMe << std::endl;
			fileMutex.Release();
		}
	}
}

void Log::Output( LogLevel level,const string &str )
{
	OutputConsole(level,str);
	OutputFile(level,str);
}

void Log::Critical( boost::format &fmt )
{
	Critical(fmt.str());
}

void Log::Critical( string fmt )
{
	Output(LOGLEVEL_CRITICAL,fmt);
}

void Log::Error( boost::format &fmt )
{
	Error(fmt.str());
}

void Log::Error( string fmt )
{
	Output(LOGLEVEL_ERROR,fmt);
}

void Log::Warning( boost::format &fmt )
{
	Warning(fmt.str());
}

void Log::Warning( string fmt )
{
	Output(LOGLEVEL_WARNING,fmt);
}

void Log::Info( boost::format &fmt )
{
	Info(fmt.str());
}

void Log::Info( string fmt )
{
	Output(LOGLEVEL_INFO,fmt);
}

void Log::Debug( boost::format &fmt )
{
	Debug(fmt.str());
}

void Log::Debug( string fmt )
{
	Output(LOGLEVEL_DEBUG,fmt);
}

void Log::Timing( boost::format &fmt )
{
	Debug(fmt.str());
}

void Log::Timing( string fmt )
{
	Output(LOGLEVEL_TIMING,fmt);
}

/* 
 * Set the minimum importance level of messages displayed on the console.
 * Inputs are string equivalant of logging levels or their integer value.
 */
bool Log::SetConsoleLogLevel( string LevelString )
{
	return SetLogLevel ( LevelString , true ) ;
}

/* 
 * Set the minimum importance level of messages displayed to file.
 * Inputs are string equivalant of logging levels or their integer value.
 */
bool Log::SetFileLogLevel( string LevelString )
{
	return SetLogLevel ( LevelString , false ) ;
}

bool Log::SetLogLevel( string LevelString , bool ForConsole )
{
	bool setLevelSuccess = false ;
	int newLevel ;

	try
	{
	    newLevel = boost::lexical_cast<int>( LevelString );

		if ( newLevel >= LOGLEVEL_FIRST && newLevel <= LOGLEVEL_LAST )
		{
			setLevelSuccess = true ;
		}
	}
	catch( boost::bad_lexical_cast const& )
	{
		for ( newLevel = LOGLEVEL_FIRST ; newLevel <= LOGLEVEL_LAST ; newLevel++ )
		{
			if ( iequals ( LevelString , LogLevel_Strings[newLevel - 1] ) ) // String is 0 based array.
			{
				setLevelSuccess = true ;
				break ;
			}
		}
	}

	if ( setLevelSuccess == true )
	{
		if ( ForConsole == true )
		{
			ConsoleLogLevel = ( LogLevel )newLevel ;
		}
		else
		{
			FileLogLevel = ( LogLevel )newLevel ;
		}
	}

	return setLevelSuccess ;
}

/*
 * Activate/Deactivate display of message level on console messages.
 * Input possibilities are true(on/1) or false(off/0).
 */
bool Log::SetConsoleDisplayLogLevel( string DisplayString )
{
	return SetDisplayLogLevel ( DisplayString , true ) ;
}

/*
 * Activate/Deactivate display of message level on file messages.
 * Input possibilities are true(on/1) or false(off/0).
 */
bool Log::SetFileDisplayLogLevel( string DisplayString )
{
	return SetDisplayLogLevel ( DisplayString , false ) ;
}

bool Log::SetDisplayLogLevel( string DisplayString , bool ForConsole )
{
	bool setDisplaySuccess = false ;
	bool nowActivated ;

	try
	{
	    nowActivated = boost::lexical_cast<bool>( DisplayString );
		setDisplaySuccess = true ;
	}
	catch( boost::bad_lexical_cast const& )
	{
		if ( ( iequals ( DisplayString , "on" ) ) ||
		      ( iequals ( DisplayString , "true" ) ) ||
			   ( iequals ( DisplayString , "1" ) ) )
		{
			nowActivated = true ;
			setDisplaySuccess = true ;
		}
		else if ( ( iequals ( DisplayString , "off" ) ) ||
		           ( iequals ( DisplayString , "false" ) ) ||
			        ( iequals ( DisplayString , "0" ) ) )
		{
			nowActivated = false ;
			setDisplaySuccess = true ;
		}
	}

	if ( setDisplaySuccess == true )
	{
		if ( ForConsole == true )
		{
			ConsoleDisplayLogLevel = nowActivated ;
		}
		else
		{
			FileDisplayLogLevel = nowActivated ;
		}
	}

	return setDisplaySuccess ;
}

/*
 * Activate/Deactivate timing logging to the console.
 * Input possibilities are true(on/1) or false(off/0).
 */
bool Log::SetConsoleLogTiming( string TimingString )
{
	return SetLogTiming ( TimingString , true ) ;
}

/*
 * Activate/Deactivate timing logging to the console.
 * Input possibilities are true(on/1) or false(off/0).
 */
bool Log::SetFileLogTiming( string TimingString )
{
	return SetLogTiming ( TimingString , false ) ;
}

bool Log::SetLogTiming( string TimingString , bool ForConsole )
{
	bool setTimingSuccess = false ;
	bool nowActivated ;

	try
	{
	    nowActivated = boost::lexical_cast<bool>( TimingString );
		setTimingSuccess = true ;
	}
	catch( boost::bad_lexical_cast const& )
	{
		if ( ( iequals ( TimingString , "on" ) ) ||
		      ( iequals ( TimingString , "true" ) )  ||
			   ( iequals ( TimingString , "1" ) ) )
		{
			nowActivated = true ;
			setTimingSuccess = true ;
		}
		else if ( ( iequals ( TimingString , "off" ) ) ||
		           ( iequals ( TimingString , "false" ) )  ||
			        ( iequals ( TimingString , "0" ) ) )
		{
			nowActivated = false ;
			setTimingSuccess = true ;
		}
	}

	if ( setTimingSuccess == true )
	{
		if ( ForConsole == true )
		{
			ConsoleLoggingTiming = nowActivated ;
		}
		else
		{
			FileLoggingTiming = nowActivated ;
		}
	}

	return setTimingSuccess ;
}