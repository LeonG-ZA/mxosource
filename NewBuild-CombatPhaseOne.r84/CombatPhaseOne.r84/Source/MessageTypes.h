// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2006-2010 Rajko Stojadinovic
// Copyright (C) 2012 Michael Lingg
// Copyright (C) 2012 Arcady
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

#ifndef MXOSIM_MESSAGETYPES_H
#define MXOSIM_MESSAGETYPES_H

#include "Common.h"
#include "Util.h"
#include "Crypto.h"
#include "PlayerCharacterModel.h"
#include "NPCModel.h"

class MsgBaseClass
{
public:
	class PacketNoLongerValid {};

	MsgBaseClass() {m_buf.clear();}
	virtual ~MsgBaseClass() {}
	virtual const ByteBuffer& toBuf() = 0;
protected:
	ByteBuffer m_buf;
};

typedef shared_ptr<MsgBaseClass> msgBaseClassPtr;

class ObjectUpdateMsg : public MsgBaseClass
{
public:
	ObjectUpdateMsg(CharacterModel* MessageClient);
	~ObjectUpdateMsg();
protected:
	CharacterModel* TheCharacter;
};

/*class DeletePlayerMsg : public ObjectUpdateMsg
{
public:
	DeletePlayerMsg(uint32 objectId);
	~DeletePlayerMsg();
	const ByteBuffer& toBuf();
};*/

/*class CloseDoorMsg : public ObjectUpdateMsg
{
public:
	CloseDoorMsg(uint32 objectId );
	~CloseDoorMsg();
	const ByteBuffer& toBuf();
};*/

class PlayerSpawnMsg : public ObjectUpdateMsg
{
public:
	PlayerSpawnMsg(PlayerCharacterModel* MessageCharacter);
	~PlayerSpawnMsg();
	const ByteBuffer& toBuf();
protected:
	PlayerCharacterModel* TheCharacter;
};

class NPCSpawnMsg : public ObjectUpdateMsg
{
public:
	NPCSpawnMsg(NPCModel* MessageCharacter);
	~NPCSpawnMsg();
	const ByteBuffer& toBuf();
protected:
	NPCModel* TheCharacter;
};

/*class PlayerAppearanceMsg : public ObjectUpdateMsg
{
public:
	PlayerAppearanceMsg(uint32 objectId);
	~PlayerAppearanceMsg();
	const ByteBuffer& toBuf();
};*/

/*class EmoteMsg : public ObjectUpdateMsg
{
public:
	EmoteMsg(uint32 objectId, uint32 emoteId, uint8 emoteCount);
	~EmoteMsg();
	const ByteBuffer& toBuf();
private:
	uint8 m_emoteCount;
	static map<uint32,uint8> m_emotesMap;
	uint8 m_emoteAnimation;
};*/

class AnimationStateMsg : public ObjectUpdateMsg
{
public:
	AnimationStateMsg( CharacterModel* MessageCharacter );
	~AnimationStateMsg();
	const ByteBuffer& toBuf();
};

class RunAnimationCodeMsg : public ObjectUpdateMsg
{
public:
	static const unsigned CODE_LENGTH = 3 ;
	
	RunAnimationCodeMsg ( CharacterModel* MessageCharacter , const byte AnimationCode [ CODE_LENGTH ] ) ;
	~RunAnimationCodeMsg () ;
	const ByteBuffer& toBuf () ;
private:
	byte TheCode [ CODE_LENGTH ] ;
} ;

class PositionStateMsg : public ObjectUpdateMsg
{
public:
	PositionStateMsg( CharacterModel* MessageCharacter );
	~PositionStateMsg();
	const ByteBuffer& toBuf();
};

/*class JackoutEffectMsg : public ObjectUpdateMsg
{
public:
	JackoutEffectMsg(uint32 objectId, bool jackout=true);
	~JackoutEffectMsg();
	const ByteBuffer& toBuf();
private:
	bool m_jackout;
};*/

