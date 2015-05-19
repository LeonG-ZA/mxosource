// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2006-2010 Rajko Stojadinovic
// http://mxoemu.info
//
// ---------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ---------------------------------------------------------------------------
//
// ***************************************************************************

#include "MessageTypes.h"
#include "GameClient.h"
#include "GameServer.h"
#include "ObjectMgr.h"
#include "LocationVector.h"
#include "Config.h"
#include "ProcessConverters.h"

ObjectUpdateMsg::ObjectUpdateMsg(CharacterModel* MessageCharacter):TheCharacter(MessageCharacter)
{
}

ObjectUpdateMsg::~ObjectUpdateMsg()
{

}

/*DeletePlayerMsg::DeletePlayerMsg( uint32 objectId ) :ObjectUpdateMsg(objectId)
{

}

DeletePlayerMsg::~DeletePlayerMsg()
{

}

const ByteBuffer& DeletePlayerMsg::toBuf()
{
	return m_buf;
}

void DeletePlayerMsg::setReceiver( class GameClient *toWho )
{
	m_toWho = toWho;
	m_buf.clear();

	uint16 viewId = 0;
	try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,TheCharacter);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		throw PacketNoLongerValid();		
	}
	DEBUG_LOG(format("Player %1% delete packet serializing for client %2% with viewID %3%") % TheCharacter % m_toWho->Address() % viewId);

	//03 01 00 01 01 00 <OBJECT ID:2bytes> <nomoreAttribs (00 00)>
	const byte rawData[6] =
	{
		0x03, 0x01, 0x00, 0x01, 0x01, 0x00, 
	} ;
	m_buf.append(rawData,sizeof(rawData));
	m_buf << uint16(viewId);
	m_buf << uint16(0);
}*/
/*
CloseDoorMsg::CloseDoorMsg( uint32 objectId) :ObjectUpdateMsg(objectId)
{

}

CloseDoorMsg::~CloseDoorMsg()
{

}

const ByteBuffer& CloseDoorMsg::toBuf()
{
	return m_buf;
}

void CloseDoorMsg::setReceiver( class GameClient *toWho )
{
	m_toWho = toWho;
	m_buf.clear();

	uint16 clientDoorId = TheCharacter->GetCharacterId ();  //This is a unique number given to the client for each door that is currently opened (it is used again when the door needs to be closed)
	
	
	string msg1 = (format("Door %1% Close command") % clientDoorId).str();
	m_toWho->QueueCommand(make_shared<SystemChatMsg>(msg1));

	DEBUG_LOG(format("Door %1% Close command") % clientDoorId);

	//03 01 00 01 01 00 <OBJECT ID:2bytes>


//03 01 00 08 da 19 19 00 50 30 ef cd ab 03 84 00 00 00 00 f2 04 35 bf 00 00 00 00 f3 04 35 3f 
//41 00 00 00 00 00 c1 06 41 00 00 00 00 00 b8 8a c0 00 00 00 00 80 da e8 40 34 08 00 00 03 00 00 
//00 00 



	byte doorCloseMsg[12] =
	{
		0x03, 0x03, 0x00, 0x01, 0x80, 0x02, 0x38, 0x08, 0x00, 0x00, 0x00, 0x00, 
	};

	ByteBuffer bufferBytesDoorId;
	bufferBytesDoorId << clientDoorId;
			
	byte byteDoorId[24] = {
		0x00, 0x00, 0x00, 0x00,0x00, 0xc1, 0x06, 0x41,  //X
		0x00, 0x00, 0x00, 0x00,0x00, 0xb8, 0x8a, 0xc0,  //Y
		0x00, 0x00, 0x00, 0x00,0x80, 0xda, 0xe8, 0x40,  //Z			
	};
	//Put new coords in
	bufferBytesDoorId.read(byteDoorId, bufferBytesDoorId.size());

	//Rotation
	//insideDoor[22] = 0xbf;

	memcpy(&doorCloseMsg[7],&byteDoorId,2);   //uint16

	//03 03 00 01 80 02 38 08 00 00 00 00 

	m_buf.append(doorCloseMsg,sizeof(doorCloseMsg));

}*/

PlayerSpawnMsg::PlayerSpawnMsg(PlayerCharacterModel* MessageCharacter) : ObjectUpdateMsg(MessageCharacter)
{
	TheCharacter = MessageCharacter ;
}

PlayerSpawnMsg::~PlayerSpawnMsg()
{

}

const ByteBuffer& PlayerSpawnMsg::toBuf()
{
	byte sampleSpawnPacket[202] =
	{
		0x03, 0x01, 0x00, 0x0C, 0x0C, 0x00, 0x2F, 0xCD, 0xAB, 0x18, 0x8B, 0xEC, 0xFF, 0x05, 0x00, 0x00, 
		0x00, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 
		0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 
		0x3A, 0x90, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 
		0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 
		0x3B, 0x3B, 0x80, 0x98, 0x5A, 0x5A, 0x04, 0x86, 0x8C, 0xFF, 0x6A, 0x6A, 0xC6, 0xC5, 0xFF, 0x3C, 
		0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 
		0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x5B, 
		0x5B, 0xED, 0x7A, 0x7A, 0x7A, 0x7A, 0xC5, 0xFF, 0x75, 0xD4, 0x01, 0x00, 0x8A, 0x8A, 0x8A, 0x8A, 
		0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x00, 0x00, 0x6B, 0x6B, 0x9D, 0x4A, 0x4A, 
		0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4C, 0x4C, 
		0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x8C, 0xFF, 0x7B, 0x22, 0x80, 0x88, 0x17, 0x1C, 0x7C, 0x00, 
		0x10, 0x00, 0x00, 0xC5, 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 
	} ;

	byte *firstNamePos = &sampleSpawnPacket[0x11];
	memset(firstNamePos,0,32);
	memcpy(firstNamePos,TheCharacter->GetFirstName().c_str(),TheCharacter->GetFirstName().length());
	byte *lastNamePos = &sampleSpawnPacket[0x32];
	memset(lastNamePos,0,32);
	memcpy(lastNamePos,TheCharacter->GetLastName().c_str(),TheCharacter->GetLastName().length());
	byte *handlePos = &sampleSpawnPacket[0x5F];
	memset(handlePos,0,32);

	memcpy(handlePos,TheCharacter->GetHandle().c_str(),TheCharacter->GetHandle().length());

	byte *rsiDataPos = &sampleSpawnPacket[0x8C];
	memset(rsiDataPos,0,15);
	TheCharacter->GetRSI()->ToBytes(rsiDataPos,15);

	byte *positionPos = &sampleSpawnPacket[0x9E];
	TheCharacter->GetPosition()->toDoubleBuf(positionPos,sizeof(double)*3);

	uint16 temp16;
	uint8 temp8;

	byte *healthCpos = &sampleSpawnPacket[0x54];
	temp16 = TheCharacter->GetCurrentHealth();
	memcpy(healthCpos,&temp16,sizeof(temp16));

	byte *healthMpos = &sampleSpawnPacket[0x7F];
	temp16 = TheCharacter->GetMaxHealth();
	memcpy(healthMpos,&temp16,sizeof(temp16));

	byte *innerStrCpos = &sampleSpawnPacket[0x5A];
	temp16 = TheCharacter->GetCurrentIS();
	memcpy(innerStrCpos,&temp16,sizeof(temp16));

	byte *innerStrMpos = &sampleSpawnPacket[0x9B];
	temp16 = TheCharacter->GetMaxIS();
	memcpy(innerStrMpos,&temp16,sizeof(temp16));

	byte *professionPos = &sampleSpawnPacket[0x82];
	uint32 temp32 = TheCharacter->GetProfession();
	memcpy(professionPos,&temp32,sizeof(temp32));

	byte *levelPos = &sampleSpawnPacket[0xB8];
	temp8 = TheCharacter->GetLevel();
	memcpy(levelPos,&temp8,sizeof(temp8));

	byte *alignmentPos = &sampleSpawnPacket[0xBE];
	temp8 = TheCharacter->GetAlignment();
	memcpy(alignmentPos,&temp8,sizeof(temp8));

	byte *viewIdPos = &sampleSpawnPacket[0xC5];
	uint16 goId = 0;

	goId = TheCharacter->GetGOId () ; //TheCharacter->GetViewID () ;
	memcpy(viewIdPos,&goId,sizeof(goId));

	m_buf.clear();
	m_buf.append(sampleSpawnPacket,sizeof(sampleSpawnPacket));
	return m_buf;
}

NPCSpawnMsg::NPCSpawnMsg(NPCModel* MessageCharacter) : ObjectUpdateMsg(MessageCharacter)
{
	TheCharacter = MessageCharacter ;
}

NPCSpawnMsg::~NPCSpawnMsg()
{

}

