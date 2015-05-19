#include "ReceiveRequestRPC.h"



ReceiveRequestRPC::ReceiveRequestRPC ( queue < RequestRPCFunction* >* NewRequestList ) : RequestRPC ( NewRequestList )
{
}



void ReceiveRequestRPC::Jump ( PlayerCharacterModel* JumpCharacter , LocationVector JumpTarget )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	JumpRPC* theFunction = new JumpRPC ( JumpCharacter , JumpTarget ) ;

	RequestList->push ( ( RequestRPCFunction* )theFunction ) ;
}



void ReceiveRequestRPC::Chat ( PlayerCharacterModel* SourceCharacter , std::string ChatMessage )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	ChatRPC* theFunction = new ChatRPC ( SourceCharacter , ChatMessage ) ;

	RequestList->push ( ( RequestRPCFunction* )theFunction ) ;
}



void ReceiveRequestRPC::HandleCloseCombat ( PlayerCharacterModel* AttackingCharacter , CharacterModel* TargetCharacter , unsigned TargetProtocol , unsigned Slot )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	CloseCombatRPC* theFunction = new CloseCombatRPC ( AttackingCharacter , TargetCharacter, TargetProtocol , Slot ) ;

	RequestList->push ( ( RequestRPCFunction* )theFunction ) ;
}



void ReceiveRequestRPC::TacticUpdate ( PlayerCharacterModel* TacticCharacter , unsigned TacticIndex )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	UpdateTacticRPC* theFunction = new UpdateTacticRPC ( TacticCharacter , TacticIndex ) ;

	RequestList->push ( ( RequestRPCFunction* )theFunction ) ;
}



void ReceiveRequestRPC::Withdraw ( PlayerCharacterModel* WithdrawCharacter )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	WithdrawRPC* theFunction = new WithdrawRPC ( WithdrawCharacter ) ;

	RequestList->push ( ( RequestRPCFunction* )theFunction ) ;
}



void ReceiveRequestRPC::ActivateAbility ( PlayerCharacterModel* ActivateCharacter , unsigned AbilityIndex )
{
		//	Allocate memory to store the arguments in so they can be stored as
		//	a void pointer.  Must be freed after use.
	ActivateAbilityRPC* theFunction = new ActivateAbilityRPC ( ActivateCharacter , AbilityIndex ) ;

	RequestList->push ( ( RequestRPCFunction* )theFunction ) ;
}
