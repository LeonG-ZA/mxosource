#include "HandleRequestRPC.h"



HandleRequestRPC::HandleRequestRPC ( RequestList_E** TheFirstRequest ) : RequestRPC ( TheFirstRequest )
{
	MyRequestHandler = NULL ;
}



void HandleRequestRPC::SetRequestHandler ( RequestHandler* NewRequestHandler )
{
	MyRequestHandler = NewRequestHandler ;
}



HandleRequestRPC::RequestStatus_T HandleRequestRPC::ProcessRequest ( void )
{
	RequestStatus_T RequestStatus = SUCCESS ;
	RequestList_E* firstRequest = getFirstRequest () ;

		// If there are messages in the queue process the first one.
	if ( firstRequest != NULL )
	{

		firstRequest->TheFunction->ProcessRPC ( MyRequestHandler ) ;

			// Free the request memory.
		delete ( firstRequest->TheFunction ) ;
		delete ( firstRequest ) ;
	}
		// No messages left, indicate this to the caller.
	else
	{
		RequestStatus = NO_REQUEST ;
	}

	return RequestStatus ;
}



HandleRequestRPC::RequestList_E* HandleRequestRPC::getFirstRequest ( void )
{
	RequestList_E* firstRequest = *FirstRequest ;

	if ( firstRequest != NULL )
	{
		*FirstRequest = ( *FirstRequest )->NextRequest ;
	}

	return firstRequest ;
}
