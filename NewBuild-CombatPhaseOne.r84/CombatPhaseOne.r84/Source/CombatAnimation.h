#ifndef MXOSIM_COMBATANIMATION_H
#define MXOSIM_COMBATANIMATION_H

#include "Common.h"

class CombatAnimation
{
	public :

		CombatAnimation ( std::string NewName , byte NewAnimationCode[2] ) ;

		string GetName ( void ) ;

		byte* GetAnimationCode ( void ) ;
		
	private :

		std::string MyName ;
		byte MyAnimationCode[2] ;
} ;

#endif