#include "ControlServer.h"

#include "RequestHandler.h"
#include "Log.h"



ControlServer::ControlServer ( WorldModel* NewWorldModel , HandleRequestRPC* NewRequestRPC , ProduceResponseRPC* NewResponseRPC ) : MyAITimer ( 100 , Timer::REPEATING ) , MyCombatTimer ( 100 , Timer::REPEATING )
{
	MyRequestRPC = NewRequestRPC ;

	MyNPCConfig = new NPCConfig () ;
	
	NPCCollector* newNPCCollector = new NPCCollector () ;

	MyCombatHandler = new CombatHandler ( NewWorldModel , NewResponseRPC ) ;

	RequestHandler* newRequestHandler = new RequestHandler ( NewResponseRPC , NewWorldModel , newNPCCollector , MyCombatHandler ) ;
	
	MyRequestRPC->SetRequestHandler ( newRequestHandler ) ;

	MyAIHandler = new AIHandler ( newNPCCollector , NewResponseRPC ) ;
}



void ControlServer::ProcessRequests ( void )
{
	while ( MyRequestRPC->ProcessRequest () == HandleRequestRPC::SUCCESS ) ;

	if ( MyAITimer.TimedOut () )
	{
		MyAIHandler->ProcessAI () ;
	}

	if ( MyCombatTimer.TimedOut () )
	{
		MyCombatHandler->ProcessCombat () ;
	}
}
