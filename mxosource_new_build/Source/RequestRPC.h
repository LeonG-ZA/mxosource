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
//	The Request RPC class handles passing requests from the client (converted
//	from the raw client message) from the view process to the control process.
//
//	The base class defines the structures used for RPC that is needed by both
//	view and control code.  The functions for view to request an RPC and
//	control to process an RPC will be implemented in children of this base class.
//
//	RPC requests are being stored in a list, rather than allowing view to 
//	directly call control functions, in order to allow the control class to determine
//	when view function requests are executed.
//
//	A linked list was chosen as it is easily resized as compared to an array
//	and works very efficiently for a queue (a pointer to the end of the list
//	could make it more efficient).  A different structure may be needed in the
//	future if priorities are introduced.
//
// ***************************************************************************

#ifndef MXOSIM_REQUESTRPC_H
#define MXOSIM_REQUESTRPC_H

#include "RequestRPCFunctions.h"



class RequestRPC
{

	protected :

			//
			//	The structure for an element of the list of RPCs.
			//
		typedef struct RequestList_E
		{
			RequestRPCFunction* TheFunction ;
			RequestList_E* NextRequest ;
		} ;
		
			// The pointer to the start of the RPC list.
		RequestList_E** FirstRequest ;

	public :

		RequestRPC ( RequestList_E** TheFirstRequest )
		{
			FirstRequest = TheFirstRequest ;
			*FirstRequest = NULL ;
		}

} ;

#endif MXOSIM_REQUESTRPC_H