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
// You should have Requestd a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ---------------------------------------------------------------------------
//
//	The RPC request handler allows the top RPC function set by the View process
//	in the queue to be executed in the RequestHandler in the control process.
//
// ***************************************************************************

#ifndef MXOSIM_HANDLEREQUESTRPC_H
#define MXOSIM_HANDLEREQUESTRPC_H

#include "RequestRPC.h"

#include "RequestHandler.h"



class HandleRequestRPC : RequestRPC 
{

	public :

			//
			//	Used to indicate if the request queue has been emptied.
			//
		typedef enum RequestStatus_T
		{
			SUCCESS = 0 ,
			NO_REQUEST = 1 ,
		} ;

		HandleRequestRPC ( queue < RequestRPCFunction* >* NewRequestList ) ;
		
			//
			//	The RPC classes are created before processes are created in
			//	order to be passed to the processes when they are constructed.
			//	RequestHandler is created by the Control process when it is
			//	created so cannot be passed to the RPC class when it is
			//	constructed.
			//
			//	Additionally RequestHandler is only used by the Control process
			//	So should not be created when the RPC classes are created as the
			//	RPC classes are shared between multiple processes.
			//
		void SetRequestHandler ( RequestHandler* NewRequestHandler ) ;

			//
			//	Calls the first RequestHandler function that correspond with the
			//	functions currently stored in the queue in order to process the
			//	request with game logic.
			//
		RequestStatus_T ProcessRequest ( void ) ;

	private :

			//	The handler to call to execute RPC functions to process game logic
		RequestHandler* MyRequestHandler ;

} ;

#endif MXOSIM_HANDLEREQUESTRPC_H