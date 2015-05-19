#include "ResponseHandler.h"

#include "GameClient.h"
#include "GameServer.h"
#include "PlayerCharacterModel.h"



ResponseHandler::ResponseHandler () : MyCombatAnimations ()
{

}



void ResponseHandler::BroadcastPosition ( CharacterModel* TheCharacter , CharacterModel* ExcludePlayer )
{
	broadcastState ( make_shared< PositionStateMsg >( TheCharacter ) , ExcludePlayer ) ;
}



void ResponseHandler::WorldChat ( CharacterModel* TheCharacter , CharacterModel* ExcludePlayer , std::string TheMessage )
{
	broadcastCommand ( make_shared< ChatMsg >( TheCharacter , TheMessage , ChatMsg::AREA ) , ExcludePlayer ) ;
}



void ResponseHandler::SystemMessage ( CharacterModel* ToCharacter , std::string TheMessage )
{
	sendCommand ( make_shared< ChatMsg >( ( CharacterModel* )NULL , TheMessage , ChatMsg::SYSTEM ) , ToCharacter ) ;
}



void ResponseHandler::SpawnCharacter ( PlayerCharacterModel* TheCharacter )
{
	broadcastState ( make_shared< PlayerSpawnMsg >( TheCharacter ) , NULL ) ;
}


void ResponseHandler::SpawnNPC ( NPCModel* TheCharacter )
{
	broadcastState ( make_shared< NPCSpawnMsg >( TheCharacter ) , NULL ) ;
}


void ResponseHandler::StateUpdate ( PlayerCharacterModel* TheCharacter , ByteBuffer StateData )
{
	broadcastState ( make_shared< StateUpdateMsg >( ( CharacterModel* )TheCharacter , StateData ) , NULL ) ;
}



void ResponseHandler::NPCStateUpdate ( NPCModel* TheCharacter )
{
	//broadcastState ( TheCharacter , make_shared< NPCMovementUpdateMsg >( TheCharacter ) ) ;
}



void ResponseHandler::NPCPosUpdate ( NPCModel* TheCharacter )
{
	//broadcastState ( TheCharacter , make_shared< NPCPosUpdateMsg >( TheCharacter ) ) ;
}



void ResponseHandler::StartRotatePlayer ( CharacterModel* RotatePlayeracter , StartPlayerRotateMsg::TurnDir_T TurnDirection )
{
	broadcastState ( make_shared< StartPlayerRotateMsg >( RotatePlayeracter , TurnDirection ) , NULL ) ;
}



void ResponseHandler::UpdateRotatePlayer ( CharacterModel* RotatePlayeracter )
{
	broadcastState ( make_shared< UpdatePlayerRotateMsg >( RotatePlayeracter ) , NULL ) ;
}



void ResponseHandler::StopRotatePlayer ( CharacterModel* RotatePlayeracter )
{
	broadcastState ( make_shared< StopPlayerRotateMsg >( RotatePlayeracter ) , NULL ) ;
}



void ResponseHandler::StartCharState ( CharacterModel* StateCharacter , StartPlayerStateMsg::State_T NewState )
{
	broadcastState ( make_shared< StartPlayerStateMsg >( StateCharacter , NewState ) , NULL ) ;
}



void ResponseHandler::UpdateCharState ( CharacterModel* StateCharacter )
{
	broadcastState ( make_shared< UpdatePlayerStateMsg >( StateCharacter ) , NULL ) ;
}



void ResponseHandler::StopCharState ( CharacterModel* StateCharacter )
{
	broadcastState ( make_shared< StopPlayerStateMsg >( StateCharacter ) , NULL ) ;
}



void ResponseHandler::UpdateNPCState ( CharacterModel* StateCharacter , UpdateNPCStateMsg::State_T NewState )
{
	broadcastState ( make_shared< UpdateNPCStateMsg >( StateCharacter , NewState ) , NULL ) ;
}



void ResponseHandler::ChangeNPCState ( CharacterModel* StateCharacter , ChangeNPCStateMsg::State_T NewState )
{
	broadcastState ( make_shared< ChangeNPCStateMsg >( StateCharacter , NewState ) , NULL ) ;
}



void ResponseHandler::RunAnimationCode ( CharacterModel* AnimateCharacter , byte* AnimationCode )
{
	broadcastState ( make_shared< RunAnimationCodeMsg >( AnimateCharacter , AnimationCode ) , NULL ) ;
}



