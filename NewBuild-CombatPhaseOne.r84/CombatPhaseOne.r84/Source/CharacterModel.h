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

#include "CombatTactic.h"
#include "Common.h"
#include "LocationVector.h"
#include "RsiData.h"
#include "Timer.h"

class CharacterModel
{

	public :
	
		/*typedef enum Tactics_T
		{
			SPEED ,
			POWER ,
			BLOCK ,
			GRAB ,
		} ;*/

		typedef enum MovementState_T
		{
			IDLE ,
			WALKING ,
			WALKING_BACKWARD ,
			RUNING ,
		} ;

		typedef enum Mood_T
		{
			MOOD1 ,
			MOOD2 ,
			DEAD ,
		} ;
		
		typedef enum DebuffState_T
		{
			NONE ,
			DAZED ,
		} ;

			// Load the character into memory from the DB.
		CharacterModel ( unsigned NewGOId ) ;

		CharacterModel ( LocationVector* TargetLocation , unsigned NewGOId ) ;
		
		string GetHandle () ;
		string GetBackground () ;

		LocationVector* GetPosition () ;

		RsiData* GetRSI () ;

		unsigned GetCurrentHealth () ;
		unsigned GetMaxHealth () ;

		unsigned GetCurrentIS () ;
		unsigned GetMaxIS () ;

		unsigned GetLevel () ;
		unsigned GetAlignment () ;

		unsigned GetDistrict () ;

		unsigned GetCash () ;

			//
			// This was originally the ViewID but I believe Game Object ID is more correct.
			//
		unsigned GetGOId () ;

			//
			// Returns true if the updates need to be propogated and resets
			// the flag to false.
			//
		bool PropagateUpdates ( void ) ;
		
			// Actions a character can perform.
			
			// Is there a way around providing a set position function?
		void SetPosition ( LocationVector* NewPosition ) ;
		void SetHeading ( double NewHeading ) ;
		void SetPositionX ( double NewPosX ) ;
		void SetPositionY ( double NewPosY ) ;
		void SetPositionZ ( double NewPosZ ) ;
		
		void SetTarget ( CharacterModel* NewTarget ) ;
		CharacterModel* GetTarget ( void ) ;

		bool IsNPC ( void ) ;

			// Eventually add range checks to prevent jumping too far and corrisponding status return?
		void Jump ( LocationVector* JumpTarget ) ;

			// The animation index needs to be incremented for each new
			// animation for this character.
		void IncrementAnimationIndex ( void ) ;
		unsigned GetAnimationIndex ( void ) ;

			// What kind of movement is the character currently performing.
		void SetMovementState ( MovementState_T NewState ) ;
		MovementState_T GetMovementState ( void ) ;

		void TakeDamage ( unsigned Damage ) ;

		void SetMood ( Mood_T NewMood ) ;
		Mood_T GetMood ( void ) ;

		void SetTactic ( CombatTactic* NewTactic ) ;
		CombatTactic* GetTactic ( void ) ;

		unsigned GetBaseAttackDamage ( void ) ;

		unsigned GetCharacterType ( void ) ;
		
		void SetDebuffState ( DebuffState_T NewDebuffState , unsigned DebuffTime /*In MS*/ ) ;
		bool HasDebuff ( DebuffState_T* TheDebuff ) ;

	protected :
	
		void positionUpdated ( void ) ;
		void attributesUpdated ( void ) ;

		void setHeading ( uint8 NewHeading ) ;
		
		string Handle ;
		string Background ;

		RsiData* RSI ;

		LocationVector* Position ;
		double Heading ;

		unsigned CurrentHealth ;
		unsigned MaxHealth ;

		unsigned CurrentIS ;
		unsigned MaxIS ;

		unsigned Level ;
		unsigned Alignment ; // Enum?

		unsigned District ;
		
		unsigned Cash ;

		unsigned GOId ;
		
	////////// Non persistant data.  All resets to default on login.		
		CharacterModel* TargetCharacter ;
		
		// AbilityTree, future object to be stored in CharacterModel.  Allows NPCs to have loaded abilities.

		bool NPC ;

			// Client expects the animation index to be incremented by 1 for each new animation.
		uint8 AnimationIndex ;

			// Flag provided publicly to indicate to the view process that updates need to be propogated to other clients.
		bool TransmitUpdates ;

		MovementState_T MyMovementState ;

		Mood_T MyMood ;

		CombatTactic* ActiveTactic ;

		unsigned CharacterType ;
		
		DebuffState_T MyDebuffState ;
		Timer* DebuffTimer ;
} ;

#endif MXOSIM_CHARACTERMODEL_H