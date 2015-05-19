#include "NPCCollector.h"



NPCCollector::NPCCollector ( void )
{

}



bool NPCCollector::AddNPC ( unsigned GOId , NPCController* NewNPC )
{
	bool npcAdded = false ;

	if ( MyNPCs.find ( GOId ) == MyNPCs.end () )
	{
		MyNPCs[GOId] = NewNPC ;

		npcAdded = true ;
	}

	return npcAdded ;
}



void NPCCollector::SetFirstNPC ( void )
{
	CurrentNPCItterator = MyNPCs.begin () ;
}



void NPCCollector::SetNextNPC ( void )
{
	if ( CurrentNPCItterator != MyNPCs.end () )
	{
		CurrentNPCItterator++ ;
	}
}



NPCController* NPCCollector::GetCurrentNPC ( void )
{
	NPCController* foundNPC = NULL ;
	if ( CurrentNPCItterator != MyNPCs.end () )
	{
		foundNPC = CurrentNPCItterator->second ;
	}

	return foundNPC ;
}

		
		
NPCController* NPCCollector::GetNPC ( unsigned GOId )
{
	NPCController* foundNPC ;
	map < unsigned , NPCController* >::iterator npcItterator ;

	npcItterator = MyNPCs.find ( GOId ) ;
	if ( npcItterator != MyNPCs.end () )
	{
		foundNPC = npcItterator->second ;
	}
	else
	{
		foundNPC = NULL ;
	}

	return foundNPC ;
}



unsigned NPCCollector::NPCCount ( void )
{
	return MyNPCs.size () ;
}
