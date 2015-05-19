#include "RequestHandler.h"



RequestHandler::RequestHandler ( ProduceResponseRPC* NewResponseProducer )
{
	MyResponseProducer = NewResponseProducer ;
}



void RequestHandler::Jump ( CharacterModel* JumpCharacter , LocationVector* TargetLocation )
{
	JumpCharacter->SetPosition ( TargetLocation ) ;

		//	Immediately send this update to all clients rather than waiting for
		//	the periodic update.
	MyResponseProducer->BroadcastPosition ( JumpCharacter , false ) ;
}



void RequestHandler::Chat ( CharacterModel* MessageSource , std::string TheMessage )
{
		//	Only world chat is currently being used, other chat ranges will need to be added later.
	MyResponseProducer->BroadcastWorldChat ( MessageSource , TheMessage ) ;
}
