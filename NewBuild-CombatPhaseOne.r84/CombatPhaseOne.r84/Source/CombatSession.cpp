#include "CombatSession.h"

#include "Log.h"


CombatSession::CombatSession ( unsigned NewInterlockId , CharacterModel* AttackingCharacter , CharacterModel* DefendingCharacter , ProduceResponseRPC* NewResponseRPC , bool NewSessionReal )
{
	InterlockId = NewInterlockId ;
	FirstCombatant.CombatantCharacter = AttackingCharacter ;
	FirstCombatant.AttackTimer = NULL ;
	FirstCombatant.ActiveAbility = NULL ;
	SecondCombatant.CombatantCharacter = DefendingCharacter ;
	SecondCombatant.AttackTimer = NULL ;
	SecondCombatant.ActiveAbility = NULL ;

	CombatState = ENGAGING ;
	
//	TargetProtocol = DefendingProtocol ;
//	TheSlot = Slot ;

	InterlockIndex = 1 ;

	MyResponseRPC = NewResponseRPC ;

	RealSession = NewSessionReal ;
}



/*unsigned CombatSession::GetInterlockId ( void )
{
	return InterlockId ;
}*/



void CombatSession::StartTimers ( void )
{
	FirstCombatant.AttackTimer = new Timer ( COMBAT_ROUND_TIME * 2 , Timer::REPEATING , false , -COMBAT_ROUND_TIME ) ;
	SecondCombatant.AttackTimer = new Timer ( COMBAT_ROUND_TIME * 2 , Timer::REPEATING , false ) ;
}


	
bool CombatSession::ProcessSession ( void )
{
	bool combatComplete = false ;

	if ( CombatState == CombatSession::ENGAGING )
	{

			if ( FirstCombatant.CombatantCharacter->GetMovementState () == CharacterModel::IDLE )
			{
				double range = FirstCombatant.CombatantCharacter->GetPosition ()->Distance ( SecondCombatant.CombatantCharacter->GetPosition () ) ;
				if ( range <= 400 )
				{
					StartTimers () ;
					SetState ( CombatSession::ENGAGED ) ;
					MyResponseRPC->SpawnInterlock ( FirstCombatant.CombatantCharacter , InterlockId ) ;
					MyResponseRPC->ViewInterlock ( FirstCombatant.CombatantCharacter , SecondCombatant.CombatantCharacter , InterlockId , InterlockIndex , 0 ) ;
					InterlockIndex++ ;
					StartTimers () ;
				}
			}
		}
		else
		{
			combatComplete = false ;

			for ( unsigned characterIndex = 0 ; ( characterIndex < 2 ) && ( combatComplete == false ) ; characterIndex++ )
			{
	if ( FirstCombatant.AttackTimer->TimedOut () )
	{
		combatComplete |= performAttack ( FIRST_CHARACTER ) ;
	}
	if ( SecondCombatant.AttackTimer->TimedOut () )
	{
		combatComplete |= performAttack ( SECOND_CHARACTER ) ;
	}
			}
		}

	if ( combatComplete == true )
	{
		Withdraw () ;
	}

	return combatComplete ;
}

	
	
void CombatSession::SetAbility ( SessionCharacter_T AbilityCharacter , CombatAbility* NewAbility )
{
	if ( AbilityCharacter == FIRST_CHARACTER )
	{
		FirstCombatant.ActiveAbility = NewAbility ;
	}
	else
	{
		SecondCombatant.ActiveAbility = NewAbility ;
	}
}



CharacterModel* CombatSession::GetCharacter ( SessionCharacter_T AbilityCharacter )
{
	if ( AbilityCharacter == FIRST_CHARACTER )
	{
		return FirstCombatant.CombatantCharacter ;
	}
	else
	{
		return SecondCombatant.CombatantCharacter ;
	}
}



/*CombatTactic* CombatSession::GetTactic ( SessionCharacter_T AbilityCharacter )
{
	if ( AbilityCharacter == FIRST_CHARACTER )
	{
		return FirstCombatant.CombatantCharacter->GetTactic () ;
	}
	else
	{
		return SecondCombatant.CombatantCharacter->GetTactic () ;
	}
}*/



/*CombatAbility* CombatSession::GetAbility ( SessionCharacter_T AbilityCharacter )
{
	if ( AbilityCharacter == FIRST_CHARACTER )
	{
		return FirstCombatant.ActiveAbility ;
	}
	else
	{
		return SecondCombatant.ActiveAbility ;
	}
}*/



void CombatSession::IncrementInterlockIndex ( void )
{
	InterlockIndex++ ;
}



unsigned CombatSession::GetInterlockIndex ( void )
{
	return InterlockIndex ;
}



unsigned CombatSession::GetInterlockId ( void )
{
	return InterlockId ;
}



void CombatSession::SetState ( CombatState_T NewState )
{
	CombatState = NewState ;
}



CombatSession::CombatState_T CombatSession::GetState ( void )
{
	return CombatState ;
}



/*bool CombatSession::AlignToAttack ( bool FirstAttack )
{
//	return CombatSession->AttackerMover->AlignToTarget () ;
	return true ;
}*/



