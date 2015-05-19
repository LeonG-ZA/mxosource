#include "CombatHandler.h"

#include "CombatTactic.h"




CombatHandler::CombatHandler ( WorldModel* NewWorldModel , ProduceResponseRPC* NewResponseRPC )
{
	MyWorldModel = NewWorldModel ;
	MyResponseRPC = NewResponseRPC ;
}



void CombatHandler::ProcessCombat ( void )
{
	map < unsigned , CombatSession >::iterator currentCombatSession ;

	for ( currentCombatSession = CombatSessions.begin () ;
	      currentCombatSession != CombatSessions.end () ;
	      currentCombatSession++ )
	{
		if ( currentCombatSession->second.IsRealSession () == true )
		{
			if ( currentCombatSession->second.ProcessSession () == true )
			{
				map < unsigned , CombatSession >::iterator deleteSession = currentCombatSession ;
			
				MyWorldModel->FreeSessionID ( deleteSession->first ) ;
				currentCombatSession = CombatSessions.erase ( deleteSession ) ;

				if ( currentCombatSession == CombatSessions.end () )
				{
					break ;
				}
			}
		}
	}
}



void CombatHandler::HandleCloseCombat ( CharacterModel* AttackingCharacter , CharacterModel* TargetCharacter , unsigned TargetProtocol , unsigned Slot )
{
	startCombat ( AttackingCharacter , TargetCharacter , TargetProtocol , Slot ) ;
}



//void CombatHandler::HandleFreeCombat ( CharacterModel* AttackingCharacter , CharacterModel* TargetCharacter )
//{
//	startCombat ( AttackingCharacter , TargetCharacter ) ;
//}



CombatHandler::SetAbilityStatus_T CombatHandler::SetCombatAbility ( PlayerCharacterModel* ActivateCharacter , CombatAbility* TheAbility )
{
	SetAbilityStatus_T setStatus = NOT_IN_COMBAT ;

	CombatSession* activeSession = getCombatSession ( ActivateCharacter ) ;
	if ( activeSession != NULL )
	{
		CombatSession::SessionCharacter_T abilityCharacter ;
		if ( activeSession->GetCharacter ( CombatSession::FIRST_CHARACTER )->GetGOId () == ActivateCharacter->GetGOId () )
		{
			abilityCharacter = CombatSession::FIRST_CHARACTER ;
		}
		else
		{
			abilityCharacter = CombatSession::SECOND_CHARACTER ;
		}
		
		setStatus = SUCCESS ;
		
		activeSession->SetAbility ( abilityCharacter , TheAbility ) ;
	}
	return setStatus ;
}



unsigned CombatHandler::SpawnInterlock ( CharacterModel* AttackerCharacter , CharacterModel* DefenderCharacter )
{
	unsigned newSessionId = MyWorldModel->GetNewSessionId () ;
	
	CombatSessions.insert ( CombatSessions.begin () ,
	                        std::pair < unsigned , CombatSession >
	                        ( newSessionId ,
	                           CombatSession ( newSessionId , AttackerCharacter , DefenderCharacter ,
	                                           MyResponseRPC , false ) ) );

	map < unsigned , CombatSession >::iterator newSessionIterator = CombatSessions.find ( newSessionId ) ;

	newSessionIterator->second.SetState ( CombatSession::ENGAGED ) ;

	MyResponseRPC->SpawnInterlock ( AttackerCharacter , newSessionId ) ;
	MyResponseRPC->ViewInterlock ( AttackerCharacter , DefenderCharacter , newSessionId , newSessionIterator->second.GetInterlockIndex () , 0 ) ;

	return newSessionId ;
}



