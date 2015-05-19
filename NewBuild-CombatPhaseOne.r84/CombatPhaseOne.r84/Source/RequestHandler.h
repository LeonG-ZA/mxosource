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
//	The Request Handler proccesses commands from the client and runs game
//	logic to determine the responses to the client(s).
//
// ***************************************************************************

#ifndef MXOSIM_REQUESTHANDLER_H
#define MXOSIM_REQUESTHANDLER_H

#include "CombatHandler.h"
#include "NPCCollector.h"



class RequestHandler
{

	public :

			//
			// Store the respone RPC to use to send response messages back to
			// the view process.
			//
		RequestHandler ( ProduceResponseRPC* NewResponseProducer , WorldModel* NewWorldModel , NPCCollector* NewNPCCollector , CombatHandler* MyCombatHandler ) ;

			//
			//	Jump the specified character to the specified coordinates.
			//
		void Jump ( PlayerCharacterModel* JumpCharacter , LocationVector* TargetLocation ) ;

			//
			//	Handle a chat request from a client
			//
		void Chat ( PlayerCharacterModel* MessageSource , std::string TheMessage ) ;

		void HandleCloseCombat ( PlayerCharacterModel* AttackingCharacter , CharacterModel* TargetCharacter , unsigned TargetProtocol , unsigned Slot ) ;

		void TacticUpdate ( PlayerCharacterModel* AttackingCharacter , unsigned TacticIndex ) ;

		void Withdraw ( PlayerCharacterModel* WithdrawCharacter ) ;

		void ActivateAbility ( PlayerCharacterModel* ActivateCharacter , unsigned AbilityIndex ) ;

	private :

		ProduceResponseRPC* MyResponseProducer ;
		
		WorldModel* MyWorldModel ;

		NPCCollector* MyNPCCollector ;

		CombatHandler* MyCombatHandler ;
		
		ActivatableAbilities MyActivatableAbilities ;

} ;

#endif MXOSIM_REQUESTHANDLER_H