bool CombatSession::performAttack ( CombatSession::SessionCharacter_T AttackingCharacter )
{
	bool combatComplete = false ;

	CombatantSession_T* attackingCombatant ;
	CombatantSession_T* defendingCombatant ;

	if ( AttackingCharacter == CombatSession::FIRST_CHARACTER )
	{
		attackingCombatant = &FirstCombatant ;
		defendingCombatant = &SecondCombatant ;
	}
	else
	{
		attackingCombatant = &SecondCombatant ;
		defendingCombatant = &FirstCombatant ;
	}

		// Update interlock needs to be dependent on hit or miss...
	unsigned attackAnimationIndex ;
	string attackName ;
	
	CharacterModel::DebuffState_T* attackDebuff = NULL ;

	if ( attackingCombatant->ActiveAbility != NULL )
	{
		attackAnimationIndex = attackingCombatant->ActiveAbility->GetAnimationIndex () ;
		attackName = attackingCombatant->ActiveAbility->GetName () ;
		
		attackingCombatant->ActiveAbility->GetDebuff ( attackDebuff ) ;
	}
	else
	{
		attackAnimationIndex = attackingCombatant->CombatantCharacter->GetTactic ()->GetAnimationIndex () ;
		attackName = attackingCombatant->CombatantCharacter->GetTactic ()->GetName () ;
	}
	
	MyResponseRPC->ViewInterlock ( attackingCombatant->CombatantCharacter , defendingCombatant->CombatantCharacter , InterlockId , InterlockIndex , attackAnimationIndex ) ;
	InterlockIndex++ ;
   
	if ( attackSuccess ( AttackingCharacter ) == true )
	{
		unsigned damage = attackDamage ( AttackingCharacter ) ;
   
		string combatResponse = "{c:ff0000}Hit target with " + attackName + " for "+ (format("%1%") % damage ).str() +" damage.{/c}" ;
		MyResponseRPC->SystemMessage ( attackingCombatant->CombatantCharacter , combatResponse ) ;
   
		combatResponse = "{c:ff0000}Hit for "+ (format("%1%") % damage ).str() +" damage.{/c}" ;
		MyResponseRPC->SystemMessage ( defendingCombatant->CombatantCharacter , combatResponse ) ;
   
		defendingCombatant->CombatantCharacter->TakeDamage ( damage ) ;
		MyResponseRPC->HealthUpdate ( defendingCombatant->CombatantCharacter ) ;
   INFO_LOG ( format("Damage:%1%") % damage ) ;
//		MyResponseRPC->RunAnimation ( CurrentCombatSession->TargetCharacter , MyActivatableActivity.GetTactic ( CurrentCombatSession->AttackingCharacter->GetTactic () )->GetBlockAnimationIndex () ) ;

		if ( attackDebuff != NULL )
		{
			handleDebuff ( attackingCombatant->CombatantCharacter , defendingCombatant->CombatantCharacter , attackDebuff ) ;
		}
   
		if ( defendingCombatant->CombatantCharacter->GetCurrentHealth () <= 0 )
		{
			defendingCombatant->CombatantCharacter->SetMood ( CharacterModel::DEAD ) ;
			MyResponseRPC->MoodUpdate ( defendingCombatant->CombatantCharacter ) ;
   
			combatComplete = true ;
		}
	}
	else
	{
		string combatResponse = "{c:ff0000}Missed target!{/c}" ;
		MyResponseRPC->SystemMessage ( attackingCombatant->CombatantCharacter , combatResponse ) ;
   
		combatResponse = "{c:ff0000}Attack missed you!.{/c}" ;
		MyResponseRPC->SystemMessage ( defendingCombatant->CombatantCharacter , combatResponse ) ;
   
//	MyResponseRPC->RunAnimation ( defendingCombatant->CombatantCharacter , MyActivatableActivity.GetTactic ( attackingCombatant->CombatantCharacter->GetTactic () )->GetMissAnimationIndex () ) ;
	}

	if ( attackingCombatant->ActiveAbility != NULL )
	{
		attackingCombatant->ActiveAbility = NULL ;
	}

	return combatComplete ;
}



bool CombatSession::attackSuccess ( SessionCharacter_T AttackingCharacter )
{
	bool success ;
	unsigned result = rand () % 100 ;
	if ( result >= 50 )
	{
		success = true ;
	}
	else
	{
		success = false ;
	}
	return success ;
}



unsigned CombatSession::attackDamage ( SessionCharacter_T AttackingCharacter )
{
	unsigned attackDamage ;
	CharacterModel* attackingCharacter ;
	
	if ( AttackingCharacter == FIRST_CHARACTER )
	{
		attackingCharacter = FirstCombatant.CombatantCharacter ;
	}
	else
	{
		attackingCharacter = SecondCombatant.CombatantCharacter ;
	}

	attackDamage = attackingCharacter->GetBaseAttackDamage () ;

	attackDamage = attackDamage * attackingCharacter->GetTactic ()->GetDamageBonus () ;

	return attackDamage ;
}



void CombatSession::handleDebuff ( CharacterModel* AttackingCharacter , CharacterModel* DefendingCharacter , CharacterModel::DebuffState_T* AttackDebuff )
{
	/*switch ( AttackDebuff )
	{
		case CombatAbility::DAZED :
			
		break ;
		
		case default :
			// How exactly did we get here?
		break ;
	}*/
	
	if ( *AttackDebuff != CharacterModel::DAZED )
	{
		//DefendingCharacter 
	}
}



void CombatSession::Withdraw ( void )
{
	MyResponseRPC->DespawnInterlock ( InterlockId ) ;
}



bool CombatSession::IsRealSession ( void )
{
	return RealSession ;
}