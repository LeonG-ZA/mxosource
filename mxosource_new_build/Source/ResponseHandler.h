// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2011 - Michael Lingg
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
//	The Command Handler proccesses commands from the client and determines
//	the proper responses.
//
// ***************************************************************************

#ifndef MXOSIM_ResponseHandler_H
#define MXOSIM_ResponseHandler_H

#include "CharacterModel.h"
#include "MessageTypes.h"



class ResponseHandler
{

	public :

		ResponseHandler () ;

			//
			//	Broadcast the position of the specified character via an
			//	WorldChatMsg.  Do not send the message to the client that
			//	controls the updated character if ExcludeSource is true.
			//
		void BroadcastPosition ( CharacterModel* TheCharacter , bool ExcludeSource ) ;

			//
			//	Broadcast the chat message TheMessage and the handle of the
			//	character, TheCharacter, that sent the chat via a WorldChatMsg.
			//	Always excludes the client that controls TheCharacter.
			//
		void BroadcastWorldChat ( CharacterModel* TheCharacter , std::string TheMessage ) ;

	private :

			//
			//	Broadcasts the specified message, theMsg, as a state message
			//	to all clients, except for the client that controls
			//	TheCharacter if ExcludeSource is true.
			//
		void broadcastState ( CharacterModel* TheCharacter , msgBaseClassPtr theMsg , bool ExcludeSource ) ;

			//
			//	Broadcasts the specified message, theMsg, as a command message
			//	to all clients, except for the client that controls
			//	TheCharacter if ExcludeSource is true.
			//
		void broadcastCommand ( CharacterModel* TheCharacter , msgBaseClassPtr theMsg , bool ExcludeSource ) ;

} ;

#endif MXOSIM_ResponseHandler_H