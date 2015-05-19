#include "AIHandler.h"



AIHandler::AIHandler ( NPCCollector* TheNPCCollector , ProduceResponseRPC* TheResponseRPC )
{
	MyNPCCollector = TheNPCCollector ;
	MyResponseRPC = TheResponseRPC ;
}



void AIHandler::ProcessAI ( void )
{
	NPCController* currentNPC = NULL ;

	MyNPCCollector->SetFirstNPC () ;
	while ( ( currentNPC = MyNPCCollector->GetCurrentNPC () ) != NULL )
	{
		currentNPC->NPCPeriodicUpdate () ;
		MyNPCCollector->SetNextNPC () ;
	}
}
