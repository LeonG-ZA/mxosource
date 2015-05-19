#include "RequestRPCFunctions.h"


#include "RequestHandler.h"



RequestRPCFunction::RequestRPCFunction ( void )
{
}



JumpRPC::JumpRPC ( CharacterModel* NewCharacter , LocationVector NewCoords )
{
	JumpCharacter = NewCharacter ;
	TargetCoords = NewCoords ;
}



void JumpRPC::ProcessRPC ( RequestHandler* TheRequestHandler )
{
	TheRequestHandler->Jump ( JumpCharacter , &TargetCoords ) ;
}



ChatRPC::ChatRPC ( CharacterModel* MessageCharacter , std::string NewMsg )
{
	SourceCharacter = MessageCharacter ;
	TheMsg = NewMsg ;
}



void ChatRPC::ProcessRPC ( RequestHandler* TheRequestHandler )
{
	TheRequestHandler->Chat ( SourceCharacter , TheMsg ) ;
}