class StateUpdateMsg : public ObjectUpdateMsg
{
public:
	StateUpdateMsg(CharacterModel* MessageCharacter, ByteBuffer stateData);
	~StateUpdateMsg();
	const ByteBuffer& toBuf();
protected:
	void setStateData ( byte* NewStateData ) ;
private:
	ByteBuffer restOfData;
};

class StaticMsg : public MsgBaseClass
{
public:
	StaticMsg() {}
	StaticMsg(const ByteBuffer &inputBuf) {m_buf = inputBuf;}
	~StaticMsg() {}
	const ByteBuffer& toBuf() {return m_buf;}
};

class EmptyMsg : public StaticMsg
{
public:
	EmptyMsg() {m_buf.clear();}
	~EmptyMsg() {}
};

class DoorAnimationMsg : public StaticMsg
{
public:

	DoorAnimationMsg(uint32 doorId, uint16 viewId, double X, double Y, double Z, double ROT, int doorType);
	~DoorAnimationMsg();
};

class WhereAmIResponse : public StaticMsg
{
public:
	WhereAmIResponse(const class LocationVector &currPos);
	~WhereAmIResponse();
};

class WhisperMsg : public StaticMsg
{
public:
	WhisperMsg(string sender, string message);
	~WhisperMsg();
};

class SystemMsg : public StaticMsg
{
public:
	SystemMsg(uint16 opcode,string message)
	{
		m_buf.clear();

		m_buf << uint16(swap16(opcode));
		m_buf << uint8(0);
		m_buf << uint32(0);

		m_buf << uint32(0); //no sender
		size_t putMessageStrLenPosHere = m_buf.wpos();
		m_buf << uint32(0); //we will overwrite this later

		//0x15 bytes of 0s
		for (int i=0;i<0x15;i++)
			m_buf << uint8(0);

		size_t messageStrLenPos = m_buf.wpos();
		m_buf.writeString(message);

		//go back and put position there
		m_buf.put(putMessageStrLenPosHere,uint32(messageStrLenPos));
	}
	~SystemMsg() {}
};

class BroadcastMsg : public SystemMsg
{
public:
	BroadcastMsg(string message) : SystemMsg(0x2EC7,message) {}
	~BroadcastMsg() {}
};

class ModalMsg : public SystemMsg
{
public:
	ModalMsg(string message) : SystemMsg(0x2ED7,message) {}
	~ModalMsg() {}
};

class SystemChatMsg : public SystemMsg
{
public:
	SystemChatMsg(string message) : SystemMsg(0x2E07,message) {}
	~SystemChatMsg() {}
};

class PlayerChatMsg : public StaticMsg
{
public:
	PlayerChatMsg(string charHandle,string theChatMsg) 
	{
		m_buf.clear();
		
		m_buf << swap16(0x2E10);
		m_buf << uint8(0); //could be 1 as well ?
		m_buf << uint32(swap32(0x12610200)); //some id, different, ill just use something

		size_t injectHandleLenPosHere = m_buf.wpos();
		m_buf << uint32(0); //will come back here and overwrite later
		size_t injectMessageLenPosHere = m_buf.wpos();
		m_buf << uint32(0); //also placeholder

		//0x15 0s
		vector<byte> fifteen(0x15,0);
		m_buf.append(fifteen);

		size_t handleLenPos=m_buf.wpos();
		m_buf.writeString(charHandle);
		size_t messageLenPos = m_buf.wpos();
		m_buf.writeString(theChatMsg);

		//overwrite the pos we didnt before
		m_buf.put(injectHandleLenPosHere,uint32(handleLenPos));
		m_buf.put(injectMessageLenPosHere,uint32(messageLenPos));
	}
	~PlayerChatMsg() {}
};

