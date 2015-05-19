// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2012 - Michael Lingg
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
// The AI Handler contains the periodic process (ProcessAI) that will be
// called by the control process to perform AI updates.
//
// Process AI calls the appropriate functions to perform AI updates.
// At the moment only the individual NPC Controllers exist so these are all
// called in order.
// Future updates will include an AI heirarcy with world AI processing
// (probably in this class), which calls faction AI processing, which calls
// regional AI processing (zones and similar), which calls small tactical
// group processing, which finally will call individual NPC processing.
//
// Additional commenting will not be included for now since this will drastically change.
// ***************************************************************************

#ifndef MXOSIM_AIHANDLER_H
#define MXOSIM_AIHANDLER_H

#include "NPCCollector.h"
#include "ProduceResponseRPC.h"
#include "WorldModel.h"



class AIHandler
{

	public :

		AIHandler ( NPCCollector* TheNPCCollector , ProduceResponseRPC* TheResponseRPC ) ;

		void ProcessAI ( void ) ;

	private :

		const ProduceResponseRPC* MyResponseRPC ;
		const WorldModel* MyWorldModel ;
		
		NPCCollector* MyNPCCollector ;
} ;

#endif MXOSIM_AIHANDLER_H