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
//	The merged RPC class takes the producer and consumer sides of the RPC
//	classes and merges them into a single class that can have a single
//	instance.  The merged instance is then cast as the producer or consumer
//	parent class in order to provide the view and control processes with only
//	the functionality they require.
//
// ***************************************************************************

#ifndef MXOSIM_MERGEDREQUESTRPC_H
#define MXOSIM_MERGEDREQUESTRPC_H

#include "HandleRequestRPC.h"
#include "ReceiveRequestRPC.h"

class MergedRequestRPC : public HandleRequestRPC , public ReceiveRequestRPC
{

	public :

		MergedRequestRPC ( void ) : HandleRequestRPC ( &FirstRequest ) , ReceiveRequestRPC ( &FirstRequest ) {}
		
	private :

			// The pointer to the start of the RPC list.
		RequestList_E* FirstRequest ;
} ;

#endif MXOSIM_MERGEDREQUESTRPC_H