const ByteBuffer& NPCSpawnMsg::toBuf()
{
	byte sampleSpawnPacket[119] =
	{
		0x03, 0x01, 0x00, 0x0c, 0x57, 0x02, 0x8d, 0xcd, 0xab, 0x14,
		0xae, 0x54, 0x65, 0x73, 0x74, 0x48, 0x61, 0x6e, 0x64, 0x6c,
		0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x22, 0x00, 0x00,
		0x10, 0x00, 0xc4, 0xfb, 0x01, 0x80, 0xc1, 0xb7, 0x01, 0x00,
		0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0xb3, 0x40, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x88, 0xb3, 0x40, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x88, 0xb3, 0x40, 0xd0, 0x0c, 0x00, 0x83, 0x0c,
                          // Current Health       // Max Health
		0x01, 0xa8, 0x32, 0xfa, 0x00, 0xb2, 0x04, 0x5e, 0x01, 0x06,
		0x08, 0x00, 0x00, 0x83, 0x37, 0xba, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x02, 0x0e, 0x05, 0x00, 0x00, 0x00, 0x00,

	} ;

	
	byte *positionPos = &sampleSpawnPacket[61];
	TheCharacter->GetPosition()->toDoubleBuf(positionPos,sizeof(double)*3);

	uint16 *currentHealthPos = ( uint16* )&sampleSpawnPacket[93];
	*currentHealthPos = ( uint16 )TheCharacter->GetCurrentHealth () ;

	uint16 *maxHealthPos = ( uint16* )&sampleSpawnPacket[97];
	*maxHealthPos = ( uint16 )TheCharacter->GetMaxHealth () ;

	uint16 *idPos = (uint16*)&sampleSpawnPacket[114];
	*idPos = (uint16)TheCharacter->GetGOId ();
/*
	byte *rsiDataPos = &sampleSpawnPacket[11];
	memset(rsiDataPos,0,15);
	TheCharacter->GetRSI()->ToBytes(rsiDataPos,15);

	byte *positionPos = &sampleSpawnPacket[0x9E];
	TheCharacter->GetPosition()->toDoubleBuf(positionPos,sizeof(double)*3);
*/


//	ERROR_LOG(format("NPC %1%:%2% spawn packet serializing for client %3% with viewID %4%") % TheCharacter->GetHandle() % TheCharacter % TheCharacter->GetCharacterId () % GOId);

	m_buf.clear();
	m_buf.append(sampleSpawnPacket,sizeof(sampleSpawnPacket));
	return m_buf;
}

/*PlayerAppearanceMsg::PlayerAppearanceMsg( uint32 objectId ) :ObjectUpdateMsg(objectId)
{

}

PlayerAppearanceMsg::~PlayerAppearanceMsg()
{

}

const ByteBuffer& PlayerAppearanceMsg::toBuf()
{
	m_buf.clear();
	m_buf << uint8(0x03);
	
	uint16 viewId = 0;
	try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,TheCharacter);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();		
	}

	m_buf << uint16(viewId);
	m_buf << uint8(0x02);
	m_buf << uint8(0x80);
	m_buf << uint8(0x81);
	vector<byte> rsiBuf(15,0);

	PlayerObject *player = NULL;
	try
	{
		player = sObjMgr.getGOPtr(TheCharacter);
	}
	catch (ObjectMgr::ObjectNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}

	player->getRsiData(&rsiBuf[0],rsiBuf.size());
	m_buf.append(rsiBuf);
	m_buf << uint16(0); //nomoreattribs
	return m_buf;
}*/

StateUpdateMsg::StateUpdateMsg( CharacterModel* MessageCharacter, ByteBuffer stateData ) :ObjectUpdateMsg(MessageCharacter)
{
	restOfData=stateData;
}

StateUpdateMsg::~StateUpdateMsg()
{

}

void StateUpdateMsg::setStateData ( byte* NewStateData )
{
	restOfData.clear () ;
	restOfData.append ( NewStateData , sizeof ( NewStateData ) ) ;
}

const ByteBuffer& StateUpdateMsg::toBuf()
{
	m_buf.clear();

/*	PlayerObject *m_player = NULL;
	try
	{
		m_player = sObjMgr.getGOPtr(TheCharacter);
	}
	catch (ObjectMgr::ObjectNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}*/
	uint16 goId = TheCharacter->GetGOId () ;//TheCharacter->GetViewID () ;
	/*try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,TheCharacter);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();		
	}*/


	m_buf << uint8(0x03);
	m_buf << uint16(goId);
	m_buf.append(restOfData.contents(),restOfData.size());
	return m_buf;
}

