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
// The Character Mover class handles sending movement commands to move a
// character to a specified waypoint.
//
// First the character is turned in the right direction and then they begin
// walking until they are estimated to have reached the waypoint.
//
// We do not have position updates for NPCs so current position is based only
// on estimates (position updats for PCs could be factored in eventually) so
// the final item is to jump the character to the target position.
//
// The software handles sending the appropriate messages if the character is
// a PC or NPC.
//
// ***************************************************************************

#ifndef MXOSIM_CHARACTERMOVER_H
#define MXOSIM_CHARACTERMOVER_H

#include "CharacterModel.h"
#include "LocationVector.h"
#include "NPCModel.h"
#include "ProduceResponseRPC.h"

#include <time.h>



class CharacterMover
{

	public :

			// Possible movement speeds available to the client.
		typedef enum MovementSpeed_T
		{
			IDLE ,
			WALK ,
			WALK_BACKWARD ,
			RUN ,
		} ;
	
			//
			// Provides the character being moved and the RPC handler to send
			// messages back to the view process.
			//
		CharacterMover ( CharacterModel* TheCharacterModel ,
		                 ProduceResponseRPC* TheResponseRPC ) ;
		
			//
			// Sets the position the caracter will be moved to.
			//
		void SetTarget ( LocationVector* NewTarget ) ;
		
		void SetTarget ( CharacterModel* NewTarget ) ;

			//
			// Sets the planned speed of the character movement.
			//
		void SetMovementSpeed ( MovementSpeed_T NewSpeed ) ;
		
			//
			// Returns the current speed the character is moving.
			//
		CharacterModel::MovementState_T GetMovementState ( void ) ;

			//
			// 
			//
		bool AlignToTarget ( void ) ;
		
			//
			// Performs update processing to continue moving the character
			// toward the target waypoint.
			// Returns true if the character is at the target waypoint.
			// Be careful not to use this when the character is idling as the
			// character will never reach the waypoint.
			//
		bool MoveToTarget ( void ) ;

	private :
	
			//
			// Updates the present estimated position of the character using
			// a vector where the direction is the angle between the
			// character's present position and the target waypoint and the
			// magnitude is the character's movement speed and time since
			// previous update.
			//
		void updateEstimatedPosition ( void ) ;
		
			//
			// Handles sending the appropriate rotation message for PC vs NPC.
			//
		void rotateToPosition ( double NewHeading ) ;
		
			//
			// Handles sending the appropriate movement state message for PC vs
			// NPC.
			//
		void startMovement ( CharacterModel::MovementState_T MovementState ) ;

			//
			// Handles sending the stop message for PC vs NPC, always sets state
			// to IDLE.  Includes telling the client to move the character to the
			// final target position.
			// If EndPosition is NULL, the character position will not be changed
			// and the current estimated position will be sent to the client(s).
			//
		void stopMovement ( void ) ;

			// The character being moved.
		CharacterModel* const MyCharacterModel ;
		
			// The RPC handler to send movement commands to the view state.
		ProduceResponseRPC* const MyResponseRPC ;
		
			// Target position for movement.
		LocationVector MyTarget ;
		CharacterModel* TargetCharacter ;
		bool StaticTarget ;

			// Target speed for movement.
		MovementSpeed_T MyMovementSpeed ;
		
			// Last time the position was updated.  Value of -1 indicates the
			// character was not previously moving.
		clock_t LastMovementTime ;
		
			// Estimate for walking speed every MS, probably not right.
		static const unsigned WalkDistancePerS = 150 ;
			// Backward slower than forward?
			
			// I think running is exactly three times the speed of walking?
		static const unsigned RunDistancePerS = 3 * WalkDistancePerS ;
} ;

#endif MXOSIM_CHARACTERMOVER_H