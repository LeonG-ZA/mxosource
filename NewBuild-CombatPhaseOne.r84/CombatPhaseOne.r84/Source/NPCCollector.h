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
// The NPC Collector contains a list of all NPCs in the world.  This will
// eventually be replaced by multiple heirarcy collectors.  This class wont be
// commented up right now as it is just a simple list interface.
//
// ***************************************************************************

#ifndef MXOSIM_NPCCOLLECTOR_H
#define MXOSIM_NPCCOLLECTOR_H

#include "Common.h"
#include "NPCController.h"



class NPCCollector
{

	public :

		NPCCollector ( void ) ;

		bool AddNPC ( unsigned GOId , NPCController* NewNPC ) ;

		void SetFirstNPC ( void ) ;
		void SetNextNPC ( void ) ;
		NPCController* GetCurrentNPC ( void ) ;

		NPCController* GetNPC ( unsigned GOId ) ;

		unsigned NPCCount ( void ) ;

	private :

		map < unsigned , NPCController* > MyNPCs ;
		map < unsigned , NPCController* >::iterator CurrentNPCItterator ;

} ;

#endif MXOSIM_NPCCOLLECTOR_H