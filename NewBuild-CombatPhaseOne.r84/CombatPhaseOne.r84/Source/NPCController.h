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
// The NPC Controller handles the actions of individual NPCs based on commands
// they have been given.
//
// Activities:
//    Idle : NPC is not moving, this does not prevent reactionary actions
//           such as attacking a PC in range if hostile.
//    Transiting : NPC is moving from a start point to an end point.
//    Patrolling : Puts the NPC on a patrol path that is repeated indefinitely.
//
// SetActivity allows the NPC to be set to an activity.  Should this override
// the queue entirely or just push the queue back and set a new first activity? Thinking the latter.
//
// QueueActivity queues a new activity to be executed after the current
// activities in the queue completes.
//
// ***************************************************************************

#ifndef MXOSIM_NPCCONTROLLER_H
#define MXOSIM_NPCCONTROLLER_H

#include "CharacterMover.h"
#include "Route.h"



class NPCController
{

	public :
	
		typedef enum Activity_T
		{
			ACT_IDLE ,
			ACT_TRANSITING ,
			ACT_PATROLLING ,
		} ;

			//
			// Set the NPC model and RPC handle associated with this clas.  Both
			// inputs will be consts.
			//
		NPCController ( NPCModel* TheCharacterModel ,
		                ProduceResponseRPC* TheResponseRPC ) ;
		
			//
			// Override the activity queue and make this activity the current
			// active activity.
			//
		void OverrideActivity ( Activity_T NewActivity , Route* NewRoute ) ;
		
			//
			// Add a new activity to the end of the current activity queue.
			//
		void QueueActivity ( Activity_T NewActivity , Route* NewRoute ) ;
		
			//
			// Empties the activity queue of all activites.  Primarily used for
			// removing a patrol from the queue.
			//Do we need something to only remove the first activity from the queue instead?
		void ClearQueue ( void ) ;

		void SetSpeed ( CharacterMover::MovementSpeed_T NewSpeed ) ;
		
			//
			// Performs periodic update for the current activity.
			// This can include checking to see if an NPC has reached a waypoint
			// as well as checking current tactic response to combat.
			//
		void NPCPeriodicUpdate ( void ) ;

	private :
	
			// An activity and its supporting information.
		typedef struct ActivityData_T
		{
			Activity_T TheActivity ;
			Route* SupportingData ; // This is going to be a union of possible support data later or replaced by an activity class.
		} ;

		Route* GetAssociatedRoute ( void ) ;
	
			//
			// Causes an NPC to continue walking to the current patrol waypoint
			// and increments to the next waypoint if the current one has been
			// reached.
			//
		void updatePatrol ( void ) ;

			//
			// Same activity as updatepatrol except the function will move to
			// the next activity if the end of the route is reached.
			//
		void updateTransit ( void ) ;

			// The RPC handler to tell the view process to send client messages.
		const ProduceResponseRPC* MyResponseRPC ;

			// The character being controlled.
		const NPCModel* MyCharacter ;
		
			// The "queue" of activities being performed, actions are always
			// pulled from the front but the front activity can be overridden
			// by a high priority action.
		list<ActivityData_T> ActivityQueue ;
		
			// An instance of the class which controls moving a character to a
			// specified point.
		CharacterMover* MyCharacterMover ;
} ;

#endif MXOSIM_NPCCONTROLLER_H