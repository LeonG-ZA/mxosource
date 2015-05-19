#include "ControlServer.h"

#include "RequestHandler.h"



ControlServer::ControlServer ( HandleRequestRPC* NewRequestRPC , ProduceResponseRPC* NewResponseRPC )
{
	MyRequestRPC = NewRequestRPC ;
	
	RequestHandler* newRequestHandler = new RequestHandler ( NewResponseRPC ) ;
	
	MyRequestRPC->SetRequestHandler ( newRequestHandler ) ;
}



void ControlServer::ProcessRequests ( void )
{
	while ( MyRequestRPC->ProcessRequest () == HandleRequestRPC::SUCCESS ) ;
}
