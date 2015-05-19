#include "ProduceResponseRPC.h"



ProduceResponseRPC::ProduceResponseRPC ( ResponseList_E** TheFirstResponse ) : ResponseRPC ( TheFirstResponse )
{
}



void ProduceResponseRPC::BroadcastPosition ( CharacterModel* ResponseSource , bool ExcludeSource )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	BroadcastPositionRPC* theFunction = new BroadcastPositionRPC ( ResponseSource , ExcludeSource ) ;

	addResponse ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::BroadcastWorldChat ( CharacterModel* MessageSource , std::string TheMessage )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	BroadcastWorldChatRPC* theFunction = new BroadcastWorldChatRPC ( MessageSource , TheMessage ) ;

	addResponse ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::addResponse ( ResponseRPCFunction* TheFunction )
{
		//	Allocate a new list element to memomry so it remains persistant.
		//	Must free after use.
	ResponseList_E* newResponse = new ResponseList_E ;

	newResponse->TheFunction = TheFunction ;
	newResponse->NextResponse = NULL ;

		// Start the list if its empty.
	if ( *FirstResponse == NULL )
	{
		*FirstResponse = newResponse ;
	}
		// Otherwise append to the end of the list.
	else
	{
		ResponseList_E* NextResponse = *FirstResponse ;

		while ( NextResponse->NextResponse != NULL )
		{
			NextResponse = NextResponse->NextResponse ;
		}

		NextResponse->NextResponse = newResponse ;
	}
}