/*map<uint32,uint8> EmoteMsg::m_emotesMap;

EmoteMsg::EmoteMsg(uint32 objectId, uint32 emoteId, uint8 emoteCount):ObjectUpdateMsg(objectId)
{
	m_emoteCount=emoteCount;
	if (m_emotesMap.empty())
	{
		m_emotesMap[swap32(0xe6020058)]=0x01; // /beckon
		m_emotesMap[swap32(0x9b010058)]=0x02; // /bigwave
		m_emotesMap[swap32(0xe7020058)]=0x03; // /bow
		m_emotesMap[swap32(0x0d00003a)]=0x04; // /clap
		m_emotesMap[swap32(0x1000003a)]=0x05; // /crossarms
		m_emotesMap[swap32(0x1100003a)]=0x06; // /nod
		m_emotesMap[swap32(0x570d0058)]=0x07; // /agree
		m_emotesMap[swap32(0x590d0058)]=0x08; // /yes
		m_emotesMap[swap32(0x1600003a)]=0x09; // /orangutan
		m_emotesMap[swap32(0x10050004)]=0x0a; // /point
		m_emotesMap[swap32(0xd1020058)]=0x0b; // /pointback
		m_emotesMap[swap32(0x1300003a)]=0x0c; // /pointleft
		m_emotesMap[swap32(0x1400003a)]=0x0d; // /pointright
		m_emotesMap[swap32(0xf50c0058)]=0x0e; // /pointup
		m_emotesMap[swap32(0xfa0c0058)]=0x0f; // /pointdown
		m_emotesMap[swap32(0x0e00003a)]=0x10; // /salute
		m_emotesMap[swap32(0x1200003a)]=0x11; // /shakehead
		m_emotesMap[swap32(0x580d0058)]=0x12; // /disagree
		m_emotesMap[swap32(0x0d00003e)]=0x13; // /no
		m_emotesMap[swap32(0xe9020058)]=0x14; // /stomp
		m_emotesMap[swap32(0x0f00003a)]=0x15; // /tapfoot
		m_emotesMap[swap32(0x0c00003a)]=0x16; // /wave
		m_emotesMap[swap32(0xef0c0058)]=0x17; // /dangerarea
		m_emotesMap[swap32(0x130e0058)]=0x18; // /comeforward
		m_emotesMap[swap32(0xf00c0058)]=0x19; // /enemyinsight
		m_emotesMap[swap32(0xf00c0058)]=0x1a; // /enemy
		m_emotesMap[swap32(0xf20c0058)]=0x1b; // /disperse
		m_emotesMap[swap32(0x1500003a)]=0x1c; // /lookaround
		m_emotesMap[swap32(0xf60c0058)]=0x1d; // /takecover
		m_emotesMap[swap32(0xf70c0058)]=0x1e; // /cover
		m_emotesMap[swap32(0xf30c0058)]=0x1f; // /mapcheck
		m_emotesMap[swap32(0xf80c0058)]=0x20; // /onehandedhandstand
		m_emotesMap[swap32(0xfb0c0058)]=0x21; // /giggle
		m_emotesMap[swap32(0xf40c0058)]=0x22; // /handstand
		m_emotesMap[swap32(0xfc0c0058)]=0x23; // /hearnoevil
		m_emotesMap[swap32(0xfd0c0058)]=0x24; // /seenoevil
		m_emotesMap[swap32(0xfe0c0058)]=0x25; // /speaknoevil
		m_emotesMap[swap32(0x000d0058)]=0x26; // /coverears
		m_emotesMap[swap32(0x010d0058)]=0x27; // /covermouth
		m_emotesMap[swap32(0xff0c0058)]=0x28; // /covereyes
		m_emotesMap[swap32(0x130d0058)]=0x29; // /blowkiss
		m_emotesMap[swap32(0x1f0d0058)]=0x2a; // /blush
		m_emotesMap[swap32(0x270d0058)]=0x2b; // /cheer
		m_emotesMap[swap32(0x650d0058)]=0x2c; // /crackknuckles
		m_emotesMap[swap32(0x650d0058)]=0x2d; // /crackknuckles
		m_emotesMap[swap32(0x7a0d0058)]=0x2e; // /cry
		m_emotesMap[swap32(0x7b0d0058)]=0x2f; // /curtsey
		m_emotesMap[swap32(0x7c0d0058)]=0x30; // /formalbow
		m_emotesMap[swap32(0x7d0d0058)]=0x31; // /formalcurtsey
		m_emotesMap[swap32(0x810d0058)]=0x32; // /bowhead
		m_emotesMap[swap32(0x810d0058)]=0x33; // /bowhead
		m_emotesMap[swap32(0x7e0d0058)]=0x34; // /insult
		m_emotesMap[swap32(0xa9150058)]=0x35; // /scream
		m_emotesMap[swap32(0xa5150058)]=0x36; // /anguish
		m_emotesMap[swap32(0x820d0058)]=0x37; // /karatepower
		m_emotesMap[swap32(0xb90d0058)]=0x38; // /karatepower2
		m_emotesMap[swap32(0xba0d0058)]=0x39; // /karatepower3
		m_emotesMap[swap32(0x830d0058)]=0x3a; // /karatespeed
		m_emotesMap[swap32(0x840d0058)]=0x3b; // /karatespeed2
		m_emotesMap[swap32(0x850d0058)]=0x3c; // /karatespeed3
		m_emotesMap[swap32(0xbb0d0058)]=0x3d; // /karatedefense
		m_emotesMap[swap32(0xbc0d0058)]=0x3e; // /karatedefense2
		m_emotesMap[swap32(0xbd0d0058)]=0x3f; // /karatedefense3
		m_emotesMap[swap32(0x870d0058)]=0x40; // /kneel
		m_emotesMap[swap32(0x880d0058)]=0x41; // /takeaknee
		m_emotesMap[swap32(0x890d0058)]=0x42; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x43; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x44; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x45; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x46; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x47; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x48; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x49; // /kungfu
		m_emotesMap[swap32(0x890d0058)]=0x4a; // /kungfu
		m_emotesMap[swap32(0xbe0d0058)]=0x4b; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x4c; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x4d; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x4e; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x4f; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x50; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x51; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x52; // /aikido
		m_emotesMap[swap32(0xbe0d0058)]=0x53; // /aikido
		m_emotesMap[swap32(0x920d0058)]=0x54; // /laugh
		m_emotesMap[swap32(0x7f0d0058)]=0x55; // /rude
		m_emotesMap[swap32(0xa40d0058)]=0x56; // /loser
		m_emotesMap[swap32(0x010000b4)]=0x57; // /bigtrouble
		m_emotesMap[swap32(0x940d0058)]=0x58; // /okay
		m_emotesMap[swap32(0x940d0058)]=0x59; // /ok
		m_emotesMap[swap32(0x960d0058)]=0x5a; // /peace
		m_emotesMap[swap32(0x770d0058)]=0x5b; // /pullhair
		m_emotesMap[swap32(0x970d0058)]=0x5c; // /rolldice
		m_emotesMap[swap32(0x780d0058)]=0x5d; // /sarcasticclap
		m_emotesMap[swap32(0x790d0058)]=0x5e; // /golfclap
		m_emotesMap[swap32(0x980d0058)]=0x5f; // /scratchhead
		m_emotesMap[swap32(0x990d0058)]=0x60; // /shrug
		m_emotesMap[swap32(0x9a0d0058)]=0x62; // /stretch
		m_emotesMap[swap32(0xf90c0058)]=0x63; // /suckitdown
		m_emotesMap[swap32(0x9b0d0058)]=0x64; // /surrender
		m_emotesMap[swap32(0x9c0d0058)]=0x65; // /thumbsup
		m_emotesMap[swap32(0x1200003a)]=0x66; // /shakehead
		m_emotesMap[swap32(0xa20d0058)]=0x67; // /shameshame
		m_emotesMap[swap32(0x9d0d0058)]=0x68; // /twothumbsup
		m_emotesMap[swap32(0x9f0d0058)]=0x69; // /puke
		m_emotesMap[swap32(0x9e0d0058)]=0x6a; // /vomit
		m_emotesMap[swap32(0xa00d0058)]=0x6b; // /whistle
		m_emotesMap[swap32(0xa60d0058)]=0x6c; // /grovel
		m_emotesMap[swap32(0xe1020058)]=0x6d; // /yawn
		m_emotesMap[swap32(0xa70d0058)]=0x6e; // /plead
		m_emotesMap[swap32(0xa50d0058)]=0x6f; // /shakefist
		m_emotesMap[swap32(0x4e0200b4)]=0x70; // /cool
		m_emotesMap[swap32(0xb70d0058)]=0x71; // /crackneck
		m_emotesMap[swap32(0x150e0058)]=0x72; // /assemble
		m_emotesMap[swap32(0x880200b4)]=0x73; // /mockcry
		m_emotesMap[swap32(0xa70200b4)]=0x74; // /throat
		m_emotesMap[swap32(0x9b0200b4)]=0x75; // /powerpose
		m_emotesMap[swap32(0xbf0200b4)]=0x76; // /thumbsdown
		m_emotesMap[swap32(0xcc0200b4)]=0x77; // /twothumbsdown
		m_emotesMap[swap32(0x250e0058)]=0x78; // /taunt
		m_emotesMap[swap32(0x1f0e0058)]=0x79; // /moveout
		m_emotesMap[swap32(0x1f0e0058)]=0x7a; // /move
		m_emotesMap[swap32(0x1d0e0058)]=0x7b; // /iamready
		m_emotesMap[swap32(0x1b0e0058)]=0x7c; // /rdy
		m_emotesMap[swap32(0x190e0058)]=0x7d; // /ready
		m_emotesMap[swap32(0x0d000080)]=0x7e; // /stop
		m_emotesMap[swap32(0x0d0e0058)]=0x7f; // /bigcheer
		m_emotesMap[swap32(0xd80200b4)]=0x80; // /whoa
		m_emotesMap[swap32(0xd5140058)]=0x81; // /talkrelieved
		m_emotesMap[swap32(0x17140058)]=0x82; // /talk1
		m_emotesMap[swap32(0x19140058)]=0x83; // /talk2
		m_emotesMap[swap32(0x1b140058)]=0x84; // /talk3
		m_emotesMap[swap32(0x1d140058)]=0x85; // /talkangry
		m_emotesMap[swap32(0x21140058)]=0x86; // /talkforceful
		m_emotesMap[swap32(0x1f140058)]=0x87; // /talkexcited
		m_emotesMap[swap32(0x23140058)]=0x88; // /talkscared
		m_emotesMap[swap32(0xd1140058)]=0x89; // /talkchuckle
		m_emotesMap[swap32(0xd3140058)]=0x8a; // /talkhurt
		m_emotesMap[swap32(0xd5140058)]=0x8b; // /talkrelieved
		m_emotesMap[swap32(0xe3150058)]=0x8c; // /talknegative
		m_emotesMap[swap32(0xdd150058)]=0x8d; // /talkpuzzled
		m_emotesMap[swap32(0xd9140058)]=0x8e; // /talkwhisperobvious
		m_emotesMap[swap32(0xdb150058)]=0x8f; // /talkgroup
		m_emotesMap[swap32(0xd7150058)]=0x90; // /talkflirtatious
		m_emotesMap[swap32(0xd9150058)]=0x91; // /talkaffirmative
		m_emotesMap[swap32(0xe1140058)]=0x92; // /overheat
		m_emotesMap[swap32(0xdd140058)]=0x93; // /thewave
		m_emotesMap[swap32(0xdb140058)]=0x94; // /snake
		m_emotesMap[swap32(0xdf140058)]=0x95; // /tsuj
		m_emotesMap[swap32(0x27140058)]=0x96; // /touchearpiece
		m_emotesMap[swap32(0xe1140058)]=0x97; // /overheat
		m_emotesMap[swap32(0x51160058)]=0x98; // /backflop
		m_emotesMap[swap32(0x55160058)]=0x99; // /backflop1
		m_emotesMap[swap32(0x53160058)]=0x9a; // /backflop2
		m_emotesMap[swap32(0x57160058)]=0x9b; // /ballet
		m_emotesMap[swap32(0x59160058)]=0x9c; // /bang
		m_emotesMap[swap32(0x5b160058)]=0x9d; // /cutitout
		m_emotesMap[swap32(0x5d160058)]=0x9e; // /giddyup
		m_emotesMap[swap32(0x5f160058)]=0x9f; // /horns
		m_emotesMap[swap32(0x61160058)]=0xa0; // /mimewall
		m_emotesMap[swap32(0x63160058)]=0xa1; // /mimeelbow
		m_emotesMap[swap32(0x67160058)]=0xa2; // /mimerope
		m_emotesMap[swap32(0x65160058)]=0xa3; // /picknose
		m_emotesMap[swap32(0x6b160058)]=0xa4; // /duh
		m_emotesMap[swap32(0x6d160058)]=0xa5; // /timeout
		m_emotesMap[swap32(0x6f160058)]=0xa6; // /whichway
		m_emotesMap[swap32(0x820200b4)]=0xa9; // /kickdoor
		m_emotesMap[swap32(0x710200b4)]=0xaa; // /examine
		m_emotesMap[swap32(0x8e0200b4)]=0xac; // /pickup
		m_emotesMap[swap32(0xb30200b4)]=0xad; // /takepill
		m_emotesMap[swap32(0x0b0000e0)]=0xaf; // /cough
		m_emotesMap[swap32(0xa10200b4)]=0xb0; // /righton
		m_emotesMap[swap32(0x0c0000e0)]=0xb1; // /sleep
		m_emotesMap[swap32(0xea020058)]=0xb2; // /tiphat
		m_emotesMap[swap32(0xa10000b4)]=0xb3; // /confused
		m_emotesMap[swap32(0x000d0058)]=0xb4; // /coverears
		m_emotesMap[swap32(0x760200b4)]=0xb5; // /eyedrops
		m_emotesMap[swap32(0x8e0200b4)]=0xb6; // /pickup
		m_emotesMap[swap32(0x0e050004)]=0xba; // /talkdepressed
		m_emotesMap[swap32(0x0d050004)]=0xbb; // /throw
		m_emotesMap[swap32(0xc50200b4)]=0xbc; // /toss
		m_emotesMap[swap32(0x4b00003a)]=0xbe; // /shakehands
		m_emotesMap[swap32(0xe4020058)]=0xc1; // /slap
		m_emotesMap[swap32(0xab0d0058)]=0xc7; // /dogsniff
		m_emotesMap[swap32(0xfd0d0058)]=0xca; // /hug
		m_emotesMap[swap32(0xca0d0058)]=0xcd; // /weddingkiss
		m_emotesMap[swap32(0xff0d0058)]=0xd0; // /holdbothhands
		m_emotesMap[swap32(0xfb0d0058)]=0xd3; // /kissthering
		m_emotesMap[swap32(0xf50d0058)]=0xd6; // /manhug
		m_emotesMap[swap32(0x050e0058)]=0xd9; // /pound
		m_emotesMap[swap32(0x030e0058)]=0xdc; // /weddingring
		m_emotesMap[swap32(0x010e0058)]=0xdf; // /propose
		m_emotesMap[swap32(0xc5140058)]=0xe2; // /dap
		m_emotesMap[swap32(0xfb0d0058)]=0xe5; // /kiss
		m_emotesMap[swap32(0x05160058)]=0xe8; // /weddingcake
		m_emotesMap[swap32(0x59160058)]=0xee; // /bangbang
	}
	if (m_emotesMap.find(emoteId) != m_emotesMap.end() )
		m_emoteAnimation=m_emotesMap[emoteId];
	else
		m_emoteAnimation=0;
}

EmoteMsg::~EmoteMsg()
{

}

const ByteBuffer& EmoteMsg::toBuf()
{
	byte sampleEmoteMsg[] =
	{
		0x03, 0x02, 0x00, 0x01, 0x28, 0xAA, 0x40, 0x00, 0x25, 0x01, 0x00, 0x00, 0x10, 0xBB, 0xBB, 0xBB, 
		0xBB, 0xCC, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xDD, 0x2A, 0x9F, 0x1E, 0x20, 0x00, 0x00, 
	} ;

	sampleEmoteMsg[5] = m_emoteCount;

	if (m_emoteAnimation>0)
	{
		sampleEmoteMsg[9]=m_emoteAnimation;
	}
	else
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}

	PlayerObject *m_player = NULL;
	try
	{
		m_player = sObjMgr.getGOPtr(TheCharacter);
	}
	catch (ObjectMgr::ObjectNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}
	m_player->getPosition().toFloatBuf(&sampleEmoteMsg[0x0D],sizeof(float)*3);
	uint16 viewId = 0;
	try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,TheCharacter);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();		
	}
	memcpy(&sampleEmoteMsg[1],&viewId,sizeof(viewId));

	m_buf.clear();
	m_buf.append(sampleEmoteMsg,sizeof(sampleEmoteMsg));
	return m_buf;
}*/

