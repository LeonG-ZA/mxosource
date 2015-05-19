#include "CharacterModel.h"
#include "Common.h"
#include "Database\Database.h"
#include "Log.h"



CharacterModel::CharacterModel ( unsigned NewUserID , unsigned NewCharacterID ) : UserID ( NewUserID ) , CharacterID ( NewCharacterID )
{
	scoped_ptr<QueryResult> sqlQuery(
		sDatabase.Query(
			format( "SELECT `handle`, `firstName`, `lastName`, `background`,\
				`x`, `y`, `z`, `rot`, \
					`healthC`, `healthM`, `innerStrC`, `innerStrM`,\
						`level`, `profession`, `alignment`, `pvpflag`, `exp`, `cash`, `district`\
							FROM `characters` WHERE `userId` = '%1%' AND `charId` = '%2%' LIMIT 1") % UserID % CharacterID) );

	if (sqlQuery == NULL)
	{
		ERROR_LOG(format("MS_LoadCharacterRequest: Character doesn't exist or user ID %1% doesn't own it") % UserID );
		return;
	}

	Field *queryResult = sqlQuery->Fetch();

	Handle = queryResult[0].GetString () ;
	FirstName = queryResult[1].GetString () ;
	LastName = queryResult[2].GetString () ;
	Background = queryResult[3].GetString () ;

	RsiData RSI ;

	Position = new LocationVector ( queryResult[4].GetDouble () , queryResult[5].GetDouble () , queryResult[6].GetDouble () , queryResult[7].GetUInt8 () ) ;


	CurrentHealth = queryResult[8].GetUInt16 () ;
	MaxHealth = queryResult[9].GetUInt16 () ;

	CurrentIS = queryResult[10].GetUInt16 () ;
	MaxIS = queryResult[11].GetUInt16 () ;

	Level = queryResult[12].GetUInt16 () ;
	Profession = queryResult[13].GetUInt16 () ;
	Alignment = queryResult[14].GetUInt16 () ;

	Experience = queryResult[16].GetUInt16 () ;
	Cash = queryResult[17].GetUInt16 () ;

	District = queryResult[18].GetUInt16 () ;

	loadRSI () ;

	GOID = CharacterID ;

	DBNeedsUpdate = false ;
	TransmitUpdates = false ;
}



unsigned CharacterModel::GetCharacterID ()
{
	return CharacterID ;
}



string CharacterModel::GetHandle ()
{
	return Handle ;
}



string CharacterModel::GetFirstName ()
{
	return FirstName ;
}



string CharacterModel::GetLastName ()
{
	return LastName ;
}



string CharacterModel::GetBackground ()
{
	return Background ;
}



LocationVector* CharacterModel::GetPosition ()
{
	return Position ;
}

	
	
void CharacterModel::SetPosition ( LocationVector* NewPosition )
{
	Position->x = NewPosition->x ;
	Position->y = NewPosition->y ;
	Position->z = NewPosition->z ;
	Position->rot = NewPosition->rot ;

	DBNeedsUpdate = true ;
	TransmitUpdates = true ;
}



void CharacterModel::SetPositionX ( double NewPosX )
{
	Position->x = NewPosX ;

	DBNeedsUpdate = true ;
	TransmitUpdates = true ;
}



void CharacterModel::SetPositionY ( double NewPosY )
{
	Position->y = NewPosY ;

	DBNeedsUpdate = true ;
	TransmitUpdates = true ;
}



void CharacterModel::SetPositionZ ( double NewPosZ )
{
	Position->z = NewPosZ ;

	DBNeedsUpdate = true ;
	TransmitUpdates = true ;
}



void CharacterModel::SetHeading ( double NewHeading )
{
	Position->rot = NewHeading ;

	DBNeedsUpdate = true ;
	TransmitUpdates = true ;
}



