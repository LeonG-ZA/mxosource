#include "CombatAnimation.h"


CombatAnimation::CombatAnimation ( std::string NewName , byte NewAnimationCode[2] )
{
	MyName = NewName ;

	MyAnimationCode[0] = NewAnimationCode[0] ;
	MyAnimationCode[1] = NewAnimationCode[1] ;
}



string CombatAnimation::GetName ( void )
{
	return MyName ;
}



byte* CombatAnimation::GetAnimationCode ( void )
{
	return ( byte* )&MyAnimationCode ;
}
