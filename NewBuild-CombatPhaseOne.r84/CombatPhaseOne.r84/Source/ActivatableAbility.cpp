#include "ActivatableAbility.h"



ActivatableAbilities::ActivatableAbilities ( void )
{
		// Update later to be loaded from xml file.
	CombatTactics.push_back ( CombatTactic ( "Speed" , 1.00 , 1.00 , 1.15 , 0.80 , 0.90 , 1 ) ) ;
	CombatTactics.push_back ( CombatTactic ( "Power" , 1.20 , 0.80 , 0.90 , 1.00 , 1.00 , 2 ) ) ;
	CombatTactics.push_back ( CombatTactic ( "Grab" , 0.85 , 0.35 , 1.05 , 2.00 , 1.30 , 3 ) ) ;
	CombatTactics.push_back ( CombatTactic ( "Block" , 0.00 , 1.60 , 0.00 , 0.00 , -0.10 , 4 ) ) ;
	
	CombatAbilities.push_back ( CombatAbility ( "Cheap Shot" , 7 , 5 ) ) ;
	CombatAbilities.push_back ( CombatAbility ( "Head Butt" , 16 , 6 ) ) ;
}



CombatTactic* ActivatableAbilities::GetTactic ( unsigned TacticIndex )
{
	CombatTactic* matchingTactic = NULL ;

	if ( TacticIndex < CombatTactics.size () )
	{
		matchingTactic = &CombatTactics [ TacticIndex ] ;
	}

	return matchingTactic ;
}



CombatAbility* ActivatableAbilities::GetAbility ( unsigned AbilityIndex )
{
	CombatAbility* matchingTactic = NULL ;

	if ( AbilityIndex < CombatAbilities.size () )
	{
		matchingTactic = &CombatAbilities [ AbilityIndex ] ;
	}

	return matchingTactic ;
}
