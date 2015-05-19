#ifndef MXOSIM_COMBATHANDLER_H
#define MXOSIM_COMBATHANDLER_H

#include "CombatSession.h"
#include "WorldModel.h"



class CombatHandler
{

	public :
	
			// Replace with a boolean if there is only one failure condition...
		typedef enum SetAbilityStatus_T
		{
			SUCCESS ,
			NOT_IN_COMBAT ,
		} ;
	
		CombatHandler ( WorldModel* NewWorldModel , ProduceResponseRPC* NewResponseRPC ) ;

		void ProcessCombat ( void ) ;
		
		void HandleCloseCombat ( CharacterModel* AttackingCharacter , CharacterModel* TargetCharacter , unsigned TargetProtocol , unsigned Slot ) ;

		//void HandleFreeCombat ( CharacterModel* AttackingCharacter , CharacterModel* TargetCharacter ) ;
		
		SetAbilityStatus_T SetCombatAbility ( PlayerCharacterModel* ActivateCharacter , CombatAbility* TheAbility ) ;

		unsigned SpawnInterlock ( CharacterModel* AttackerCharacter , CharacterModel* DefenderCharacter ) ;
		std::stringstream RunCombatAnimation ( CharacterModel* TargetCharacter , std::string TargetAnimation ) ;

		void Withdraw ( CharacterModel* WithdrawCharacter ) ;

	private :

		void startCombat ( CharacterModel* AttackingCharacter ,
		                   CharacterModel* TargetCharacter , unsigned TargetProtocol , unsigned Slot ) ;
								 
		CombatSession* getCombatSession ( CharacterModel* CombatCharacter ) ;
		CombatSession* getFirstCombatSession ( void ) ;
		CombatSession* getNextCombatSession ( unsigned PreviousSessionId ) ;

		//static const unsigned CombatRoundTime = 4000 ;
	
		ProduceResponseRPC* MyResponseRPC ;

		WorldModel* MyWorldModel ;

		map < unsigned , CombatSession > CombatSessions ;
} ;

#endif MXOSIM_COMBATHANDLER_H