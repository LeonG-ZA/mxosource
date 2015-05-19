#include "RequestHandler.h"

#include "CommandParser.h"
#include "Log.h"

RequestHandler::RequestHandler ( ProduceResponseRPC* NewResponseProducer , WorldModel* NewWorldModel , NPCCollector* NewNPCCollector , CombatHandler* NewCombatHandler ) : MyActivatableAbilities ()
{
	MyResponseProducer = NewResponseProducer ;
	MyWorldModel = NewWorldModel ;
	MyNPCCollector = NewNPCCollector ;
	MyCombatHandler = NewCombatHandler ;
}



void RequestHandler::Jump ( PlayerCharacterModel* JumpCharacter , LocationVector* TargetLocation )
{
	JumpCharacter->Jump ( TargetLocation ) ;

		//	Immediately send this update to all clients rather than waiting for
		//	the periodic update.
	MyResponseProducer->BroadcastPosition ( ( CharacterModel* )JumpCharacter , NULL ) ;
}



void RequestHandler::Chat ( PlayerCharacterModel* MessageSource , std::string TheMessage )
{
// Add command parser.

	CommandParser commandParser ( TheMessage ) ;

	switch ( commandParser.GetCommand () )
	{
			// Add a new spawn point to the world model.
		case CommandParser::ADD_SPAWN_POINT :
		{
			std::stringstream responseMessage ;
			unsigned spawnIndex = MyWorldModel->AddSpawnPoint ( MessageSource->GetPosition () ) ;

			responseMessage << "Spawn Point created with index " << spawnIndex ;
			MyResponseProducer->SystemMessage ( MessageSource , responseMessage.str () ) ;
		}
		break ;
			// Create a new patrol path in the world model.
		case CommandParser::ADD_PATROL_PATH :
		{
			std::stringstream responseMessage ;
			unsigned pathIndex = MyWorldModel->AddPatrolPath () ;

			responseMessage << "Spawn Point created with index " << pathIndex ;
			MyResponseProducer->SystemMessage ( MessageSource , responseMessage.str () ) ;
		}
		break ;
			// Create a new patrol point for the specified path and add it to the world.
		case CommandParser::ADD_PATROL_POINT :
		{
			std::stringstream responseMessage ;
			unsigned pathIndex = atoi ( commandParser.GetArgument ( 0 ).c_str () ) ;

			if ( MyWorldModel->GetPathCount () > pathIndex )
			{
				Path* thePath = MyWorldModel->GetPath ( pathIndex ) ;

				unsigned pointIndex = thePath->AddPoint ( MessageSource->GetPosition () ) ;

				responseMessage << "Patrol Point on path " << pathIndex << " created with index " << pointIndex ;
			}
			else
			{
				responseMessage << "Patrol Path " << pathIndex << " does not exist!" ;
			}
			MyResponseProducer->SystemMessage ( MessageSource , responseMessage.str () ) ;
		}
		break ;
		case CommandParser::UNKNOWN_BUILD :
			// Send back unknown notice.
		break ;
			// Spawn a new NPC at the specified spawn point.
		case CommandParser::SPAWN_NPC :
		{
			std::stringstream responseMessage ;
			unsigned spawnIndex = atoi ( commandParser.GetArgument ( 0 ).c_str () ) ;

			if ( MyWorldModel->GetSpawnCount () > spawnIndex )
			{
				LocationVector* spawnPoint = MyWorldModel->GetSpawnPoint ( spawnIndex ) ;
				NPCModel* newChar = MyWorldModel->AddNPC ( spawnPoint ) ;
				
				MyResponseProducer->SpawnNPC ( newChar ) ;

				NPCController* newNPC = new NPCController ( newChar , MyResponseProducer ) ;
				MyNPCCollector->AddNPC ( newChar->GetGOId () , newNPC ) ;

				responseMessage << "New NPC Spawned " << newChar->GetGOId () ;
			}
			else
			{
				responseMessage << "Spawn Point " << spawnIndex << " does not exist!" ;
			}
			MyResponseProducer->SystemMessage ( MessageSource , responseMessage.str () ) ;
		}
		break ;
			// Add the specified route patrol activity to the specified NPC.
		case CommandParser::ADD_NPC_PATROL :
		{
			std::stringstream responseMessage ;
			unsigned npcGOId = atoi ( commandParser.GetArgument ( 0 ).c_str () ) ;
			unsigned patrolIndex = atoi ( commandParser.GetArgument ( 1 ).c_str () ) ;

			if ( MyWorldModel->GetPathCount () > patrolIndex )
			{
				NPCController* theNPC = MyNPCCollector->GetNPC ( npcGOId ) ;

				if ( theNPC != NULL )
				{

					Path* thePath = MyWorldModel->GetPath ( patrolIndex ) ;

					Route* theRoute = new Route ( thePath ) ;
					theNPC->QueueActivity ( NPCController::ACT_PATROLLING , theRoute ) ;

					responseMessage << "NPC " << npcGOId << " added to patrolling path " << patrolIndex ;
				}
				else
				{
					responseMessage << "NPC " << npcGOId << " does not exist!" ;
				}
			}
			else
			{
				responseMessage << "Patrol Path " << patrolIndex << " does not exist!" ;
			}
			MyResponseProducer->SystemMessage ( MessageSource , responseMessage.str () ) ;
		}
		break ;
			// Add the specified route transit activity to the specified NPC.
		case CommandParser::ADD_NPC_TRANSIT :
		{
			std::stringstream responseMessage ;
			unsigned npcGOId = atoi ( commandParser.GetArgument ( 0 ).c_str () ) ;
			unsigned patrolIndex = atoi ( commandParser.GetArgument ( 1 ).c_str () ) ;

			if ( MyWorldModel->GetPathCount () > patrolIndex )
			{
				NPCController* theNPC = MyNPCCollector->GetNPC ( npcGOId ) ;

				if ( theNPC != NULL )
				{

					Path* thePath = MyWorldModel->GetPath ( patrolIndex ) ;

					Route* theRoute = new Route ( thePath ) ;
					theNPC->QueueActivity ( NPCController::ACT_TRANSITING , theRoute ) ;

					responseMessage << "NPC " << npcGOId << " added to transit path " << patrolIndex ;
				}
				else
				{
					responseMessage << "NPC " << npcGOId << " does not exist!" ;
				}
			}
			else
			{
				responseMessage << "Patrol Path " << patrolIndex << " does not exist!" ;
			}

			MyResponseProducer->SystemMessage ( MessageSource , responseMessage.str () ) ;
		}
		break ;
			// Set the speed the NPC should be moving at.
		case CommandParser::SET_NPC_SPEED :
		{
			std::stringstream responseMessage ;
			unsigned npcGOId = atoi ( commandParser.GetArgument ( 0 ).c_str () ) ;
			CharacterMover::MovementSpeed_T theActivity = ( CharacterMover::MovementSpeed_T ) atoi ( commandParser.GetArgument ( 1 ).c_str () ) ;

			NPCController* theNPC = MyNPCCollector->GetNPC ( npcGOId ) ;
			if ( theNPC != NULL )
			{
				theNPC->SetSpeed ( theActivity ) ;

				responseMessage << "NPC " << npcGOId << " set to speed " << theActivity ;// Update for specific speed strings?
			}
			else
			{
				responseMessage << "NPC " << npcGOId << " does not exist!" ;
			}

			MyResponseProducer->SystemMessage ( MessageSource , responseMessage.str () ) ;
		}
		break ;
			// Read a character GO ID and 6 digit animation code and perform
			// that animation on that character.
		case CommandParser::RUN_ANIMATION :
		{
			std::stringstream responseMessage ;
			unsigned charGOId = atoi ( commandParser.GetArgument ( 0 ).c_str () ) ;
			std::string animation = commandParser.GetArgument ( 1 ) ;

			CharacterModel* theCharacter = MyWorldModel->GetCharacter ( charGOId ) ;
			if ( theCharacter != NULL )
			{
				responseMessage << MyCombatHandler->RunCombatAnimation ( theCharacter , animation ) ;
			}
			else
			{
				responseMessage << "Character " << charGOId << " does not exist!" ;
			}

			MyResponseProducer->SystemMessage ( MessageSource , responseMessage.str () ) ;
		}
		break ;
		case CommandParser::SPAWN_INTERLOCK :
		{
			std::stringstream responseMessage ;
			unsigned attackerGOId = atoi ( commandParser.GetArgument ( 0 ).c_str () ) ;
			unsigned defenderGOId = atoi ( commandParser.GetArgument ( 1 ).c_str () ) ;

			CharacterModel* attackerCharacter = MyWorldModel->GetCharacter ( attackerGOId ) ;
			CharacterModel* defenderCharacter = MyWorldModel->GetCharacter ( defenderGOId ) ;

			if ( ( attackerCharacter != NULL ) && ( defenderCharacter != NULL ) )
			{
				responseMessage << "Interlock combat session " << MyCombatHandler->SpawnInterlock ( attackerCharacter , defenderCharacter ) << " starting between " << attackerCharacter->GetGOId () << " and " + defenderCharacter->GetGOId () ;
			}
			else
			{
				responseMessage << "Character " << attackerGOId << " or " << attackerGOId << " does not exist!" ;
			}

			MyResponseProducer->SystemMessage ( MessageSource , responseMessage.str () ) ;
		}
		break ;
			// No command, send out world chat.
		default :
				//	Otherwise only world chat is currently being used, other chat ranges will need to be added later.
			MyResponseProducer->WorldChat ( MessageSource , MessageSource , TheMessage ) ;
		break ;
	}
}