AnimationStateMsg::AnimationStateMsg( CharacterModel* MessageCharacter ):ObjectUpdateMsg(MessageCharacter)
{
	
}

AnimationStateMsg::~AnimationStateMsg()
{

}

const ByteBuffer& AnimationStateMsg::toBuf()
{
	byte sampleAnimationBuf[9] =
	{
		0x03, 0x02, 0x00, 0x01, 0x01, 0xAA, 0xBB, 0x00, 0x00, 
	} ;

	//sampleAnimationBuf[5] = TheCharacter->GetCurrentAnimation();
	//sampleAnimationBuf[6] = TheCharacter->GetCurrentMood();

	uint16 goId = TheCharacter->GetGOId () ;
	memcpy(&sampleAnimationBuf[1],&goId,sizeof(goId));

	m_buf.clear();
	m_buf.append(sampleAnimationBuf,sizeof(sampleAnimationBuf));
	return m_buf;
}


RunAnimationCodeMsg::RunAnimationCodeMsg ( CharacterModel* MessageCharacter , const byte AnimationCode [ CODE_LENGTH ] ) : ObjectUpdateMsg ( MessageCharacter )
{
	TheCode[0] = AnimationCode[0] ;
	TheCode[1] = AnimationCode[1] ;
	TheCode[2] = AnimationCode[2] ;
}

RunAnimationCodeMsg::~RunAnimationCodeMsg ()
{

}

const ByteBuffer& RunAnimationCodeMsg::toBuf()
{
	byte playerAnimationBuf[] =
	{         // GO ID                // Animation order
		0x03, 0x02, 0x00, 0x01, 0x28, 0x00, 0x40, 0x00,
		0x00, 0x00, 0x00, // The Animation
		0x00, 0x01,
		// Position
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x9f, 0x1e, 0x20, 0x00, 0x00
	} ;

	byte npcAnimationBuf[] =
	{         // GO ID                // Animation order
		0x03, 0x02, 0x00, 0x22, 0xEE, 0x00, 0x01, 0x0C,
		0x00, 0x00, 0x00, // The Animation
		0x00, 0x10, 0x02, 0x00, 0x88, 0x00, 0x0c, 0x00, 0x00, 0x80,
		0x80, 0x80, 0x80, 0x40,
		0x00, 0xF0,		// NPC Health
		0x00, 0x00
	} ;
	
	byte* animationMessage ;
	
	if ( TheCharacter->IsNPC () == true )
	{
		animationMessage = npcAnimationBuf ;
	}
	else
	{
		animationMessage = playerAnimationBuf ;
	}
	
	uint16* idPTR = ( uint16* )&animationMessage [ 1 ] ;
	*idPTR = TheCharacter->GetGOId () ;

	uint8* indexPTR = ( uint8* )&animationMessage [ 5 ] ;
	*indexPTR = TheCharacter->GetAnimationIndex () ;
	TheCharacter->IncrementAnimationIndex () ;
	
	byte* animationPtr = &animationMessage [ 8 ] ;
	memcpy ( animationPtr , TheCode , CODE_LENGTH ) ; 

	if ( TheCharacter->IsNPC () == false )
	{
		TheCharacter->GetPosition()->toFloatBuf ( &animationMessage[13] , sizeof ( float ) * 3 ) ;
	}

	m_buf.clear();
	if ( TheCharacter->IsNPC () == true )
	{
		m_buf.append(animationMessage,sizeof(npcAnimationBuf));
	}
	else
	{
		m_buf.append(animationMessage,sizeof(playerAnimationBuf));
	}

	return m_buf;
}


PositionStateMsg::PositionStateMsg( CharacterModel* MessageCharacter ):ObjectUpdateMsg( MessageCharacter )
{
}

PositionStateMsg::~PositionStateMsg()
{

}

const ByteBuffer& PositionStateMsg::toBuf()
{
	m_buf.clear();
	m_buf << uint8(0x03);

	/*PlayerObject *m_player = NULL;
	try
	{
		m_player = sObjMgr.getGOPtr(TheCharacter);
	}
	catch (ObjectMgr::ObjectNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}*/
	uint16 goId = TheCharacter->GetGOId () ;
/*	try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,TheCharacter);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();		
	}*/

	m_buf << uint16(goId);
	m_buf << uint8(1);

	m_buf << uint8(0x08); //pos update

	TheCharacter->GetPosition ()->toFloatBuf ( m_buf );

	return m_buf;
}

