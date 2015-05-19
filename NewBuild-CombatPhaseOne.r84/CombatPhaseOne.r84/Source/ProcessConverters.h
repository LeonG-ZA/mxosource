#ifndef MXOSIM_PROCESSCONVERTERS_H
#define MXOSIM_PROCESSCONVERTERS_H

#include "CharacterMover.h"
#include "MessageTypes.h"

	// Converts MovementState_T to the appropriate message state enum
	// Should look for a cleaner way to do this.
static MovementState::State_T ModelToMessageMovementState ( CharacterModel::MovementState_T MovementState )
{
	switch ( MovementState )
	{
		case MovementState::IDLE :
		default :
			return UpdateNPCStateMsg::IDLE ;
		break ;
		case MovementState::WALK :
			return UpdateNPCStateMsg::WALK ;
		break ;
		case MovementState::WALK_BACKWARD :
			return UpdateNPCStateMsg::WALK_BACKWARD ;
		break ;
		case MovementState::RUN :
			return UpdateNPCStateMsg::RUN ;
		break ;
	}
}

static CharacterModel::MovementState_T MessageToModelMovementState ( MovementState::State_T MovementState )
{
	switch ( MovementState )
	{
		case MovementState::IDLE :
		default :
			return CharacterModel::IDLE ;
		break ;
		case MovementState::WALK :
			return CharacterModel::WALKING ;
		break ;
		case MovementState::WALK_BACKWARD :
			return CharacterModel::WALKING_BACKWARD ;
		break ;
		case MovementState::RUN :
			return CharacterModel::RUNING ;
		break ;
	}
}

static CharacterModel::MovementState_T MovementToModelMovementState ( CharacterMover::MovementSpeed_T MovementSpeed )
{
	switch ( MovementSpeed )
	{
		case CharacterMover::IDLE :
		default :
			return CharacterModel::IDLE ;
		break ;
		case CharacterMover::WALK :
			return CharacterModel::WALKING ;
		break ;
		case CharacterMover::WALK_BACKWARD :
			return CharacterModel::WALKING_BACKWARD ;
		break ;
		case CharacterMover::RUN :
			return CharacterModel::RUNING ;
		break ;
	}
}

static MoodUpdateMsg::Mood_T ModelToMessageMood ( CharacterModel::Mood_T Mood )
{
	switch ( Mood )
	{
		case CharacterModel::MOOD1 :
		default :
			return MoodUpdateMsg::MOOD1 ;
		break ;
		case CharacterModel::MOOD2 :
			return MoodUpdateMsg::MOOD2 ;
		break ;
		case CharacterModel::DEAD :
			return MoodUpdateMsg::DEAD ;
		break ;
	}
}

#endif