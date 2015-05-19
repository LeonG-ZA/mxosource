#include "HandleRequestRPC.h"



HandleRequestRPC::HandleRequestRPC ( queue < RequestRPCFunction* >* NewRequestList ) : RequestRPC ( NewRequestList )
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

		// If there are messages in the queue process the first one.
	if ( RequestList->empty () == false )
	{
		RequestRPCFunction* firstFunction = RequestList->front () ;

		firstFunction->ProcessRPC ( MyRequestHandler ) ;

			// Free the request memory.
		delete ( firstFunction ) ;

		RequestList->pop () ;
	}
		// No messages left, indicate this to the caller.
	else
	{
		RequestStatus = NO_REQUEST ;
	}

	return RequestStatus ;
}
