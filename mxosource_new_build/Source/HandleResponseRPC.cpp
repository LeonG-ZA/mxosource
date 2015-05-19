#include "HandleResponseRPC.h"



HandleResponseRPC::HandleResponseRPC ( ResponseList_E** TheFirstResponse ) : ResponseRPC ( TheFirstResponse )
{
}



void HandleResponseRPC::SetResponseHandler ( ResponseHandler* NewResponseHandler )
{
	MyResponseHandler = NewResponseHandler ;
}



HandleResponseRPC::ResponseStatus_T HandleResponseRPC::ProcessResponse ( void )
{
	ResponseStatus_T responseStatus = SUCCESS ;
	ResponseList_E* firstResponse = getFirstResponse () ;

		// If there are messages in the queue process the first one.
	if ( firstResponse != NULL )
	{

		firstResponse->TheFunction->ProcessRPC ( MyResponseHandler ) ;

			// Free the request memory.
		delete ( firstResponse->TheFunction ) ;
		delete ( firstResponse ) ;
	}
		// No messages left, indicate this to the caller.
	else
	{
		responseStatus = NO_RESPONSE ;
	}

	return responseStatus ;
}



HandleResponseRPC::ResponseList_E* HandleResponseRPC::getFirstResponse ( void )
{
	ResponseList_E* firstResponse = *FirstResponse ;

	if ( firstResponse != NULL )
	{
		*FirstResponse = ( *FirstResponse )->NextResponse ;
	}

	return firstResponse ;
}
