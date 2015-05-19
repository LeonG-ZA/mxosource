#ifndef MXOSIM_ACTIVATABLEMAPPER_H
#define MXOSIM_ACTIVATABLEMAPPER_H

#include "Common.h"

class ActivatableMapper
{
	public :

		ActivatableMapper () ;

		int GetAbilityIndex ( unsigned AbilityCode ) ;

	private :

		map < unsigned , unsigned > AbilityIndexes ;
} ;

#endif