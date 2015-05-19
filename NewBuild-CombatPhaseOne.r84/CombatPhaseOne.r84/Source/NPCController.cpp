#include "NPCController.h"

#include "Log.h"



NPCController::NPCController ( NPCModel* TheCharacterModel , ProduceResponseRPC* TheResponseRPC ) : MyCharacter ( TheCharacterModel ) , MyResponseRPC ( TheResponseRPC )
{
		// Handles moving a character to a specified waypoint.
	MyCharacterMover = new CharacterMover ( TheCharacterModel , TheResponseRPC ) ;
}



void NPCController::OverrideActivity ( Activity_T NewActivity , Route* NewRoute )
{
	ActivityData_T newActivity = { NewActivity , NewRoute } ;

		// Put the activity to the front of the "queue" so that this action
		// immediately becomes the active one.
	ActivityQueue.push_front ( newActivity ) ;

		// If supporting activities other than routes become prevalant, this will have to change...
		// Seems like there should be a cleaner way to set the first waypoint...
	MyCharacterMover->SetTarget ( ActivityQueue.front ().SupportingData->GetCurrentRoutePoint () ) ;
}



void NPCController::QueueActivity ( Activity_T NewActivity , Route* NewRoute )
{
	ActivityData_T newActivity = { NewActivity , NewRoute } ;

		// Standard activity queueing on a FIFO basis.
	ActivityQueue.push_back ( newActivity ) ;

	if ( ActivityQueue.size () == 1 )
	{
			// If supporting activities other than routes become prevalant, this will have to change...
		// Seems like there should be a cleaner way to set the first waypoint...
		MyCharacterMover->SetTarget ( ActivityQueue.front ().SupportingData->GetCurrentRoutePoint () ) ;
	}
}



void NPCController::ClearQueue ( void )
{
	ActivityQueue.clear () ;
}




void NPCController::SetSpeed ( CharacterMover::MovementSpeed_T NewSpeed )
{
	MyCharacterMover->SetMovementSpeed ( NewSpeed ) ;
}



void NPCController::NPCPeriodicUpdate ( void )
{
	if ( ActivityQueue.size () == 0 )
	{
		MyCharacterMover->MoveToTarget () ;
	}
	else
	{
			// Handle the possible activities.
		switch ( ActivityQueue.front ().TheActivity )
		{
			case ACT_IDLE :
				// Is there really an idle activity or just no activity?
			break ;
			
				// Transit and patrol are being processed rather similarly,
				//	should they be combined somehow?
			case ACT_TRANSITING :
				if ( GetAssociatedRoute () != NULL )
				{
					updateTransit () ;
				}
			break ;
			
			case ACT_PATROLLING :
				if ( GetAssociatedRoute () != NULL )
				{
					updatePatrol () ;
				}
			break ;
		}
	}
}



Route* NPCController::GetAssociatedRoute ( void )
{
		// If supporting activities other than routes become prevalant, this will have to change...
	return ActivityQueue.front ().SupportingData ;
}



void NPCController::updatePatrol ( void )
{
		// If the waypoint is reached, increment to the next one
	if ( MyCharacterMover->MoveToTarget () == true )
	{
		ActivityQueue.front ().SupportingData->IncrementRoute () ;
		MyCharacterMover->SetTarget ( ActivityQueue.front ().SupportingData->GetCurrentRoutePoint () ) ;
	}
}



void NPCController::updateTransit ( void )
{

		// If the waypoint is reached, increment to the next one
	if ( MyCharacterMover->MoveToTarget () == true )
	{
			// If no more waypoints, move to the next activity.
		if ( ActivityQueue.front ().SupportingData->IncrementRoute () == false )
		{
			ActivityQueue.erase ( ActivityQueue.begin () ) ;
		}
		if ( ActivityQueue.empty () == false )
		{
			MyCharacterMover->SetTarget ( ActivityQueue.front ().SupportingData->GetCurrentRoutePoint () ) ;
		}
	}
}
