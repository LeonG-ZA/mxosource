#include "HandleResponseRPC.h"



HandleResponseRPC::HandleResponseRPC ( queue < ResponseRPCFunction* >* NewResponseList ) : ResponseRPC ( NewResponseList )
{
}



void HandleResponseRPC::SetResponseHandler ( ResponseHandler* NewResponseHandler )
{
	MyResponseHandler = NewResponseHandler ;
}



HandleResponseRPC::ResponseStatus_T HandleResponseRPC::ProcessResponse ( void )
{
	ResponseStatus_T responseStatus = SUCCESS ;

		// If there are messages in the queue process the first one.
	if ( ResponseList->empty () == false )
	{
		ResponseRPCFunction* firstFunction = ResponseList->front () ;

		firstFunction->ProcessRPC ( MyResponseHandler ) ;

			// Free the request memory.
		delete ( firstFunction ) ;

		ResponseList->pop () ;
	}
		// No messages left, indicate this to the caller.
	else
	{
		responseStatus = NO_RESPONSE ;
	}

	return responseStatus ;
}
