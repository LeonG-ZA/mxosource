#include "RequestRPCFunctions.h"


#include "RequestHandler.h"



RequestRPCFunction::RequestRPCFunction ( void )
{
}



JumpRPC::JumpRPC ( PlayerCharacterModel* NewCharacter , LocationVector NewCoords )
{
	JumpCharacter = NewCharacter ;
	TargetCoords = NewCoords ;
}



void JumpRPC::ProcessRPC ( RequestHandler* TheRequestHandler )
{
	TheRequestHandler->Jump ( JumpCharacter , &TargetCoords ) ;
}



ChatRPC::ChatRPC ( PlayerCharacterModel* MessageCharacter , std::string NewMsg )
{
	SourceCharacter = MessageCharacter ;
	TheMsg = NewMsg ;
}



void ChatRPC::ProcessRPC ( RequestHandler* TheRequestHandler )
{
	TheRequestHandler->Chat ( SourceCharacter , TheMsg ) ;
}



CloseCombatRPC::CloseCombatRPC ( PlayerCharacterModel* AttackingCharacter , CharacterModel* TargetCharacter , unsigned TargetProtocol , unsigned Slot )
{
	TheCharacter = AttackingCharacter ;
	OtherCharacter = TargetCharacter ;
	Protocol = TargetProtocol ;
	TheSlot = Slot ;
}



void CloseCombatRPC::ProcessRPC ( RequestHandler* TheRequestHandler )
{
	TheRequestHandler->HandleCloseCombat ( TheCharacter , OtherCharacter , Protocol , TheSlot ) ;
}



UpdateTacticRPC::UpdateTacticRPC ( PlayerCharacterModel* TacticCharacter , unsigned TacticIndex )
{
	TheCharacter = TacticCharacter ;
	TheTactic = TacticIndex ;
}



void UpdateTacticRPC::ProcessRPC ( RequestHandler* TheRequestHandler )
{
	TheRequestHandler->TacticUpdate ( TheCharacter , TheTactic ) ;
}



WithdrawRPC::WithdrawRPC ( PlayerCharacterModel* WithdrawCharacter )
{
	TheCharacter = WithdrawCharacter ;
}



void WithdrawRPC::ProcessRPC ( RequestHandler* TheRequestHandler )
{
	TheRequestHandler->Withdraw ( TheCharacter ) ;
}



ActivateAbilityRPC::ActivateAbilityRPC ( PlayerCharacterModel* AbilityCharacter , unsigned AbilityIndex )
{
	TheCharacter = AbilityCharacter ;
	TheIndex = AbilityIndex ;
}



void ActivateAbilityRPC::ProcessRPC ( RequestHandler* TheRequestHandler )
{
	TheRequestHandler->ActivateAbility ( TheCharacter , TheIndex ) ;
}


