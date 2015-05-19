#include "NPCConfig.h"



NPCConfig::NPCConfig ( void )
{
	loadSpawnPoints () ;
}



void NPCConfig::CreateSpawnPoint ( LocationVector* SpawnLocation )
{
	pugi::xml_node spawnPoints = SpawnPoints.child ( "SpawnPoints" ) ;
	
		// Document does not include spawn points, create the parent node.
	if ( spawnPoints == NULL )
	{
		spawnPoints = SpawnPoints.append_child ( "SpawnPoints" ) ;
	}
	
	pugi::xml_node spawnPoint = spawnPoints.append_child ( "SpawnPoint" ) ;
    spawnPoint.append_attribute ( "PosX" ) = SpawnLocation->x ;
    spawnPoint.append_attribute ( "PosY" ) = SpawnLocation->y ;
    spawnPoint.append_attribute ( "PosZ" ) = SpawnLocation->z ;
}



LocationVector NPCConfig::GetFirstSpawnLocation ()
{
	CurrentSpawnPoint = SpawnPoints.child ( "SpawnPoints" ).child ( "SpawnPoint" ) ;
	
	pugi::xml_attribute spawnAttribute = CurrentSpawnPoint.first_attribute () ;
	double posX = spawnAttribute.as_double () ;
	spawnAttribute = spawnAttribute.next_attribute () ;
	double posY = spawnAttribute.as_double () ;
	spawnAttribute = spawnAttribute.next_attribute () ;
	double posZ = spawnAttribute.as_double () ;

		// NONONONO!!!! Fix in later version!
	LocationVector resultVector ( posX , posY , posZ ) ;
	
	return resultVector ;
}



LocationVector NPCConfig::GetNextSpawnLocation ()
{
	CurrentSpawnPoint = CurrentSpawnPoint.next_sibling () ;
	
	pugi::xml_attribute spawnAttribute = CurrentSpawnPoint.first_attribute () ;
	double posX = spawnAttribute.as_double () ;
	spawnAttribute = spawnAttribute.next_attribute () ;
	double posY = spawnAttribute.as_double () ;
	spawnAttribute = spawnAttribute.next_attribute () ;
	double posZ = spawnAttribute.as_double () ;

		// NONONONO!!!! Fix in later version!
	LocationVector resultVector ( posX , posY , posZ ) ;
	
	return resultVector ;
}



void NPCConfig::loadSpawnPoints ( void )
{
	std::string xmlPath = XML_PATH ;

	xmlPath.append ( "..\\NPCs\\NPCSpawnPoints.xml" ) ;

	if ( SpawnPoints.load_file ( xmlPath.c_str () ) != pugi::status_ok )
	{
		SpawnPoints.load ( "" ) ;
	}
}
