#ifndef MXOSIM_COMBATSESSION_H
#define MXOSIM_COMBATSESSION_H

#include "ActivatableAbility.h"
#include "ProduceResponseRPC.h"



class CombatSession
{

	public :
	
		typedef enum SessionCharacter_T
		{
			FIRST_CHARACTER ,
			SECOND_CHARACTER ,
		} ;

		typedef enum CombatState_T
		{
			ENGAGING ,
			ENGAGED ,
		} ;

		CombatSession ( unsigned NewInterlockId , CharacterModel* AttackingCharacter , CharacterModel* DefendingCharacter , ProduceResponseRPC* NewResponseRPC , bool NewSessionReal = true ) ;

		unsigned GetInterlockId ( void ) ;
		void StartTimers ( void ) ;
		
		bool ProcessSession ( void ) ;

			// Move some of this into the combat session rather than leave it in CombatHandler.
		bool AlignToAttack ( SessionCharacter_T AttackingCharacter ) ;
		
		void SetAbility ( SessionCharacter_T AbilityCharacter , CombatAbility* NewAbility ) ;

		CharacterModel* GetCharacter ( SessionCharacter_T AbilityCharacter ) ;

		//CombatTactic* GetTactic ( SessionCharacter_T AbilityCharacter ) ;

		//CombatAbility* GetAbility ( SessionCharacter_T AbilityCharacter ) ;

		void IncrementInterlockIndex ( void ) ;
		unsigned GetInterlockIndex ( void ) ;

		void SetState ( CombatState_T NewState ) ;
		CombatState_T GetState ( void ) ;

		bool IsRealSession ( void ) ;

		void Withdraw ( void ) ;

	private :
	
		typedef struct CombatantSession_T
		{
			CharacterModel* CombatantCharacter ;
			Timer* AttackTimer ;
			CombatAbility* ActiveAbility ;
		} ;

		ProduceResponseRPC* MyResponseRPC ;
		
			// Put all first character stuff in a struct and second character stuff in a separate struct and commonize attacker/defender functions?
		CombatantSession_T FirstCombatant ;
		CombatantSession_T SecondCombatant ;
		CombatState_T CombatState ;
		//unsigned TargetProtocol ;
		//unsigned TheSlot ;
		unsigned InterlockId ;	// Not session data, data for the object that controls the session?
		unsigned InterlockIndex ;	

		bool RealSession ;

		static const unsigned COMBAT_ROUND_TIME = 5000 ;	// This should probably not be a local constant...


		bool performAttack ( CombatSession::SessionCharacter_T AttackingCharacter ) ;

		bool attackSuccess ( SessionCharacter_T AttackingCharacter ) ;

		unsigned attackDamage ( SessionCharacter_T AttackingCharacter ) ;
		
		void handleDebuff ( CharacterModel* AttackingCharacter , CharacterModel* DefendingCharacter , CharacterModel::DebuffState_T* AttackDebuff ) ;

} ;

#endif MXOSIM_COMBATSESSION_H