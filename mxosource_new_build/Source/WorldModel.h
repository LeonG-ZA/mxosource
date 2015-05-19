#ifndef MXOSIM_WORLDMODEL_H
#define MXOSIM_WORLDMODEL_H

#include "CharacterModel.h"

class WorldModel
{

	public :

		WorldModel ( void ) ;

		int AddCharacter ( unsigned NewUserID , unsigned NewCharacterID ) ;

		CharacterModel* FindCharacter ( unsigned FindCharacterID ) ;

		CharacterModel* GetFirstCharacter ( void ) ;

		CharacterModel* GetNextCharacter ( void ) ;

		void ProcessUpdates ( void ) ;

	private :

		struct CharacterList_E
		{
			CharacterModel* TheCharacter ;
			CharacterList_E* NextCaracter ;
		} ;

		CharacterList_E* FirstCharacter ;

		CharacterList_E* CurrentCharacter ;

} ;

#endif MXOSIM_WORLDMODEL_H