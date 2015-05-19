#include "CombatTactic.h"



CombatTactic::CombatTactic ( std::string NewName , double NewDamageBonus , double NewEvasionBonus , double NewAccuracyBonus , double NewForceBonus , double NewDefenseBonus , unsigned NewAnimationIndex )
{
	MyName = NewName ;
	MyDamageBonus = NewDamageBonus ;

	MyAnimationIndex = NewAnimationIndex ;
}



string CombatTactic::GetName ( void )
{
	return MyName ;
}



double CombatTactic::GetDamageBonus ( void )
{
	return MyDamageBonus ;
}



unsigned CombatTactic::GetAnimationIndex ( void )
{
	return MyAnimationIndex ;
}
