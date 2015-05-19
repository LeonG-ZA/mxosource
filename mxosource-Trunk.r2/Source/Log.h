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

#ifndef MXOSIM_LOG_H
#define MXOSIM_LOG_H

#include "Singleton.h"
#include "Threading/NativeMutex.h"

class Log : public Singleton< Log >
{
public:
	typedef enum
	{
		LOGLEVEL_CRITICAL = 1,
		LOGLEVEL_ERROR = 2,
		LOGLEVEL_WARNING = 3,
		LOGLEVEL_INFO = 4,
		LOGLEVEL_DEBUG = 5,
		LOGLEVEL_FIRST = LOGLEVEL_CRITICAL,
		LOGLEVEL_LAST = LOGLEVEL_DEBUG,
		LOGLEVEL_TIMING = 10, // Activated independent of other log levels.
	}LogLevel;

	Log();
	~Log();
	void OpenLogFile( const char *logFileName );

	//logging funcs
	void Critical( boost::format &fmt );
	void Critical( string fmt );
	void Error( boost::format &fmt );
	void Error( string fmt );
	void Warning( boost::format &fmt );
	void Warning( string fmt );
	void Info( boost::format &fmt );
	void Info( string fmt );
	void Debug( boost::format &fmt );
	void Debug( string fmt );
	void Timing( boost::format &fmt );
	void Timing( string fmt );
	
	bool SetConsoleLogLevel( string LevelString );
	bool SetFileLogLevel( string LevelString );
	bool SetConsoleDisplayLogLevel( string DisplayString );
	bool SetFileDisplayLogLevel( string DisplayString );
	bool SetConsoleLogTiming( string TimingString );
	bool SetFileLogTiming( string TimingString );

private:
	string ProcessString(LogLevel level,const string &str,bool displayLogLevel);
	void OutputConsole(LogLevel level,const string &str);
	void OutputFile(LogLevel level,const string &str);
	void Output(LogLevel level,const string &str);

	bool SetLogLevel( string LevelString , bool ForConsole );
	bool SetDisplayLogLevel( string LevelString , bool ForConsole );
	bool SetLogTiming( string TimingString , bool ForConsole );

	NativeMutex printMutex;
	NativeMutex fileMutex;
	ofstream LogFile;

	LogLevel ConsoleLogLevel;
	LogLevel FileLogLevel;

	bool ConsoleDisplayLogLevel;
	bool FileDisplayLogLevel;

	bool ConsoleLoggingTiming;
	bool FileLoggingTiming;
};

#define sLog Log::getSingleton()

#define CRITICAL_LOG sLog.Critical
#define ERROR_LOG sLog.Error
#define WARNING_LOG sLog.Warning
#define INFO_LOG sLog.Info
#define DEBUG_LOG sLog.Debug
#define TIMING_LOG sLog.Timing

#endif
