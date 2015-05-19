#ifndef MXOSIM_CONTROLSERVER_H
#define MXOSIM_CONTROLSERVER_H

#include "HandleRequestRPC.h"
#include "ProduceResponseRPC.h"



class ControlServer
{

	public :

		ControlServer ( HandleRequestRPC* NewRequestRPC , ProduceResponseRPC* NewResponseRPC ) ;

		void ProcessRequests ( void ) ;

	private :

		HandleRequestRPC* MyRequestRPC ;

} ;

#endif MXOSIM_CONTROLSERVER_H