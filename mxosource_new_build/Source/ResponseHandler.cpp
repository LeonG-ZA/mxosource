#include "GameServer.h"
#include "ResponseHandler.h"



ResponseHandler::ResponseHandler ()
{

}



void ResponseHandler::BroadcastPosition ( CharacterModel* TheCharacter , bool ExcludeSource )
{
	broadcastState ( TheCharacter , make_shared< PositionStateMsg >( TheCharacter ) , ExcludeSource ) ;
}



void ResponseHandler::BroadcastWorldChat ( CharacterModel* TheCharacter , std::string TheMessage )
{
	broadcastCommand ( TheCharacter , make_shared< WorldChatMsg >( TheCharacter , TheMessage ) , true ) ;
}



void ResponseHandler::broadcastState ( CharacterModel* TheCharacter , msgBaseClassPtr theMsg , bool ExcludeSource )
{
	GameClient* theClient ;
	
	if ( ExcludeSource == true )
	{
		theClient = *( sGame.GetClientsWithCharacterId ( TheCharacter->GetCharacterID () ).data () ) ;
	}
	else
	{
		theClient = NULL ;
	}

	sGame.AnnounceStateUpdate ( theClient , theMsg ) ;
}



void ResponseHandler::broadcastCommand ( CharacterModel* TheCharacter , msgBaseClassPtr theMsg , bool ExcludeSource )
{
	GameClient* theClient ;
	
	if ( ExcludeSource == true )
	{
		theClient = *( sGame.GetClientsWithCharacterId ( TheCharacter->GetCharacterID () ).data () ) ;
	}
	else
	{
		theClient = NULL ;
	}
	ByteBuffer testBuf = theMsg->toBuf();
	sGame.AnnounceCommand ( theClient , theMsg ) ;
}