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
//	The RPC response producer allows a function call from the control process
//	(the prodcer) to be stored in the RPC queue.  The RPC response handler
//	then executes the functions in the view process (the consumer).
//
//	Because the control process does have access to the functions of the view
//	process, an enum is used to tell the view process which function is being
//	requested.  Each function has a different set of arguments but the
//	structure containing these arguments is converted to a void* so all the
//	different functions can be stored in the same struct.  The arguments are
//	then converted back to their proper format before the RPC call is executed.
//
// ***************************************************************************

#ifndef MXOSIM_PRODUCERESPONSERPC_H
#define MXOSIM_PRODUCERESPONSERPC_H

#include "CharacterModel.h"
#include "ResponseRPC.h"



class ProduceResponseRPC : public ResponseRPC
{

	public :

		ProduceResponseRPC ( ResponseList_E** TheFirstRequest ) ;

			//
			//	Broadcast a position state update message to the clients.
			//
			//	Actual function is performed by ResponseHandler::BroadcastPosition.
			//
		void BroadcastPosition ( CharacterModel* ResponseSource , bool ExcludeSource ) ;

			//
			//	Broadcasts a chat message to all clients other than the client
			//	that send the message.
			//
			//	Actual function is performed by ResponseHandler::BroadcastWorldChat.
			//
		void BroadcastWorldChat ( CharacterModel* MessageSource , std::string TheMessage ) ;

	private :

			//
			//	Stores a new response in the queue.
			//
		void addResponse ( ResponseRPCFunction* TheFunction ) ;

} ;

#endif MXOSIM_PRODUCERESPONSERPC_H