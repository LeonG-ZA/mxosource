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

#ifndef MXOSIM_CHARACTERMODEL_H
#define MXOSIM_CHARACTERMODEL_H

#include "Common.h"
#include "LocationVector.h"
#include "RsiData.h"

class CharacterModel
{

	public :

			// Load the character into memory from the DB.
		CharacterModel ( unsigned NewUserID , unsigned NewCharacterID ) ;

		unsigned GetCharacterID () ;

		string GetHandle () ;
		string GetFirstName () ;
		string GetLastName () ;
		string GetBackground () ;

		LocationVector* GetPosition () ;
		void SetPosition ( LocationVector* NewPosition ) ;
		void SetPositionX ( double NewPosX ) ;
		void SetPositionY ( double NewPosY ) ;
		void SetPositionZ ( double NewPosZ ) ;
		void SetHeading ( double NewHeading ) ;
		void SetHeading ( uint8 NewHeading ) ;

		RsiData* GetRSI () ;

		unsigned GetCurrentHealth () ;
		unsigned GetMaxHealth () ;

		unsigned GetCurrentIS () ;
		unsigned GetMaxIS () ;

		unsigned GetProfession () ;
		unsigned GetLevel () ;
		unsigned GetAlignment () ;

		unsigned GetDistrict () ;

		unsigned GetExperience () ;
		unsigned GetCash () ;

			//
			// This was originally the ViewID but I believe Game Object ID is more correct.
			//
		void SetGOID ( unsigned NewGOID ) ;
		unsigned GetGOID () ;

			//
			// Update the DB with any changes to the character.
			//
		void UpdateDB () ;

			//
			// Returns true if the updates need to be propogated and resets
			// the flag to false.
			//
		bool PropagateUpdates ( void ) ;

	private :

			//
			//	Loads the RSI from the DB into memory.
			//
		void loadRSI () ;

		const unsigned UserID ;
		const unsigned CharacterID ;

		string Handle ;
		string FirstName ;
		string LastName ;
		string Background ;

		RsiData* RSI ;

		LocationVector* Position ;
		double Heading ;

		unsigned CurrentHealth ;
		unsigned MaxHealth ;

		unsigned CurrentIS ;
		unsigned MaxIS ;

		unsigned Profession ; // Update to enum.
		unsigned Level ;
		unsigned Alignment ; // Enum?

		unsigned District ;

		unsigned Experience ;
		unsigned Cash ;

		unsigned GOID ;

			// Flag used to indicate if there has been an update that needs to be stored in the DB.
		bool DBNeedsUpdate ;

			// Flag provided publicly to indicate to the view process that updates need to be propogated to other clients.
		bool TransmitUpdates ;
} ;

#endif MXOSIM_CHARACTERMODEL_H