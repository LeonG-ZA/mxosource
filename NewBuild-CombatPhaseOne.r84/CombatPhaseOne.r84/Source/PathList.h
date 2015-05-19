#ifndef MXOSIM_PATHLIST_H
#define MXOSIM_PATHLIST_H

#include "LocationVector.h"



class PathList
{

	public :

		PathList ( void ) ;
		~PathList ( void ) ;

		bool SetCurrentPath ( unsigned PathID ) ;
		unsigned GetCurrentID ( void ) ;

		void AddPath ( LocationVector* NewPoint ) ;
		void RemovePatrolPoint ( LocationVector* OldPoint ) ;
		
			// Do we care about getting the first patrol point or always just
			// the next point where the first point loaded is the first point?
			
			// Using external points for indexing so multiple route instances
			// can be made of the present route.
		void IncrementPatrolPoint ( unsigned* CurrentPoint ) ;
		LocationVector* GetPatrolPoint ( unsigned CurrentPoint ) ;
		
	private :
	
		vector<LocationVector*> ThePath ;

		unsigned CurrentPathID ;

} ;

#endif MXOSIM_PATHLIST_H