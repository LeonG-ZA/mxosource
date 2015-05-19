#ifndef MXOSIM_COMBATANIMATIONLIST_H
#define MXOSIM_COMBATANIMATIONLIST_H

#include "CombatAnimation.h"

class CombatAnimationList
{
	public :

		CombatAnimationList ( void ) ;

		CombatAnimation* GetAnimation ( unsigned AnimationIndex ) ;

	private :

		vector < CombatAnimation > CombatAnimations ;
} ;

#endif