/*JackoutEffectMsg::JackoutEffectMsg( uint32 objectId, bool jackout ):ObjectUpdateMsg(objectId),m_jackout(jackout)
{
}

JackoutEffectMsg::~JackoutEffectMsg()
{

}

const ByteBuffer& JackoutEffectMsg::toBuf()
{
	unsigned char rawData[] =
	{
		0x03, 0x02, 0x00, 0x03, 0x28, 0x03, 0xC0, 0x00, 0x74, 0x00, 0x10, 0xA1, 0x4B, 0x36, 0x48, 0x00, 
		0x40, 0x62, 0xC4, 0x8A, 0x58, 0x43, 0x47, 0xBE, 0x65, 0x82, 0x21, 0x80, 0x80, 0x80, 0x80, 0x80, 
		0x80, 0x10, 0x01, 0x00, 0x00
	} ;

	PlayerObject *m_player = NULL;
	try
	{
		m_player = sObjMgr.getGOPtr(TheCharacter);
	}
	catch (ObjectMgr::ObjectNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();
	}
	uint16 viewId = 0;
	try
	{
		viewId = sObjMgr.getViewForGO(m_toWho,TheCharacter);
	}
	catch (ObjectMgr::ClientNotAvailable)
	{
		m_buf.clear();
		throw PacketNoLongerValid();		
	}

	memcpy(&rawData[0x01],&viewId,sizeof(viewId));
	m_player->getPosition().toFloatBuf(&rawData[0x0B],sizeof(float[3]));

//	memset(&rawData[0x17],0,sizeof(uint32));

	if (m_jackout)
		rawData[0x22] = 1;
	else
		rawData[0x22] = 0;

	m_buf = ByteBuffer(rawData,sizeof(rawData));
	return m_buf;
}*/

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

DoorAnimationMsg::DoorAnimationMsg( uint32 doorId, uint16 viewId, double X, double Y, double Z, double ROT, int doorType )
{
	Y += 50;

	double distanceToGo = 6; //This will be 1/2 of 1 unit
	double sAngle = sin(ROT);
	double xInc = distanceToGo * sAngle; 
	double zInc = sqrt(distanceToGo * distanceToGo - xInc * xInc);
	xInc *= 10;
	zInc *= 10;
	X -= xInc;
	if (abs(ROT) > M_PI/2)
	{
		Z += zInc;
	}
	else
	{
		Z -= zInc;
	}

	byte rotation = 0xbf;
	//Rotation
	if (abs(ROT) > 2.3) // facing North
	{
		rotation = 0x03; //0xbf;
	}
	else if (abs(ROT) < .78) // facing South
	{
		rotation = 0x03;  //gave up and am having u facing south hve doors open same as u facing north..
	}
	else if (ROT < 0) // facing west
	{
		rotation = 0x3f;
	}
	else 
	{
		rotation = 0xbf;  //facing east
	}

	/*

	03 01 00 08 da 19 19 00 50 30 ef cd ab 03 84 00 00 00 00 f2 04 35 bf 00 00 00 00 f3 04 35 3f 
	41 00 00 00 00 00 c1 06 41 00 00 00 00 00 b8 8a c0 00 00 00 00 80 da e8 40 34 08 00 00 03 00 00 
	00 00 

	*/
	//Get door type

	//int doorType = 1;

	if (doorType == 0) // inside (white hallsays slums)
	{
		byte insideDoor[65] =
		{
			0x03, 0x01, 0x00, 0x08, 0xda, 0x19, 

			0xAA, 0xAA, 0xAA, 0xAA, 

			0xef, 

			0xcd, 0xab, 0x03, 0x84, 0x00, 0x00, 0x00, 0x00, 
			0xf2, 0x04,	
			//rotatation?
			0x35, 0xbf, 

			0x00, 0x00, 0x00, 0x00, 
			//??        ??    
			0xf3, 0x04, 0x35, 
			//width 
			0x3f, 
			//?? (not 41 breaks door openin)
			0x41, 
			0x00, 0x00, 0x00, 0x00,0xBB, 0xBB, 0xBB, 0xBB,  //X
			0x00, 0x00, 0x00, 0x00,0xCC, 0xCC, 0xCC, 0xCC,  //Y
			0x00, 0x00, 0x00, 0x00,0xDD, 0xDD, 0xDD, 0xDD,  //Z

			0x34, 0x08, 0x00, 0x00, 
			0xBB, 0xBB, 0x00, 0x00, 0x00, 
		};
		//03 03 00 01 80 02 38 08 00 00 00 00 

		//uint2 clientDoorId = rand() % 255;
		//insideDoor[22] = rand() % 128 + 63;

		memcpy(&insideDoor[6],&doorId,sizeof(doorId));   //uint32
		memcpy(&insideDoor[60],&viewId,sizeof(viewId));   //uint16


		ByteBuffer bufferBytesCoords;
		bufferBytesCoords << double(X) << double(Y) << double(Z);

		byte byteCoords[24] = {
			0x00, 0x00, 0x00, 0x00,0x00, 0xc1, 0x06, 0x41,  //X
			0x00, 0x00, 0x00, 0x00,0x00, 0xb8, 0x8a, 0xc0,  //Y
			0x00, 0x00, 0x00, 0x00,0x80, 0xda, 0xe8, 0x40,  //Z			
		};
		//Put new coords in
		bufferBytesCoords.read(byteCoords, bufferBytesCoords.size());


		memcpy(&insideDoor[32],&byteCoords,sizeof(byteCoords));

		//Rotation
		insideDoor[22] = rotation;

		m_buf.clear();
		m_buf.append(insideDoor,sizeof(insideDoor));
	}
	else if (doorType == 1) //outside
	{
		byte OutsideDoor[65] =
		{
			0x03, 0x01, 0x00, 0x08, 0x33, 0x1b, 
			0xAA, 0xAA, 0xAA, 0xAA,  
			0xbc, 0xcd, 0xab, 0x03, 0x84, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x04, 0x35, 0xbf, 0x00, 0x00, 
			0x00, 0x00, 0xf3, 0x04, 0x35, 0x3f, 0x41, 0x00, 0x00, 0x00, 0x00, 0x20, 0xf4, 0xea, 0x40, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x62, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0xcd, 
			0x40, 0x34, 0x08, 0x00, 0x00, 0xBB, 0xBB, 0x00, 0x00, 0x00, 
		};

		memcpy(&OutsideDoor[6],&doorId,sizeof(doorId));   //uint32
		memcpy(&OutsideDoor[60],&viewId,sizeof(viewId));   //uint16


		ByteBuffer bufferBytesCoords;
		bufferBytesCoords << double(X) << double(Y) << double(Z);

		byte byteCoords[24] = {
			0x00, 0x00, 0x00, 0x00,0x00, 0xc1, 0x06, 0x41,  //X
			0x00, 0x00, 0x00, 0x00,0x00, 0xb8, 0x8a, 0xc0,  //Y
			0x00, 0x00, 0x00, 0x00,0x80, 0xda, 0xe8, 0x40,  //Z			
		};
		//Put new coords in
		bufferBytesCoords.read(byteCoords, bufferBytesCoords.size());

		memcpy(&OutsideDoor[32],&byteCoords,sizeof(byteCoords));   //uint16

		//Rotation
		OutsideDoor[22] = rotation;

		//ROT

		m_buf.clear();
		m_buf.append(OutsideDoor,sizeof(OutsideDoor));
	}
	//memcpy(&doorX[6],&doorId,sizeof(doorId));   //uint32
	//memcpy(&doorX[55],&viewId,sizeof(viewId));   //uint16
	//m_buf.append(doorX,sizeof(insideDoor));



	/*uint32 theDoorId = doorId;
	memcpy(&rawData[6],&theDoorId,sizeof(theDoorId));
	uint16 theViewId = viewId;
	memcpy(&rawData[0x2C],&theViewId,sizeof(theViewId));
	m_buf.clear();
	m_buf.append(rawData,sizeof(rawData));*/
}

DoorAnimationMsg::~DoorAnimationMsg()
{

}

WhereAmIResponse::WhereAmIResponse( const LocationVector &currPos )
{
	byte whereamipacket[] =
	{
		0x81, 0x54, 0x00, 0xB4, 0xC3, 0x46, 0x00, 0x80, 0xFC, 0xC3, 0x00, 0xE2, 0x01, 0x47, 0x07, 0x01,	0x00
	} ;

	currPos.toFloatBuf(&whereamipacket[2],sizeof(whereamipacket)-2);
	m_buf.clear();
	m_buf.append(whereamipacket,sizeof(whereamipacket));
}

WhereAmIResponse::~WhereAmIResponse()
{

}

WhisperMsg::WhisperMsg( string sender, string message )
{
	m_buf << uint16(swap16(0x2E11));
	m_buf << uint8(0);
	m_buf << uint32(0);

	size_t putSenderStrLenPosHere = m_buf.wpos();
	m_buf << uint32(0); //will overwrite this later
	size_t putMessageStrLenPosHere = m_buf.wpos();
	m_buf << uint32(0); //will overwrite this later

	//0x15 bytes of 0s
	for (int i=0;i<0x15;i++)
		m_buf << uint8(0);

	size_t senderStrLenPos = m_buf.wpos();

	sender = sGame.GetChatPrefix() + string("+") + sender;
	m_buf.writeString(sender);

	size_t messageStrLenPos = m_buf.wpos();
	m_buf.writeString(message);

	//go back and put positions there
	m_buf.put(putSenderStrLenPosHere,uint32(senderStrLenPos));
	m_buf.put(putMessageStrLenPosHere,uint32(messageStrLenPos));
}

