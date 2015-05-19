#include "ProduceResponseRPC.h"



ProduceResponseRPC::ProduceResponseRPC ( queue < ResponseRPCFunction* >* NewResponseList ) : ResponseRPC ( NewResponseList )
{
}



void ProduceResponseRPC::BroadcastPosition ( CharacterModel* ResponseSource , CharacterModel* ExcludePlayer )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	BroadcastPositionRPC* theFunction = new BroadcastPositionRPC ( ResponseSource , ExcludePlayer ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::WorldChat ( CharacterModel* MessageSource , CharacterModel* ExcludePlayer , std::string TheMessage )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	WorldChatRPC* theFunction = new WorldChatRPC ( MessageSource , ExcludePlayer , TheMessage ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}


		
void ProduceResponseRPC::SystemMessage ( CharacterModel* ToCharacter , std::string TheMessage )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	SystemMessageRPC* theFunction = new SystemMessageRPC ( ToCharacter , TheMessage ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::SpawnCharacter ( PlayerCharacterModel* TheCharacter )
{
	SpawnCharacterRPC* theFunction = new SpawnCharacterRPC ( TheCharacter ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}


void ProduceResponseRPC::SpawnNPC ( NPCModel* TheCharacter )
{
	SpawnNPCRPC* theFunction = new SpawnNPCRPC ( TheCharacter ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::StartRotatePlayer ( CharacterModel* RotatePlayeracter , StartPlayerRotateMsg::TurnDir_T TurnDirection )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	StartRotatePlayerRPC* theFunction = new StartRotatePlayerRPC ( RotatePlayeracter , TurnDirection ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::UpdateRotatePlayer ( CharacterModel* RotatePlayeracter )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	UpdateRotatePlayerRPC* theFunction = new UpdateRotatePlayerRPC ( RotatePlayeracter ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::StopRotatePlayer ( CharacterModel* RotatePlayeracter )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	StopRotatePlayerRPC* theFunction = new StopRotatePlayerRPC ( RotatePlayeracter ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::StartCharState ( CharacterModel* StateCharacter , StartPlayerStateMsg::State_T NewState )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	StartCharStateRPC* theFunction = new StartCharStateRPC ( StateCharacter , NewState ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::UpdateCharState ( CharacterModel* StateCharacter )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	UpdateCharStateRPC* theFunction = new UpdateCharStateRPC ( StateCharacter ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::StopCharState ( CharacterModel* StateCharacter )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	StopCharStateRPC* theFunction = new StopCharStateRPC ( StateCharacter ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::UpdateNPCState ( CharacterModel* StateCharacter , UpdateNPCStateMsg::State_T NewState )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	UpdateNPCStateRPC* theFunction = new UpdateNPCStateRPC ( StateCharacter , NewState ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::ChangeNPCState ( CharacterModel* StateCharacter , ChangeNPCStateMsg::State_T NewState )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	ChangeNPCStateRPC* theFunction = new ChangeNPCStateRPC ( StateCharacter , NewState ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::RunAnimationCode ( CharacterModel* AnimateCharacter , byte AnimationCode[3] )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	RunAnimationCodeRPC* theFunction = new RunAnimationCodeRPC ( AnimateCharacter , AnimationCode ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::RunAnimation ( CharacterModel* AnimateCharacter , unsigned AnimationIndex )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	RunAnimationRPC* theFunction = new RunAnimationRPC ( AnimateCharacter , AnimationIndex ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::HealthUpdate ( CharacterModel* UpdateCharacter )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	HealthUpdateRPC* theFunction = new HealthUpdateRPC ( UpdateCharacter ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::MoodUpdate ( CharacterModel* UpdateCharacter )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	MoodUpdateRPC* theFunction = new MoodUpdateRPC ( UpdateCharacter ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::SpawnInterlock ( CharacterModel* FirstCharacter , unsigned InterlockId )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	SpawnInterlockRPC* theFunction = new SpawnInterlockRPC ( FirstCharacter , InterlockId ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::ViewInterlock ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , unsigned CombatAnimationIndex )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	ViewInterlockRPC* theFunction = new ViewInterlockRPC ( FirstCharacter , SecondCharacter , InterlockID , InterlockIndex , CombatAnimationIndex ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::InterlockAnimation ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , byte AnimationCode[2] )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	InterlockAnimationRPC* theFunction = new InterlockAnimationRPC ( FirstCharacter , SecondCharacter , InterlockID , InterlockIndex , AnimationCode ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}



void ProduceResponseRPC::DespawnInterlock ( unsigned RemoveInterlock )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	DespawnInterlockRPC* theFunction = new DespawnInterlockRPC ( RemoveInterlock ) ;

	ResponseList->push ( ( ResponseRPCFunction* )theFunction ) ;
}