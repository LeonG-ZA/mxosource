#include "CharacterModel.h"
#include "Common.h"



CharacterModel::CharacterModel ( unsigned NewGOId ) : GOId ( NewGOId )
{
//	ActiveTactic = SPEED ;
	TargetCharacter = NULL ;
	AnimationIndex = 0 ;
	MyMovementState = IDLE ;
	MyMood = MOOD1 ;
	ActiveTactic = NULL ;
	
	MyDebuffState = NONE ;
	DebuffTimer = NULL ;
}



string CharacterModel::GetHandle ()
{
	return Handle ;
}



string CharacterModel::GetBackground ()
{
	return Background ;
}



LocationVector* CharacterModel::GetPosition ()
{
	return Position ;
}



void CharacterModel::Jump ( LocationVector* JumpTarget )
{
	this->SetPosition ( JumpTarget ) ;
}

	
	
void CharacterModel::SetHeading ( double NewHeading )
{
	Position->rot = NewHeading ;

	positionUpdated () ;
}



void CharacterModel::setHeading ( uint8 NewHeading )
{
	Position->setMxoRot ( NewHeading ) ;

	positionUpdated () ;
}
		
		

RsiData* CharacterModel::GetRSI ()
{
	return RSI ;
}



unsigned CharacterModel::GetCurrentHealth ()
{
	return CurrentHealth ;
}



unsigned CharacterModel::GetMaxHealth ()
{
	return MaxHealth ;
}



unsigned CharacterModel::GetCurrentIS ()
{
	return CurrentIS ;
}



unsigned CharacterModel::GetMaxIS ()
{
	return MaxIS ;
}



unsigned CharacterModel::GetLevel ()
{
	return Level ;
}



unsigned CharacterModel::GetAlignment ()
{
	return Alignment ;
}



unsigned CharacterModel::GetGOId ()
{
	return GOId ;
}



unsigned CharacterModel::GetDistrict ()
{
	return District ;
}



unsigned CharacterModel::GetCash ()
{
	return Cash ;
}



void CharacterModel::SetTarget ( CharacterModel* NewTarget )
{
	TargetCharacter = NewTarget ;
}



CharacterModel* CharacterModel::GetTarget ( void )
{
	return TargetCharacter ;
}



bool CharacterModel::IsNPC ( void )
{
	return NPC ;
}



bool CharacterModel::PropagateUpdates ( void )
{
	bool propagateUpdates = TransmitUpdates ;

	if ( TransmitUpdates == true )
	{
		TransmitUpdates = false ;
	}

	return propagateUpdates ;
}



void CharacterModel::SetPosition ( LocationVector* NewPosition )
{
	Position->x = NewPosition->x ;
	Position->y = NewPosition->y ;
	Position->z = NewPosition->z ;
	Position->rot = NewPosition->rot ;

	positionUpdated () ;
}



void CharacterModel::SetPositionX ( double NewPosX )
{
	Position->x = NewPosX ;

	positionUpdated () ;
}



void CharacterModel::SetPositionY ( double NewPosY )
{
	Position->y = NewPosY ;

	positionUpdated () ;
}



void CharacterModel::SetPositionZ ( double NewPosZ )
{
	Position->z = NewPosZ ;

	positionUpdated () ;
}



void CharacterModel::IncrementAnimationIndex ( void )
{
	AnimationIndex++ ;
}



unsigned CharacterModel::GetAnimationIndex ( void )
{
	return AnimationIndex ;
}



void CharacterModel::SetMovementState ( MovementState_T NewState )
{
	MyMovementState = NewState ;
}



CharacterModel::MovementState_T CharacterModel::GetMovementState ( void )
{
	return MyMovementState ;
}



void CharacterModel::TakeDamage ( unsigned Damage )
{
	if ( CurrentHealth > Damage )
	{
		CurrentHealth -= Damage ;
	}
	else
	{
		CurrentHealth = 0 ;
	}
}



void CharacterModel::SetMood ( Mood_T NewMood )
{
	MyMood = NewMood ;
}



CharacterModel::Mood_T CharacterModel::GetMood ( void )
{
	return MyMood ;
}



void CharacterModel::SetTactic ( CombatTactic* NewTactic )
{
	ActiveTactic = NewTactic ;
}



CombatTactic* CharacterModel::GetTactic ( void )
{
	return ActiveTactic ;
}



void CharacterModel::positionUpdated ( void )
{
	TransmitUpdates = true ;
}



void CharacterModel::attributesUpdated ( void )
{

}



unsigned CharacterModel::GetBaseAttackDamage ( void )
{
	return 100 ;
}



unsigned CharacterModel::GetCharacterType ( void )
{
	return CharacterType ;
}



void CharacterModel::SetDebuffState ( DebuffState_T NewDebuffState , unsigned DebuffTime )
{
	// Code to prevent adding a new debuf when one is already set?
	MyDebuffState = NewDebuffState ;
	
	DebuffTimer = new Timer ( DebuffTime ) ;
}



bool CharacterModel::HasDebuff ( DebuffState_T* TheDebuff )
{
	bool debuffActive = false ;
	TheDebuff = NULL ;
	
	if ( DebuffTimer != NULL )
	{
		if ( DebuffTimer->TimedOut () == false )
		{
			debuffActive == true ;
			TheDebuff = &MyDebuffState ;
		}
		else
		{
			delete ( DebuffTimer ) ;
		}
	}
	
	return debuffActive ;
}
