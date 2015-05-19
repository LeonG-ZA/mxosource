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

#ifndef MXOSIM_PLAYERCHARACTERMODEL_H
#define MXOSIM_PLAYERCHARACTERMODEL_H

#include "CharacterModel.h"



class PlayerCharacterModel : public CharacterModel
{

	public :

			// Load the character into memory from the DB.
		PlayerCharacterModel ( unsigned NewUserID , unsigned NewCharacterID , unsigned NewGOId ) ;

		unsigned GetCharacterId () ;

		string GetFirstName () ;
		string GetLastName () ;

		unsigned GetProfession () ;

		unsigned GetExperience () ;

			//
			// Update the DB with any changes to the character.
			//
		void UpdateDB () ;

			// Actions a character can perform.
			
	protected :

			//
			//	Loads the RSI from the DB into memory.
			//
		void loadRSI () ;

		void positionUpdated ( void ) ;
		void attributesUpdated ( void ) ;
		
		const unsigned UserID ;
		const unsigned CharacterID ;

		string FirstName ;
		string LastName ;

		unsigned Profession ; // Update to enum.

		unsigned Experience ;

			// Flag used to indicate if there has been an update that needs to be stored in the DB.
		bool DBNeedsUpdate ;
} ;

#endif MXOSIM_PLAYERCHARACTERMODEL_H