WhisperMsg::~WhisperMsg()
{

}

/*PlayerDetailsMsg::PlayerDetailsMsg( PlayerObject *thePlayer )
{
	m_buf.clear();

	if (thePlayer == NULL)
		throw PacketNoLongerValid();

	m_buf << uint16(swap16(0x8193));

	m_buf << uint32(thePlayer->getClient().GetWorldCharId());

	size_t putHandleStrLenPosHere = m_buf.wpos();
	m_buf << uint16(0); //placeholder

	m_buf << uint32(0); //zero int

	size_t putFirstNameStrLenPosHere = m_buf.wpos();
	m_buf << uint16(0); //placeholder

	size_t putLastNameStrLenPosHere = m_buf.wpos();
	m_buf << uint16(0); //placeholder

	m_buf << uint32(300); //character trait	

	m_buf << uint8(thePlayer->getAlignment()); //organization

	size_t putCrewStrLenPosHere = m_buf.wpos();
	m_buf << uint16(0); //placeholder

	size_t putFactionStrLenPosHere = m_buf.wpos();
	m_buf << uint16(0); //placeholder

	m_buf << uint32(9001); //CQ points

	size_t handleStrLenPos = m_buf.wpos();
	m_buf.writeString(thePlayer->getHandle());
	size_t firstNameStrLenPos = m_buf.wpos();
	m_buf.writeString(thePlayer->getFirstName());
	size_t lastNameStrLenPos = m_buf.wpos();
	m_buf.writeString(thePlayer->getLastName());

	string crewName,factionName;
	size_t crewNameStrLenPos = m_buf.wpos();
	m_buf.writeString(crewName);
	size_t factionNameStrLenPos = m_buf.wpos();
	m_buf.writeString(factionName);

	//go back and put position there
	m_buf.put(putHandleStrLenPosHere,uint16(handleStrLenPos));
	m_buf.put(putFirstNameStrLenPosHere,uint16(firstNameStrLenPos));
	m_buf.put(putLastNameStrLenPosHere,uint16(lastNameStrLenPos));
	m_buf.put(putCrewStrLenPosHere,uint16(crewNameStrLenPos));
	m_buf.put(putFactionStrLenPosHere,uint16(factionNameStrLenPos));
}

PlayerDetailsMsg::~PlayerDetailsMsg()
{

}*/

LoadWorldCmd::LoadWorldCmd( mxoLocation theLoc,string theSky )
{
	m_buf.clear();
	locs[TUTORIAL] = "resource/worlds/final_world/tutorial_v2/tutorial_v2.metr";
	locs[SLUMS] = "resource/worlds/final_world/slums_barrens_full.metr";
	locs[DOWNTOWN] = "resource/worlds/final_world/downtown/dt_world.metr";
	locs[INTERNATIONAL] = "resource/worlds/final_world/international/it.metr";		
	locs[ARCHIVE01] = "resource/worlds/final_world/constructs/archive/archive01/archive01.metr";
	locs[ARCHIVE02] = "resource/worlds/final_world/constructs/archive/archive02/archive02.metr";
	locs[ASHENCOURT] = "resource/worlds/final_world/constructs/archive/archive_ashencourte/archive_ashencourte.metr";
	locs[DATAMINE] = "resource/worlds/final_world/constructs/archive/archive_datamine/datamine.metr";
	locs[SAKURA] = "resource/worlds/final_world/constructs/archive/archive_sakura/archive_sakura.metr";
	locs[SATI] = "resource/worlds/final_world/constructs/archive/archive_sati/sati.metr";
	locs[WIDOWSMOOR] = "resource/worlds/final_world/constructs/archive/archive_widowsmoor/archive_widowsmoor.metr";
	locs[YUKI] = "resource/worlds/final_world/constructs/archive/archive_yuki/archive_yuki.metr";
	locs[LARGE01] = "resource/worlds/final_world/constructs/large/large01/large01.metr";
	locs[LARGE02] = "resource/worlds/final_world/constructs/large/large02/large02.metr";
	locs[MEDIUM01] = "resource/worlds/final_world/constructs/medium/medium01/medium01.metr";
	locs[MEDIUM02] = "resource/worlds/final_world/constructs/medium/medium02/medium02.metr";
	locs[MEDIUM03] = "resource/worlds/final_world/constructs/medium/medium03/medium03.metr";
	locs[SMALL03] = "resource/worlds/final_world/constructs/small/small03/small03.metr";
	locs[CAVES] = "resource/worlds/final_world/zion_caves.metr";


	m_buf << uint16(swap16(0x060E))
		<< uint8(0)
		<< uint32(theLoc)
		<< float(sGame.GetSimTime())
		<< uint8(1);
	string metrFile = locs[theLoc];
	size_t putSkyStrLenOffsetHere = m_buf.wpos();
	m_buf << uint16(0); //will replace later
	m_buf.writeString(metrFile);
	size_t skyStrLenOffset = m_buf.wpos();
	m_buf.writeString(theSky);
	m_buf.put(putSkyStrLenOffsetHere,uint16(skyStrLenOffset));
}

LoadWorldCmd::~LoadWorldCmd()
{

}

const byte ChatMsg::OPCode [ ChatMsg::NUM_MESSAGE ] [ ChatMsg::OP_CODE_SIZE ] =
{
	{ 0x2E , 0x05 } ,
	{ 0x2E , 0x02 } ,
	{ 0x2E , 0x03 } ,
	{ 0x2E , 0x17 } ,
	{ 0x2E , 0xD7 } ,
	{ 0x2E , 0xC7 } ,
	{ 0x2E , 0x10 } ,
	{ 0x2E , 0x11 } ,
	{ 0x2E , 0x07 } ,
} ;

ChatMsg::ChatMsg ( CharacterModel* FromCharacter , std::string NewMessage , MessageTypes_T NewType )
{
	SourceCharacter = FromCharacter ;
	TheMessage = NewMessage ;
	MessageType = NewType ;
}

ChatMsg::~ChatMsg ()
{

}

const ByteBuffer& ChatMsg::toBuf ()
{
	byte sampleChatPacket[36] =
	{
		0x2e, 0x10, 0x00, 0xcc, 0xcf, 0x00, 0x00, 0x24, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	} ;

	string handle = "" ;

	if ( SourceCharacter != NULL )
	{
		handle = SourceCharacter->GetHandle () ;
	}

	byte* opPtr = ( byte* ) &sampleChatPacket[0] ;
	opPtr[0] = OPCode [ MessageType ] [ 0 ] ; 
	opPtr[1] = OPCode [ MessageType ] [ 1 ] ; 

		// Length of header data plus length of handle plus length of (?)3.
	uint16* headerLengthPTR = ( uint16* )&sampleChatPacket[11] ;
	*headerLengthPTR = 36 + handle.length () + 3 ;

	m_buf.clear () ;
	m_buf.append ( sampleChatPacket , sizeof ( sampleChatPacket ) ) ;

		// Add handle of sending character.
	m_buf.writeString ( handle ) ;

		// Add the message.
	m_buf.writeString ( TheMessage ) ;

	return m_buf;
}



StartPlayerRotateMsg::StartPlayerRotateMsg ( CharacterModel* RotatePlayeracter , TurnDir_T TurnDir ) : StateUpdateMsg ( RotatePlayeracter , NULL )
{
	byte startTurn[] =
	{
		            // Turn direction
		0x01, 0x02, 0x04, 0x00, 0x00
	} ;

		// 
	byte* directionPtr = ( byte* )&startTurn[2] ;
	*directionPtr = ( byte )TurnDir ;

	StateUpdateMsg::setStateData ( startTurn ) ;
}

StartPlayerRotateMsg::~StartPlayerRotateMsg ()
{

}

const ByteBuffer& StartPlayerRotateMsg::toBuf ()
{
	return StateUpdateMsg::toBuf () ;
}



UpdatePlayerRotateMsg::UpdatePlayerRotateMsg ( CharacterModel* RotatePlayeracter ) : StateUpdateMsg ( RotatePlayeracter , NULL )
{
	byte updateTurn[] =
	{
		            // Character Heading
		0x01, 0x04, 0xFF, 0x00, 0x00
	} ;

		// 
	byte* characterHeading = ( byte* )&updateTurn[2] ;
	*characterHeading = ( byte )RotatePlayeracter->GetPosition ()->getMxoRot () ;

	StateUpdateMsg::setStateData ( updateTurn ) ;
}

