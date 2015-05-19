#include "CombatAnimationList.h"



CombatAnimationList::CombatAnimationList ( void )
{
		// Update later to be loaded from xml file.
	byte newAnimation[2] = { 0x14, 0x52 } ;
	CombatAnimations.push_back ( CombatAnimation ( "Idle" , newAnimation ) ) ;
	newAnimation[0] = 0x92 ; newAnimation[1] = 0x1F ;
	CombatAnimations.push_back ( CombatAnimation ( "Animation1" , newAnimation ) ) ;
	newAnimation[0] = 0x99 ; newAnimation[1] = 0x1F ;
	CombatAnimations.push_back ( CombatAnimation ( "Animation2" , newAnimation ) ) ;
	newAnimation[0] = 0x91 ; newAnimation[1] = 0x1F ;
	CombatAnimations.push_back ( CombatAnimation ( "Animation3" , newAnimation ) ) ;
	newAnimation[0] = 0x95 ; newAnimation[1] = 0x1F ;
	CombatAnimations.push_back ( CombatAnimation ( "Animation3" , newAnimation ) ) ;
	newAnimation[0] = 0x1D ; newAnimation[1] = 0x01 ;
	CombatAnimations.push_back ( CombatAnimation ( "CheapShot" , newAnimation ) ) ;
	newAnimation[0] = 0x1C ; newAnimation[1] = 0x01 ;
	CombatAnimations.push_back ( CombatAnimation ( "HeadButt" , newAnimation ) ) ;
}



CombatAnimation* CombatAnimationList::GetAnimation ( unsigned AnimationIndex )
{
	CombatAnimation* matchingAnimation = NULL ;

	if ( AnimationIndex < CombatAnimations.size () )
	{
		matchingAnimation = &CombatAnimations [ AnimationIndex ] ;
	}

	return matchingAnimation ;
}