class BackgroundResponseMsg : public StaticMsg
{
public:
	BackgroundResponseMsg(string playerBackground)
	{
		m_buf.clear();

		m_buf << uint16(swap16(0x8195));
		size_t insertBackgroundStrLenPosHere = m_buf.wpos();
		m_buf << uint16(0); //placeholder
		m_buf << uint8(0);
		size_t backgroundStrLenPos = m_buf.wpos();
		m_buf.writeString(playerBackground);

		//go back and put position there
		m_buf.put(insertBackgroundStrLenPosHere,uint16(backgroundStrLenPos));
	}
	~BackgroundResponseMsg() {}
};

/*class PlayerDetailsMsg : public StaticMsg
{
public:
	PlayerDetailsMsg(class PlayerObject *thePlayer);
	~PlayerDetailsMsg();
};*/

class PlayerBackgroundMsg : public StaticMsg
{
public:
	PlayerBackgroundMsg(string playerBackground)
	{
		m_buf.clear();

		m_buf << uint16(swap16(0x8198));
		size_t insertBackgroundStrLenPosHere = m_buf.wpos();
		m_buf << uint16(0); //placeholder
		m_buf << uint8(1);
		size_t backgroundStrLenPos = m_buf.wpos();
		m_buf.writeString(playerBackground);

		//go back and put position there
		m_buf.put(insertBackgroundStrLenPosHere,uint16(backgroundStrLenPos));
	}
	~PlayerBackgroundMsg() {}
};

class HexGenericMsg : public StaticMsg
{
public:
	HexGenericMsg(string hexadecimalData)
	{
		m_buf.clear();
		string output;
		CryptoPP::HexDecoder decoder;
		decoder.Attach( new CryptoPP::StringSink( output ) );
		decoder.Put( (const byte*)hexadecimalData.c_str(), hexadecimalData.length() );
		decoder.MessageEnd();
		m_buf.append(output);
	}
	~HexGenericMsg() {}
};

class LoadWorldCmd : public StaticMsg
{
public:
	typedef enum
	{
		TUTORIAL = 0x00,
		SLUMS = 0x01,
		DOWNTOWN = 0x02,
		INTERNATIONAL = 0x03,
		ARCHIVE01 = 0x04,
		ARCHIVE02 = 0x05,
		ASHENCOURT = 0x06,
		DATAMINE = 0x07,
		SAKURA = 0x08,
		SATI = 0x09,
		WIDOWSMOOR = 0x0A,
		YUKI = 0x0B,
		LARGE01 = 0x0C,
		LARGE02 = 0x0D,
		MEDIUM01 = 0x0E,
		MEDIUM02 = 0x0F,
		MEDIUM03 = 0x10,
		SMALL03 = 0x11,
		CAVES = 0x12,
	} mxoLocation;

	LoadWorldCmd(mxoLocation theLoc,string theSky);
	~LoadWorldCmd();
private:
	map<mxoLocation,string> locs;
};

class SetOptionCmd : public StaticMsg
{
public:
	SetOptionCmd(string optionName,bool booleanVal)
	{
		InitialSetUp(optionName,TYPE_BOOL);
		uint32 intermediaryVal = 2;
		if (booleanVal)
			intermediaryVal=1;

		m_buf << sizeof(intermediaryVal);
		m_buf << intermediaryVal;
	}
	SetOptionCmd(string optionName,uint16 shortIntVal)
	{
		InitialSetUp(optionName,TYPE_UINT16);

		m_buf << sizeof(shortIntVal);
		m_buf << shortIntVal;
	}
	SetOptionCmd(string optionName,uint32 intVal)
	{
		InitialSetUp(optionName,TYPE_UINT32);

		m_buf << sizeof(intVal);
		m_buf << intVal;
	}
	SetOptionCmd(string optionName,string stringVal)
	{
		InitialSetUp(optionName,TYPE_STRING);

		if (stringVal.length() < 1)
			m_buf << uint16(0);
		else
			m_buf.writeString(stringVal);
	}
	SetOptionCmd(string optionName,vector<string> stringVals)
	{
		InitialSetUp(optionName,TYPE_STRING);

		stringstream cummulative;

		foreach(const string &theStr,stringVals)
		{
			if (cummulative.str().length() > 1)
				cummulative << ",";

			cummulative << theStr;
		}

		string stringVal = cummulative.str();

		if (stringVal.length() < 1)
			m_buf << uint16(0);
		else
			m_buf.writeString(stringVal);
	}
	~SetOptionCmd() {}
private:
	typedef enum
	{
		TYPE_BOOL = 0x11,
		TYPE_UINT16 = 0x1B,
		TYPE_UINT32 = 0x17,
		TYPE_STRING = 0x19,
		TYPE_STRINGLIST = 0x14,
	} varType;
	void InitialSetUp(string optionName,varType theType)
	{
		m_buf.clear();
		m_buf << uint16(swap16(0x3A05));
		m_buf << uint8(0);
		m_buf << uint16(theType);
		m_buf.writeString(optionName);
	}
};