UpdatePlayerRotateMsg::~UpdatePlayerRotateMsg ()
{

}

const ByteBuffer& UpdatePlayerRotateMsg::toBuf ()
{
	return StateUpdateMsg::toBuf () ;
}



StopPlayerRotateMsg::StopPlayerRotateMsg ( CharacterModel* RotatePlayeracter ) : StateUpdateMsg ( RotatePlayeracter , NULL )
{
	byte stopTurn[] =
	{
		                  // Final Heading
		0x01, 0x06, 0x00, 0x5c, 0x00, 0x00
	} ;

		// 
	byte* characterHeading = ( byte* )&stopTurn[3] ;
	*characterHeading = ( byte )RotatePlayeracter->GetPosition ()->getMxoRot () ;

	StateUpdateMsg::setStateData ( stopTurn ) ;
}

StopPlayerRotateMsg::~StopPlayerRotateMsg ()
{

}

const ByteBuffer& StopPlayerRotateMsg::toBuf ()
{
	return StateUpdateMsg::toBuf () ;
}



StartPlayerStateMsg::StartPlayerStateMsg ( CharacterModel* TargetCharacter , State_T NewState ) : StateUpdateMsg ( TargetCharacter , NULL )
{
	byte startState[] =
	{
		            // State Type
		0x01, 0x02, 0x03, 0x00, 0x00,
	} ;

		// 
	byte* statePtr = ( byte* )&startState[2] ;
	*statePtr = ( byte )NewState ;

	StateUpdateMsg::setStateData ( startState ) ;
}

StartPlayerStateMsg::~StartPlayerStateMsg ()
{
}

const ByteBuffer& StartPlayerStateMsg::toBuf ()
{
	const ByteBuffer& startBuf = StateUpdateMsg::toBuf () ;
ERROR_LOG(format("Start:%1%")%Bin2Hex(startBuf));
	return startBuf ;
}



UpdatePlayerStateMsg::UpdatePlayerStateMsg ( CharacterModel* TargetCharacter ) : StateUpdateMsg ( TargetCharacter , NULL )
{
	byte updateState[] =
	{
		            // Update Position
		0x01, 0x08, 0xeb, 0x94, 0x89, 0x44, 0x1d, 0x42, 0x5d, 0x44, 0x9d, 0xa5, 0x86, 0x46, 0x00, 0x00,
	} ;

		// 
	TargetCharacter->GetPosition ()->toFloatBuf ( &updateState[2] , 15 ) ;

	StateUpdateMsg::setStateData ( updateState ) ;
}

UpdatePlayerStateMsg::~UpdatePlayerStateMsg ()
{

}

const ByteBuffer& UpdatePlayerStateMsg::toBuf ()
{
	return StateUpdateMsg::toBuf () ;
}


	// WTF is this message???????
StopPlayerStateMsg::StopPlayerStateMsg ( CharacterModel* TargetCharacter ) : StateUpdateMsg ( TargetCharacter , NULL )
{
	byte stopState[] =
	{
		                  // Final Position
		0x01, 0x0a, 0x00, 0xeb, 0x94, 0x89, 0x44, 0x1d, 0x42, 0x5d, 0x44, 0x9d, 0xa5, 0x86, 0x46, 0x00, 0x00,
	} ;

		// 
	TargetCharacter->GetPosition ()->toFloatBuf ( &stopState[3] , 15 ) ;

	StateUpdateMsg::setStateData ( stopState ) ;
}

StopPlayerStateMsg::~StopPlayerStateMsg ()
{

}

const ByteBuffer& StopPlayerStateMsg::toBuf ()
{
	return StateUpdateMsg::toBuf () ;
}



UpdateNPCStateMsg::UpdateNPCStateMsg ( CharacterModel* UpdateCharacter , State_T NewState )
{
		// 

	TheCharacter = UpdateCharacter ;
	TheState = NewState ;
}

UpdateNPCStateMsg::~UpdateNPCStateMsg ()
{

}

const ByteBuffer& UpdateNPCStateMsg::toBuf ()
{
	byte updateState[] =
	{
		            // 
		0x03, 0x00, 0x00, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00,
	} ;

	uint16* idPTR = ( uint16* )&updateState[1] ;
	*idPTR = TheCharacter->GetGOId () ;

	uint8* statePTR = ( uint8* )&updateState[5] ;
	*statePTR = TheState ;

	uint8* rotatePTR = ( uint8* )&updateState[6] ;
	*rotatePTR = TheCharacter->GetPosition ()->getMxoRot () ;

	TheCharacter->GetPosition ()->toFloatBuf ( &updateState[7] , 15 ) ;

		// Add the message.
	m_buf.clear();
	m_buf.append(updateState,sizeof(updateState));

	return m_buf;
}



ChangeNPCStateMsg::ChangeNPCStateMsg ( CharacterModel* UpdateCharacter , State_T NewState )
{
		// 

	TheCharacter = UpdateCharacter ;
	TheState = NewState ;
}

ChangeNPCStateMsg::~ChangeNPCStateMsg ()
{

}
/*What message really is this?*/
const ByteBuffer& ChangeNPCStateMsg::toBuf ()
{
	byte updateState[] =
	{
		            // 
		0x03, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	} ;

	uint16* idPTR = ( uint16* )&updateState[1] ;
	*idPTR = TheCharacter->GetGOId () ;

	uint8* statePTR = ( uint8* )&updateState[5] ;
	*statePTR = TheState ;

		// Add the message.
	m_buf.clear();
	m_buf.append(updateState,sizeof(updateState));

	return m_buf;
}



HealthUpdateMsg::HealthUpdateMsg ( CharacterModel* UpdateCharacter )
{
		// 

	TheCharacter = UpdateCharacter ;
}

HealthUpdateMsg::~HealthUpdateMsg ()
{

}

const ByteBuffer& HealthUpdateMsg::toBuf ()
{
	byte updateState[] =
	{
		      // GOId     // 2 - PC, 4 - NPC            // Inner Str
		0x03, 0x00, 0x00, 0x02, 0x80, 0x80, 0x80, 0x50, 0x00, 0x00,
		// Health
		0x00, 0x00, 0x00, 0x00
	} ;

	uint16* idPTR = ( uint16* )&updateState[1] ;
	*idPTR = TheCharacter->GetGOId () ;

	if ( TheCharacter->IsNPC () == true )
	{
		uint8* typePTR = ( uint8* )&updateState[3] ;
		*typePTR = 0x4 ;

		typePTR = ( uint8* )&updateState[7] ;
		*typePTR = 0x40 ;

		uint16* healthPTR = ( uint16* )&updateState[8] ;
		*healthPTR = TheCharacter->GetCurrentHealth () ;
INFO_LOG(format("Health:%1%")%*healthPTR);
	}
	else
	{

		uint16* strPTR = ( uint16* )&updateState[8] ;
		*strPTR = TheCharacter->GetCurrentIS () ;

		uint16* healthPTR = ( uint16* )&updateState[10] ;
		*healthPTR = TheCharacter->GetCurrentHealth () ;
INFO_LOG(format("Health:%1%")%*healthPTR);
	}

		// Add the message.
	m_buf.clear();
	m_buf.append(updateState,sizeof(updateState));

	return m_buf;
}



const byte MoodUpdateMsg::AnimationCodes[MoodUpdateMsg::NUM_MOODS] =
{
	0x00 ,
	0x0C ,
	0x0D ,
} ;

MoodUpdateMsg::MoodUpdateMsg ( CharacterModel* UpdateCharacter )
{
		// 

	TheCharacter = UpdateCharacter ;
}

MoodUpdateMsg::~MoodUpdateMsg ()
{

}

const ByteBuffer& MoodUpdateMsg::toBuf ()
{
	byte updateState[] =
	{
		      // GOId     // 3 - PC, 2 - NPC
		0x03, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
		                              // Mood
	} ;

	uint16* idPTR = ( uint16* )&updateState[1] ;
	*idPTR = TheCharacter->GetGOId () ;

	if ( TheCharacter->IsNPC () == true )
	{
		uint8* typePTR = ( uint8* )&updateState[3] ;
		*typePTR = 0x2 ;
	}

	byte* moodPTR = ( byte* )&updateState[5] ;
	*moodPTR = AnimationCodes[ ModelToMessageMood ( TheCharacter->GetMood () ) ] ;

		// Add the message.
	m_buf.clear();
	m_buf.append(updateState,sizeof(updateState));

	return m_buf;
}



ViewInterlockMsg::ViewInterlockMsg ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockId , unsigned InterlockIndex , byte CombatAnimation[2] )
{
		// 
	First = FirstCharacter ;
	Second = SecondCharacter ;
	TheId = InterlockId ;
	TheIndex = InterlockIndex ;
	TheAnimation[0] = CombatAnimation[0] ;
	TheAnimation[1] = CombatAnimation[1] ;
}

