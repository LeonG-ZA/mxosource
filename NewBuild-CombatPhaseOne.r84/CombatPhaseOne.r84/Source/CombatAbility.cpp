#include "CombatAbility.h"



CombatAbility::CombatAbility ( std::string NewName , unsigned NewBaseDamage , unsigned NewAnimationIndex )
{
	MyName = NewName ;
	MyBaseDamage = NewBaseDamage ;
	MyAnimationIndex = NewAnimationIndex ;
	
	HasDebuff = false ;
}



CombatAbility::CombatAbility ( std::string NewName , unsigned NewBaseDamage , unsigned NewAnimationIndex , Debuff_T NewAbilityDebuff )
{
	CombatAbility ( NewName , NewBaseDamage , NewAnimationIndex ) ;

	HasDebuff = true ;
	
	CombatDebuff.Type = NewAbilityDebuff.Type ;
}



string CombatAbility::GetName ( void )
{
	return MyName ;
}



unsigned CombatAbility::GetBaseDamage ( void )
{
	return MyBaseDamage ;
}



unsigned CombatAbility::GetAnimationIndex ( void )
{
	return MyAnimationIndex ;
}



bool CombatAbility::GetDebuff ( CharacterModel::DebuffState_T* TheDebuff )
{
	TheDebuff = NULL ;

	if ( HasDebuff == true )
	{
		TheDebuff = &( CombatDebuff.Type ) ;
	}

	return HasDebuff ;
}