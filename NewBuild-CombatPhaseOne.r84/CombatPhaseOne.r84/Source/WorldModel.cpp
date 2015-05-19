#include "WorldModel.h"

WorldModel::WorldModel ( void ) : CharacterList () , SpawnPoints ( 0 ) , Paths ( 0 ) 
{
	LastContinuousNPCId = MinNPCId - 1 ;
	LastContinuousPCId = MinPCId - 1 ;
	LastContinuousSessionId = 25000 ;  // Session IDs can be either in NPC or PC range but can't be the same as a PC or NPC.  Further work shold be done eventually to intersperse interlock session IDs with NPC and PC IDs.
}



PlayerCharacterModel* WorldModel::AddPlayerCharacter ( unsigned NewUserId , unsigned NewCharacterId )
{
	unsigned newGOId = getNextGOId ( PC ) ;

		// Loads the character from the DB into a PlayerCharacterModel and sets
		// the Game Object ID.
	PlayerCharacterModel* newPlayerCharacter = new PlayerCharacterModel ( NewUserId , NewCharacterId , newGOId ) ;

		// Assign to the map via the Game Object ID
	CharacterList[newGOId] = ( CharacterModel *)newPlayerCharacter ;

	return newPlayerCharacter ;
}



NPCModel* WorldModel::AddNPC ( LocationVector* NPCLocation )
{
	unsigned newGOId = getNextGOId ( NPC ) ;

	NPCModel* newNPC = new NPCModel ( NPCLocation , newGOId ) ;

	CharacterList[newGOId] = ( CharacterModel *)newNPC ;

	return newNPC ;
}



PlayerCharacterModel* WorldModel::FindPlayer ( unsigned UserId )
{
	PlayerCharacterModel* foundCharacter = NULL ;
	map < unsigned, CharacterModel* >::iterator characterItterator ;
	bool characterFound = false ;

	characterItterator = CharacterList.lower_bound ( MinPCId ) ;
	while ( ( characterItterator != CharacterList.end () ) && ( characterFound == false ) )
	{
		if ( ( ( PlayerCharacterModel* ) characterItterator->second )->GetCharacterId () == UserId )
		{
			characterFound = true ;
			foundCharacter = ( PlayerCharacterModel* ) characterItterator->second ;
		}
		else
		{
			characterItterator++ ;
		}
	}
	
	return foundCharacter ;
}



CharacterModel* WorldModel::GetCharacter ( unsigned GOId )
{
	CharacterModel* foundCharacter = NULL ;

	map < unsigned, CharacterModel* >::iterator characterIterator = CharacterList.find ( GOId ) ;

	if ( characterIterator != CharacterList.end () )
	{
		foundCharacter = characterIterator->second ;
	}

	return foundCharacter ;
}



CharacterModel* WorldModel::GetFirstCharacter ( void )
{
	CharacterModel* firstCharacter = NULL ;
	
	if ( CharacterList.empty() == false )
	{
		map < unsigned, CharacterModel* >::iterator characterItterator = CharacterList.begin () ;
		firstCharacter = characterItterator->second ;
	}
	
	return firstCharacter ;
}



CharacterModel* WorldModel::GetNextCharacter ( unsigned PreviousGOId )
{
	CharacterModel* nextCharacter = NULL ;
	
		// Get the next character after the specified GOId.
	map < unsigned, CharacterModel* >::iterator characterItterator = CharacterList.lower_bound ( PreviousGOId ) ;
	characterItterator++ ;
	
		// Only return it if there is another character in the list.
	if ( characterItterator != CharacterList.end () )
	{
		nextCharacter = characterItterator->second ;
	}
	
	return nextCharacter ;
}



void WorldModel::ProcessUpdates ( void )
{
	map < unsigned, CharacterModel* >::iterator characterItterator ;
	for ( characterItterator = CharacterList.begin () ; characterItterator != CharacterList.end () ; characterItterator++ )
	{
		if ( ( characterItterator->second )->IsNPC () == false )
		{
			( ( PlayerCharacterModel* ) characterItterator->second )->UpdateDB () ;
		}
	}
}



unsigned WorldModel::AddSpawnPoint ( LocationVector NewSpawnPoint )
{
	LocationVector* newSpawn = new LocationVector ( NewSpawnPoint ) ;
	SpawnPoints.push_back ( newSpawn ) ;

	return SpawnPoints.size () - 1 ;
}



unsigned WorldModel::GetSpawnCount ( void )
{
	return SpawnPoints.size() ;
}



LocationVector* WorldModel::GetSpawnPoint ( unsigned SpawnId )
{
	return SpawnPoints[SpawnId] ;
}



unsigned WorldModel::AddPatrolPath ( void )
{
	Path* newPath = new Path ( Path::Patrol ) ;
	Paths.push_back ( newPath ) ;

	return Paths.size () - 1 ;
}



unsigned WorldModel::GetPathCount ( void )
{
	return Paths.size() ;
}



Path* WorldModel::GetPath ( unsigned PatrolId )
{
	return Paths[PatrolId] ;
}



unsigned WorldModel::GetNewSessionId ( void )
{
	unsigned currentSessionId = LastContinuousSessionId + 1 ;

	while ( UsedSessionIds.find ( currentSessionId ) != UsedSessionIds.end () ) 
	{
		currentSessionId++ ;
	}
	
		// problems will happen if we run out of IDs, clean up when this becomes intermixed with NPC and PC IDs.
	UsedSessionIds.insert ( currentSessionId ) ;

	LastContinuousSessionId = currentSessionId ;

	return currentSessionId ;
}



bool WorldModel::FreeSessionID ( unsigned RemoveID )
{
	bool combatIDFound = false ;
	std::set < unsigned >::iterator idIterator = UsedSessionIds.find ( RemoveID ) ;
	
	if ( idIterator != UsedSessionIds.end () )
	{
		combatIDFound = true ;
		UsedSessionIds.erase ( idIterator ) ;
	}
	
	return combatIDFound ;
}



unsigned WorldModel::getNextGOId ( CharacterType_T CharacterType )
{
	unsigned foundId = 0 ;

	unsigned lowerLimit, upperLimit ;
	unsigned* characterId ;
	map < unsigned, CharacterModel* >::iterator characterIterator ;

	switch ( CharacterType )
	{
		case PC :
			lowerLimit = MinPCId ;
			characterId = &LastContinuousPCId ;
			upperLimit = MaxPCId ;
		break ;
		case NPC :
			lowerLimit = MinNPCId ;
			characterId = &LastContinuousNPCId ;
			upperLimit = MaxNPCId ;
		break ;
	}

	( *characterId )++ ;

	while ( CharacterList.find ( *characterId ) != CharacterList.end () ) 
	{
		*characterId++ ;
	}

	if ( *characterId < upperLimit )
	{
		foundId = *characterId ;
	}
	else
	{
		*characterId = MinNPCId ;
	}

	return foundId ;
}



WorldModel::~WorldModel ()
{
	map < unsigned, CharacterModel* >::iterator characterItterator ;

	for ( characterItterator = CharacterList.begin () ; characterItterator != CharacterList.end () ; characterItterator++ )
	{
		free ( characterItterator->second ) ;
	}
}