ViewInterlockMsg::~ViewInterlockMsg ()
{

}

const ByteBuffer& ViewInterlockMsg::toBuf ()
{
	byte spawnInterlock[] =
	{         //IL ID, 16 bits  // Position, 24 bytes
		0x03, 0x52, 0x01, 0x01, 0x66, 0x66, 0x66, 0x66, 0x66, 0xcb,
		0xcf, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7e, 0x40,
		                                                // Packet counter, 32 bits.
		0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x97, 0x40, 0x01, 0x00,
		            // 0x03, 32 bits.       Attacker ID, 16 bits
		0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x83, 0x00, 0x8d, 0x00,
		                                                // Attacker type, 16 bits.
		// Defender ID, 16 bits.7 bytes const.
		0x00, 0x80, 0x2f, 0x00, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00,
		            // Defender Type, 16 bits.
		      // Attacker distance to center? 5 bytes (missing some bytes in here?)
		0x00, 0x00, 0x00, 0x8c, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00,
		                                    // Defender distance to center? 5 bytes.
		0x00, 0x00, 0x00, 0x00, 0x00, 0x8c, 0xc2, 0x00, 0x00, 0x00,
		                              // 16 bit const.        // 5 byte duel counter "const"
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x23,
		                                          // Duel counter, 16 bit.
		                        // 22 bytes const.
		0x01, 0x00, 0x00, 0x64, 0xfc, 0xa0, 0x0f, 0xa0, 0x0f, 0x03,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8f, 0x58,
		                                    // Current animation, 16 bits.
		0x00, 0x00, 0x8b, 0x0b, 0x00, 0x24, 0x14, 0x52, 0x00, 0x00,
		                                                // 50 bytes const.

		0x8b, 0x0b, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x37, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x00, 0x00, 0x10,
		0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		                                          // Weapon 1, 32 bit.
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		      // Weapon 2, 32 bit.
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	} ;

	First->GetPosition ()->toDoubleBuf ( &spawnInterlock[4] , 24 ) ;

	uint16* IlIdPtr = ( uint16* )&spawnInterlock[1] ;
	*IlIdPtr = ( uint16 )TheId  ;

	uint32* indexPtr1 = ( uint32* )&spawnInterlock[28] ;
	*indexPtr1 = ( uint32 )TheIndex ;

	uint16* defenderIDPtr = ( uint16* )&spawnInterlock[36] ;
	uint16* defenderTypePtr = ( uint16* )&spawnInterlock[38] ;

	*defenderIDPtr = ( uint16 )Second->GetGOId () ;
	*defenderTypePtr = ( uint16 )Second->GetCharacterType () ;

	uint16* attackerIDPtr = ( uint16* )&spawnInterlock[40] ;
	uint16* attackerTypePtr = ( uint16* )&spawnInterlock[42] ;

	*attackerIDPtr = ( uint16 )First->GetGOId () ; ;
	*attackerTypePtr = ( uint16 )First->GetCharacterType () ;

	uint16* indexPtr2 = ( uint16* )&spawnInterlock[77] ;
	*indexPtr2 = ( uint16 )TheIndex ;

	byte* constPtr = ( byte* )&spawnInterlock[79] ;
	if ( TheIndex % 2 != 0 ) {
		*constPtr = 0x23 ;
		constPtr++ ;
		*constPtr = 0x01 ;
		constPtr++ ;
		*constPtr = 0x00 ;
		constPtr++ ;
		*constPtr = 0x00 ;
		constPtr++ ;
		*constPtr = 0x64 ;
	} else {
		*constPtr = 0x23 ;
		constPtr++ ;
		*constPtr = 0x02 ;
		constPtr++ ;
		*constPtr = 0x00 ;
		constPtr++ ;
		*constPtr = 0x00 ;
		constPtr++ ;
		*constPtr = 0x18 ;
	}

	uint16* animPTR = ( uint16* )&spawnInterlock[106] ;
	memcpy ( animPTR , TheAnimation , 2 ) ; 

		// Add the message.
	m_buf.clear();
	m_buf.append(spawnInterlock,sizeof(spawnInterlock));

	return m_buf;
}



SpawnInterlockMsg::SpawnInterlockMsg ( CharacterModel* FirstCharacter , unsigned InterlockId )
{
		// 
	First = FirstCharacter ;
	TheId = InterlockId ;
}

SpawnInterlockMsg::~SpawnInterlockMsg ()
{

}

const ByteBuffer& SpawnInterlockMsg::toBuf ()
{
	byte spawnInterlock[] =
	{         // 10 bytes const
		0x03, 0x01, 0x00, 0x0c, 0x37, 0x00, 0x38, 0xcd, 0xab, 0x02,
		      // Double position (24 bytes)
		0x05, 0x66, 0x66, 0x66, 0x66, 0x66, 0xcb, 0xcf, 0x40, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xf0, 0x7e, 0x40, 0x00, 0x00, 0x00,
		                              // Sim time, 32 bits    // Interlock id, 16 bits
		0x00, 0x00, 0xe0, 0x97, 0x40, 0x4d, 0x00, 0x00, 0x00, 0x52,
		      // 3 bytes of 0.
		0x01, 0x00, 0x00, 0x00,
	} ;

	First->GetPosition ()->toDoubleBuf ( &spawnInterlock[11] , 24 ) ;

	uint32* simTimePtr = ( uint32* )&spawnInterlock[35] ;
	*simTimePtr = ( uint32 )sGame.GetSimTime () ;

	uint16* interlockIdPtr = ( uint16* )&spawnInterlock[39] ;
	*interlockIdPtr = ( uint16 )TheId ;

		// Add the message.
	m_buf.clear();
	m_buf.append(spawnInterlock,sizeof(spawnInterlock));

	return m_buf;
}



DespawnInterlockMsg::DespawnInterlockMsg ( unsigned RemoveInterlock )
{
		// 
	TheInterlock = RemoveInterlock ;
}

DespawnInterlockMsg::~DespawnInterlockMsg ()
{

}

const ByteBuffer& DespawnInterlockMsg::toBuf ()
{
	byte despawnInterlock[] =
	{                                       // Interlock ID, 16 bits.
		0x03, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00,
	} ;

	uint16* interlockIdPtr = ( uint16* )&despawnInterlock[6] ;
	*interlockIdPtr = ( uint16 )TheInterlock ;

		// Add the message.
	m_buf.clear();
	m_buf.append(despawnInterlock,sizeof(despawnInterlock));

	return m_buf;
}



ResetInterlockMsg::ResetInterlockMsg ( unsigned InterlockID )
{
		// 
	ResetID = InterlockID ;
}

ResetInterlockMsg::~ResetInterlockMsg ()
{

}

const ByteBuffer& ResetInterlockMsg::toBuf ()
{
	byte spawnInterlock[] =
	{         //IL ID, 16 bits  // Position, 24 bytes
		0x03, 0x52, 0x01, 0x01, 0x66, 0x66, 0x66, 0x66, 0x66, 0xcb,
		0xcf, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7e, 0x40,
		                                                // Packet counter, 32 bits.
		0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x97, 0x40, 0xFF, 0xFF,
		            // 0x03, 32 bits.       Attacker ID, 16 bits
		0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x83, 0x00, 0x8d, 0x00,
		                                                // Attacker type, 16 bits.
		// Defender ID, 16 bits.7 bytes const.
		0x00, 0x80, 0x2f, 0x00, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00,
		            // Defender Type, 16 bits.
		      // Attacker distance to center? 5 bytes (missing some bytes in here?)
		0x00, 0x00, 0x00, 0x8c, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00,
		                                    // Defender distance to center? 5 bytes.
		0x00, 0x00, 0x00, 0x00, 0x00, 0x8c, 0xc2, 0x00, 0x00, 0x00,
		                              // 16 bit const.        // 5 byte duel counter "const"
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x23,
		                                          // Duel counter, 16 bit.
		                        // 22 bytes const.
		0x02, 0x00, 0x00, 0x18, 0xfc, 0xa0, 0x0f, 0xa0, 0x0f, 0x03,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8f, 0x58,
		                                    // Current animation, 16 bits.
		0x00, 0x00, 0x8b, 0x0b, 0x00, 0x24, 0x14, 0x52, 0x00, 0x00,
		                                                // 50 bytes const.

		0x8b, 0x0b, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x37, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x00, 0x00, 0x10,
		0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		                                          // Weapon 1, 32 bit.
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		      // Weapon 2, 32 bit.
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	} ;

	uint16* IlIdPtr = ( uint16* )&spawnInterlock[1] ;
	*IlIdPtr = ( uint16 )ResetID  ;

		// Add the message.
	m_buf.clear();
	m_buf.append(spawnInterlock,sizeof(spawnInterlock));

	return m_buf;
}
