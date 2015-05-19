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
#include "Log.h"
#include "ConsoleThread.h"
#include "Util.h"
#include "Master.h"
#include "Crypto.h"
#include "GameServer.h"
#include "AuthServer.h"

#include <boost/algorithm/string.hpp>
using boost::iequals;

bool ConsoleThread::run()
{
	SetThreadName("Console Thread");

	for (;;) 
	{
		string command;
		cin >> command;

		if (iequals(command, "exit"))
		{
			Master::m_stopEvent = true;
			INFO_LOG("Got exit command. Shutting down...");
			break;
		}
		else if (iequals(command,"createAccount"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string username,password;
			lineParser >> username;
			lineParser >> password;

			if (username.length() < 1 || password.length() < 1)
			{
				WARNING_LOG("Invalid username or password");
			}
			else
			{
				bool accountCreated = sAuth.CreateAccount(username,password);
				if (accountCreated)
					INFO_LOG(format("Created account with username %1% password %2%") % username % password );
			}
		}
		else if (iequals(command,"changePassword"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string username,newPassword;
			lineParser >> username;
			lineParser >> newPassword;

			bool passwordChanged = sAuth.ChangePassword(username,newPassword);
			if (passwordChanged)
				INFO_LOG(format("Account %1% now has password %2%") % username % newPassword );
		}
		else if (iequals(command,"createWorld"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string worldName;
			lineParser >> worldName;

			bool worldCreated = sAuth.CreateWorld(worldName);
			if (worldCreated)
				INFO_LOG(format("Created world named %1%") % worldName );
		}
			else if (iequals(command,"help") || iequals(command,"-help") || iequals(command,"-h"))
		{
			INFO_LOG("1## SetItemCash <itemId> <itemPrice> -> set the price of any item stored in 'myitems' table.");
			INFO_LOG("2## CreateAccount <nameAccount> <passwordAccount> -> create a new account");
			INFO_LOG("3## ChangePassword <nameAccount> <NEWpasswordAccount> -> update PSW account");
			INFO_LOG("4## CreateCharacter <worldName> <nameAccount> <nameCharacter> <firstnameCharacter> <surnameCharacter> <rank(=player,lesig,mod,admin)> -> create a new character");

			INFO_LOG("5## SetProcedureADMIN <value> -> set a different safe VALUE for ADMIN commands.");

			INFO_LOG("6## UpdateMissionSystem -> when you add a new Mission, type this command to update the counter");
			INFO_LOG("7## UpdateNPCSystem -> when you add a new NPC, type this command to update the counter");
			INFO_LOG("8## UpdateClothSystem -> when you add a new CLOTH, type this command to update the counter");

			INFO_LOG("9## SetLimitAreaChat -> out of this value, players cannot chat");
			INFO_LOG("10## RebootNow -> kick players");
		}
		else if (iequals(command,"rebootnow"))
		{
			sAuth.REBOOTMESSAGE();
			INFO_LOG("REBOOT EXECUTED");
		}
		else if (iequals(command,"setprocedureadmin"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string value;
			lineParser >> value;

				sAuth.SetProcedureAdmin(value);
				INFO_LOG(format("SET_PROCEDURE_ADMIN VALUE setted -> %1%") % value );
		}
		else if (iequals(command,"setitemcash"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string itemId, price;
			lineParser >> itemId;
			lineParser >> price;

			sAuth.SetItemCash(itemId,price);
		}
		else if (iequals(command,"updatemissionsystem"))
		{
			sAuth.UpdateMissionSystem();
		}
		else if (iequals(command,"updatenpcsystem"))
		{
			sAuth.UpdateNPCSystem();
		}
		else if (iequals(command,"updateclothsystem"))
		{
			sAuth.UpdateClothSystem();
		}
		else if (iequals(command,"setlimitareachat"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string limit;
			lineParser >> limit;

			sAuth.SetLimitAreaChat(limit);
		}
		else if (iequals(command,"createCharacter"))
		{
			string theLine;
			getline(cin,theLine);
			stringstream lineParser;
			lineParser.str(theLine);
			string worldName,userName,charHandle,firstName,lastName,rank;
			lineParser >> worldName;
			lineParser >> userName;
			lineParser >> charHandle;
			lineParser >> firstName;
			lineParser >> lastName;
			lineParser >> rank;

			bool characterCreated = sAuth.CreateCharacter(worldName,userName,charHandle,firstName,lastName,rank);
			if (characterCreated)
				INFO_LOG(format("Inserted character %1% into world %2% for user %3% with name %4% %5% and rank %6%")
				% charHandle % worldName % userName % firstName % lastName % rank);
		}
		else if (iequals(command, "broadcastMsg") || iequals(command, "modalMsg"))
		{
			string theAnnouncement;
			getline(cin,theAnnouncement);
			while (theAnnouncement.c_str()[0] == ' ' || theAnnouncement.c_str()[0] == '\n')
			{
				theAnnouncement = theAnnouncement.substr(1);
			}

			if (theAnnouncement.size() > 0)
			{
				if (iequals(command, "broadcastMsg"))
					sGame.AnnounceCommand(NULL,make_shared<BroadcastMsg>(theAnnouncement));
				else if (iequals(command, "modalMsg"))
					sGame.AnnounceCommand(NULL,make_shared<ModalMsg>(theAnnouncement));

				cout << "OK" << std::endl;
			}
			else
			{
				cout << "EMPTY MESSAGE" << std::endl;
			}
		}
		else if (iequals(command, "send") || iequals(command, "sendCmd") )
		{
			stringstream hexStream;
			for (;;)
			{
				string word;
				cin >> word;

				string::size_type semicolonPos = word.find_first_of(";");
				if (semicolonPos != string::npos)
				{
					word = word.substr(0,semicolonPos);
					if (word.length() > 0)
					{
						hexStream << word;
					}
					break;
				}
				else
				{
					hexStream << word;
				}
			}

			string binaryOutput;
			try
			{
				CryptoPP::HexDecoder hexDecoder(new CryptoPP::StringSink(binaryOutput));
				hexDecoder.Put((const byte*)hexStream.str().data(),hexStream.str().size(),true);
				hexDecoder.MessageEnd();
			}
			catch (...)
			{
				cout << "Invalid hex string" << std::endl;		
				continue;
			}

			bool rpcCmd=false;
			if (iequals(command, "sendCmd"))
				rpcCmd=true;

			sGame.Broadcast(ByteBuffer(binaryOutput),rpcCmd);
			cout << "OK" << std::endl;
		}
		else if ((iequals(command,"updateConsoleLogging")) ||
		          (iequals(command,"updateFileLogging")))
		{
			string theLine ;
			getline ( cin , theLine ) ;
			stringstream lineParser ;
			lineParser.str ( theLine ) ;
			string updateType,parameter ;
			lineParser >> updateType ;
			lineParser >> parameter ;

			if ( updateType.length() < 1 )
			{
				WARNING_LOG ( "Missing logging update type" ) ;
			}
			else if ( parameter.length() < 1 )
			{
				WARNING_LOG ( "Missing logging parameter" ) ;
			}
			else
			{
				bool updateMade ;
				
				if ( iequals ( command,"updateConsoleLogging" ) )
				{
					if ( iequals ( updateType,"level" ) )
					{
						updateMade = sLog.SetConsoleLogLevel ( parameter ) ;
						if ( updateMade )
						{
							INFO_LOG ( format ( "Updated console log level to %1%" ) % parameter ) ;
						}
						else
						{
							INFO_LOG ( "Update to console log level failed" ) ;
						}
					}
					else if ( iequals ( updateType,"timing" ) )
					{
						updateMade = sLog.SetConsoleLogTiming ( parameter ) ;
						if ( updateMade )
						{
							INFO_LOG ( format ( "Updated console timing to %1%" ) % parameter ) ;
						}
							else
						{
							INFO_LOG ( "Update to console timing failed" ) ;
						}
					}
					else if ( iequals ( updateType,"displayloglevel" ) )
					{
						updateMade = sLog.SetConsoleDisplayLogLevel ( parameter ) ;
						if ( updateMade )
						{
							INFO_LOG ( format ( "Displaying Log Level %1%" ) % parameter ) ;
						}
						else
						{
							INFO_LOG ( "Update to display log level failed" ) ;
						}
					}
					else
					{
						INFO_LOG ( "Logging update type not recognized" ) ;
					}
				}
				else
				{
					if ( iequals ( updateType,"level" ) )
					{
						updateMade = sLog.SetFileLogLevel ( parameter ) ;
						if ( updateMade )
						{
							INFO_LOG ( format ( "Updated file log level to %1%" ) % parameter ) ;
						}
						else
						{
							INFO_LOG ( "Update to file log level failed") ;
						}
					}
					else if ( iequals ( updateType,"timing" ) )
					{
						updateMade = sLog.SetFileLogTiming ( parameter ) ;
						if ( updateMade )
						{
							INFO_LOG ( format ( "Updated file timing to %1%" ) % parameter ) ;
						}
						else
						{
							INFO_LOG ( "Update to file timing failed" ) ;
						}
					}
					else if ( iequals ( updateType,"displayloglevel" ) )
					{
						updateMade = sLog.SetFileDisplayLogLevel ( parameter ) ;
						if ( updateMade )
						{
							INFO_LOG ( format ( "Displaying Log Level %1%" ) % parameter ) ;
						}
						else
						{
							INFO_LOG ( "Update to display log level failed" ) ;
						}
					}
					else
					{
						INFO_LOG (  "Logging update type not recognized" ) ;
					}
				}

			}
		}
		else
		{
			INFO_LOG ( format ( "Command not recognized %1%" ) % command ) ;
		}
	}

	return true;
}
