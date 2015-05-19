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
//	Character Model is used to keep character database in memory rather than
//	accessing the database every time a read or writ is needed.
//
// ***************************************************************************

#ifndef MXOSIM_NPCMODEL_H
#define MXOSIM_NPCMODEL_H

#include "CharacterModel.h"



class NPCModel : public CharacterModel
{

	public :

			// Load the character into memory from the DB.
		NPCModel ( LocationVector* TargetLocation , unsigned NewGOId ) ;
		
			// Either a true NPC or a PC based NPC.
		bool IsTrueNPC ( void ) ;
		
	protected :

		bool TrueNPC ;
} ;

#endif MXOSIM_NPCMODEL_H