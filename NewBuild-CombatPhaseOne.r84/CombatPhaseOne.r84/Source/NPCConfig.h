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
//	....
//
// ***************************************************************************

#ifndef MXOSIM_NPCCONFIG_H
#define MXOSIM_NPCCONFIG_H

#include "Common.h"
#include "LocationVector.h"
#include "xml\pugixml.hpp"

class NPCConfig
{

	public :

		NPCConfig ( void ) ;
		
		void CreateSpawnPoint ( LocationVector* SpawnLocation ) ;
		
// To go away when spawn locations become children of nodes.
		LocationVector GetFirstSpawnLocation () ;
		LocationVector GetNextSpawnLocation () ;

	protected :

#define XML_PATH "..\\npcs\\"
	
		typedef struct SpawnPoint_T
		{
			LocationVector* SpawnLocation ;
		} ;
	
		void loadSpawnPoints () ;
	
		pugi::xml_document SpawnPoints ;

// To go away when spawn points become children of nodes.
		pugi::xml_node CurrentSpawnPoint ;

} ;

#endif MXOSIM_NPCCONFIG_H