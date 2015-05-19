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
//	The Response RPC class handles the passing responses between the control
//	code and the view code.
//
//	The base class defines the structures used for RPC that is needed by both
//	control and view code.  The functions for control to request an RPC and
//	view to process an RPC will be implemented in children of this base class.
//
//	RPC requests are being stored in a list, rather than allowing control to 
//	directly call view functions, in order to allow the view class to determine
//	when control function requests are executed.
//
//	A linked list was chosen as it is easily resized as compared to an array
//	and works very efficiently for a queue (a pointer to the end of the list
//	could make it more efficient).  A different structure may be needed in the
//	future if priorities are introduced.
//
// ***************************************************************************

#ifndef MXOSIM_RESPONSERPC_H
#define MXOSIM_RESPONSERPC_H

#include "ResponseRPCFunctions.h"



class ResponseRPC
{

	protected :

			// The pointer to the start of the RPC list.
		queue < ResponseRPCFunction* >* ResponseList ;

	public :

		ResponseRPC ( queue < ResponseRPCFunction* >* NewResponseList )
		{
			ResponseList = NewResponseList ;
		}

} ;

#endif MXOSIM_RESPONSERPC_H