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
// The RPC response producer allows a function call from the control process
// (the prodcer) to be stored in the RPC queue.  The RPC response handler
// then executes the functions in the view process (the consumer).
//
// Because the control process does have access to the functions of the view
// process, an enum is used to tell the view process which function is being
// requested.  Each function has a different set of arguments but the
// structure containing these arguments is converted to a void* so all the
// different functions can be stored in the same struct.  The arguments are
// then converted back to their proper format before the RPC call is executed.
//
// ***************************************************************************

#ifndef MXOSIM_PRODUCERESPONSERPC_H
#define MXOSIM_PRODUCERESPONSERPC_H

#include "ResponseRPC.h"



class ProduceResponseRPC : public ResponseRPC
{

	public :

		ProduceResponseRPC ( queue < ResponseRPCFunction* >* NewResponseList ) ;

			//
			// Broadcast a position state update message to the clients.
			//
			// Actual function is performed by ResponseHandler::BroadcastPosition.
			//
		void BroadcastPosition ( CharacterModel* ResponseSource , CharacterModel* ExcludePlayer ) ;

			//
			// Broadcasts a chat message to all clients other than the client
			// that send the message.
			//
			// Actual function is performed by ResponseHandler::WorldChat.
			//
		void WorldChat ( CharacterModel* MessageSource , CharacterModel* ExcludePlayer , std::string TheMessage ) ;

		void SystemMessage ( CharacterModel* ToCharacter , std::string TheMessage ) ;

			//
			// Sends a command to start rotating a PC in a specified direction.
			//
			// Actual function is performed by ResponseHandler::StartRotatePlayer.
			//
		void StartRotatePlayer ( CharacterModel* RotatePlayeracter , StartPlayerRotateMsg::TurnDir_T TurnDirection ) ;

			//
			// Sends a command to update the heading of a rotating PC.
			// New heading is pulled from the CharacterModel.
			//
			// Actual function is performed by ResponseHandler::UpdateRotatePlayer.
			//
		void UpdateRotatePlayer ( CharacterModel* RotatePlayeracter ) ;

			//
			// Stops a rotating PC with a specified heading.
			// Final heading is pulled from the CharacterModel.
			//
			// Actual function is performed by ResponseHandler::StopRotatePlayer.
			//
		void StopRotatePlayer ( CharacterModel* RotatePlayeracter ) ;

			//
			// Starts a state for a player character, believe only movement is available.
			//
			// Actual function is performed by ResponseHandler::StartCharState.
			//
		void StartCharState ( CharacterModel* StateCharacter , StartPlayerStateMsg::State_T NewState ) ;

			//
			// Updates a player character state in progress for position and
			// rotation.  Position and rotation stored in the CharacterModel.
			//
			// Actual function is performed by ResponseHandler::UpdateCharState.
			//
		void UpdateCharState ( CharacterModel* StateCharacter ) ;

			//
			// Stops a player character state in progress with final position
			// and rotation.  Position and rotation stored in the CharacterModel.
			//
			// Actual function is performed by ResponseHandler::StopCharState.
			//
		void StopCharState ( CharacterModel* StateCharacter ) ;

			//
			// Updates an NPC current state as well as rotation and position.
			// Rotation and position stored in the CharacterModel.
			//
			// Actual function is performed by ResponseHandler::UpdateNPCState.
			//
		void UpdateNPCState ( CharacterModel* StateCharacter , UpdateNPCStateMsg::State_T NewState ) ;

			//
			// Updates an NPC current state as well as rotation and position.
			// Rotation and position stored in the CharacterModel.
			//
			// Actual function is performed by ResponseHandler::ChangeNPCState.
			//
		void ChangeNPCState ( CharacterModel* StateCharacter , ChangeNPCStateMsg::State_T NewState ) ;

			//
			// Spawns a new player character using the values in PlayerCharacterModel.
			//
			// Actual function is performed by ResponseHandler::SpawnCharacter.
			//
		void SpawnCharacter ( PlayerCharacterModel* TheCharacter ) ;

			//
			// Spawns a new NPC using the values in NPCModel.
			//
			// Actual function is performed by ResponseHandler::SpawnNPC.
			//
		void SpawnNPC ( NPCModel* TheCharacter ) ;

			//
			// Runs a 6 digit animation code on a character.
			//
			// Actual function is performed by ResponseHandler::RunAnimationCode.
			//
		void RunAnimationCode ( CharacterModel* AnimateCharacter , byte AnimationCode[3] ) ;

			//
			// Runs an enumerated animation code on a character.
			//
			// Actual function is performed by ResponseHandler::RunAnimationCode.
			//
		void RunAnimation ( CharacterModel* AnimateCharacter , unsigned AnimationIndex ) ;

		void HealthUpdate ( CharacterModel* UpdateCharacter ) ;

		void MoodUpdate ( CharacterModel* UpdateCharacter ) ;

		void SpawnInterlock ( CharacterModel* FirstCharacter , unsigned InterlockID ) ;

		void ViewInterlock ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , unsigned CombatAnimationIndex ) ;

		void InterlockAnimation ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , byte AnimationCode[2] ) ;

		void DespawnInterlock ( unsigned RemoveInterlock ) ;

	private :

} ;

#endif MXOSIM_PRODUCERESPONSERPC_H