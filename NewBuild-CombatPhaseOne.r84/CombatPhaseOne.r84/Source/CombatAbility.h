#ifndef MXOSIM_COMBATABILITY_H
#define MXOSIM_COMBATABILITY_H

#include "Common.h"
#include "CharacterModel.h"

class CombatAbility
{
	public :
	
			// Should probably become a class of its own if it gets any more complex.
		typedef struct Debuff_T
		{
			CharacterModel::DebuffState_T Type ;
			// Chance of debuff?
		} ;

		CombatAbility ( std::string NewName , unsigned NewBaseDamage , unsigned NewAnimationIndex ) ;
		CombatAbility ( std::string NewName , unsigned NewBaseDamage , unsigned NewAnimationIndex , Debuff_T NewAbilityDebuff ) ;
//		CombatAbility ( std::string NewName , unsigned NewBaseDamage , unsigned NewAnimationIndex , Buff_T AbilityBuff ) ;
		
		string GetName ( void ) ;
		unsigned GetBaseDamage ( void ) ;
		unsigned GetAnimationIndex ( void ) ;
		
		bool GetDebuff ( CharacterModel::DebuffState_T* TheDebuff ) ;
		
	private :

		std::string MyName ;
		unsigned MyBaseDamage ;

		//TargetRequirement_T TargetRequirement ;

		//unsigned Range ;

		bool HasDebuff ;
		Debuff_T CombatDebuff ;

		unsigned MyAnimationIndex ;
	
} ;

#endif