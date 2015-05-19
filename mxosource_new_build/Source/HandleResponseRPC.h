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
//	The RPC response handler allows the top RPC function set by the control
//	process in the queue to be executed in the ResponseHandler in the view
//	process.
//
// ***************************************************************************

#ifndef MXOSIM_HANDLERESPONSERPC_H
#define MXOSIM_HANDLERESPONSERPC_H

#include "ResponseHandler.h"
#include "ResponseRPC.h"

class HandleResponseRPC : public ResponseRPC
{

	public :

			//
			//	Used to indicate if the response queue has been emptied.
			//
		typedef enum ResponseStatus_T
		{
			SUCCESS = 0 ,
			NO_RESPONSE = 1 ,
		} ;

		HandleResponseRPC ( ResponseList_E** TheFirstResponse ) ;
		
			//
			//	The RPC classes are created before processes are created in
			//	order to be passed to the processes when they are constructed.
			//	ResponseHandler is created by the view process when it is
			//	created so cannot be passed to the RPC class when it is
			//	constructed.
			//
			//	Additionally ResponseHandler is only used by the view process,
			//	so should not be created when the RPC classes are created as the
			//	RPC classes are shared between multiple processes.
			//
		void SetResponseHandler ( ResponseHandler* NewResponseHandler ) ;

			//
			//	Calls the first ResponseHandler function that correspond with the
			//	functions currently stored in the queue in order to produce the
			//	corrisponding message.
			//
		ResponseStatus_T ProcessResponse ( void ) ;

	private :
	
			// Returns a pointer to the first request in the list and moves
			// the list pointer forward by one record.
		ResponseList_E* getFirstResponse ( void ) ;

			//	The handler to call to execute RPC functions to produce a
			//	message to the client
		ResponseHandler* MyResponseHandler ;

} ;

#endif MXOSIM_HANDLERESPONSERPC_H