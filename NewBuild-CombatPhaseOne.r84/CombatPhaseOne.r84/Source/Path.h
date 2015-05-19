// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2012 - Michael Lingg
// http://mxoemu.info
//
// ---------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ---------------------------------------------------------------------------
//
//	Path creates a list of points along a path.  In includes patrol and
//	transit paths.  Patrol paths are circular, when the path completes it
//	cycles back to the beginnning in an endless loop.  Transit are one pass
//	paths that stop after one cycle.
//
//	Paths provide the template for an NPC to follow, they do not track the
//	current point being used.  See the Route class for the implementation of
//	the route to be associated with an NPC.
//
// ***************************************************************************

#ifndef MXOSIM_PATH_H
#define MXOSIM_PATH_H

#include "LocationVector.h"



class Path
{

	public :

		typedef enum PathType_T
		{
				// Continuous loop patrol route.
			Patrol ,
			
				// Single pass path.
			Transit ,
		} ;
	
			//
			//	Constructor specifies the path type, unchangable.
			//
		Path ( PathType_T NewPathType ) ;
		~Path ( void ) ;

			//
			//	Add a new point to the end of the path.
			//
		unsigned AddPoint ( LocationVector* NewPoint ) ;
			// Need future functions to modify points and add to a specific index.
		
			//
			//	Not implemented.
			//
		void RemovePoint ( LocationVector* OldPoint ) ;
		
			// Do we care about getting the first patrol point or always just
			// the next point where the first point loaded is the first point?
			
			// 
			//	Given a point index, increments the index and returns true
			//	if the new index is valid.
			//
		bool IncrementPoint ( unsigned* CurrentPoint ) ;
		
			//
			//	Returns the path point at the specified index.
			//
		LocationVector* GetPoint ( unsigned CurrentPoint ) ;
		
	private :

			// The type this path is.
		const PathType_T PathType ;
		
			// The actual path, in order by index.
		vector<LocationVector> ThePath ;
} ;

#endif MXOSIM_PATH_H