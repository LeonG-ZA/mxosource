#include "ResponseRPCFunctions.h"



ResponseRPCFunction::ResponseRPCFunction ( void )
{
}



BroadcastPositionRPC::BroadcastPositionRPC ( CharacterModel* NewCharacter , CharacterModel* ExcludePlayer )
{
	TheCharacter = NewCharacter ;
	ExcludedPlayer = ExcludePlayer ;
}



void BroadcastPositionRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->BroadcastPosition ( TheCharacter , ExcludedPlayer ) ;
}



WorldChatRPC::WorldChatRPC ( CharacterModel* MessageCharacter , CharacterModel* ExcludePlayer , std::string NewMsg )
{
	SourceCharacter = MessageCharacter ;
	ExcludedPlayer = ExcludePlayer ;
	TheMsg = NewMsg ;
}



void WorldChatRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->WorldChat ( SourceCharacter , ExcludedPlayer , TheMsg ) ;
}



SystemMessageRPC::SystemMessageRPC ( CharacterModel* ToCharacter , std::string NewMsg )
{
	TheCharacter = ToCharacter ;
	TheMsg = NewMsg ;
}



void SystemMessageRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->SystemMessage ( TheCharacter , TheMsg ) ;
}



StartRotatePlayerRPC::StartRotatePlayerRPC ( CharacterModel* RotatePlayeracter , StartPlayerRotateMsg::TurnDir_T TurnDirection )
{
	TheCharacter = RotatePlayeracter ;
	TheDirection = TurnDirection ;
}



void StartRotatePlayerRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->StartRotatePlayer ( TheCharacter , TheDirection ) ;
}



UpdateRotatePlayerRPC::UpdateRotatePlayerRPC ( CharacterModel* RotatePlayeracter )
{
	TheCharacter = RotatePlayeracter ;
}



void UpdateRotatePlayerRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->UpdateRotatePlayer ( TheCharacter ) ;
}



SpawnCharacterRPC::SpawnCharacterRPC ( PlayerCharacterModel* SpawnCharacter )
{
	TheCharacter = SpawnCharacter ;
}



void SpawnCharacterRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->SpawnCharacter ( TheCharacter ) ;
}



SpawnNPCRPC::SpawnNPCRPC ( NPCModel* SpawnCharacter )
{
	TheCharacter = SpawnCharacter ;
}



void SpawnNPCRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->SpawnNPC ( TheCharacter ) ;
}



StopRotatePlayerRPC::StopRotatePlayerRPC ( CharacterModel* RotatePlayeracter )
{
	TheCharacter = RotatePlayeracter ;
}



void StopRotatePlayerRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->StopRotatePlayer ( TheCharacter ) ;
}



StartCharStateRPC::StartCharStateRPC ( CharacterModel* StateCharacter , StartPlayerStateMsg::State_T NewState )
{
	TheCharacter = StateCharacter ;
	TheState = NewState ;
}



void StartCharStateRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->StartCharState ( TheCharacter , TheState ) ;
}



UpdateCharStateRPC::UpdateCharStateRPC ( CharacterModel* StateCharacter )
{
	TheCharacter = StateCharacter ;
}



void UpdateCharStateRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->UpdateCharState ( TheCharacter ) ;
}



StopCharStateRPC::StopCharStateRPC ( CharacterModel* StateCharacter )
{
	TheCharacter = StateCharacter ;
}



void StopCharStateRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->StopCharState ( TheCharacter ) ;
}



UpdateNPCStateRPC::UpdateNPCStateRPC ( CharacterModel* StateCharacter , UpdateNPCStateMsg::State_T NewState )
{
	TheCharacter = StateCharacter ;
	TheState = NewState ;
}



void UpdateNPCStateRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->UpdateNPCState ( TheCharacter , TheState ) ;
}



ChangeNPCStateRPC::ChangeNPCStateRPC ( CharacterModel* StateCharacter , ChangeNPCStateMsg::State_T NewState )
{
	TheCharacter = StateCharacter ;
	TheState = NewState ;
}



void ChangeNPCStateRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->ChangeNPCState ( TheCharacter , TheState ) ;
}



RunAnimationCodeRPC::RunAnimationCodeRPC ( CharacterModel* AnimateCharacter , byte AnimationCode[3] )
{
	TheCharacter = AnimateCharacter ;
	memcpy ( TheAnimation , AnimationCode , 3 ) ;
}



void RunAnimationCodeRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->RunAnimationCode ( TheCharacter , TheAnimation ) ;
}



RunAnimationRPC::RunAnimationRPC ( CharacterModel* AnimateCharacter , unsigned AnimationIndex )
{
	TheCharacter = AnimateCharacter ;
	TheAnimationIndex = AnimationIndex ;
}



void RunAnimationRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->RunAnimation ( TheCharacter , TheAnimationIndex ) ;
}



HealthUpdateRPC::HealthUpdateRPC ( CharacterModel* UpdateCharacter )
{
	TheCharacter = UpdateCharacter ;
}



void HealthUpdateRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->HealthUpdate ( TheCharacter ) ;
}



MoodUpdateRPC::MoodUpdateRPC ( CharacterModel* UpdateCharacter )
{
	TheCharacter = UpdateCharacter ;
}



void MoodUpdateRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->MoodUpdate ( TheCharacter ) ;
}



SpawnInterlockRPC::SpawnInterlockRPC ( CharacterModel* FirstCharacter , unsigned InterlockId )
{
	First = FirstCharacter ;
	TheInterlockId = InterlockId ;
}



void SpawnInterlockRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->SpawnInterlock ( First , TheInterlockId ) ;
}



ViewInterlockRPC::ViewInterlockRPC ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , unsigned CombatAnimationIndex )
{
	First = FirstCharacter ;
	Second = SecondCharacter ;
	TheInterlockID = InterlockID ;
	TheInterlockIndex = InterlockIndex ;
	TheAnimationIndex = CombatAnimationIndex ;
}



void ViewInterlockRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->ViewInterlock ( First , Second , TheInterlockID , TheInterlockIndex , TheAnimationIndex ) ;
}



InterlockAnimationRPC::InterlockAnimationRPC ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , byte AnimationCode[2] )
{
	First = FirstCharacter ;
	Second = SecondCharacter ;
	TheInterlockID = InterlockID ;
	TheInterlockIndex = InterlockIndex ;
	TheAnimationCode[0] = AnimationCode[0] ;
	TheAnimationCode[1] = AnimationCode[1] ;
}



void InterlockAnimationRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->InterlockAnimation ( First , Second , TheInterlockID , TheInterlockIndex , TheAnimationCode ) ;
}



DespawnInterlockRPC::DespawnInterlockRPC ( unsigned RemoveInterlock )
{
	TheInterlock = RemoveInterlock ;
}



void DespawnInterlockRPC::ProcessRPC ( ResponseHandler* TheResponseHandler )
{
	TheResponseHandler->DespawnInterlock ( TheInterlock ) ;
}