class SetExperienceCmd : public StaticMsg
{
public:
	SetExperienceCmd(uint64 theExp)
	{
		m_buf.clear();
		m_buf << uint16(swap16(0x80e5))
			<< uint64(theExp);
	}
	~SetExperienceCmd() {}
};

class SetInformationCmd : public StaticMsg
{
public:
	SetInformationCmd(uint64 theCash)
	{
		m_buf.clear();
		m_buf << uint16(swap16(0x80e4))
			<< uint64(theCash);
	}
	~SetInformationCmd() {}
};

class EventURLCmd : public StaticMsg
{
public:
	EventURLCmd(string eventURL)
	{
		m_buf.clear();
		//81 a5 00 00 07 00 05
		m_buf << uint16(swap16(0x81a5)) // can also be 81a9
			  << uint16(0)
			  << uint16(7)
			  << uint8(5);
		m_buf.writeString(eventURL);
	}
	~EventURLCmd() {}
};

struct MsgBlock 
{
	uint16 sequenceId;
	list<ByteBuffer> subPackets;

	bool FromBuffer(ByteBuffer &source)
	{
		{
			uint16 theId;
			if (source.remaining() < sizeof(theId))
				return false;

			source >> theId;
			sequenceId = swap16(theId);
		}
		uint8 numSubPackets;
		if (source.remaining() < sizeof(numSubPackets))
			return false;

		source >> numSubPackets;
		subPackets.clear();
		for (uint8 i=0;i<numSubPackets;i++)
		{
			uint8 firstTwoBytes[2];
			if (source.remaining() < sizeof(uint8))
				return false;
			source >> firstTwoBytes[0];

			int sizeOfPacketSize = 1;
			if (firstTwoBytes[0] > 0x7F)
			{
				sizeOfPacketSize = 2;
				firstTwoBytes[0] -= 0x80;

				if (source.remaining() < sizeof(uint8))
					return false;

				source >> firstTwoBytes[1];
			}
			uint16 subPacketSize = 0;
			if (sizeOfPacketSize == 1)
			{
				subPacketSize = firstTwoBytes[0];
			}
			else if (sizeOfPacketSize == 2)
			{
				memcpy(&subPacketSize,firstTwoBytes,sizeof(subPacketSize));
				subPacketSize = swap16(subPacketSize);
			}

			if (subPacketSize<1)
				return false;

			vector<byte> dataBuf(subPacketSize);
			if (source.remaining() < dataBuf.size())
				return false;

			source.read(&dataBuf[0],dataBuf.size());
			subPackets.push_back(ByteBuffer(&dataBuf[0],dataBuf.size()));
		}
		return true;
	}
	bool FromBuffer(const byte* buf,size_t size)
	{
		ByteBuffer derp;
		derp.append(buf,size);
		derp.wpos(0);
		derp.rpos(0);
		return FromBuffer(derp) && (derp.remaining() == 0);
	}
	void ToBuffer(ByteBuffer &destination)
	{
		destination << uint16(swap16(sequenceId));

		destination << uint8(subPackets.size());
		for (list<ByteBuffer>::iterator it=subPackets.begin();it!=subPackets.end();++it)
		{
			uint16 packetSize = it->size();
			if (packetSize > 0x7f)
			{
				packetSize = htons(packetSize | 0x8000);
				destination << uint16(packetSize);
			}
			else
			{
				destination << uint8(packetSize);
			}
			destination.append(it->contents(),it->size());
		}
	}
	uint32 GetTotalSize() const
	{
		uint32 startSize = sizeof(uint16)+sizeof(uint8);
		for (list<ByteBuffer>::const_iterator it=subPackets.begin();it!=subPackets.end();++it)
		{
			startSize += it->size();
		}		
		return startSize;
	}
};