void CharacterModel::SetHeading ( uint8 NewHeading )
{
	Position->setMxoRot ( NewHeading ) ;

	DBNeedsUpdate = true ;
	TransmitUpdates = true ;
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



unsigned CharacterModel::GetProfession ()
{
	return Profession ;
}



unsigned CharacterModel::GetLevel ()
{
	return Level ;
}



unsigned CharacterModel::GetAlignment ()
{
	return Alignment ;
}



void CharacterModel::SetGOID ( unsigned NewGOID )
{
	GOID = NewGOID ;
}

	
	
unsigned CharacterModel::GetGOID ()
{
	return GOID ;
}



unsigned CharacterModel::GetDistrict ()
{
	return District ;
}



unsigned CharacterModel::GetExperience ()
{
	return Experience ;
}



unsigned CharacterModel::GetCash ()
{
	return Cash ;
}



	//
	//	Store position data in the database if anything has changed.  Update in the figure for other important persistant data.
	//
void CharacterModel::UpdateDB ()
{
	if ( DBNeedsUpdate == true )
	{
		bool storeSuccess = sDatabase.Execute(format("UPDATE `characters` SET `x` = '%1%', `y` = '%2%', `z` = '%3%', `rot` = '%4%', `lastOnline` = NOW() WHERE `charId` = '%5%'")
			% Position->x
			% Position->y
			% Position->z
			% (unsigned)Position->getMxoRot()
			% CharacterID );

		if ( storeSuccess == true )
		{
			DBNeedsUpdate = false ;
		}
	}
}



void CharacterModel::loadRSI ()
{
	scoped_ptr<QueryResult> result(sDatabase.Query(format("SELECT `sex`, `body`, `hat`, `face`, `shirt`,\
														  `coat`, `pants`, `shoes`, `gloves`, `glasses`,\
														  `hair`, `facialdetail`, `shirtcolor`, `pantscolor`,\
														  `coatcolor`, `shoecolor`, `glassescolor`, `haircolor`,\
														  `skintone`, `tattoo`, `facialdetailcolor`, `leggings` FROM `rsivalues` WHERE `charId` = '%1%' LIMIT 1") % CharacterID) );
	if (result == NULL)
	{
		INFO_LOG(format("SpawnRSI(%1%): Character's RSI doesn't exist") % UserID );
		RSI = new RsiDataMale ;
		const byte defaultRsiValues[] = {0x00,0x0C,0x71,0x48,0x18,0x0C,0xE2,0x00,0x23,0x00,0xB0,0x00,0x40,0x00,0x00};
		RSI->FromBytes(defaultRsiValues,sizeof(defaultRsiValues));
	}
	else
	{
		Field *field = result->Fetch();
		uint8 sex = field[0].GetUInt8();
		if (sex == 0) //male
		{
			RSI = new RsiDataMale ;
		}
		else
		{
			RSI = new RsiDataFemale ;
		}

		RsiData &playerRef = *RSI ;

		if (sex == 0) //male
		{
			playerRef["Sex"]=0;
		}
		else
		{
			playerRef["Sex"]=1;
		}
		playerRef["Body"] =			field[1].GetUInt8();
		playerRef["Hat"] =			field[2].GetUInt8();
		playerRef["Face"] =			field[3].GetUInt8();
		playerRef["Shirt"] =		field[4].GetUInt8();
		playerRef["Coat"] =			field[5].GetUInt8();
		playerRef["Pants"] =		field[6].GetUInt8();
		playerRef["Shoes"] =		field[7].GetUInt8();
		playerRef["Gloves"] =		field[8].GetUInt8();
		playerRef["Glasses"] =		field[9].GetUInt8();
		playerRef["Hair"] =			field[10].GetUInt8();
		playerRef["FacialDetail"]=	field[11].GetUInt8();
		playerRef["ShirtColor"] =	field[12].GetUInt8();
		playerRef["PantsColor"] =	field[13].GetUInt8();
		playerRef["CoatColor"] =	field[14].GetUInt8();
		playerRef["ShoeColor"] =	field[15].GetUInt8();
		playerRef["GlassesColor"]=	field[16].GetUInt8();
		playerRef["HairColor"] =	field[17].GetUInt8();
		playerRef["SkinTone"] =		field[18].GetUInt8();
		playerRef["Tattoo"] =		field[19].GetUInt8();
		playerRef["FacialDetailColor"] =	field[20].GetUInt8();

		if (sex != 0)
		{
			playerRef["Leggings"] =	field[21].GetUInt8();
		}
	}
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