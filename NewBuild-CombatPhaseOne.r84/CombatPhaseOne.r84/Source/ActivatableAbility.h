#ifndef MXOSIM_ACTIVATABLEABILITY_H
#define MXOSIM_ACTIVATABLEABILITY_H

#include "CombatTactic.h"
#include "CombatAbility.h"

class ActivatableAbilities
{
	public :

		ActivatableAbilities ( void ) ;

		CombatTactic* GetTactic ( unsigned TacticIndex ) ;
		CombatAbility* GetAbility ( unsigned AbilityIndex ) ;

	private :

		vector < CombatTactic > CombatTactics ;
		vector < CombatAbility > CombatAbilities ;
} ;

#endif MXOSIM_ACTIVATABLEABILITY_H