class OrderedPacket : public StaticMsg
{
public:
	OrderedPacket() {m_buf.clear();}
	OrderedPacket(ByteBuffer &source)
	{
		m_buf.clear();
		FromBuffer(source);
	}
	OrderedPacket(const byte* buf,size_t size)
	{
		m_buf.clear();
		FromBuffer(buf,size);
	}
	OrderedPacket(MsgBlock justOneBlock)
	{
		m_buf.clear();
		msgBlocks.push_back(justOneBlock);
	}
	bool FromBuffer(ByteBuffer &source)
	{
		uint8 zeroFourId=0;
		if (source.remaining() < sizeof(zeroFourId))
			return false;

		source >> zeroFourId;

		if (zeroFourId != 0x04)
			return false;

		uint8 numOrderPackets=0;
		if (source.remaining() < sizeof(numOrderPackets))
			return false;

		source >> numOrderPackets;

		if (numOrderPackets < 1)
			return false;

		msgBlocks.clear();
		for (uint8 i=0;i<numOrderPackets;i++)
		{
			MsgBlock thePacket;
			if (thePacket.FromBuffer(source) == false)
				return false;

			msgBlocks.push_back(thePacket);
		}
		return true;
	}
	bool FromBuffer(const byte* buf,size_t size)
	{
		ByteBuffer derp;
		derp.append(buf,size);
		derp.wpos(0);
		derp.rpos(0);
		return FromBuffer(derp) && (derp.remaining()==0);
	}
private:
	void ToBuffer(ByteBuffer &destination)
	{
		destination << uint8(0x04);
		destination << uint8(msgBlocks.size());

		for (list<MsgBlock>::iterator it=msgBlocks.begin();it!=msgBlocks.end();++it)
		{
			it->ToBuffer(destination);
		}
	}
public:
	const ByteBuffer& toBuf()
	{
		m_buf.clear();
		ToBuffer(m_buf);
		return m_buf;
	}
	uint32 GetTotalSize() const
	{
		uint32 startSize = sizeof(uint8)*2;
		for (list<MsgBlock>::const_iterator it=msgBlocks.begin();it!=msgBlocks.end();++it)
		{
			startSize += it->GetTotalSize();
		}		
		return startSize;
	}
	list<MsgBlock> msgBlocks;
};

class ChatMsg : public MsgBaseClass
{
public:
	typedef enum MessageTypes_T
	{
		TEAM ,
		CREW ,
		FACTION ,
		MODAL ,
		FRAMEMODAL ,
		BROADCAST ,
		AREA ,
		WHISPER ,
		//POPUP_FRIENDLY ,?
		//POPUP_HOSTILE ,?
		SYSTEM ,
		NUM_MESSAGE ,
	} ;

	ChatMsg ( CharacterModel* FromCharacter , std::string NewMessage , MessageTypes_T NewType ) ;
	~ChatMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	static const unsigned OP_CODE_SIZE = 2 ;
	static const byte OPCode [ NUM_MESSAGE ] [ OP_CODE_SIZE ] ;

	CharacterModel* SourceCharacter ;
	std::string TheMessage ;
	MessageTypes_T MessageType ;
};

class MovementState
{
public :
	typedef enum TurnDir_T
	{
		LEFT = 4 ,
		RIGHT = 5 ,
	} ;

	typedef enum State_T
	{
		IDLE = 0 ,
		WALK = 1 ,
		WALK_BACKWARD = 2 ,
		RUN = 3 ,
		NUM_STATES ,
	} ;
} ;

