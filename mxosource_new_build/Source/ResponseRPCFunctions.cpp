#include "ResponseRPCFunctions.h"


#include "ResponseHandler.h"



ResponseRPCFunction::ResponseRPCFunction ( void )
{
}



BroadcastPositionRPC::BroadcastPositionRPC ( CharacterModel* NewCharacter , bool ExcludeSource )
{
	TheCharacter = NewCharacter ;
	Exclude = ExcludeSource ;
}



void BroadcastPositionRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->BroadcastPosition ( TheCharacter , Exclude ) ;
}



BroadcastWorldChatRPC::BroadcastWorldChatRPC ( CharacterModel* MessageCharacter , std::string NewMsg )
{
	SourceCharacter = MessageCharacter ;
	TheMsg = NewMsg ;
}



void BroadcastWorldChatRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->BroadcastWorldChat ( SourceCharacter , TheMsg ) ;
}
