#ifndef MXOSIM_CONTROLSERVER_H
#define MXOSIM_CONTROLSERVER_H

#include "AIHandler.h"
#include "HandleRequestRPC.h"
#include "NPCConfig.h"



class ControlServer
{

	public :

		ControlServer ( WorldModel* NewWorldModel , HandleRequestRPC* NewRequestRPC , ProduceResponseRPC* NewResponseRPC ) ;

		void ProcessRequests ( void ) ;

	private :

		HandleRequestRPC* MyRequestRPC ;
		NPCConfig* MyNPCConfig ;

		AIHandler* MyAIHandler ;
		Timer MyAITimer ;

		CombatHandler* MyCombatHandler ;
		Timer MyCombatTimer ;
} ;

#endif MXOSIM_CONTROLSERVER_H