#include "NPCModel.h"



NPCModel::NPCModel ( LocationVector* TargetLocation , unsigned NewGOId ) : CharacterModel ( NewGOId )
{
	Handle = "DefaultHandle" ;
	Background = "DefaultBackground" ;

	Position = new LocationVector ( TargetLocation->x , TargetLocation->y , TargetLocation->z ) ;
	Position->rot = 99999 ;

	CurrentHealth = 500 ;
	MaxHealth = 500 ;

	CurrentIS = 100 ;
	MaxIS = 100 ;

	Level = 5 ;
	Alignment = 50 ;

	Cash = 50000 ;

	District = 1 ;

	RSI = new RsiDataMale ;
	const byte defaultRsiValues[] = {0x00,0x0C,0x71,0x48,0x18,0x0C,0xE2,0x00,0x23,0x00,0xB0,0x00,0x40,0x00,0x00};
	RSI->FromBytes(defaultRsiValues,sizeof(defaultRsiValues));

	GOId = NewGOId ;

	TransmitUpdates = false ;

	NPC = true ;

	CharacterType = 0x8d ; // There are other types too, those need to be handled eventually.
}



bool NPCModel::IsTrueNPC ( void )
{
	return NPC ;
}
