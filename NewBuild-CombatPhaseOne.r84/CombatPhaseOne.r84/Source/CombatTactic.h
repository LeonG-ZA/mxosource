#ifndef MXOSIM_COMBATTACTIC_H
#define MXOSIM_COMBATTACTIC_H

#include "Common.h"

class CombatTactic
{
	public :

		CombatTactic ( std::string NewName , double NewDamageBonus , double NewEvasionBonus , double NewAccuracyBonus , double NewForceBonus , double NewDefenseBonus , unsigned NewAnimationIndex ) ;

		string GetName ( void ) ;

		double GetDamageBonus ( void ) ;
		
		unsigned GetAnimationIndex ( void ) ;

	private :

		std::string MyName ;

		double MyDamageBonus ;
		double MyEvasionBonus ;
		double MyAccuracyBonus ;
		double MyForceBonus ;
		double MyDefenseBonus ;

		unsigned MyAnimationIndex ;
} ;

#endif