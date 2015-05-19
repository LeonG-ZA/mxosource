#include "CharacterMover.h"

#include "ProcessConverters.h"
#include "Log.h"


CharacterMover::CharacterMover ( CharacterModel* TheCharacterModel ,
                                 ProduceResponseRPC* TheResponseRPC ) :
	MyTarget () ,
	MyCharacterModel ( TheCharacterModel ) , MyResponseRPC ( TheResponseRPC )
{
	MyMovementSpeed = IDLE ;
	
	LastMovementTime = -1 ;

	TargetCharacter = NULL ;
	StaticTarget = true ;
}



void CharacterMover::SetTarget ( LocationVector* NewTarget )
{
	MyTarget = NewTarget ;
	StaticTarget = true ;
}



		
void CharacterMover::SetTarget ( CharacterModel* NewTarget )
{
	TargetCharacter = NewTarget ;
	StaticTarget = false ;
}



void CharacterMover::SetMovementSpeed ( MovementSpeed_T NewSpeed ) // Do we need to set present position here, I don't think so...?
{
	MyMovementSpeed = NewSpeed ;
}



CharacterModel::MovementState_T CharacterMover::GetMovementState ( void )
{
	return MyCharacterModel->GetMovementState () ;
}



bool CharacterMover::AlignToTarget ( void )
{
	bool aligned = true ;

	LocationVector* targetLocation ;

	if ( StaticTarget == true )
	{
		targetLocation = &MyTarget ;
	}
	else
	{
		targetLocation = TargetCharacter->GetPosition () ;
	}

		// Calculate the appropriate heading toward the waypoint and turn the
		// character.
	double headingToWaypoint = MyCharacterModel->GetPosition ()->
	                            CalcAngTo ( targetLocation ) ;

		// Walking backward???
	if ( fabs ( headingToWaypoint - MyCharacterModel->GetPosition ()->rot ) > M_PI * 0.005 )
	{
			// PC movement has problems with rotation and movement at the same
			// time.  Not sure if NPC has the same issue.
		if ( GetMovementState () != IDLE )
		{
				// Stop movement and tell client present position of character.
			stopMovement () ;
		}
	
		rotateToPosition ( headingToWaypoint ) ;

		aligned = false ;
	}

	return aligned ;
}
		


bool CharacterMover::MoveToTarget ( void )
{
	bool reachedTarget = false ;

	LocationVector* targetLocation ;

	if ( StaticTarget == true )
	{
		targetLocation = &MyTarget ;
	}
	else
	{
		targetLocation = TargetCharacter->GetPosition () ;
	}

		// If waypoint has not been provided yet, report waypoint reached.
	if ( ( targetLocation->x == 0 ) &&
		 ( targetLocation->y == 0 ) &&
		 ( targetLocation->z == 0 ) )
	{
		reachedTarget = true ;
	}
	else if ( AlignToTarget () )
	{
		if ( GetMovementState () != IDLE )
		{
			updateEstimatedPosition () ;
			
				// Have we reached the waypoint?
			double targetDistance = MyCharacterModel->GetPosition ()->Distance2D ( MyTarget ) ;

				// If so, indicate we have reached the target to the calling function,
				// stop moving and set movement speed to idle.
			if ( targetDistance < ( WalkDistancePerS / 4 ) )  // Handle past and still going that way?
			{
				stopMovement () ;
	
				reachedTarget = true ;
	
				MyTarget.x = 0 ;
				MyTarget.y = 0 ;
				MyTarget.z = 0 ;
			}
				// Otherwise if we are not supposed to be moving, stop moving.
			else if ( MyMovementSpeed == IDLE )
			{
				stopMovement () ;
			}
		}
			// If we are supposed to be moving, start.
		else if ( MyMovementSpeed != IDLE )
		{
			startMovement ( MovementToModelMovementState ( MyMovementSpeed ) ) ;
		}
	}

	return reachedTarget ;
}



void CharacterMover::updateEstimatedPosition ( void )
{
		// Get the time stamp for the updates.
	clock_t updateTime = clock () ;
	unsigned speed ;
	unsigned distance ;
	
		// Determine the appropriate movement speed.
	switch ( GetMovementState () )
	{
		case IDLE :
			speed = 0 ;
		break ;
		case WALK :
			speed = WalkDistancePerS ;
		break ;
		case WALK_BACKWARD :
			speed = -WalkDistancePerS ;
		break ;
		case RUN :
			speed = RunDistancePerS ;
		break ;
	}
	
		// If we were moving during the previous update.
	if ( LastMovementTime >= 0 )
	{
		LocationVector* targetLocation ;

		if ( StaticTarget == true )
		{
			targetLocation = &MyTarget ;
		}
		else
		{
			targetLocation = TargetCharacter->GetPosition () ;
		}

			// Estmiate the new character position.
		distance = speed * ( updateTime - LastMovementTime ) / CLOCKS_PER_SEC ;
		MyCharacterModel->GetPosition()->AddVectorRatio ( targetLocation , distance ) ;
	}
	
		// If we are supposed to be moving, update the last time the
		// estimated position was estimated.
	if ( GetMovementState () != IDLE )
	{
		LastMovementTime = updateTime ;
	}
		// Otherwise indicate no movement.
	else
	{
		LastMovementTime = -1 ;
	}
}



void CharacterMover::rotateToPosition ( double NewHeading )
{
			// All messages pull the position out of the character model so the
			// target position is set inside the character model.
		MyCharacterModel->SetHeading ( NewHeading ) ;

			// Rotate to target.  Using only the end rotation position rather
			// than incremental updates to send less messages.
		if ( MyCharacterModel->IsNPC () == true )
		{
				// NPC rotation includes both rotation and position.  Handling
				// them separately to be consistent with PC movement.  Could
				// change later.
			MyResponseRPC->UpdateNPCState (
			 MyCharacterModel , ModelToMessageMovementState ( GetMovementState () ) ) ;
		}
		else
		{
				// End position causes the player to rotate smoothly to target
				// heading.
			MyResponseRPC->StopRotatePlayer ( MyCharacterModel ) ;
		}
}



void CharacterMover::startMovement ( CharacterModel::MovementState_T MovementState )
{
		// Set the speed we will request from the client.
	MyCharacterModel->SetMovementState ( MovementState ) ;

		// Start character moving from present position.
	if ( MyCharacterModel->IsNPC () == true )
	{
			// Only working NPC state message, updates state along with position
			// and rotation.  Would rather use movement only but that message
			// doesn't work.
		MyResponseRPC->UpdateNPCState (
		 MyCharacterModel , ModelToMessageMovementState ( GetMovementState () ) ) ;
	}
	else
	{
			// Updates the PC movement to the new state, does not change position.
		MyResponseRPC->StartCharState (
		 MyCharacterModel , ModelToMessageMovementState ( GetMovementState () ) ) ;
	}
}



void CharacterMover::stopMovement ( void )
{
		// Stop the character.
	MyCharacterModel->SetMovementState ( CharacterModel::IDLE ) ;

		// Stop character movement at the present position.
	if ( MyCharacterModel->IsNPC () == true )
	{
			// Only working NPC state message, updates state along with position
			// and rotation.  Would rather use movement only but that message
			// doesn't work.
		MyResponseRPC->UpdateNPCState (
		 MyCharacterModel , ModelToMessageMovementState ( GetMovementState () ) ) ;
	}
	else
	{
			// Updates the PC movement to the new state, does not change position.
		MyResponseRPC->StartCharState (
		 MyCharacterModel , ModelToMessageMovementState ( GetMovementState () ) ) ;
	}
}
