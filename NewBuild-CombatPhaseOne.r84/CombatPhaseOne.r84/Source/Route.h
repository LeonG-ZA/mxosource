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
//	The Route class tracks the current point of interest within a path.
//	Essentially the path is a template for the route and the route is an
//	active instance.
//
//	The Route class allows transitioning from one point to the next in the
//	path.
//
// ***************************************************************************

#ifndef MXOSIM_ROUTE_H
#define MXOSIM_ROUTE_H

#include "Path.h"



class Route
{

	public :

			// Creates a Route based on a specific path.
			// The path cannot be changed.
		Route ( Path* ThePath ) ;
		~Route ( void ) ;

			// Increments to the next point in the route.  Returns false
			// if no more valid points exist.
		bool IncrementRoute ( void ) ;
		
			// Returns the current point in the route.
		LocationVector* GetCurrentRoutePoint ( void ) ;
		
	private :
	
		Path* const MyPath ;
		unsigned CurrentPoint ;

} ;

#endif MXOSIM_ROUTE_H