std::stringstream CombatHandler::RunCombatAnimation ( CharacterModel* TargetCharacter , std::string TargetAnimation )
{
	std::stringstream responseMessage ;

				bool inInterlock = false ;
				CombatSession* currentCombatSession = getCombatSession ( TargetCharacter ) ;

				if ( currentCombatSession != NULL )
				{
					inInterlock = true ;
				}
			
				if ( ( ( inInterlock == false ) && ( TargetAnimation.length () == 6 ) ) ||
				     ( ( inInterlock == true ) && ( TargetAnimation.length () == 4 ) ) )
				{
					unsigned animationInt ;
					std::stringstream ss ;

					ss << std::hex << TargetAnimation.c_str () ;
					ss >> animationInt ;

					if ( inInterlock == false )
					{
						byte animationCode [ RunAnimationCodeMsg::CODE_LENGTH ] ;
						animationCode[2] = animationInt & 0xFF ;
						animationCode[1] = animationInt >> 8 & 0xFF ;
						animationCode[0] = animationInt >> 16 & 0xFF ;
			
						MyResponseRPC->RunAnimationCode ( TargetCharacter , animationCode ) ;

						responseMessage << "Character " << TargetCharacter->GetGOId () << " performs animation " << TargetAnimation.c_str () ;
					}
					else
					{
						byte animationCode [ 2 ] ;
						bool firstAttack = false ;

						if ( TargetCharacter->GetGOId () == currentCombatSession->GetCharacter ( CombatSession::FIRST_CHARACTER )->GetGOId () )
						{
							firstAttack = true ;
							responseMessage << "Character " << TargetCharacter->GetGOId () << " performs interlock animation " << TargetAnimation << " against character " << currentCombatSession->GetCharacter ( CombatSession::SECOND_CHARACTER )->GetGOId () ;
						}
						else
						{
							responseMessage << "Character " << TargetCharacter->GetGOId () << " performs interlock animation " << TargetAnimation << " against character " << currentCombatSession->GetCharacter ( CombatSession::FIRST_CHARACTER )->GetGOId () ;
						}

						animationCode[1] = animationInt & 0xFF ;
						animationCode[0] = animationInt >> 8 & 0xFF ;

						MyResponseRPC->InterlockAnimation ( TargetCharacter , currentCombatSession->GetCharacter ( CombatSession::SECOND_CHARACTER ) , currentCombatSession->GetInterlockId () , currentCombatSession->GetInterlockIndex () , animationCode ) ;
						currentCombatSession->IncrementInterlockIndex () ;
					}
				}
				else
				{
					responseMessage << "Animation length wrong: " << TargetAnimation ;
				}
	return responseMessage ;
}



void CombatHandler::Withdraw ( CharacterModel* WithdrawCharacter )
{
	CombatSession* withdrawSession = getCombatSession ( WithdrawCharacter ) ;
	withdrawSession->Withdraw () ;

	MyWorldModel->FreeSessionID ( withdrawSession->GetInterlockId () ) ;
	CombatSessions.erase ( withdrawSession->GetInterlockId () ) ;
}

		
		
void CombatHandler::startCombat ( CharacterModel* AttackingCharacter ,
                                  CharacterModel* TargetCharacter , unsigned TargetProtocol , unsigned Slot )
{
	unsigned nextSessionId = MyWorldModel->GetNewSessionId () ;
	
	CombatSessions.insert ( std::pair < unsigned , CombatSession >
	                        ( nextSessionId ,
	                          CombatSession ( nextSessionId , AttackingCharacter , TargetCharacter ,
	                                          MyResponseRPC ) ) );
}



CombatSession* CombatHandler::getCombatSession ( CharacterModel* CombatCharacter )
{
	CombatSession* foundSession = NULL ;

	for ( map < unsigned , CombatSession >::iterator sessionIterator = CombatSessions.begin () ;
	      ( sessionIterator != CombatSessions.end () ) && ( foundSession == NULL ) ;
	      sessionIterator++ )
	{
		if ( ( sessionIterator->second.GetCharacter ( CombatSession::FIRST_CHARACTER )->GetGOId () == CombatCharacter->GetGOId () ) ||
		     ( sessionIterator->second.GetCharacter ( CombatSession::SECOND_CHARACTER )->GetGOId () == CombatCharacter->GetGOId () ) )
		{
			foundSession = &( sessionIterator->second ) ;
		}
	}

	return foundSession ;
}



CombatSession* CombatHandler::getFirstCombatSession ( void )
{
	CombatSession* foundSession = NULL ;
	
	if ( CombatSessions.begin () != CombatSessions.end () )
	{
		foundSession = & ( CombatSessions.begin ()->second ) ;
	}
	
	return foundSession ;
}



CombatSession* CombatHandler::getNextCombatSession ( unsigned PreviousSessionId )
{
	CombatSession* foundSession = NULL ;

	map < unsigned, CombatSession >::iterator sessionIterator = CombatSessions.find ( PreviousSessionId ) ;

	if ( sessionIterator != CombatSessions.end () )
	{
		sessionIterator++ ;

		if ( sessionIterator != CombatSessions.end () )
		{
			foundSession = & ( CombatSessions.begin ()->second ) ;
		}
	}

	return foundSession ;
}