void RequestHandler::HandleCloseCombat ( PlayerCharacterModel* AttackingCharacter , CharacterModel* TargetCharacter , unsigned TargetProtocol , unsigned Slot )
{
		// Hacky way of setting default, look for a better way...
	if ( AttackingCharacter->GetTactic () == NULL )
	{
		AttackingCharacter->SetTactic ( MyActivatableAbilities.GetTactic ( 1 ) ) ;
	}
	if ( TargetCharacter->GetTactic () == NULL )
	{
		TargetCharacter->SetTactic ( MyActivatableAbilities.GetTactic ( 1 ) ) ;
	}
	MyCombatHandler->HandleCloseCombat ( AttackingCharacter , TargetCharacter , TargetProtocol , Slot ) ;
}



void RequestHandler::TacticUpdate ( PlayerCharacterModel* AttackingCharacter , unsigned TacticIndex )
{
	CombatTactic* theTactic = MyActivatableAbilities.GetTactic ( TacticIndex ) ;
	if ( theTactic != NULL )
	{
		AttackingCharacter->SetTactic ( theTactic ) ;
	}
}



void RequestHandler::Withdraw ( PlayerCharacterModel* WithdrawCharacter )
{
	MyCombatHandler->Withdraw ( WithdrawCharacter ) ;
}



void RequestHandler::ActivateAbility ( PlayerCharacterModel* ActivateCharacter , unsigned AbilityIndex )
{
	CombatAbility* theAbility = MyActivatableAbilities.GetAbility ( AbilityIndex ) ;
	if ( theAbility != NULL )
	{
		MyCombatHandler->SetCombatAbility ( ActivateCharacter , theAbility ) ;
	}
}