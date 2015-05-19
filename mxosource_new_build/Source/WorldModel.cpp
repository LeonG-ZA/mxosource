#include "WorldModel.h"

WorldModel::WorldModel ( void )
{
	FirstCharacter = NULL ;
}



int WorldModel::AddCharacter ( unsigned NewUserID , unsigned NewCharacterID )
{
	CharacterList_E* lastCharacter = FirstCharacter ;
	CharacterList_E* newCharacter = new CharacterList_E ;

	newCharacter->NextCaracter = NULL ;
	newCharacter->TheCharacter = new CharacterModel ( NewUserID , NewCharacterID ) ;

	if ( lastCharacter == NULL )
	{
		FirstCharacter = newCharacter ;
	}
	else
	{
		while ( lastCharacter->NextCaracter != NULL )
		{
			lastCharacter = lastCharacter->NextCaracter ;
		}
		lastCharacter->NextCaracter = newCharacter ;
	}

	return 0 ;
}



CharacterModel* WorldModel::FindCharacter ( unsigned FindCharacterID )
{
	CharacterList_E* currentCharacter = FirstCharacter ;
	CharacterModel* foundCharacter = NULL ;
	
	while ( ( currentCharacter != NULL ) &&
	         ( currentCharacter->TheCharacter->GetCharacterID () != FindCharacterID ) )
	{
		currentCharacter = currentCharacter->NextCaracter ;
	}

	if ( ( currentCharacter != NULL ) &&
	      ( currentCharacter->TheCharacter->GetCharacterID () == FindCharacterID ) )
	{
		foundCharacter = currentCharacter->TheCharacter ;
	}

	return foundCharacter ;
}



void WorldModel::ProcessUpdates ( void )
{
	CharacterList_E* currentCharacter = FirstCharacter ;
	
	while ( currentCharacter != NULL )
	{
		currentCharacter->TheCharacter->UpdateDB () ;
		currentCharacter = currentCharacter->NextCaracter ;
	}
}




CharacterModel* WorldModel::GetFirstCharacter ( void )
{
	CharacterModel* firstCharacter = NULL ;

	CurrentCharacter = FirstCharacter ;

	if ( CurrentCharacter != NULL )
	{
		firstCharacter = CurrentCharacter->TheCharacter ;
	}

	return firstCharacter ;
}

CharacterModel* WorldModel::GetNextCharacter ( void )
{
	CharacterModel* nextCharacter = NULL ;

	CurrentCharacter = CurrentCharacter->NextCaracter ;

	if ( CurrentCharacter != NULL )
	{
		nextCharacter = CurrentCharacter->TheCharacter ;
	}

	return nextCharacter ;
}