void ResponseHandler::RunAnimation ( CharacterModel* AnimateCharacter , unsigned AnimationIndex )
{
	CombatAnimation* theAnimation = MyCombatAnimations.GetAnimation ( AnimationIndex ) ;

	if ( theAnimation != NULL )
	{
		broadcastState ( make_shared< RunAnimationCodeMsg >( AnimateCharacter , theAnimation->GetAnimationCode () ) , NULL ) ;
	}
}



void ResponseHandler::HealthUpdate ( CharacterModel* UpdateCharacter )
{
	broadcastState ( make_shared< HealthUpdateMsg >( UpdateCharacter ) , NULL ) ;
}



void ResponseHandler::MoodUpdate ( CharacterModel* UpdateCharacter )
{
	broadcastState ( make_shared< MoodUpdateMsg >( UpdateCharacter ) , NULL ) ;
}



void ResponseHandler::ViewInterlock ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , unsigned CombatAnimationIndex )
{
	CombatAnimation* theAnimation = MyCombatAnimations.GetAnimation ( CombatAnimationIndex ) ;

	if ( theAnimation != NULL )
	{
		broadcastState ( make_shared< ViewInterlockMsg >( FirstCharacter , SecondCharacter , InterlockID , InterlockIndex , theAnimation->GetAnimationCode () ) , NULL ) ;
	}
}



void ResponseHandler::SpawnInterlock ( CharacterModel* FirstCharacter , unsigned InterlockID )
{
	broadcastState ( make_shared< SpawnInterlockMsg >( FirstCharacter , InterlockID ) , NULL ) ;
}



void ResponseHandler::InterlockAnimation ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , byte AnimationCode[2] )
{
	broadcastState ( make_shared< ViewInterlockMsg >( FirstCharacter , SecondCharacter , InterlockID , InterlockIndex , ( byte* )AnimationCode ) , NULL ) ;
}



void ResponseHandler::DespawnInterlock ( unsigned RemoveInterlock )
{
	broadcastState ( make_shared< DespawnInterlockMsg >( RemoveInterlock ) , NULL ) ;
//	broadcastState ( make_shared< ResetInterlockMsg >( RemoveInterlock ) , NULL ) ;
}



void ResponseHandler::broadcastState ( msgBaseClassPtr TheMsg , CharacterModel* ExcludeCharacter )
{
	GameClient* theClient = NULL ;
	unsigned characterID = 0 ;

	if ( ExcludeCharacter != NULL )
	{
		if ( ExcludeCharacter->IsNPC () == false )
		{
			characterID = ( ( PlayerCharacterModel* ) ExcludeCharacter )->GetCharacterId () ;
		}
	}
	
	if ( characterID != 0 )
	{
		vector<GameClient*> gameClients = sGame.GetClientsWithCharacterId ( characterID ) ;
		if ( gameClients.empty () == false )
		{
			theClient = gameClients.front () ;
		}
	}

	sGame.AnnounceStateUpdate ( theClient , TheMsg ) ;
}



void ResponseHandler::broadcastCommand ( msgBaseClassPtr TheMsg , CharacterModel* ExcludeCharacter )
{
	GameClient* theClient = NULL ;
	unsigned characterID = 0 ;

	if ( ExcludeCharacter != NULL )
	{
		if ( ExcludeCharacter->IsNPC () == false )
		{
			characterID = ( ( PlayerCharacterModel* ) ExcludeCharacter )->GetCharacterId () ;
		}
	}
	
	if ( characterID != 0 )
	{
		vector<GameClient*> gameClients = sGame.GetClientsWithCharacterId ( characterID ) ;
		if ( gameClients.empty () == false )
		{
			theClient = gameClients.front () ;
		}
	}

	sGame.AnnounceCommand ( theClient , TheMsg ) ;
}



void ResponseHandler::sendCommand ( msgBaseClassPtr TheMsg , CharacterModel* ToCharacter )
{
	unsigned characterID = 0 ;

	if ( ToCharacter != NULL )
	{
		if ( ToCharacter->IsNPC () == false )
		{	
			characterID = ( ( PlayerCharacterModel* ) ToCharacter )->GetCharacterId () ;
		}
	}

	if ( characterID != 0 )
	{
		vector<GameClient*> gameClients = sGame.GetClientsWithCharacterId ( characterID ) ;
		if ( gameClients.empty () == false )
		{
			gameClients.front ()->QueueCommand ( TheMsg ) ;
		}
	}
}
