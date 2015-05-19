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
//	The RPC request receiver allows a function call from the view process
//	(the prodcer) to be stored in the RPC queue.  The RPC request handler
//	then executes the function in the control process (the consumer).
//
//	Because the view process does have access to the functions of the control
//	process, an enum is used to tell the control process which function is
//	being requested.  Each function has a different set of arguments but the
//	structure containing these arguments is converted to a void* so all the
//	different functions can be stored in the same struct.  The arguments are
//	then converted back to their proper format before the RPC call is executed.
//
// ***************************************************************************

#ifndef MXOSIM_RECEIVEREQUESTRPC_H
#define MXOSIM_RECEIVEREQUESTRPC_H

#include "CharacterModel.h"
#include "RequestRPC.h"

class ReceiveRequestRPC : public RequestRPC
{

	public :

		ReceiveRequestRPC ( RequestList_E** TheFirstRequest ) ;

			//
			//	Cause the specified player to perform a jump to the target coordinates.
			//
			//	Actual function is performed by RequestHandler::Jump.
			//
		void Jump ( CharacterModel* JumpCharacter , LocationVector JumpTarget ) ;

			//
			//	Handle a chat request from the client
			//
			//	Actual function is performed by RequestHandler::Chat.
			//
		void Chat ( CharacterModel* SourceCharacter , std::string ChatMessage ) ;

	private :

			//
			//	Stores a new request in the queue.
			//
		void addRequest ( RequestRPCFunction* NewFunction ) ;

} ;

#endif MXOSIM_RECEIVEREQUESTRPC_H