class StartPlayerRotateMsg : public MovementState , public StateUpdateMsg
{
public:
	StartPlayerRotateMsg ( CharacterModel* RotatePlayeracter , MovementState::TurnDir_T TurnDir ) ;
	~StartPlayerRotateMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	MovementState::TurnDir_T Direction ;
};

class UpdatePlayerRotateMsg : public MovementState , public StateUpdateMsg
{
public:
	UpdatePlayerRotateMsg ( CharacterModel* RotatePlayeracter ) ;
	~UpdatePlayerRotateMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
};

class StopPlayerRotateMsg : public MovementState , public StateUpdateMsg
{
public:
	StopPlayerRotateMsg ( CharacterModel* RotatePlayeracter ) ;
	~StopPlayerRotateMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
};



class StartPlayerStateMsg : public MovementState , public StateUpdateMsg
{
public:
	StartPlayerStateMsg ( CharacterModel* TargetCharacter , MovementState::State_T NewState ) ;
	~StartPlayerStateMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	MovementState::State_T State ;
};

class UpdatePlayerStateMsg : public MovementState , public StateUpdateMsg
{
public:
	UpdatePlayerStateMsg ( CharacterModel* TargetCharacter ) ;
	~UpdatePlayerStateMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
};

class StopPlayerStateMsg : public MovementState , public StateUpdateMsg
{
public:
	StopPlayerStateMsg ( CharacterModel* TargetCharacter ) ;
	~StopPlayerStateMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
};

class UpdateNPCStateMsg : public MovementState , public MsgBaseClass
{
public:
	UpdateNPCStateMsg ( CharacterModel* UpdateCharacter , MovementState::State_T NewState ) ;
	~UpdateNPCStateMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	CharacterModel* TheCharacter ;
	MovementState::State_T TheState ;
};

class ChangeNPCStateMsg : public MovementState , public MsgBaseClass
{
public:
	ChangeNPCStateMsg ( CharacterModel* UpdateCharacter , MovementState::State_T NewState ) ;
	~ChangeNPCStateMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	CharacterModel* TheCharacter ;
	MovementState::State_T TheState ;
};

class HealthUpdateMsg : public MsgBaseClass
{
public:
	HealthUpdateMsg ( CharacterModel* UpdateCharacter ) ;
	~HealthUpdateMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	CharacterModel* TheCharacter ;
};

class MoodUpdateMsg : public MsgBaseClass
{
public:
	typedef enum Mood_T
	{
		MOOD1 ,
		MOOD2 ,
		DEAD ,
		NUM_MOODS ,
	} ;

	MoodUpdateMsg ( CharacterModel* UpdateCharacter ) ;
	~MoodUpdateMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	static const byte AnimationCodes[NUM_MOODS] ;

	CharacterModel* TheCharacter ;
};

class ViewInterlockMsg : public MsgBaseClass
{
public:
	ViewInterlockMsg ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockId , unsigned InterlockIndex , byte CombatAnimation[2] ) ;
	~ViewInterlockMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	CharacterModel* First ;
	CharacterModel* Second ;
	unsigned TheId ;
	unsigned TheIndex ;
	byte TheAnimation[2] ;
};

class SpawnInterlockMsg : public MsgBaseClass
{
public:
	SpawnInterlockMsg ( CharacterModel* FirstCharacter , unsigned InterlockId ) ;
	~SpawnInterlockMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	CharacterModel* First ;
	unsigned TheId ;
};

class DespawnInterlockMsg : public MsgBaseClass
{
public:
	DespawnInterlockMsg ( unsigned RemoveInterlock ) ;
	~DespawnInterlockMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	unsigned TheInterlock ;
};

class ResetInterlockMsg : public MsgBaseClass
{
public:
	ResetInterlockMsg ( unsigned InterlockID ) ;
	~ResetInterlockMsg ( void ) ;
	const ByteBuffer& toBuf ( void ) ;
protected:
	unsigned ResetID ;
};

#endif
