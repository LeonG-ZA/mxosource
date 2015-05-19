#include "ActivatableMapper.h"



ActivatableMapper::ActivatableMapper ( void )
{
	AbilityIndexes[ 0x00c6 ] = 0 ;
	AbilityIndexes[ 0x00c5 ] = 1 ;
}



int ActivatableMapper::GetAbilityIndex ( unsigned AbilityCode )
{
	int matchingCode = -1 ;

	map < unsigned , unsigned >::iterator foundIndex = AbilityIndexes.find ( AbilityCode ) ;

	if ( foundIndex != AbilityIndexes.end () )
	{
			// Handle overflow possibility but I don't expect we will ever have that many abilities.
		if ( ( ( int )foundIndex->second ) >= 0 )
		{
			matchingCode = foundIndex->second ;
		}
	}
	
	return matchingCode ;
}
