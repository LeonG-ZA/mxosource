#include "ReceiveRequestRPC.h"



ReceiveRequestRPC::ReceiveRequestRPC ( RequestList_E** TheFirstRequest ) : RequestRPC ( TheFirstRequest )
{
}



void ReceiveRequestRPC::Jump ( CharacterModel* JumpCharacter , LocationVector JumpTarget )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	JumpRPC* theFunction = new JumpRPC ( JumpCharacter , JumpTarget ) ;

	addRequest ( ( RequestRPCFunction* )theFunction ) ;
}



void ReceiveRequestRPC::Chat ( CharacterModel* SourceCharacter , std::string ChatMessage )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	ChatRPC* theFunction = new ChatRPC ( SourceCharacter , ChatMessage ) ;

	addRequest ( ( RequestRPCFunction* )theFunction ) ;
}



void ReceiveRequestRPC::addRequest ( RequestRPCFunction* NewFunction )
{
		//	Allocate a new list element to memomry so it remains persistant.
		//	Must free after use.
	RequestList_E* newRequest = new RequestList_E ;

	newRequest->TheFunction = NewFunction ;
	newRequest->NextRequest = NULL ;

		// Start the list if its empty.
	if ( *FirstRequest == NULL )
	{
		*FirstRequest = newRequest ;
	}
		// Otherwise append to the end of the list.
	else
	{
		RequestList_E* nextRequest = *FirstRequest ;

		while ( nextRequest->NextRequest != NULL )
		{
			nextRequest = nextRequest->NextRequest ;
		}

		nextRequest->NextRequest = newRequest ;
	}
}
