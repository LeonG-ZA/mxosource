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
//	The Command Handler proccesses commands from the client and determines
//	the proper responses.
//
// ***************************************************************************

#ifndef MXOSIM_RESPONSEHANDLER_H
#define MXOSIM_RESPONSEHANDLER_H

#include "CombatAnimationList.h"
#include "MessageTypes.h"

class ResponseHandler
{

	public :

		ResponseHandler () ;

			//
			//	Broadcast the position of the specified character via an
			//	WorldChatMsg.
			//	ExcludePlayerID allows a client associated with the player ID
			//	to be excluded from the broadcast.  Typically used to avoid
			//	sending the update back to the client that originally sent it.
			//	ExcludePlayerID = 0 broadcasts the message to all clients.
			//
		void BroadcastPosition ( CharacterModel* TheCharacter , CharacterModel* ExcludePlayer ) ;

			//
			//	Broadcast the chat message TheMessage and the handle of the
			//	character, TheCharacter, that sent the chat via a WorldChatMsg.
			//	Always excludes the client that controls TheCharacter.
			//	ExcludePlayerID allows a client associated with the player ID
			//	to be excluded from the broadcast.  Typically used to avoid
			//	sending the chat back to the client that originally sent it.
			//	ExcludePlayerID = 0 broadcasts the message to all clients.
			//
		void WorldChat ( CharacterModel* TheCharacter , CharacterModel* ExcludePlayer , std::string TheMessage ) ;

		void SystemMessage ( CharacterModel* ToCharacter , std::string TheMessage ) ;

			//
			// Send the message that spanws a new player character.
			// All of the attributes are stored in the Player Character Model.
			//
		void SpawnCharacter ( PlayerCharacterModel* TheCharacter ) ;
		
			//
			// Send the message that spawns a new NPC.
			// All of the attributes are stored in NPC Model.
			//
		void SpawnNPC ( NPCModel* TheCharacter ) ;

		void StateUpdate ( PlayerCharacterModel* TheCharacter , ByteBuffer StateData ) ;
		void NPCStateUpdate ( NPCModel* TheCharacter ) ;
		void NPCPosUpdate ( NPCModel* TheCharacter ) ;

		void StartRotatePlayer ( CharacterModel* RotatePlayeracter , StartPlayerRotateMsg::TurnDir_T TurnDirection ) ;
		void UpdateRotatePlayer ( CharacterModel* RotatePlayeracter ) ;
		void StopRotatePlayer ( CharacterModel* RotatePlayeracter ) ;

		void StartCharState ( CharacterModel* StateCharacter , StartPlayerStateMsg::State_T NewState ) ;
		void UpdateCharState ( CharacterModel* StateCharacter ) ;
		void StopCharState ( CharacterModel* StateCharacter ) ;

		void UpdateNPCState ( CharacterModel* StateCharacter , UpdateNPCStateMsg::State_T NewState ) ;
		void ChangeNPCState ( CharacterModel* StateCharacter , ChangeNPCStateMsg::State_T NewState ) ;

			//
			//	Runs a six digit animation on the specified character.
			//
		void RunAnimationCode ( CharacterModel* AnimateCharacter , byte* AnimationCode ) ;

			//
			//	Runs an enumerated animation on the specified character.
			//
		void RunAnimation ( CharacterModel* AnimateCharacter , unsigned AnimationIndex ) ;

		void HealthUpdate ( CharacterModel* UpdateCharacter ) ;

		void MoodUpdate ( CharacterModel* UpdateCharacter ) ;

		void ViewInterlock ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , unsigned CombatAnimationIndex ) ;

		void SpawnInterlock ( CharacterModel* FirstCharacter , unsigned InterlockID ) ;

		void InterlockAnimation ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , byte AnimationCode[2] ) ;

		void DespawnInterlock ( unsigned RemoveInterlock ) ;

	private :

			//
			//	Broadcasts the specified message, theMsg, as a state message
			//	to all clients, except for the client that controls
			//	TheCharacter if ExcludeSource is true.
			//
		void broadcastState ( msgBaseClassPtr TheMsg , CharacterModel* ExcludePlayer ) ;

			//
			//	Broadcasts the specified message, theMsg, as a command message
			//	to all clients, except for the client that controls
			//	TheCharacter if ExcludeSource is true.
			//
		void broadcastCommand ( msgBaseClassPtr TheMsg , CharacterModel* ExcludePlayer ) ;

		void sendCommand ( msgBaseClassPtr TheMsg , CharacterModel* TargetPlayer ) ;

		CombatAnimationList MyCombatAnimations ;

} ;

#endif MXOSIM_RESPONSEHANDLER_H