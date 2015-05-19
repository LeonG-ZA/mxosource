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

#include "Common.h"
#include "PlayerObject.h"
#include "Log.h"
#include "Database/Database.h"
#include "GameServer.h"
#include "GameClient.h"
#include "Config.h"

#include <boost/algorithm/string.hpp>
using boost::iequals;
using boost::erase_all;

#include "base64.h"
#include "irrxml/irrXML.h"
#include "irrxml/irrTypes.h"

#include <math.h>
#include <time.h>
#include "MersenneTwister.h"

#include "AnimationClass.h"

#define regioncount 18

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::SetNPC_SERIE(double distance, double quantity,string spawnMode,bool deleteAll,string RSI,string handle){

		if(deleteAll == true || lesig_is_spawning == true){
			for(int i = 0; i<quantity; i++){
				ByteBuffer test;
				test.append( make_shared<HexGenericMsg>("030100010100")->toBuf() );

				uint16 TURN_NPC = i+NPC_number;
				test << uint32(TURN_NPC);

				SendPacketToCmd(Bin2Hex(test));
			}
			lesig_is_spawning = false;
			return;
		}

		if(quantity == 0 || quantity > 40){
			return;
		}

		distance *= 100;

		uint16 quantity_players = 1; //if 0 can cause error
		uint16 som_levels = 0;
		vector<uint32> allObjects = sObjMgr.getAllGOIds();
		foreach(uint32 objId, allObjects){
			PlayerObject* player = NULL;
			try{
				player = sObjMgr.getGOPtr(objId);
			}catch (ObjectMgr::ObjectNotAvailable){
				continue;
			}
			if(player->m_handle != m_handle && player != NULL){
				quantity_players += 1;
				som_levels += player->m_lvl;
			}
		}

		uint16 med_level_npcs = 0;
		med_level_npcs = (int)(som_levels / quantity_players)+1; //quantity_players != 0

		double angle_of_spawn = 0;
		angle_of_spawn = (int)(360/quantity);

		ByteBuffer spawnNPC[100];
		ByteBuffer setNPCrot[100];
		ByteBuffer setNPCpos[100];

		for(int i = 0; i<quantity; i++){
			LocationVector pos; pos.y = this->getPosition().y;
			double s_sen = sin((angle_of_spawn * 3.14 * i)/180);
			double c_cos = cos((angle_of_spawn * 3.14 * i)/180);

			if(spawnMode == "spiral_positive"){
				distance = distance+25;
			}else if(spawnMode == "spiral_negative"){
				distance = distance-25;
			}else if(spawnMode == "circle_specular"){
				s_sen = sin(((angle_of_spawn+180) * 3.14 * i)/180);
				c_cos = cos(((angle_of_spawn+180) * 3.14 * i)/180);
			} 

			if(spawnMode == "elliptic"){
				s_sen = sin((angle_of_spawn * 3.14 * i)/180);
				pos.x = (this->getPosition().x) + (distance * c_cos );
				pos.z = (this->getPosition().z) + ((distance+200) * s_sen );
			}else{
				pos.x = (this->getPosition().x) + (distance * s_sen );
				pos.z = (this->getPosition().z) + (distance * c_cos );

				//string instr = "SetNPC(1, \"HOSTILE\", 100, 5, 500, 500, \"00000000\", "+ (format("%1%")% (pos.x/100) ).str() +", "+ (format("%1%")% (pos.y/100) ).str() +", "+ (format("%1%")% (pos.z/100) ).str() +", \"NONE\", false);";
				//INFO_LOG(instr);
			}

			spawnNPC[i].append( make_shared<HexGenericMsg>("0301000c57028Dcdab")->toBuf() );
			size_t set_a_here = spawnNPC[i].wpos();

			spawnNPC[i].writeString(handle);
			for(int j = 0; j<32-(int)handle.length(); j++){
				spawnNPC[i] << uint8(0);
			}
			spawnNPC[i].put(set_a_here, uint16(swap16(0x128e)) );
			spawnNPC[i].append( make_shared<HexGenericMsg>("10000022c501e80180c3"+RSI+"fa090028")->toBuf() );

			spawnNPC[i] << double(pos.x) << double(pos.y) << double(pos.z);
			spawnNPC[i].append( make_shared<HexGenericMsg>("c0018201a8")->toBuf() );

			spawnNPC[i] << uint8(med_level_npcs) << uint16(500) << uint16(swap16(0xb204)) << uint16(500);
			spawnNPC[i].append( make_shared<HexGenericMsg>("120800008126b900000213")->toBuf() );

			uint16 TURN_NPC = i+NPC_number;
			spawnNPC[i] << uint16( TURN_NPC ) << uint32(0); //spawn NPC

			setNPCrot[i] << uint8(0x03) << uint16( TURN_NPC ) << uint16(swap16(0x0204)) ;
			if(spawnMode == "spiral_positive"){
				setNPCrot[i] << uint8(( (angle_of_spawn/1.41)*i) +127);
			}else if(spawnMode == "circle_positive"){
				setNPCrot[i] << uint8(( (angle_of_spawn/1.41)*i ));
			}else if(spawnMode == "circle_negative"){
				setNPCrot[i] << uint8(( (angle_of_spawn/1.41)*i) -127);
			}else{
				setNPCrot[i] << uint8(( (angle_of_spawn/1.41)*i ));
			}
			setNPCrot[i] << uint32(0); //set rotation NPC

			setNPCpos[i] << uint8(0x03) << uint16( TURN_NPC ) << uint16(swap16(0x0208));
			setNPCpos[i] << float(pos.x) << float(pos.y) << float(pos.z) << uint8(0); //respawn at original location
		}

		foreach(uint32 objId, allObjects){
			PlayerObject* player = NULL;
			try{
				player = sObjMgr.getGOPtr(objId);
			}catch (ObjectMgr::ObjectNotAvailable){
				continue;
			}
			if(player->m_district == m_district && player != NULL){
				for(int i = 0; i<quantity; i++){
					player->SendPacketToMe( Bin2Hex(spawnNPC[i]) );
					player->SendPacketToMe( Bin2Hex(setNPCrot[i]) );
					player->SendPacketToMe( Bin2Hex(setNPCpos[i]) );
				}
			}
		}

	lesig_is_spawning = true;
}
void PlayerObject::SetNPC_inMission(string type, int id, int level, int hpM, string RSI, double x, double y, double z, string handle, bool DEFEATED){
	NPC_type[NPC_number] = type; 
	NPC_id[NPC_number] = id; 
	NPC_level[NPC_number] = level; 

	NPC_hpM[NPC_number] = hpM;

	NPC_RSI[NPC_number] = RSI;

	NPC_X[NPC_number] = x;
	NPC_Y[NPC_number] = y;
	NPC_Z[NPC_number] = z;

	NPC_HANDLE[NPC_number] = handle;

	ByteBuffer spawn_NPC;

	if(type != "NONE" && type != "IGNORE"){
		if(type == "FRIENDLY"){
			spawn_NPC.append( make_shared<HexGenericMsg>("0301000c57022ecdab")->toBuf() );
			size_t set_a_here = spawn_NPC.wpos();

			spawn_NPC.writeString(handle);
			for(int i = 0; i<32-(int)handle.length(); i++){
				spawn_NPC << uint8(0);
			}
			spawn_NPC.put(set_a_here, uint16(swap16(0x118e)) );

			spawn_NPC.append( make_shared<HexGenericMsg>("10000022c00180c9"+RSI+"01")->toBuf() );
			spawn_NPC << double(x*100) << double(y*100) << double(z*100);
			spawn_NPC.append( make_shared<HexGenericMsg>("8800d701028201a8")->toBuf() );

			spawn_NPC << uint8(level) << uint16( level*100 ) << uint16(swap16(0xb204)) << uint16( level*100 );
			spawn_NPC.append( make_shared<HexGenericMsg>("fe07000088010101")->toBuf() );
			spawn_NPC << uint16( id );
			spawn_NPC.append( make_shared<HexGenericMsg>("000000")->toBuf() );
		}else if(type == "FRIENDLY_noInteraction"){
			spawn_NPC.append( make_shared<HexGenericMsg>("0301000c5702f0cdab")->toBuf() );
			size_t set_a_here = spawn_NPC.wpos();

			spawn_NPC.writeString(handle);
			for(int i = 0; i<32-(int)handle.length(); i++){
				spawn_NPC << uint8(0);
			}
			spawn_NPC.put(set_a_here, uint16(swap16(0x128e)) );
			
			spawn_NPC.append( make_shared<HexGenericMsg>("10000022c47d0180d9"+RSI+"010a")->toBuf() );
			spawn_NPC << double(x*100) << double(y*100) << double(z*100);
			spawn_NPC.append( make_shared<HexGenericMsg>("cc02022603030183000188")->toBuf() );
			spawn_NPC << uint8(level);
			spawn_NPC.append( make_shared<HexGenericMsg>("a209fe0700000801")->toBuf() );
			spawn_NPC << uint16(id);
			spawn_NPC.append( make_shared<HexGenericMsg>("000000")->toBuf() );
		}else if(type == "HOSTILE"){
			spawn_NPC.append( make_shared<HexGenericMsg>("0301000c57028dcdab")->toBuf() );
			size_t set_a_here = spawn_NPC.wpos();

			spawn_NPC.writeString(handle);
			for(int i = 0; i<32-(int)handle.length(); i++){
				spawn_NPC << uint8(0);
			}
			spawn_NPC.put(set_a_here, uint16(swap16(0x14ae)) );
			
			spawn_NPC.append( make_shared<HexGenericMsg>("1000002200001000c4fb0180c1"+RSI)->toBuf() );
			spawn_NPC << double(x*100) << double(y*100) << double(z*100);
			spawn_NPC.append( make_shared<HexGenericMsg>("d00c00830c01a8")->toBuf() ); //<d00c0183> <- weapon
			spawn_NPC << uint8(level) << uint16( hpM ) << uint16(swap16(0xb204)) << uint16( hpM );

			string fx = "00000000";
			spawn_NPC.append( make_shared<HexGenericMsg>("060800008337ba0000"+fx+"020e")->toBuf() );
			spawn_NPC << uint16(id);
			spawn_NPC.append( make_shared<HexGenericMsg>("000000")->toBuf() );
		}else if(type == "AGENT"){
			spawn_NPC.append( make_shared<HexGenericMsg>("0301000c5702f3cdab")->toBuf() );
			size_t set_a_here = spawn_NPC.wpos();
			
			spawn_NPC.writeString(handle);
			for(int i = 0; i<32-(int)handle.length(); i++){
				spawn_NPC << uint8(0);
			}
			spawn_NPC.put(set_a_here, uint16(swap16(0x118e)) );

			spawn_NPC.append( make_shared<HexGenericMsg>("10000022c00180c1"+RSI)->toBuf() );
			spawn_NPC << double(x*100) << double(y*100) << double(z*100);
			spawn_NPC.append( make_shared<HexGenericMsg>("c001832a01aa04ff")->toBuf() );
			spawn_NPC << uint16( hpM ) << uint16(swap16(0xb202)) << uint16( hpM );
			spawn_NPC.append( make_shared<HexGenericMsg>("020800008122b6000002ff")->toBuf() );
			spawn_NPC << uint16(id);
			spawn_NPC.append( make_shared<HexGenericMsg>("000000")->toBuf() );
		}

		SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear();

		if(DEFEATED == true){
			spawn_NPC << uint8(0x03) << uint16(id);
			spawn_NPC.append( make_shared<HexGenericMsg>("02010d000000000000000000000000000000")->toBuf() );
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear(); //lie on ground

			spawn_NPC << uint8(0x03) << uint16(id);
			spawn_NPC.append( make_shared<HexGenericMsg>("04808080600100000000")->toBuf() );
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear(); //unavailable on click
		}
	}

	NPC_number++;
}
void PlayerObject::SetNPC_ContactMission(string type, int id, string RSI, double x, double y, double z, string handle, uint8 root){
	ByteBuffer spawn_NPC;
	spawn_NPC.append( make_shared<HexGenericMsg>("030100087819"+type+"a5 cdab053d")->toBuf() );

	spawn_NPC << double(x*100) << double(y*100) << double(z*100);

	spawn_NPC.append( make_shared<HexGenericMsg>( parseStringToHex(handle,"PACKETSCHEME_NO") )->toBuf() );
	for(int i = 0; i<32-(int)handle.length(); i++){
		spawn_NPC << uint8(0);
	}

	spawn_NPC.append( make_shared<HexGenericMsg>(RSI+"0a000000000000000000803f000000002ebd3bb3")->toBuf() );
	spawn_NPC << uint16( id );
	spawn_NPC.append( make_shared<HexGenericMsg>("000000")->toBuf() );

	SendPacketToMe( Bin2Hex(spawn_NPC) );
}
void PlayerObject::SetNPC_Free(string type, int id, int level, string RSI, double x, double y, double z, string handle, bool DEFEATED, int rotation, uint16 mood){
	ByteBuffer spawn_NPC;

	if(type != "NONE" && type != "IGNORE"){
		if(type == "FRIENDLY"){
			spawn_NPC.append( make_shared<HexGenericMsg>("0301000c5702f0cdab")->toBuf() );
			size_t set_a_here = spawn_NPC.wpos();

			spawn_NPC.writeString(handle);
			for(int i = 0; i<32-(int)handle.length(); i++){
				spawn_NPC << uint8(0);
			}
			spawn_NPC.put(set_a_here, uint16(swap16(0x128e)) );
			
			spawn_NPC.append( make_shared<HexGenericMsg>("10000022c47d0180d9"+RSI+"010a")->toBuf() );
			spawn_NPC << double(x*100) << double(y*100) << double(z*100);
			spawn_NPC.append( make_shared<HexGenericMsg>("cc02022603030183000188")->toBuf() );
			spawn_NPC << uint8(level);
			spawn_NPC.append( make_shared<HexGenericMsg>("a209fe0700000801")->toBuf() );
			spawn_NPC << uint16(id);
			spawn_NPC.append( make_shared<HexGenericMsg>("000000")->toBuf() );

			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear();

			spawn_NPC << uint8(0x03) << uint16(id) << uint16(swap16(0x0204)) << uint16(rotation) << uint16(0);
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear();

			spawn_NPC << uint8(0x03) << uint16(id) << uint16(swap16(0x0201)) << uint16(mood) << uint16(0);
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear();
		}else if(type == "NEO_RSI" || type == "DERBY_COURT_EFFECT"){
			spawn_NPC.append( make_shared<HexGenericMsg>("0301000c0c0007cdab279b81ff7800000000000000000000000000000000000000000000000000000000000000000000000ef84f7ab12000000000000000000000000000000000000000000000000000000000000000000d9a99193e850ad723402af8280a0c0000874300000800d681fff7005c8f423f0acfca01004e73002000000000000000000000000000000000000000000000000000000000000000000000c05bfad6400000000000907fc000000060b546dec0280afd01010000730075260200b0740000630B7E0E106A64D5EF007a83c00000f700dd")->toBuf() );

			if( type == "NEO_RSI"){
				y -= 0.6;
			}else if( type == "DERBY_COURT_EFFECT"){
				y -= 1.25;
			}

			spawn_NPC << double(x*100) << double(y*100) << double(z*100);
			spawn_NPC.append( make_shared<HexGenericMsg>("819032121931461290010081953ab7201c01110000007300")->toBuf() );
			spawn_NPC << uint16(id) << uint16(0);
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear();
			
			if( type == "NEO_RSI"){
				spawn_NPC << uint8(0x03) << uint16(m_goId) << uint8(0x02) << uint32(swap32(0x80808050)) << uint16(m_innerStrC) << uint16(m_healthC) << uint16(id) << uint8(0x02)  << uint16(swap16(0x8100)) << uint32(swap32(0x80804000)) << uint32(swap32(0x40000000));
				SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear();
			}

			spawn_NPC << uint8(0x03) << uint16(id) << uint16(swap16(0x0301)) <<uint8(0x0d) << uint32(0); 
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear();

			spawn_NPC << uint8(0x03) << uint16(id) << uint16(swap16(0x0104)) << uint16(rotation) << uint16(0);
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear();
		}

		if(DEFEATED == true){
			spawn_NPC << uint8(0x03) << uint16(id);
			spawn_NPC.append( make_shared<HexGenericMsg>("02010d000000000000000000000000000000")->toBuf() );
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear(); //lie on ground

			spawn_NPC << uint8(0x03) << uint16(id);
			spawn_NPC.append( make_shared<HexGenericMsg>("04808080600100000000")->toBuf() );
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear(); //unavailable on click
		}
	}
}
void PlayerObject::SetNPC(int district, string type, int id, int level, int hpM, int hpC, string RSI, double x, double y, double z, string handle, bool DEFEATED){
	NPC_district[NPC_number] = district; 
	NPC_type[NPC_number] = type; 
	NPC_id[NPC_number] = id; 
	NPC_level[NPC_number] = level; 

	NPC_hpM[NPC_number] = hpM;
	NPC_hpC[NPC_number] = hpC;

	NPC_RSI[NPC_number] = RSI;

	NPC_X[NPC_number] = x;
	NPC_Y[NPC_number] = y;
	NPC_Z[NPC_number] = z;

	NPC_HANDLE[NPC_number] = handle;

	ByteBuffer spawn_NPC;

	if(type != "NONE" && type != "IGNORE" && m_district == district){
		 if(type == "FRIENDLY_noInteraction"){
			spawn_NPC.append( make_shared<HexGenericMsg>("0301000c5702f0cdab")->toBuf() );
			size_t set_a_here = spawn_NPC.wpos();

			spawn_NPC.writeString(handle);
			for(int i = 0; i<32-(int)handle.length(); i++){
				spawn_NPC << uint8(0);
			}
			spawn_NPC.put(set_a_here, uint16(swap16(0x128e)) );
			
			spawn_NPC.append( make_shared<HexGenericMsg>("10000022c47d0180d9"+RSI+"010a")->toBuf() );
			spawn_NPC << double(x*100) << double(y*100) << double(z*100);
			spawn_NPC.append( make_shared<HexGenericMsg>("cc02022603030183000188")->toBuf() );
			spawn_NPC << uint8(level);
			spawn_NPC.append( make_shared<HexGenericMsg>("a209fe0700000801")->toBuf() );
			spawn_NPC << uint16( id );
			spawn_NPC.append( make_shared<HexGenericMsg>("000000")->toBuf() );
		}else if(type == "HOSTILE"){
			spawn_NPC.append( make_shared<HexGenericMsg>("0301000c57028dcdab")->toBuf() );
			size_t set_a_here = spawn_NPC.wpos();

			spawn_NPC.writeString(handle);
			for(int i = 0; i<32-(int)handle.length(); i++){
				spawn_NPC << uint8(0);
			}
			spawn_NPC.put(set_a_here, uint16(swap16(0x14ae)) );
			
			spawn_NPC.append( make_shared<HexGenericMsg>("1000002200001000c4fb0180c1"+RSI)->toBuf() );
			spawn_NPC << double(x*100) << double(y*100) << double(z*100);
			spawn_NPC.append( make_shared<HexGenericMsg>("d00c00830c01a8")->toBuf() ); //<d00c0183> <- weapon
			spawn_NPC << uint8(level) << uint16( hpM ) << uint16(swap16(0xb204)) << uint16( hpM );

			string fx = "00000000";
			spawn_NPC.append( make_shared<HexGenericMsg>("060800008337ba0000"+fx+"020e")->toBuf() );
			spawn_NPC << uint16( id );
			spawn_NPC.append( make_shared<HexGenericMsg>("000000")->toBuf() );
		}else if(type == "BIGMAN"){
			spawn_NPC.append( make_shared<HexGenericMsg>("0301000c5702f4cdab")->toBuf() ); //TYPE = F3, f3!=AGENT! > TYPE = F4
			size_t set_a_here = spawn_NPC.wpos();

			spawn_NPC.writeString(handle);
			for(int i = 0; i<32-(int)handle.length(); i++){
				spawn_NPC << uint8(0);
			}
			spawn_NPC.put(set_a_here, uint16(swap16(0x158e)) );
			
			spawn_NPC.append( make_shared<HexGenericMsg>("10000022d491cd01000401a80000874300008c4300005243c1c1"+RSI)->toBuf() );
			spawn_NPC << double(x*100) << double(y*100) << double(z*100);
			spawn_NPC.append( make_shared<HexGenericMsg>("f00c0701930001000020c1a8")->toBuf() ); //<d00c0183> <- weapon
			spawn_NPC << uint8(level) << uint16( hpM ) << uint16(swap16(0x3204)) << uint16( hpM );

			spawn_NPC << uint32(swap32(0x07080000)); 
			spawn_NPC << uint16( id );
			spawn_NPC.append( make_shared<HexGenericMsg>("000000")->toBuf() );
		}else if(type == "AGENT"){
			spawn_NPC.append( make_shared<HexGenericMsg>("0301000c5702f3cdab")->toBuf() );
			size_t set_a_here = spawn_NPC.wpos();
			
			spawn_NPC.writeString(handle);
			for(int i = 0; i<32-(int)handle.length(); i++){
				spawn_NPC << uint8(0);
			}
			spawn_NPC.put(set_a_here, uint16(swap16(0x118e)) );

			spawn_NPC.append( make_shared<HexGenericMsg>("10000022c00180c1"+RSI)->toBuf() );
			spawn_NPC << double(x*100) << double(y*100) << double(z*100);
			spawn_NPC.append( make_shared<HexGenericMsg>("c001832a01aa04ff")->toBuf() );
			spawn_NPC << uint16( hpM ) << uint16(swap16(0xb202)) << uint16( hpM );
			spawn_NPC.append( make_shared<HexGenericMsg>("020800008122b6000002ff")->toBuf() );
			spawn_NPC << uint16( id );
			spawn_NPC.append( make_shared<HexGenericMsg>("000000")->toBuf() );
		}

		SendPacketToMe(Bin2Hex(spawn_NPC));  spawn_NPC.clear();

		if(DEFEATED == true){
			spawn_NPC << uint8(0x03) << uint16(id);
			spawn_NPC.append( make_shared<HexGenericMsg>("02010d000000000000000000000000000000")->toBuf() );
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear(); //lie on ground

			spawn_NPC << uint8(0x03) << uint16(id);
			spawn_NPC.append( make_shared<HexGenericMsg>("04808080600100000000")->toBuf() );
			SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear(); //unavailable on click
		}
	}

	NPC_number++;
}
void PlayerObject::InstanceNPC(){
	string temp;
	uint16 proper_district = m_district;
	int kNPC = 1;

	name_file = "npc/NPC_COLLECTION.xml";
	INFO_LOG(name_file);

	using namespace irr::io;
	IrrXMLReader* xml = createIrrXMLReader( name_file.c_str() );
	while(xml && xml->read()){
		switch(xml->getNodeType()){
			case EXN_ELEMENT:
				if (iequals("data", xml->getNodeName()))
				{
					NPC_number = xml->getAttributeValueAsInt("NPC_NUMBER");
				}
				if(kNPC <= NPC_number)
				{
					string tag = "npc"+ lexical_cast<string>(kNPC);
					if (iequals(tag, xml->getNodeName()))
					{

						int idistrict = xml->getAttributeValueAsInt( "DISTRICT" );

						//check district
						if(idistrict == proper_district){
							string itype, ihandle, iRSI, iIS_DEFEATED, iID, imood;
							float ilevel, ihpM, ihpC, ix_pos, iy_pos, iz_pos, irotation;
							itype = xml->getAttributeValue( "type" );

							//hostile
							if(itype == "HOSTILE"){
								ihandle = xml->getAttributeValue( "handle" );
								iRSI = xml->getAttributeValue( "RSI" );
								iIS_DEFEATED = xml->getAttributeValue( "IS_DEFEATED" );

								ilevel = xml->getAttributeValueAsFloat( "level" );
								ihpM = xml->getAttributeValueAsFloat( "hpM" );
								ihpC = xml->getAttributeValueAsFloat( "hpC" );

								ix_pos = xml->getAttributeValueAsFloat( "x_pos" );
								iy_pos = xml->getAttributeValueAsFloat( "y_pos" );
								iz_pos = xml->getAttributeValueAsFloat( "z_pos" );
						
								uint16 i_THISid = kNPC+100;

								if(iIS_DEFEATED == "true"){
									SetNPC(proper_district, itype, i_THISid, ilevel, ihpM, ihpC, iRSI, ix_pos, iy_pos, iz_pos, ihandle, true);
								}else if(iIS_DEFEATED == "false"){
									SetNPC(proper_district, itype, i_THISid, ilevel, ihpM, ihpC, iRSI, ix_pos, iy_pos, iz_pos, ihandle, false);
								}
							}//end hostile

							//mission contact
							if(itype == "MISSION_CONTACT"){
								iID = xml->getAttributeValue( "ID" );
								ihandle = xml->getAttributeValue( "handle" );
								iRSI = xml->getAttributeValue( "RSI" );

								ix_pos = xml->getAttributeValueAsFloat( "x_pos" );
								iy_pos = xml->getAttributeValueAsFloat( "y_pos" );
								iz_pos = xml->getAttributeValueAsFloat( "z_pos" );

								uint16 i_THISid = kNPC+100;

								SetNPC_ContactMission(iID, i_THISid, iRSI, ix_pos, iy_pos, iz_pos, ihandle, 0x00);
							}//end mission contact

							//friendly
							if(itype == "FRIENDLY" || itype == "NEO_RSI" || itype == "DERBY_COURT_EFFECT"){
								ihandle = xml->getAttributeValue( "handle" );
								iRSI = xml->getAttributeValue( "RSI" );
								iIS_DEFEATED = xml->getAttributeValue( "IS_DEFEATED" );
								imood = xml->getAttributeValue( "mood" );

								ilevel = xml->getAttributeValueAsFloat( "level" );

								uint16 i_THISmood = 0;
								if(imood == "sit"){
									i_THISmood = 0x01;
								}else if(imood == "spy"){
									i_THISmood = 0x09;
								}else if(imood == "aggressive"){
									i_THISmood = 0x0c;
								}else if(imood == "dead"){
									i_THISmood = 0x0d;
								}else if(imood == "attention"){
									i_THISmood = 0x1a;
								}else if(imood == "normal"){
									i_THISmood = 0x00;
								}

								ix_pos = xml->getAttributeValueAsFloat( "x_pos" );
								iy_pos = xml->getAttributeValueAsFloat( "y_pos" );
								iz_pos = xml->getAttributeValueAsFloat( "z_pos" );

								temp = xml->getAttributeValue("rotation");
								irotation = xml->getAttributeValueAsFloat( "rotation" );

								uint16 i_THISid = kNPC+62000;

								if(iIS_DEFEATED == "true"){
									INFO_LOG("vero");
									SetNPC_Free(itype, i_THISid, ilevel, iRSI, ix_pos, iy_pos, iz_pos, ihandle, true, irotation, i_THISmood);
								}else if(iIS_DEFEATED == "false"){
									SetNPC_Free(itype, i_THISid, ilevel, iRSI, ix_pos, iy_pos, iz_pos, ihandle, false, irotation, i_THISmood);
								}
							}//end friendly

						}//end check district

						kNPC += 1; //anyway increase counter!
					}
				}
				break;
		}
	}

	if (xml)
	{
		delete xml;
		xml = NULL;
	}

	update_NPCs = 1;
}

void PlayerObject::SetNPC_DATANODE(int id, double x, double y, double z, bool deleteDataNode){
	ByteBuffer spawn_NPC;

	if(deleteDataNode == false){
		spawn_NPC.append( make_shared<HexGenericMsg>("03 01 00 0c e8 b7 f5 cd ab 02 09")->toBuf() );
		spawn_NPC << double(x*100) << double(y*100) << double(z*100) << uint8(1) << uint32(id);
	}else if(deleteDataNode == true){
		spawn_NPC.append( make_shared<HexGenericMsg>("030100010100")->toBuf() );
		spawn_NPC << uint32(id);
	}
	
	SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear();
}

string PlayerObject::GetValueDB(string a, string b, string c, string d){
	string def;
	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `%2%` FROM `%1%` WHERE `%3%` = '%4%' LIMIT 1") % a % b % c % d) );
	if (value == NULL){
		def = "NULL";
	}else{
		Field *field_value = value->Fetch();
		def = ""+format(field_value[0].GetString()).str();
	}
	return def;
}
void PlayerObject::SetValueDB(string a, string b, string c, string d, string e){
	bool store_data = sDatabase.Execute(format("UPDATE `%1%` SET `%2%` = '%3%' WHERE `%4%` = '%5%'")% a % b % c % d % e );
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//STRING//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

vector<std::string> PlayerObject::GetBytes(ByteBuffer &srcCmd)
{
	string BYTE_PACKET = ""+(format("%1%") % Bin2Hex(srcCmd)).str();
	std::vector<std::string> def;
	boost::split(def, BYTE_PACKET, boost::is_any_of(" "));
	return def;
}
string PlayerObject::parseStringToHex(string msg, string param)
{
	size_t bufSize = (2*msg.length())+1;
	char *buffer = new char[bufSize];
	memset(buffer,0,bufSize);
	for (int i = 0; i<(int)msg.length(); i++)
	{
		sprintf( buffer+(2*i), "%02x", (unsigned int)msg.c_str()[i]);
	}

	string def = buffer;
	delete[] buffer;
	buffer = NULL;

	if(param=="PACKETSCHEME_NO")
	{
		def = def;
	}
	else if(param=="PACKETSCHEME_YES")
	{
		def = parseIntegerToHexBIG( msg.length()+1 )+" "+def+"00";
	}
	else if(param=="PACKETSCHEME_YES_TYPEB")
	{
		def = parseIntegerToHexSMALL( msg.length() )+" "+def;
	}
	return def;
}
string PlayerObject::parseIntegerToHexBIG(int a){
	string def = "";
	if(a >=0 && a <= 255){
		char toHEX1[256];
		sprintf(toHEX1,"%x",a);
		string VALUE = (format("%02x")%toHEX1).str();
		def = ""+VALUE+" 00";
	}else if(a >255) {
		int iresult = (int)a/255;
		int irest = a%255;
		char iresultHEX[256];
		sprintf(iresultHEX,"%x",iresult);
		char irestHEX[256];
		sprintf(irestHEX,"%x",irest);
		string VALUE1 = (format("%02x")%irestHEX).str();
		string VALUE2 = (format("%02x")%iresultHEX).str();
		def = VALUE1+" "+VALUE2;
	}
	return def;
}
string PlayerObject::parseIntegerToHexSMALL(int a){
	string def = "";
	if(a<16){
		char toHEX1[256];
		sprintf(toHEX1,"%x",(a));
		def += " 0"+(format("%1%")%toHEX1).str();
	}else{
		char toHEX1[256];
		sprintf(toHEX1,"%x",(a));
		def += " "+(format("%1%")%toHEX1).str();
	}
	return def;
}

void PlayerObject::ReadXML(string nFile, string tag, string TYPE, int turn){
	tag += lexical_cast<string>(turn);

	INFO_LOG("\t\t(util)ReadXML@(reading XML)(file)"+ nFile );

	using namespace irr::io;
	IrrXMLReader* xml = createIrrXMLReader( nFile.c_str() );
	while(xml && xml->read()){
		switch(xml->getNodeType()){
			case EXN_ELEMENT:
				if (iequals(tag, xml->getNodeName()))
				{
					if(TYPE == "OBJECTIVE"){
						desc_obj[turn] = xml->getAttributeValue( "description" );
						NPC_obj[turn] = xml->getAttributeValue( "idNpc" );
						command_obj[turn] = xml->getAttributeValue( "command" );
						dial_obj[turn] = xml->getAttributeValue( "dial" );
					}
					if(TYPE == "NPC"){
						//(..)
					}
				}
				break;
		}
	}

	if (xml)
	{
		delete xml;
		xml = NULL;
	}
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//PACKETS/////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::SendRPC(string packet){
	erase_all(packet," ");
	m_parent.QueueCommand(make_shared<HexGenericMsg>(packet));
	m_parent.FlushQueue();
}
void PlayerObject::SendPacketToMe(string packet){
	erase_all(packet," ");
	msgBaseClassPtr msg_packet = make_shared<HexGenericMsg>(packet);
	m_parent.QueueState(msg_packet,true);
}
void PlayerObject::SendPacketToCmd(string packet){
	bool rpcCmd=false;
	stringstream hexStream;
	hexStream << packet;
	string binaryOutputForMyClient;
	try{
		CryptoPP::HexDecoder hexDecoder(new CryptoPP::StringSink(binaryOutputForMyClient));
		hexDecoder.Put((const byte*)hexStream.str().data(),hexStream.str().size(),true);
		hexDecoder.MessageEnd();
	}catch (...){}
	sGame.Broadcast(ByteBuffer(binaryOutputForMyClient),rpcCmd);
}
void PlayerObject::SendMsg(string handle, string msg, string TYPE){
	//2e 05 00 00 00 00 00 <24_startHandle_FROM_BEGINNING> 00 00 00 <3c_startMsg_FROM_BEGINNING> 00 (..)
	
	
	if(msg.length() < 1) return;
		
	uint8 length_BASIC = 36+handle.length()+3;
	ByteBuffer packet;
	if(TYPE == "TEAM"){ //TEAM
		packet.append( make_shared<HexGenericMsg>("2e0500cccf000024000000")->toBuf() );
		packet << uint16(length_BASIC);
		packet.append( make_shared<HexGenericMsg>("0000000000000000000000000000000000000000000000")->toBuf() );
		packet.writeString(handle);
		packet.writeString(msg);
		SendRPC(Bin2Hex(packet));
	}else if(TYPE == "CREW"){ //CREW
		packet.append( make_shared<HexGenericMsg>("2e0200cccf000024000000")->toBuf() );
		packet << uint16(length_BASIC);
		packet.append( make_shared<HexGenericMsg>("0000000000000000000000000000000000000000000000")->toBuf() );
		packet.writeString(handle);
		packet.writeString(msg);
		SendRPC(Bin2Hex(packet));
	}else if(TYPE == "FACTION"){ //FACTION
		packet.append( make_shared<HexGenericMsg>("2e0300cccf000024000000")->toBuf() );
		packet << uint16(length_BASIC);
		packet.append( make_shared<HexGenericMsg>("0000000000000000000000000000000000000000000000")->toBuf() );
		packet.writeString(handle);
		packet.writeString(msg);
		SendRPC(Bin2Hex(packet));
	}else if(TYPE == "MODAL"){ //MODAL
		packet.append( make_shared<HexGenericMsg>("2e1700cccf000024000000")->toBuf() );
		packet << uint16(length_BASIC);
		packet.append( make_shared<HexGenericMsg>("0000000000000000000000000000000000000000000000")->toBuf() );
		packet.writeString(handle);
		packet.writeString(msg);
		SendRPC(Bin2Hex(packet));
	}else if(TYPE == "FRAMEMODAL"){ //FRAMEMODAL
		packet.append( make_shared<HexGenericMsg>("2ed700cccf000024000000")->toBuf() );
		packet << uint16(length_BASIC);
		packet.append( make_shared<HexGenericMsg>("0000000000000000000000000000000000000000000000")->toBuf() );
		packet.writeString(handle);
		packet.writeString(msg);
		SendRPC(Bin2Hex(packet));
	}else if(TYPE == "BROADCAST"){ //BROADCAST
		packet.append( make_shared<HexGenericMsg>("2ec700cccf000024000000")->toBuf() );
		packet << uint16(length_BASIC);
		packet.append( make_shared<HexGenericMsg>("0000000000000000000000000000000000000000000000")->toBuf() );
		packet.writeString(handle);
		packet.writeString(msg);
		SendRPC(Bin2Hex(packet));
	}else if(TYPE == "AREA"){ //AREA
		packet.append( make_shared<HexGenericMsg>("2e1000cccf000024000000")->toBuf() );
		packet << uint16(length_BASIC);
		packet.append( make_shared<HexGenericMsg>("0000000000000000000000000000000000000000000000")->toBuf() );
		packet.writeString(handle);
		packet.writeString(msg);
		SendRPC(Bin2Hex(packet));
	}else if(TYPE == "WHISPER"){ //WHISPER
		handle = "SOE+MXO+Syntax+"+handle;
		length_BASIC = 36+handle.length()+3;
		packet.append( make_shared<HexGenericMsg>("2e11000000000024000000")->toBuf() );
		packet << uint16(length_BASIC);
		packet.append( make_shared<HexGenericMsg>("0000000000000000000000000000000000000000000000")->toBuf() );
		packet.writeString(handle);
		packet.writeString(msg);
		SendRPC(Bin2Hex(packet));
	}else if(TYPE == "POPUP_FRIENDLY"){ //SYSTEM
		packet.append( make_shared<HexGenericMsg>("80a11f12002800000000140000000000")->toBuf() );
		packet << uint16( 20+handle.length()+3 ) << uint16(1);
		packet.writeString(handle);
		packet.writeString(msg);
		SendRPC(Bin2Hex(packet));
	}else if(TYPE == "POPUP_HOSTILE"){ //SYSTEM
		packet.append( make_shared<HexGenericMsg>("80a11f12002800000000140000000000")->toBuf() );
		packet << uint16( 20+handle.length()+3 ) << uint16(5);
		packet.writeString(handle);
		packet.writeString(msg);
		SendRPC(Bin2Hex(packet));
	}else if(TYPE == "SYSTEM"){ //SYSTEM
		m_parent.QueueCommand(make_shared<SystemChatMsg>(msg));
	}
}

void PlayerObject::DestroyChar(){
	ByteBuffer packet;
	packet.append( make_shared<HexGenericMsg>("030100010100")->toBuf() );
	packet << uint16(m_goId) << uint8(0); //remove my rsi, we will use this later

	vector<uint32> allObjects = sObjMgr.getAllGOIds();
	foreach(uint32 objId, allObjects){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(objId);
		}catch (ObjectMgr::ObjectNotAvailable){
			continue;
		}
		if( player->m_handle != m_handle && player->m_district != m_district){
			player->SendPacketToMe(Bin2Hex(packet)); //send packet

			ByteBuffer test;
			test.append( make_shared<HexGenericMsg>("030100010100")->toBuf() );
			test << uint16(objId) << uint8(0); //remove others rsi, we will use this later
			SendPacketToMe(Bin2Hex(test));
		}
	}
}
void PlayerObject::UpdateConstruct(string itemId){
	if(itemId == "8a960000" || itemId == "a6b80000" || itemId == "89960000" || itemId == "6eba0000" || itemId == "88960000" || itemId == "87960000"){
	}else{
		return;
	}

	if(m_district != 1){
		GiveItemToSlot(0x6a,"00000000");
		SetValueDB("characters","district","1","handle",m_handle);
		SetValueDB("myappearence","tool","00000000","handle",m_handle);
		SetValueDB("characters","x", "12223" ,"handle", m_handle);
		SetValueDB("characters","y", "-705" ,"handle", m_handle);
		SetValueDB("characters","z", "59707" ,"handle", m_handle);

		SendMsg("","Please, relog.","FRAMEMODAL");
		SendMsg("","Your district has been restored to <?> Richland","FRAMEMODAL");
	}else{
		GiveItemToSlot(0x6a,itemId); //add tool to my inventory
		SetValueDB("myappearence","tool",itemId,"handle",m_handle); //add tool to myappearence
		string name_construct = "At your next relog, your district will be <?> ";

		if(itemId == "8a960000"){ //ASHENCOURTE
			SetValueDB("characters","district","6","handle",m_handle);
			name_construct += "Ashencourte";

			if(m_reputation == "zion" || m_reputation == "epn" || m_reputation == "niobesgroup"){
				SetValueDB("characters","x", "-17761.9" ,"handle", m_handle);
				SetValueDB("characters","z", "-5695.8" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}if(m_reputation == "machinist" || m_reputation == "cyph"){
				SetValueDB("characters","x", "19274.4" ,"handle", m_handle);
				SetValueDB("characters","z", "-5568.6" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}else if(m_reputation == "merovingian" || m_reputation == "maskedmen"){
				SetValueDB("characters","x", "8616.7" ,"handle", m_handle);
				SetValueDB("characters","z", "17940.1" ,"handle", m_handle);
				SetValueDB("characters","y", "143.7" ,"handle", m_handle);
			}else{
				//zion
				SetValueDB("characters","x", "-17761.9" ,"handle", m_handle);
				SetValueDB("characters","z", "-5695.8" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}
		}else if(itemId == "a6b80000"){ //DATAMINE
			SetValueDB("characters","district","7","handle",m_handle);
			name_construct += "Datamine";

			SetValueDB("characters","x", "13695.9" ,"handle", m_handle);
			SetValueDB("characters","z", "-17278.7" ,"handle", m_handle);
			SetValueDB("characters","y", "595" ,"handle", m_handle);

		}else if(itemId == "89960000"){ //SAKURA
			SetValueDB("characters","district","8","handle",m_handle);
			name_construct += "Sakura";

			if(m_reputation == "zion" || m_reputation == "epn" || m_reputation == "niobesgroup"){
				SetValueDB("characters","x", "-17141.3" ,"handle", m_handle);
				SetValueDB("characters","z", "-15504.5" ,"handle", m_handle);
				SetValueDB("characters","y", "595" ,"handle", m_handle);
			}if(m_reputation == "machinist" || m_reputation == "cyph"){
				SetValueDB("characters","x", "-25401.8" ,"handle", m_handle);
				SetValueDB("characters","z", "37314.9" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}else if(m_reputation == "merovingian" || m_reputation == "maskedmen"){
				SetValueDB("characters","x", "-24453.8" ,"handle", m_handle);
				SetValueDB("characters","z", "-7565.82" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}else{
				//zion
				SetValueDB("characters","x", "-17141.3" ,"handle", m_handle);
				SetValueDB("characters","z", "-15504.5" ,"handle", m_handle);
				SetValueDB("characters","y", "595" ,"handle", m_handle);
			}

		}else if(itemId == "6eba0000"){ //SATI
			SetValueDB("characters","district","9","handle",m_handle);
			name_construct += "Sati";

			SetValueDB("characters","x", "38149.2" ,"handle", m_handle);
			SetValueDB("characters","z", "-4696.92" ,"handle", m_handle);
			SetValueDB("characters","y", "95" ,"handle", m_handle);

		}else if(itemId == "88960000"){ //WINDOW'SMOOR
			SetValueDB("characters","district","10","handle",m_handle);
			name_construct += "Window's Moor";

			if(m_reputation == "zion" || m_reputation == "epn" || m_reputation == "niobesgroup"){
				SetValueDB("characters","x", "-17709.6" ,"handle", m_handle);
				SetValueDB("characters","z", "-5660.0" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}if(m_reputation == "machinist" || m_reputation == "cyph"){
				SetValueDB("characters","x", "19234.5" ,"handle", m_handle);
				SetValueDB("characters","z", "-5522.2" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}else if(m_reputation == "merovingian" || m_reputation == "maskedmen"){
				SetValueDB("characters","x", "8550.1" ,"handle", m_handle);
				SetValueDB("characters","z", "17881.7" ,"handle", m_handle);
				SetValueDB("characters","y", "35" ,"handle", m_handle);
			}else{
				//zion
				SetValueDB("characters","x", "-17709.6" ,"handle", m_handle);
				SetValueDB("characters","z", "-5660.0" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}

		}else if(itemId == "87960000"){ //YUKI
			SetValueDB("characters","district","11","handle",m_handle);
			name_construct += "Yuki";

			if(m_reputation == "zion" || m_reputation == "epn" || m_reputation == "niobesgroup"){
				SetValueDB("characters","x", "17528.3" ,"handle", m_handle);
				SetValueDB("characters","z", "15550.3" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}if(m_reputation == "machinist" || m_reputation == "cyph"){
				SetValueDB("characters","x", "-25477.3" ,"handle", m_handle);
				SetValueDB("characters","z", "37212.5" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}else if(m_reputation == "merovingian" || m_reputation == "maskedmen"){
				SetValueDB("characters","x", "-24438" ,"handle", m_handle);
				SetValueDB("characters","z", "-7609.2" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}else{
				//zion
				SetValueDB("characters","x", "17528.3" ,"handle", m_handle);
				SetValueDB("characters","z", "15550.3" ,"handle", m_handle);
				SetValueDB("characters","y", "95" ,"handle", m_handle);
			}

		}
		name_construct += "\nImportant: To return to Richland, unmount the construct book from your inventory and relog.";
		SendMsg("",name_construct,"FRAMEMODAL");
	}

	LoadAnimation(m_goId,"290f14", true);
	SendMsg("","{i}Restart your client...{/i}\n\nYou are now disconnected.","FRAMEMODAL");
	m_parent.Invalidate();
}

void PlayerObject::RPC_AFKMounted(ByteBuffer &src){
	m_currentFX = "AFK";

	ByteBuffer packet;
	packet<< uint8(0x03) << uint16(m_goId);
	packet.append( make_shared<HexGenericMsg>("03010000808080c0")->toBuf() );
	packet<< uint16(m_healthC);
	packet.append( make_shared<HexGenericMsg>("808036000000100000000000")->toBuf() );
	SendPacketToMe(Bin2Hex(packet));
}
void PlayerObject::RPC_AFKUnmounted(ByteBuffer &src){
	m_currentFX = "NULL";

	ByteBuffer packet;
	packet<< uint8(0x03) << uint16(m_goId);
	packet.append( make_shared<HexGenericMsg>("03010000808080c0")->toBuf() );
	packet<< uint16(m_healthC);
	packet.append( make_shared<HexGenericMsg>("808036000000000000000000")->toBuf() );
	SendPacketToMe(Bin2Hex(packet));
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//EXP/////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

int exp_req[51] = { 0, 500, 5000, 21000, 60000, 137500, 273000,
                       490000, 816000, 1282500, 1925000,2722500, 
                       3690000, 4842500, 6195000, 7762500, 9560000,
                       11602500, 13905000, 16482500, 19350000, 22522500,
                       26015000, 29842500, 34020000, 38562500, 43485000,
                       48802500, 54530000, 60682500, 67275000, 74322500, 
                       81840000, 89842500, 98345000, 107362500, 116910000,
                       127002500, 137655000, 148882500, 160700000, 173122500,
                       186165000, 199842500, 214170000, 229162500, 244835000,
                       261202500, 278280000, 296082500, 314625000 };

void PlayerObject::CHECK_LEVEL() // Rewritten, needs rework -bb
{
	int standard_level = 0;
	bool add_attribute = false;
	
	standard_level = CHECK_LEVEL_GAMESOCKET();
	add_attribute = (m_lvl < standard_level );		// Set flag if level changed

	if(add_attribute)
	{
		INFO_LOG(m_handle + " : MLVL" + (format("%1%")%m_lvl).str() + " standardLVL" + (format("%1%")%standard_level).str() + "adding 1 ATTR. POINT");

		CastFXOn(m_goId,"B9050028");
		
		m_lvl++;
		uint16 m_new_level = 0;
		m_new_level += m_lvl;
		SetValueDB("characters","level", (format("%1%")%m_new_level).str() ,"handle",m_handle);
		
		total++;
		if(total > 72){
			total = 72;
		}
		SetValueDB("myattributes","total", (format("%1%")%total).str() ,"handle", m_handle );
	}
}
uint8 PlayerObject::CHECK_LEVEL_GAMESOCKET() // Rewritten -bb
{
	uint8 standard_level = 0;
	
	for(int cnt = 1; cnt <= 50; cnt++)
    {
            if (m_exp < exp_req[cnt])
               {
                      standard_level = cnt;		// Set level to current loop count
                      break;
               }
     }
	return standard_level;
}
uint32 PlayerObject::DIFFERENCE_EXP_PER_LVL(bool getBasicEXP) // Rewritten, needs rework -bb
{
	uint32 global_exp = 0;
	uint32 basic_exp = 0;

	basic_exp = exp_req[m_lvl - 1]; // set basic_exp to the bottom of level m_lvl
    global_exp = exp_req[m_lvl] - basic_exp; // Set global_exp to the difference between the bottom and top of level m_lvl
	
	global_exp = (int32)(global_exp/(m_lvl*2)); // Needs comment

	uint32 param = 0;
	if(getBasicEXP == true)
		param = basic_exp;
	else if(getBasicEXP == false)
		param = global_exp;

	return param;
}
void PlayerObject::GiveEXP(uint32 new_exp){
	INFO_LOG(m_handle + " : CURRENT EXP : " + (format("%1%")%m_exp).str() );
	INFO_LOG(m_handle + " : ADDING EXP : " + (format("%1%")%new_exp).str() );
	m_exp += new_exp;

	ByteBuffer complete;
	complete << uint16(swap16(0x80e5)) << uint64(m_exp) << uint8(0) << uint32(swap32(0x11000000));
	SendRPC(Bin2Hex(complete)); complete.clear(); //update GUI, exp bar

	CHECK_LEVEL(); //send FX, if you close a level
		
	SetValueDB( "characters","exp", (format("%1%")%m_exp).str(), "handle", m_handle); //save data
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//ACCOUNT/////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::RPC_OpenEmailAccount(ByteBuffer &src){
	INFO_LOG(m_handle+" : RPC_OpenEmailAccount@(..)");

	string email_account = GetValueDB("myemailaccount","emails","handle",m_handle);
	if(email_account == "NONE"){
		return;
	}

	std::vector<std::string> def;
	boost::split(def, email_account, boost::is_any_of(","));

	string e_date, e_sender, e_subject, e_message;
	for(int i = 0; i<(int)def.size(); i++){
		if(def[i] != "NONE"){
			scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `date`, `sender`,  `subject`  FROM `myemaildb` WHERE `code` = '%1%' LIMIT 1") % def[i] ) );
			if (value == NULL){ /*do nothing*/ }else{
				Field *field_value = value->Fetch();
				e_date = ""+format(field_value[0].GetString()).str();
				e_sender = ""+format(field_value[1].GetString()).str();
				e_subject = ""+format(field_value[2].GetString()).str();

				int e_id = 0+atoi( def[i].c_str() );

				ByteBuffer packet;
				packet << uint16(swap16(0x81ed)) << uint16(25) << uint16( 28+e_sender.length() ) << uint16( 31+e_sender.length() ) << uint32(e_id);
				packet.append( make_shared<HexGenericMsg>(e_date)->toBuf() );
				packet << uint32(3); //read|(2)not read
				packet << uint16(1) << uint16( 40+e_sender.length() ) << uint8(0);
				packet.writeString(e_sender);
				packet.append( make_shared<HexGenericMsg>("010000070053595354454d00")->toBuf() );
				packet.writeString(e_subject);

				SendRPC( Bin2Hex(packet) );
			}
		}
	}
}
void PlayerObject::RPC_LoadEmail(ByteBuffer &src){
	uint32 e_id = src.read<uint32>();
	INFO_LOG(m_handle+" : RPC_LoadEmail@(id)"+ (format("%1%")%e_id).str() );

	string e_date, e_sender, e_subject, e_message;
	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `date`, `sender`,  `subject`,  `message`  FROM `myemaildb` WHERE `code` = '%1%' LIMIT 1") % e_id ) );
	if (value == NULL){ /*do nothing*/ }else{
		Field *field_value = value->Fetch();
		e_date = ""+format(field_value[0].GetString()).str();
		e_sender = ""+format(field_value[1].GetString()).str();
		e_subject = ""+format(field_value[2].GetString()).str();
		e_message = ""+format(field_value[3].GetString()).str();

		e_message = base64_decode(e_message);

		ByteBuffer packet;

		packet << uint16(swap16(0x81f3)) << uint16(25) << uint16( 25+3+e_sender.length() ) << uint16( 25+4+e_sender.length() ) << uint32(e_id);
		packet.append( make_shared<HexGenericMsg>(e_date+"0300000000")->toBuf() );
		packet << uint16( 45+e_sender.length() ) << uint16( 40+e_sender.length() );
		packet.writeString(e_sender);
		packet.append( make_shared<HexGenericMsg>("010000070053595354454d0003000000000300000900")->toBuf() );
		packet << uint8(0) << uint16( 9+e_subject.length() ) << uint8(0) << uint16( 9+e_subject.length()+e_message.length() );
		packet.writeString(e_subject);
		packet.writeString(e_message);
		packet.append( make_shared<HexGenericMsg>("61003c3f786d6c2076657273696f6e3d22312e302220656e636f64696e673d227574662d3822203f3e0a3c4d584f5f4d61696c5f4f4f423e0a093c4974656d4c6973743e0a093c2f4974656d4c6973743e0a3c2f4d584f5f4d61696c5f4f4f423e0a00")->toBuf() );

		SendRPC( Bin2Hex(packet) );
	}
}
void PlayerObject::RPC_SendEmail(ByteBuffer &src){
	INFO_LOG(m_handle+" : RPC_SendEmail@(..)");

	uint16 e_id = 0;
	for(int i = 1; i<=42; i++){
		e_id = src.read<uint8>();
	}

	string e_target = src.readString();
	std::vector<std::string> def;
	boost::split(def, e_target, boost::is_any_of("+"));
	e_target = def[3];

	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `handle`  FROM `characters` WHERE `handle` = '%1%' LIMIT 1") % e_target ) );
	if (value == NULL){
		SendMsg("","Player doesn't exist.","FRAMEMODAL");
		return;
	}else{
		Field *field_value = value->Fetch();
		e_target = ""+format(field_value[0].GetString()).str(); //overwrite handle of target, properly
	}

	INFO_LOG("\t\tRecipient: "+e_target);

	string e_subject = src.readString();
	string e_message = src.readString();
	if(e_message.length() <= 1){
		INFO_LOG("\t\tMessage: @ERROR_MESSAGELENGHT");
		SendMsg("","Message is too short.","FRAMEMODAL");
		return;
	}

	INFO_LOG("\t\tMessage: @OK");

	e_message = base64_encode(reinterpret_cast<const unsigned char*>(e_message.c_str()), e_message.length());
	
	//the new ID of email
	e_id = 0+ atoi( GetValueDB("mytotalnpcnumber","npcNumber","command","EMAILS").c_str() ); 

	//store email in DB
	sDatabase.Execute(format("INSERT INTO `myemaildb` SET `code`='%1%', `date`='92c2753c', `sender`='%2%', `subject`='%3%', `message`='%4%'") 
		% e_id
		% m_handle
		% e_subject
		% e_message );

	//update target account
	string e_info_target = GetValueDB("myemailaccount","emails","handle",e_target) + "," + (format("%1%")%e_id).str();
	SetValueDB("myemailaccount","emails", e_info_target ,"handle",e_target);

	//update counter emails
	e_id += 1;
	SetValueDB("mytotalnpcnumber","npcNumber", (format("%1%")%e_id).str() ,"command","EMAILS");

	//look if target is online
	bool target_is_online = false;
	string notice = "You received an email message:\nSender("+m_handle+")\nSubject("+e_subject+")";
	vector<uint32> objectLists = sObjMgr.getAllGOIds();
	foreach (int currObj, objectLists){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(currObj);
		}catch (ObjectMgr::ObjectNotAvailable){ continue; }
		string handle = player->getHandle();
		if( handle == e_target && player != NULL ){
			player->SendMsg("",notice,"FRAMEMODAL");
			target_is_online = true;
		}
	}

	if(target_is_online == false){
		SendMsg("","Player is not currently jacked in.","FRAMEMODAL");
	}else{
		SendMsg("","Message was sent successfully.","FRAMEMODAL");
	}
}
void PlayerObject::RPC_DeleteEmail(ByteBuffer &src){
	INFO_LOG(m_handle+" : RPC_DeleteEmail@(..)");

	uint16 e_id = src.read<uint16>();

	string email_account = GetValueDB("myemailaccount","emails","handle",m_handle);
	if(email_account == "NONE"){ // can never happen
		return;
	}

	std::vector<std::string> def;
	boost::split(def, email_account, boost::is_any_of(","));
	string new_email_account = "NONE";
	for(int i = 1; i<(int)def.size(); i++){
		uint16 turn_email = 0+atoi( def[i].c_str() );
		if(turn_email != e_id){
			new_email_account += "," + def[i];
		}
	}
	SetValueDB("myemailaccount","emails",new_email_account,"handle",m_handle);

	SendMsg("","You deleted 1 message.\nPlease close your email account, and open it again.","FRAMEMODAL");
}
void PlayerObject::RPC_CloseEmailAccount(ByteBuffer &src){
	/*do nothing*/
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//MARKET//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::RPC_StartInteractionMarket(ByteBuffer &src){
	INFO_LOG(m_handle+" : RPC_StartInteractionMarket@(..)");

	//update SellMarket
	string my_market = GetValueDB("mymarketplace","market","handle",m_handle);
	if(my_market != "NONE"){
		std::vector<std::string> def;
		boost::split(def, my_market, boost::is_any_of(","));

		for(int i = 1; i<(int)def.size(); i++){
			ByteBuffer sellItem;
			sellItem << uint16(swap16(0x812c));
			sellItem.append( make_shared<HexGenericMsg>(def[i]+" "+def[i])->toBuf() );
			sellItem << uint8(1) << uint8(2) << uint16(0) << uint64(0) << uint8(0) << uint16(swap16(0x8c01)) << uint64(0);
			SendRPC(Bin2Hex(sellItem)); sellItem.clear();
		}
	}
}
void PlayerObject::RPC_OpenMarket(ByteBuffer &src){
	src.read<uint16>();
	uint32 limit = src.read<uint32>();

	string info_market = m_handle+" : RPC_OpenMarket@(Type : ";
	string type_market = "UNSUPPORTED MARKET SECTION )(id)" + Bin2Hex((byte*)&limit,sizeof(limit),0);

	limit = swap32(limit);

	if(limit == 0x49415053){
		info_market += "Item > Apparel > Pants and Skirt )";
		type_market = "ITEM_APPAREL_PANTSANDSKIRT";
	}else if(limit == 0x49415368){
		info_market += "Item > Apparel > Shirt and Dress )";
		type_market = "ITEM_APPAREL_SHIRTANDDRESS";
	}else if(limit == 0x4941476c){
		info_market += "Item > Apparel > Gloves )";
		type_market = "ITEM_APPAREL_GLOVES";
	}else if(limit == 0x49414874){
		info_market += "Item > Apparel > Hat )";
		type_market = "ITEM_APPAREL_HAT";
	}else if(limit == 0x49414677){
		info_market += "Item > Apparel > Footwear )";
		type_market = "ITEM_APPAREL_FOOTWEAR";
	}else if(limit == 0x49414f77){
		info_market += "Item > Apparel > Outerwear )";
		type_market = "ITEM_APPAREL_OUTERWEAR";
	}
	if(limit == 0x49435030){
		info_market += "Item > Consumables > C. Pills )";
		type_market = "ITEM_CONSUMABLES_PILL";
	}else if(limit == 0x49435430){
		info_market += "Item > Consumables > C. Tool )";
		type_market = "ITEM_CONSUMABLES_TOOL";
	}
	if(limit == 0x4954506c){
		info_market += "Item > Tools > Program Launcher )";
		type_market = "ITEM_TOOL_PROGRAMLAUNCHER";
	}else if(limit == 0x49544f74){
		info_market += "Item > Tools > Other Tool )";
		type_market = "ITEM_TOOL_OTHERTOOL";
	}
	if(limit == 0x49574867){
		info_market += "Item > Weapon > Handgun )";
		type_market = "ITEM_WEAPON_HANDGUN";
	}else{
		info_market += "UNSUPPORTED MARKET SECTION )(id)" + limit;
	}

	string items;
	int quantity = 0;

	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `quantity`, `items` FROM `myvendorsandcollectors` WHERE `type` = '%1%' LIMIT 1") % type_market ) );
	if (value == NULL){
		SendMsg("","UNSUPPORTED MARKET SECTION","FRAMEMODAL");
	}else{
		Field *field_value = value->Fetch();
		quantity = field_value[0].GetInt32();
		items = field_value[1].GetString();

		std::vector<std::string> def;
		boost::split(def, items, boost::is_any_of(","));

		ByteBuffer packet;
		packet << uint16(swap16(0x8125)) << uint16(9) << uint32(0) << uint8(0) << uint16(22*quantity);

		for(int i = 0; i<(int)def.size(); i++){
			packet.append( make_shared<HexGenericMsg>(def[i])->toBuf() ); //item hex
			packet << uint32(0); //last byte is purity [00 = max]
			packet.append( make_shared<HexGenericMsg>(def[i])->toBuf() ); //item id
			packet << uint32(0); //ideal price
			packet << uint8(0) << uint32(0) << uint8(0); //organization + PLAYER_ID + last byte(0x00)
		}

		//send
		SendRPC( Bin2Hex(packet) ); packet.clear();
	}

	INFO_LOG(info_market);
}
void PlayerObject::RPC_MarketBuy(ByteBuffer &src){
	uint32 extra = src.read<uint32>();
	uint8 target_slot = src.read<uint8>();

	string BYTE_PACKET = ""+(format("%1%") % Bin2Hex(src)).str();
	std::vector<std::string> def;
	boost::split(def, BYTE_PACKET, boost::is_any_of(" "));

	string item = def[2]+def[3]+def[4]+def[5];

	INFO_LOG(m_handle+" : RPC_MarketBuy@(id)"+item);

	inventory[target_slot] = item;
	GiveItemToSlot(target_slot,item);

	SaveInventory();

	SendMsg("","You bought an item from the marketplace.","SYSTEM");

	//check if item is begin de-listed
	string my_marekt = GetValueDB("mymarketplace","market","handle",m_handle);
	std::vector<std::string> def1;
	boost::split(def1, my_marekt, boost::is_any_of(","));
	string new_market = "";

	bool found_item = false;
	for(int i = 0; i<(int)def1.size(); i++){
		if(def1[i] == item && found_item == false){
			found_item = true;
		}else{
			new_market += def1[i]+",";
		}
	}
	new_market = new_market.substr(0,new_market.length()-1);
	SetValueDB("mymarketplace","market",new_market,"handle",m_handle);

	ByteBuffer packet;
	packet << uint16(swap16(0x8127));
	packet.append( make_shared<HexGenericMsg>(item)->toBuf() );
	SendRPC( Bin2Hex(packet) ); packet.clear();
}
void PlayerObject::RPC_MarketSell(ByteBuffer &src){
	//81 2b 49 41 53 68 27 ee 89 09 01 12 00 00 00 00 00
	if(inMission == true){
		SendMsg("","You cannot sell items while on a mission.","SYSTEM");
		return;
	}

	uint32 extra = src.read<uint32>(); extra = src.read<uint32>();
	extra = src.read<uint8>();
	uint16 slot = src.read<uint8>();

	string item_to_delete2 = inventory[slot];

	INFO_LOG(m_handle+" : RPC_MarketSell@(id)"+item_to_delete2);

	RemoveItem(slot);

	//---
	//check if items is loaded as cloth, tool or weapon
	//61 Hat, 62 Glasses, 63 Shirt, 64 Gloves, 65 Coat, 66 Pant, 68 Shoes |69 Weapon, 6a Tool|
	for(int i = 0; i<9; i++){
		if( this->appearence[i] == item_to_delete2 ){

			INFO_LOG(m_handle+" : RPC_MarketSell@(Type : Unmount)"+ item_to_delete2 );

			this->appearence[i] = "00000000";

			ByteBuffer removeItem;
			if(i < 6){
				removeItem << uint8(0x5e) << uint16(i+97);
			}else{
				removeItem << uint8(0x5e) << uint16(i+98);
			}
			removeItem.append( make_shared<HexGenericMsg>("00000000000000001401")->toBuf() );
			SendRPC(Bin2Hex(removeItem)); removeItem.clear();

			if(i==0){
				SetValueDB("myappearence","hat","00000000","handle",m_handle);
				SetValueDB("rsivalues", "hat", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==1){
				SetValueDB("myappearence","glasses","00000000","handle",m_handle);
				SetValueDB("rsivalues", "glasses", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==2){
				SetValueDB("myappearence","shirt","00000000","handle",m_handle);
				SetValueDB("rsivalues", "shirt", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==3){
				SetValueDB("myappearence","gloves","00000000","handle",m_handle);
				SetValueDB("rsivalues", "gloves", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==4){
				SetValueDB("myappearence","coat","00000000","handle",m_handle);
				SetValueDB("rsivalues", "coat", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==5){
				SetValueDB("myappearence","pants","00000000","handle",m_handle);
				SetValueDB("rsivalues", "pants", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==6){
				SetValueDB("myappearence","shoes","00000000","handle",m_handle);
				SetValueDB("rsivalues", "shoes", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==7){
				SetValueDB("myappearence","weapon","00000000","handle",m_handle);
			}else if(i==8){
				SetValueDB("myappearence","tool","00000000","handle",m_handle);
			}
		}
	}

	SendMsg("","You sold an item in the marketplace.","SYSTEM");

	//update my appearence
	this->UpdateAppearance();

	//GiveCash( rand()%500 );

	string get_my_market = GetValueDB("mymarketplace","market","handle",m_handle);
	get_my_market += ","+item_to_delete2;
	SetValueDB("mymarketplace","market",get_my_market,"handle",m_handle);

	//81 2c 6bbb0000 6bbb0000 01 02 00 00 00000000 00000000 00 8c 01 00 00 00 00 00 00 00 00;
	ByteBuffer sellItem;
	sellItem << uint16(swap16(0x812c));
	sellItem.append( make_shared<HexGenericMsg>(item_to_delete2+item_to_delete2)->toBuf() );
	sellItem << uint8(1) << uint8(2) << uint16(0) << uint64(0) << uint8(0) << uint16(swap16(0x8c01)) << uint64(0);
	SendRPC(Bin2Hex(sellItem)); sellItem.clear();
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//EMOTE & HOTBAR//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::RPC_Suicide(ByteBuffer &src){
	INFO_LOG(m_handle+" : ReconstructionFrame->(sub)RPC_Suicide@(..)");
	ReconstructionFrame();
}
void PlayerObject::ReconstructionFrame(){
	ByteBuffer packet;
	packet << uint8(0x03) << uint16(m_goId) << uint32(swap32(0x03010d0f)) << uint32(swap32(0x80808001)) << uint32(1);

	SendPacketToCmd(Bin2Hex(packet));
}
void PlayerObject::RPC_Reconstruction(ByteBuffer &src){
	INFO_LOG(m_handle+" : RPC_Reconstruction@(..)");

	ByteBuffer packet;
	packet << uint8(0x03) << uint16(m_goId) << uint32(swap32(0x03010d0f)) << uint32(swap32(0x80808001)) << uint32(0);
	SendPacketToCmd(Bin2Hex(packet));

	SetMood(m_goId, "00");
}
void PlayerObject::RPC_UnmountBuff(ByteBuffer &src){
	uint8 type_buff = src.read<uint8>();
	BUFFER(type_buff, false);

	switch(type_buff){
		case 0xec:
			//unmount my rsi max
			INFO_LOG(m_handle+" : RPC_UnmountBuff@(Type : Unmount RSI Mask)");
			UnmountRSIMask();
			break;
	}
}
void PlayerObject::BUFFER(uint8 a, bool loadA){
	ByteBuffer packet;
	if(loadA == true){
		packet.append( make_shared<HexGenericMsg>("80bc15002f0000")->toBuf() );
	}else if(loadA == false){
		packet.append( make_shared<HexGenericMsg>("80bc1500410000")->toBuf() );
	}
	packet << uint8(a);
	packet.append( make_shared<HexGenericMsg>("0300000802")->toBuf() );

	int buff = 255;
	packet << uint32(buff);

	if(loadA == true){
		packet << uint32(0);
	}else if(loadA == false){
		packet << uint32(1);
	}

	packet  << uint8(0);
	SendRPC(Bin2Hex(packet));
}
void PlayerObject::RPC_CmdRandom(ByteBuffer &src){
	uint64 limit = src.read<uint64>();

	INFO_LOG(m_handle+" : RPC_CmdRandom@(Type : Generating a number, randomly )");

	if(limit <= 0){
		SendMsg("","Error. Format exception.","SYSTEM");
		return;
	}
	
	uint64 rand_limit = rand()%limit + 1;
	m_parent.QueueCommand(make_shared<SystemChatMsg>((format("%1%'s number is : %2%") % m_handle % rand_limit).str()));
	sGame.AnnounceCommand(&m_parent,make_shared<SystemChatMsg>((format("%1%'s number is : %2%") % m_handle % rand_limit).str()));
}
void PlayerObject::RPC_LoadTactic(ByteBuffer &src)
{
	uint8 skill = src.read<uint8>();
	
	switch (skill){
	case 0:
		m_tactic = "SPEED";
		SendMsg("","You are using speed tactic.","SYSTEM");

		BUFFER(0xf5, false); //block
		BUFFER(0xf7, true); //blue
		BUFFER(0xf8, false); //green
		BUFFER(0xf9, false); //red

		break;
	case 1:
		m_tactic = "POWER";
		SendMsg("","You are using power tactic.","SYSTEM");

		BUFFER(0xf5, false); //block
		BUFFER(0xf7, false); //blue
		BUFFER(0xf8, false); //green
		BUFFER(0xf9, true); //red

		break;
	case 2:
		m_tactic = "GRAB";
		SendMsg("","You are using grab tactic.","SYSTEM");

		BUFFER(0xf5, false); //block
		BUFFER(0xf7, false); //blue
		BUFFER(0xf8, true); //green
		BUFFER(0xf9, false); //red

		break;
	case 3:
		m_tactic = "BLOCK";

		BUFFER(0xf5, true); //block
		BUFFER(0xf7, false); //blue
		BUFFER(0xf8, false); //green
		BUFFER(0xf9, false); //red

		SendMsg("","You are using block tactic.","SYSTEM");

		SetMood(m_goId, "00");
		LEAVE_DUEL();

		if(m_pvp == 1 || m_pve == 1){
			m_pvp = 0; //unmount pvp state
			m_pve = 0; //unmount pve state

			//reset my data on pvp
			if(opponentId>32000){
				PlayerObject* opponent = sObjMgr.getGOPtr(opponentId);
				if(opponent->m_pvp == 1){
					LoadAnimation(opponentId,"290707", false); //preserve aggressive mood of opponent

					opponent->opponentId = 0;
					opponent->SendMsg("",m_handle+" left the duel","SYSTEM");

					opponent->x_pvp = opponent->y_pvp = opponent->z_pvp = 0;
				}

				LoadAnimation(m_goId,"293404", true);
			}

			if(defeatedNPC == false){
				opponentId = 0;
			}

			x_pvp = y_pvp = z_pvp = 0;
			x_pve = y_pve = z_pve = 0;

			m_healthC = m_healthM;
			UpdateGUIonDuel();

			SendMsg("","Your state is not PVP or PVE.","SYSTEM");
		}
		break;
	}
}
void PlayerObject::RPC_CheckReputation(ByteBuffer &src)
{
	SendMsg("","Your reputation is "+ this->m_reputation ,"SYSTEM");
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//INVENTORY///////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::SaveInventory(){
	string new_inventory = "";
	for(int i = 0; i<96; i++){
		new_inventory += inventory[i]+",";
	}
	new_inventory = new_inventory.substr(0, new_inventory.length()-1);
	erase_all(new_inventory, " ");
	SetValueDB("myinventory","inventory",new_inventory,"handle", m_handle); //save&store player inventory
}
void PlayerObject::GiveItemToSlot( uint8 slot, string item ){
	ByteBuffer packet;
	packet << uint8(0x5e) << uint8(slot) << uint8(00);
	packet.append( make_shared<HexGenericMsg>(item+"000000001401")->toBuf() );
	SendRPC(Bin2Hex(packet)); packet.clear(); //give item to me, immediately to [slot]
}
void PlayerObject::GiveCash(uint64 new_cash){
	m_cash += new_cash;

	ByteBuffer complete;
	complete << uint16(swap16(0x80e4)) << uint64(m_cash) << uint8(0) << uint32(swap32(0x0f000000));
	SendRPC(Bin2Hex(complete)); complete.clear(); //update inventory panel, cash

	complete << uint16(swap16(0x80e7)) << uint64(new_cash) << uint8(0) << uint32(swap32(0x18000000));
	SendRPC(Bin2Hex(complete)); complete.clear(); //send a message 'you gained %'

	SetValueDB( "characters","cash", (format("%1%")%m_cash).str(), "handle", m_handle); //save data
}
void PlayerObject::RPC_BuyItem(ByteBuffer &src){
	string BYTE_PACKET = ""+(format("%1%") % Bin2Hex(src)).str();
	std::vector<std::string> def;
	boost::split(def, BYTE_PACKET, boost::is_any_of(" "));

	string item = def[2]+def[3]+def[4]+def[5];

	INFO_LOG(m_handle+" : RPC_BuyItem@(id)"+item);

	for(int i = 0; i<96; i++){
		if(inventory[i] == "00000000"){ //evade double storing
			inventory[i] = item;

			GiveItemToSlot(i,item);

			break;
		}
	}

	SaveInventory();

	SendMsg("","You bought 1 item.","SYSTEM");
}
void PlayerObject::RPC_SellItem(ByteBuffer &src){
	if(inMission == true){
		SendMsg("","You cannot sell items during a mission.","SYSTEM");
		return;
	}

	uint8 extra = src.read<uint8>();
	uint16 slot = src.read<uint8>();

	item_to_delete = inventory[slot];

	INFO_LOG(m_handle+" : RPC_SellItem@(id)"+item_to_delete);

	RemoveItem(slot);

	//---
	//check if items is loaded as cloth, tool or weapon
	//61 Hat, 62 Glasses, 63 Shirt, 64 Gloves, 65 Coat, 66 Pant, 68 Shoes |69 Weapon, 6a Tool|
	for(int i = 0; i<9; i++){
		if( this->appearence[i] == item_to_delete ){

			INFO_LOG(m_handle+" : RPC_SellItem@(Type : Unmount)"+ item_to_delete );

			this->appearence[i] = "00000000";

			ByteBuffer removeItem;
			if(i < 6){
				removeItem << uint8(0x5e) << uint16(i+97);
			}else{
				removeItem << uint8(0x5e) << uint16(i+98);
			}
			removeItem.append( make_shared<HexGenericMsg>("00000000000000001401")->toBuf() );
			SendRPC(Bin2Hex(removeItem)); removeItem.clear();

			if(i==0){
				SetValueDB("myappearence","hat","00000000","handle",m_handle);
				SetValueDB("rsivalues", "hat", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==1){
				SetValueDB("myappearence","glasses","00000000","handle",m_handle);
				SetValueDB("rsivalues", "glasses", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==2){
				SetValueDB("myappearence","shirt","00000000","handle",m_handle);
				SetValueDB("rsivalues", "shirt", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==3){
				SetValueDB("myappearence","gloves","00000000","handle",m_handle);
				SetValueDB("rsivalues", "gloves", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==4){
				SetValueDB("myappearence","coat","00000000","handle",m_handle);
				SetValueDB("rsivalues", "coat", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==5){
				SetValueDB("myappearence","pants","00000000","handle",m_handle);
				SetValueDB("rsivalues", "pants", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==6){
				SetValueDB("myappearence","shoes","00000000","handle",m_handle);
				SetValueDB("rsivalues", "shoes", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==7){
				SetValueDB("myappearence","weapon","00000000","handle",m_handle);
			}else if(i==8){
				SetValueDB("myappearence","tool","00000000","handle",m_handle);
			}
		}
	}

	SendMsg("","You sold 1 item.","SYSTEM");

	// ?undo Instructions moved to ?help -bb
	//SendMsg("","You may undo the last transaction using ?undo.","FRAMEMODAL");

	//update my appearence
	this->UpdateAppearance();

	GiveCash( rand()%500 );
}
void PlayerObject::RemoveItem(uint16 slot){
	for(int i = 0; i<96; i++){
		if(i == slot){ //evade double storing
			inventory[i] = "00000000";

			ByteBuffer packet;
			packet << uint8(0x5e) << uint8(i) << uint8(00);
			packet.append( make_shared<HexGenericMsg>("00000000000000001401")->toBuf() );
			SendRPC(Bin2Hex(packet)); packet.clear(); //remove item

			break;
		}
	}

	SaveInventory();
}
void PlayerObject::RPC_CloseVendorFrame(ByteBuffer &src){
	//just catched
}
void PlayerObject::RPC_UseItem(ByteBuffer &src){
	uint16 slot = src.read<uint8>();
	UseItem(slot);
}
void PlayerObject::UseItem(uint16 slot){
	string mId = (format("%1%")%m_characterUID).str();

	//get itemId
	string itemId;
	for(int i = 0; i<96; i++){
		if(i==slot){
			itemId = inventory[i];
			break;
		}
	}

	if(itemId == "11a70000"){
		SendMsg("","{c:ff0000}You can only use the Flit Gun during combat.{/c}","SYSTEM");
		//refer to ActivateItem()@
		return;
	}

	//get itemType
	string itemType, answer;

	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `itemType` FROM `myitems` WHERE `itemId` = '%1%' LIMIT 1") % itemId) );
	if (value == NULL){ /*do nothing*/ return; }else{
		Field *field_value = value->Fetch();
		itemType = ""+format(field_value[0].GetString()).str();
	}

	//if can be loaded
	scoped_ptr<QueryResult> result_ATTEMPT(sDatabase.Query(format("SELECT `DO_THIS` FROM `myuseitem` WHERE `itemId` = '%1%' LIMIT 1") % itemId) );
	if (result_ATTEMPT == NULL){ /*do nothing*/ return; }else{
		Field *field_ATTEMPT = result_ATTEMPT->Fetch();
		string instruction = ""+format(field_ATTEMPT[0].GetString()).str();
		
		//scanning function
		if(instruction != "NONE"){
			instruction = base64_decode(instruction);
			if( instruction == "RESTORE_ATTRIBUTES" ){
				CastFXOn(m_goId,"0D000012");

				answer = "80 b2 52 00   08   00 08 02"; //belief
				SendRPC(answer);
				answer = "80 b2 4f 00   05   00 08 02"; //percepition
				SendRPC(answer);
				answer = "80 b2 51 00   0b   00 08 02"; //reason
				SendRPC(answer);
				answer = "80 b2 4e 00   08   00 08 02"; //focus
				SendRPC(answer);
				answer = "80 b2 54 00   08   00 08 02"; //vitality
				SendRPC(answer);

				int total_attributes = 2+m_lvl;
				SetValueDB("myattributes","total", (format("%1%")%total_attributes).str() ,"handle", m_handle );
				SendMsg("","Your character's attributes have been restored!","BROADCAST");

				RemoveItem(slot);
			}else if( instruction == "CHANGE_DISTRICT" ){
				/*
				85960000 ONEZERO (Machine Town)
				86960000 ZERONE
				35b30000 TRAININGMISSIONITEM
				*/

				/*
					DISTRICTS:
				TUTORIAL = 0
				SLUMS = 1
				DOWNTOWN = 2
				INTERNATIONAL = 3
				ARCHIVE01 = 4
				ARCHIVE02 = 5
				ASHENCOURT = 6		8A960000 ASHENCOURTE
				DATAMINE = 7		a6b80000 DATAMINE
				SAKURA = 8			89960000 SAKURA
				SATI = 9			6eba0000 SATI
				WIDOWSMOOR = 10		88960000 WINDOW'SMOOR
				YUKI = 11			87960000 YUKI
				LARGE01 = 12
				LARGE02 = 13
				MEDIUM01 = 14
				MEDIUM02 = 15
				MEDIUM03 =16
				SMALL03 = 17
				*/

				UpdateConstruct(itemId);
			}
		}
	}

}
void PlayerObject::RPC_LoadItem(ByteBuffer &src){
	//
	uint8 slot = src.read<uint8>();
	uint8 info = src.read<uint8>();

	if(info == 0x6b){
		string itemId = this->inventory[slot];
		INFO_LOG(m_handle+" : RPC_LoadItem@6b(..)"+itemId);
		LoadItem(itemId);
	}else if(info == 0x6a){
		string itemId = this->inventory[slot];
		INFO_LOG(m_handle+" : RPC_LoadItem@6a(..)"+itemId);
		UseItem(slot);
	}else if(info == 0x69){
		string itemId = this->inventory[slot];

		if(itemId == "22b60000"){ //Agent Gun
			INFO_LOG(m_handle+" : RPC_LoadItem@69(Type : Agent Gun)"+itemId);
			LoadWeapon(m_goId, "22b60000");
			LoadAnimation(m_goId,"000000",false);
		}
	}else{
		INFO_LOG("|"+Bin2Hex(src)+"|");
	}
}
void PlayerObject::LoadItem(string itemId){
	string itemType;
	//---
	//check if skill
	scoped_ptr<QueryResult> new_skill(sDatabase.Query(format("SELECT `animForMe`, `animForOpponentPVP`, `animForOpponentPVE`, `CAST_FX`, `FXForMe`, `FXForOpponentPVP`, `DAMAGE`, `DISTANCE`, `weaponForMe` FROM `mynewskills` WHERE `skillId` = '%1%' LIMIT 1") % itemId) );
	if (new_skill == NULL){ /*do nothign*/ }else{
		SendMsg("","Object "+itemId+" exists, skill.","SYSTEM");

		//import data
		Field *field_value = new_skill->Fetch();
		string animForMe = field_value[0].GetString();
		string animForOpponentPVP = field_value[1].GetString();
		string animForOpponentPVE = field_value[2].GetString();
		string CAST_FX = field_value[3].GetString();
		string FXForMe = field_value[4].GetString();
		string FXForOpponentPVP = field_value[5].GetString();
		int DAMAGE = atoi( ((format(field_value[6].GetString())).str()).c_str() );
		double DISTANCE = field_value[7].GetDouble();
		string weaponForMe = field_value[8].GetString();

		if(m_pvp == 1) {
			if(opponentId > 32000){  //opponent = human
				if(opponentId != m_goId){
					InDuel_PVP(animForMe, animForOpponentPVP, animForOpponentPVE, CAST_FX, FXForMe, FXForOpponentPVP, DAMAGE, DISTANCE, weaponForMe);
				}else{
					SendMsg("","You cannot attack yourself.","STSTEM");
					return;
				}
			}
		}else if(m_pve == 1) {
			//if(opponentId > 0 && opponentId < 32000){  //opponent = human
			//	SendMsg("","You can attack this NPC.","STSTEM");
			//}
		}else{
			SendMsg("","Please, start a PVE or PVP duel.\n\nIn case of PVP duel, type: /pvp\nThen, type:\n ?duel <opponentHandle>","FRAMEMODAL");
		}

		return;
	}

	//---
	//check if item
	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `itemType` FROM `myitems` WHERE `itemId` = '%1%' LIMIT 1") % itemId) );
	if (value == NULL){
		SendMsg("","Object "+itemId+" doesn't exist.","SYSTEM");
		return;
	}else{
		Field *field_value = value->Fetch();
		itemType = format(field_value[0].GetString()).str();
		SendMsg("","Object "+itemId+" exists.","SYSTEM");
	}

	//---
	//check if object
	if(itemType == "ITEM_RSI_CAPTURE" || itemType == "ITEM_RSI_DISGUISE" || itemType == "KEY" || itemType == "MISSION_QUEST" || itemType == "UNKNOWN" || itemType == "RSI_PILL"){
		SendMsg("","Please, right click on the item then select '{i}Activate Item{/i}' or '{i}Use Item{/i}'. - Attention!\nSome items cannot be loaded.","FRAMEMODAL");
		return;
	}else if(itemType == "CAST_FX"){
		if(m_currentFX == "NULL"){
			if(itemId == "92ba0000"){ //KunoDoll
				//0x6a
				m_currentFX = "KUNODOLL";

				SendMsg("","{c:ff0000}You changed your FX.{/c}","SYSTEM");
			}
		}else{
			m_currentFX = "NULL";
		}
		return;
	}

	//---
	//check if cloth
	string model, color, clothType;
	bool is_cloth = false;
	if(itemType == "CLOTH" || itemType == "SHOES" || itemType == "PANTS" || itemType == "GLASSES" || itemType == "SHIRT" || itemType == "COAT" || itemType == "HAT_BANDANA" || itemType == "HAT" || itemType == "GLOVES" ){
		scoped_ptr<QueryResult> valueA(sDatabase.Query(format("SELECT `clothType`, `model`, `color` FROM `myclothes` WHERE `clothId` = '%1%' LIMIT 1") % itemId) );
		if (valueA == NULL){ /*do nothing*/ }else{
			Field *field_valueA = valueA->Fetch();
			clothType = format(field_valueA[0].GetString()).str();
			model = format(field_valueA[1].GetString()).str();
			color = format(field_valueA[2].GetString()).str();
			is_cloth = true;
			INFO_LOG(m_handle+" : LoadItem@(Type : Cloth - " + clothType + ")"+itemId);
		}
	}

	if(is_cloth == true){
		string mId = (format("%1%")%m_characterUID).str();
		string itemAttributeForColor;

		ByteBuffer packet;
		packet << uint8(0x5e);

		//61 Hat, 62 Glasses, 63 Shirt, 64 Gloves, 65 Coat, 66 Pant, 68 Shoes |69 Weapon, 6a Tool|
		if(itemType == "HAT_BANDANA" || itemType == "HAT"){
			itemType = "HAT";
			clothType = "NULL";
			packet << uint16(swap16(0x6100));
			this->appearence[0] = itemId;
		}else if(itemType == "GLASSES"){
			clothType = "glassescolor";
			packet << uint16(swap16(0x6200));
			this->appearence[1] = itemId;
		}else if(itemType == "SHIRT"){
			clothType = "shirtcolor";
			packet << uint16(swap16(0x6300));
			this->appearence[2] = itemId;
		}else if(itemType == "GLOVES"){
			clothType = "NULL";
			packet << uint16(swap16(0x6400));
			this->appearence[3] = itemId;
		}else if(itemType == "COAT"){
			clothType = "coatcolor";
			packet << uint16(swap16(0x6500));
			this->appearence[4] = itemId;
		}else if(itemType == "PANTS"){
			clothType = "pantscolor";
			packet << uint16(swap16(0x6600));
			this->appearence[5] = itemId;
		}else if(itemType == "SHOES"){
			clothType = "shoecolor";
			packet << uint16(swap16(0x6800));
			this->appearence[6] = itemId;
		}

		packet.append( make_shared<HexGenericMsg>(itemId+"000000001401")->toBuf() );
		SendRPC(Bin2Hex(packet));

		SetValueDB("rsivalues",itemType,model,"charId",mId);
		if(clothType != "NULL"){
			SetValueDB("rsivalues",clothType,color,"charId",mId);
		}

		SetValueDB("myappearence",itemType,itemId,"handle",m_handle);

		//update my appearence
		this->UpdateAppearance();
	}

	//---
}
void PlayerObject::LoadWeapon(uint32 a, string weapon){
	ByteBuffer packet;

	packet << uint8(0x03) << uint16(m_goId);
	packet.append( make_shared<HexGenericMsg>("03010c0f808409808088"+weapon+"901100000000")->toBuf() );
	SendPacketToCmd(Bin2Hex(packet)); packet.clear();

	packet << uint8(0x03) << uint16(m_goId);
	packet.append( make_shared<HexGenericMsg>("0280808002"+weapon+"0000")->toBuf() );
	SendPacketToCmd(Bin2Hex(packet)); packet.clear();
}
void PlayerObject::RPC_UnmountItem(ByteBuffer &src){
	uint8 slotA = src.read<uint8>();

	ByteBuffer removeItem;
	GiveItemToSlot(slotA,"00000000");

	string itemA = inventory[slotA];
	UpdateConstruct(itemA);

	string item_to_unmount;
	if(slotA < 0x67){
		item_to_unmount = this->appearence[slotA-97];
	}else{
		item_to_unmount = this->appearence[slotA-98];
	}

	INFO_LOG(m_handle+" : RPC_UnmountItem@(..)"+ item_to_unmount );
	
	//---
	//check if items is loaded as cloth, tool or weapon
	//61 Hat, 62 Glasses, 63 Shirt, 64 Gloves, 65 Coat, 66 Pant, 68 Shoes |69 Weapon, 6a Tool|
	for(int i = 0; i<9; i++){
		if( this->appearence[i] == item_to_unmount ){
			this->appearence[i] = "00000000";

			if(i < 6){
				removeItem << uint8(0x5e) << uint16(i+97);
			}else{
				removeItem << uint8(0x5e) << uint16(i+98);
			}
			removeItem.append( make_shared<HexGenericMsg>("00000000000000001401")->toBuf() );
			SendRPC(Bin2Hex(removeItem)); removeItem.clear();

			if(i==0){
				SetValueDB("myappearence","hat","00000000","handle",m_handle);
				SetValueDB("rsivalues", "hat", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==1){
				SetValueDB("myappearence","glasses","00000000","handle",m_handle);
				SetValueDB("rsivalues", "glasses", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==2){
				SetValueDB("myappearence","shirt","00000000","handle",m_handle);
				SetValueDB("rsivalues", "shirt", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==3){
				SetValueDB("myappearence","gloves","00000000","handle",m_handle);
				SetValueDB("rsivalues", "gloves", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==4){
				SetValueDB("myappearence","coat","00000000","handle",m_handle);
				SetValueDB("rsivalues", "coat", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==5){
				SetValueDB("myappearence","pants","00000000","handle",m_handle);
				SetValueDB("rsivalues", "pants", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==6){
				SetValueDB("myappearence","shoes","00000000","handle",m_handle);
				SetValueDB("rsivalues", "shoes", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==7){
				SetValueDB("myappearence","weapon","00000000","handle",m_handle);
			}else if(i==8){
				SetValueDB("myappearence","tool","00000000","handle",m_handle);
			}
		}
	}

	//update my appearence
	this->UpdateAppearance();
}
void PlayerObject::RPC_SwapItems(ByteBuffer &src){
	uint8 slotA = src.read<uint8>();
	uint8 extra = src.read<uint8>();
	uint8 slotB = src.read<uint8>();

	string itemA = this->inventory[slotA];
	string itemB = this->inventory[slotB];

	ByteBuffer swapItems;
	swapItems << uint8(0x5e) << uint8(slotA) << uint8(00);
	swapItems.append( make_shared<HexGenericMsg>(itemB+"000000001401")->toBuf() );
	SendRPC(Bin2Hex(swapItems)); swapItems.clear();

	this->inventory[slotA] = itemB;

	swapItems << uint8(0x5e) << uint8(slotB) << uint8(00);
	swapItems.append( make_shared<HexGenericMsg>(itemA+"000000001401")->toBuf() );
	SendRPC(Bin2Hex(swapItems)); swapItems.clear();

	this->inventory[slotB] = itemA;

	string new_inventory = "";
	for(int i = 0; i<96; i++){
		new_inventory += this->inventory[i]+",";
	}
	new_inventory = new_inventory.substr(0, new_inventory.length()-1);
	SetValueDB("myinventory","inventory",new_inventory,"handle",m_handle);
}
void PlayerObject::RPC_RecycleItem(ByteBuffer &src){
	if(inMission == true){
		SendMsg("","You cannot recycle an item during a mission.","SYSTEM");
		return;
	}

	uint8 slotA = src.read<uint8>();

	ByteBuffer removeItem;
	GiveItemToSlot(slotA,"00000000");

	string new_inventory = "";
	for(int i = 0; i<96; i++){
		if(i == slotA){
			item_to_delete = this->inventory[i];
			this->inventory[slotA] = "00000000";

			new_inventory += "00000000,";
		}else{
			new_inventory += this->inventory[i]+",";
		}
	}
	new_inventory = new_inventory.substr(0, new_inventory.length()-1);
	SetValueDB("myinventory","inventory",new_inventory,"handle",m_handle);

	INFO_LOG(m_handle+" : RPC_RecycleItem@(..)"+ item_to_delete );
	
	//---
	//check if items is loaded as cloth, tool or weapon
	//61 Hat, 62 Glasses, 63 Shirt, 64 Gloves, 65 Coat, 66 Pant, 68 Shoes |69 Weapon, 6a Tool|
	for(int i = 0; i<9; i++){
		if( this->appearence[i] == item_to_delete ){

			INFO_LOG(m_handle+" : RPC_RecycleItem@(Type : Unmount)"+ item_to_delete );

			this->appearence[i] = "00000000";

			if(i < 6){
				removeItem << uint8(0x5e) << uint16(i+97);
			}else{
				removeItem << uint8(0x5e) << uint16(i+98);
			}
			removeItem.append( make_shared<HexGenericMsg>("00000000000000001401")->toBuf() );
			SendRPC(Bin2Hex(removeItem)); removeItem.clear();

			if(i==0){
				SetValueDB("myappearence","hat","00000000","handle",m_handle);
				SetValueDB("rsivalues", "hat", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==1){
				SetValueDB("myappearence","glasses","00000000","handle",m_handle);
				SetValueDB("rsivalues", "glasses", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==2){
				SetValueDB("myappearence","shirt","00000000","handle",m_handle);
				SetValueDB("rsivalues", "shirt", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==3){
				SetValueDB("myappearence","gloves","00000000","handle",m_handle);
				SetValueDB("rsivalues", "gloves", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==4){
				SetValueDB("myappearence","coat","00000000","handle",m_handle);
				SetValueDB("rsivalues", "coat", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==5){
				SetValueDB("myappearence","pants","00000000","handle",m_handle);
				SetValueDB("rsivalues", "pants", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==6){
				SetValueDB("myappearence","shoes","00000000","handle",m_handle);
				SetValueDB("rsivalues", "shoes", "0", "charId", (format("%1%")%m_characterUID).str() );
			}else if(i==7){
				SetValueDB("myappearence","weapon","00000000","handle",m_handle);
			}else if(i==8){
				SetValueDB("myappearence","tool","00000000","handle",m_handle);
			}
		}
	}

	// ?Undo instructions moved to ?help
	//if(show_recycling_msg_once == 0){
	//	SendMsg("","If you want to undo this operation, you can type [ ?undo ].\n\nThe command is valid until your next action.","FRAMEMODAL");
	//	show_recycling_msg_once = 1;
	//}

	//update my appearence
	this->UpdateAppearance();

	GiveCash( rand()%500 );
}
void PlayerObject::RPC_ActivateItem(ByteBuffer &src){
	//INFO_LOG("|"+Bin2Hex(src)+"|");

	string BYTE_PACKET = ""+(format("%1%") % Bin2Hex(src)).str();
	std::vector<std::string> def;
	boost::split(def, BYTE_PACKET, boost::is_any_of(" "));

	if(def[2]+def[3] == "8400" || def[2]+def[3] == "8500"){ //karate+aikido, flit_gun
		uint16 skill = src.read<uint16>();
		opponentId = src.read<uint16>();

		if(opponentId > 32000) { //opponent = human
			PlayerObject* player = sObjMgr.getGOPtr(opponentId);
			INFO_LOG(m_handle+" : RPC_ActivateItem@(Type : Attack Human)"+ player->m_handle);
		}else{ //opponent = npc
			INFO_LOG(m_handle+" : RPC_ActivateItem->(sub)RPC_UseFreeAttackStyle@(Type : Attack NPC (?) )"+ (format("%1%")%opponentId).str() );
		}
		return;
	}else  if(def[2]+def[3] == "ee03"){ //coder skill
		SendMsg("","You cannot use this skill.","SYSTEM");
		return;
	}else  if(def[2]+def[3] == "0c00"){ //hyperspeed
		if(m_currentFX == "HYPER_SPEED"){ //is active -> deactive

			ByteBuffer packet;
			packet << uint8(0x03) << uint16(m_goId);
			packet.append( make_shared<HexGenericMsg>("02 88 05 8c 1e 66 66 A63A 80 90 3200 b0 00000000")->toBuf() );
			packet << uint8(GetProgressiveByte());
			packet.append( make_shared<HexGenericMsg>("02 00 00 00 00")->toBuf() );
			SendPacketToMe(Bin2Hex(packet));

			m_currentFX = "NULL";
		}else{ //is deactive -> active

			ByteBuffer packet;
			packet << uint8(0x03) << uint16(m_goId);
			packet.append( make_shared<HexGenericMsg>("02 88 05 8c 1e 66 66 F63F 80 90 3200 b0 56060028")->toBuf() );
			packet << uint8(GetProgressiveByte());
			packet.append( make_shared<HexGenericMsg>("02 00 10 00 00")->toBuf() );
			SendPacketToMe(Bin2Hex(packet));

			m_currentFX = "HYPER_SPEED";
		}
		return;
	}else if(def[2]+def[3] == "c703" || def[2]+def[3] == "b800" || def[2]+def[3] == "3900" || def[2]+def[3] == "ee03" || def[2]+def[3] == "d900" || def[2]+def[3] == "c600"){
		/*
		EVADE CRASH
		unsupported skills (?):
			MobiusCode			c703
			HyperSprint			b800
			LogicBarrage 1.0	3900
			Hacker Style		ee03
			Evade Combat		d900
			CheapShot			c600
		*/
		if(def[2]+def[3] == "c600" && def[6] == "05"){
			SendMsg("","can be supported!","FRAMEMODAL");
		}else{
			SendMsg("","Unsupported!","SYSTEM");
		}
		return;
	}else if(def[2]+def[3] == "d403"){
		uint16 skill = src.read<uint16>();
		opponentId = src.read<uint16>();
		opponentType = src.read<uint16>();

		weapon_FLYMAN = true;
		INFO_LOG(m_handle+" : RPC_ActivateItem@(Type : Using FlitGun on NPC )"+ (format("%1%")%opponentId).str() );
		InDuel_PVE(opponentId, "29d914", "cc0000", 50, false);
		return;
	}

	//4->33! - 34=slot
	uint8 slot;
	for(int i = 0; i <= 35; i++){
		slot = src.read<uint8>();
	}

	string itemId = this->inventory[slot];

	INFO_LOG(m_handle+" : RPC_ActivateItem@(..)"+ itemId );

	//---
	//check if LESIG item
	if(itemId == "04b90000" || itemId == "05b90000" || itemId == "06b90000" || itemId == "07b90000" || itemId == "08b90000"){
		if(PAL == "ADMIN" || PAL == "MOD" || PAL == "LESIG"){
			if(itemId == "04b90000"){ //LESIGSpawnCyphAbility
				SetNPC_SERIE(2,10,"circle_positive",false,"63090058","Cypherite SMG Specialist");
			}
			if(itemId == "05b90000"){ //LESIGSpawnEPNAbility
				SetNPC_SERIE(2,10,"circle_positive",false,"F0030008","E Pluribus Neo");
			}
			if(itemId == "06b90000"){ //LESIGSpawnLupinesAbility
				SetNPC_SERIE(2,10,"circle_positive",false,"79050040","Lupine Scrapper");
			}
			if(itemId == "07b90000"){ //LESIGSpawnSWATAbility
				SetNPC_SERIE(2,10,"circle_positive",false,"24030058","Tactical Security");
			}
			if(itemId == "08b90000"){ //LESIGSpawnZionAbility
				SetNPC_SERIE(2,10,"circle_positive",false,"F0030008","Zion SMG Specialist");
			}
		}else{
			SendMsg("","{c:ff0000}You cannot use this item. (Type ?rank){/c}","SYSTEM");
		}
		return;
	}

	//object is in database?
	string itemType;
	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `itemType` FROM `myitems` WHERE `itemId` = '%1%' LIMIT 1") % itemId) );
	if (value == NULL){
		SendMsg("","Object "+itemId+" doesn't exist.","SYSTEM");
		return;
	}else{
		Field *field_value = value->Fetch();
		itemType = format(field_value[0].GetString()).str();
		SendMsg("","Object "+itemId+" exists.","SYSTEM");
	}

	//---
	//check if rsi pill
	if(itemType == "RSI_PILL"){
		string editParam, color;

		scoped_ptr<QueryResult> valueA(sDatabase.Query(format("SELECT `param`, `value` FROM `myrsipills` WHERE `pillId` = '%1%' LIMIT 1") % itemId) );
		if (valueA == NULL){ }else{
			Field *field_value = valueA->Fetch();
			editParam = format(field_value[0].GetString()).str();
			color = format(field_value[1].GetString()).str();

			SetValueDB("rsivalues", editParam, color, "charId", (format("%1%")%m_characterUID).str() );

			INFO_LOG(m_handle+" : RPC_ActivateItem@(Type : RSI Pill - "+ editParam +")"+ itemId );

			RemoveItem(slot);

			//update my appearence
			this->UpdateAppearance();
			return;
		}
	}

	//---
	//check if rsi mask(or)disguise
	if(itemType == "ITEM_RSI_CAPTURE" || itemType == "ITEM_RSI_DISGUISE"){
		scoped_ptr<QueryResult> valueA(sDatabase.Query(format("SELECT `rsiHexPacket` FROM `myrsimask` WHERE `rsiHexInventory` = '%1%' LIMIT 1") % itemId) );
		if (valueA == NULL){ }else{
			Field *field_value = valueA->Fetch();

			if(myRSImask == "NULL"){
				myRSImask = format(field_value[0].GetString()).str();

				INFO_LOG(m_handle+" : RPC_ActivateItem@(Type : Mount RSI Mask - "+ myRSImask +")"+ itemId );

				this->SetRSIMask(myRSImask);
				BUFFER(0xec, true);
			}else{
				myRSImask = "NULL";

				INFO_LOG(m_handle+" : RPC_ActivateItem@(Type : Unmount RSI Mask)"+ itemId );

				UnmountRSIMask();

				BUFFER(0xec, false);
			}
			
			return;
		}
	}
	/**/

	//---
}
void PlayerObject::SetRSIMask(string itemId){
	myRSImask = itemId;

	ByteBuffer packet;
	packet << uint8(0x03) << uint16(m_goId);
	packet.append( make_shared<HexGenericMsg>("028100808080b000000000ab04"+myRSImask+"00")->toBuf() );
	SendPacketToMe(Bin2Hex(packet)); packet.clear(); //mount rsi for my client

	vector<uint32> allObjects = sObjMgr.getAllGOIds();
	foreach(uint32 objId, allObjects){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(objId);
		}catch (ObjectMgr::ObjectNotAvailable){
			continue;
		}
		if( player->m_handle != m_handle && player->m_district == m_district){
			packet << uint8(0x03) << uint16(objId);
			packet.append( make_shared<HexGenericMsg>("0280808050")->toBuf() );
			packet << uint16( player->m_innerStrC ) << uint16( player->m_healthC ) << uint16(m_goId);
			packet.append( make_shared<HexGenericMsg>("0281008090530e8001"+myRSImask)->toBuf() );
			player->SendPacketToCmd(Bin2Hex(packet)); packet.clear(); //set rsi for all clients
		}
	}
}
void PlayerObject::UnmountRSIMask(){
	ByteBuffer packet;
	packet << uint8(0x03) << uint16(m_goId);
	packet.append( make_shared<HexGenericMsg>("028101808080300000000000000000000000")->toBuf() );
	SendPacketToMe(Bin2Hex(packet)); packet.clear(); //unmount rsi for me

	packet.append( make_shared<HexGenericMsg>("030100010100")->toBuf() );
	packet << uint16(m_goId) << uint8(0); //remove my rsi, we will use this later

	vector<uint32> allObjects = sObjMgr.getAllGOIds();
	foreach(uint32 objId, allObjects){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(objId);
		}catch (ObjectMgr::ObjectNotAvailable){
			continue;
		}
		if( player->m_handle != m_handle && player->m_district == m_district){
			player->SendPacketToMe(Bin2Hex(packet)); //send packet
		}
	}

	shared_ptr<PlayerSpawnMsg> dMsg = make_shared<PlayerSpawnMsg>(m_goId);
	sGame.AnnounceStateUpdate(&m_parent,dMsg); //spawn my rsi again, with no rsi mask

	DestroyChar(); //if district is different
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//LOOT////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::RPC_LootAllowed(ByteBuffer &src)
{
	ByteBuffer packet;

	GiveCash( rand()%500 );

	//choice = 'take all'
		std::vector<std::string> def;
		boost::split(def, m_loot, boost::is_any_of(","));

		for(int i = 0; i<(int)def.size(); i++){
			ByteBuffer packet2;
			packet2.append( make_shared<HexGenericMsg>("8189")->toBuf() );
			packet2 << uint32(m_characterUID);
			packet2.append( make_shared<HexGenericMsg>(def[i])->toBuf() );
			SendRPC(Bin2Hex(packet2));

			bool setted_item = false;
			for(int j = 0; j<96; j++){
				if(inventory[j] == "00000000" && setted_item == false){ //evade double storing
					inventory[j] = def[i];

					GiveItemToSlot(j,def[i]);

					setted_item = true;
				}
			}
		}

		SaveInventory();

		packet.append( make_shared<HexGenericMsg>("030100020600")->toBuf() );
		packet << uint16(viewId_loot);
		packet.append( make_shared<HexGenericMsg>("010000000000")->toBuf() ); 
		SendPacketToCmd(Bin2Hex(packet)); packet.clear(); //loof fx off

		m_loot = "";
		//SendMsg("","Loot accepted.","SYSTEM");

		if(inMission == true && command_obj[curr_obj] == "LOOT" ){
			ByteBuffer deleteNPC;
			deleteNPC << uint8(0x03) << uint32(swap32(0x01000101)) << uint8(0) << uint16(viewId_loot) << uint16(0);
			SendPacketToMe(Bin2Hex(deleteNPC));
			UpdateMission();
		}

		viewId_loot=0;
	//

	return;
}
void PlayerObject::RPC_LootRejected(ByteBuffer &src)
{

	m_loot = "";
	//SendMsg("","Loot rejected.","SYSTEM");

	return;
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//ANIMATION & FX//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

uint8 PlayerObject::GetProgressiveByte(){
	if(INDEX_ANIM_AND_FX == 255){
		INDEX_ANIM_AND_FX = 1;
	}else if(INDEX_ANIM_AND_FX < 255){
		INDEX_ANIM_AND_FX++;
	}
	return INDEX_ANIM_AND_FX;
}
void PlayerObject::LoadAnimation(uint32 a, string animation, bool GOOD){
	ByteBuffer packet;
	packet << uint8(0x03) << uint16(a) << uint16(swap16(0x0128)) << uint8(GetProgressiveByte()) << uint16(swap16(0x4000));
	packet.append( make_shared<HexGenericMsg>(animation)->toBuf());
	packet << uint16(swap16(0x0001)) << float(this->getPosition().x) << float(this->getPosition().y) << float(this->getPosition().z);
	packet << uint32(swap32(0x209f1e20)) << uint16(0);
	SendPacketToCmd(Bin2Hex(packet)); packet.clear();

	if(GOOD == false){
		SetMood(a,"0c");
	}else{
		SetMood(a,"00");
	}
}
void PlayerObject::LoadAnimation_NPC(uint16 npcId, int npcHP, string npcAnim){
	ByteBuffer packet;

	if(opponentType != 0xf4){
		packet << uint8(0x03) << uint16(npcId) << uint16(swap16(0x0627)) << GetProgressiveByte() << uint16(swap16(0x010c));
		packet.append( make_shared<HexGenericMsg>(npcAnim+"0010020088000c00008080808040")->toBuf() );
		packet << uint16(npcHP) << uint16(0);
	}else if(opponentType == 0xf4){
		packet << uint8(0x03) << uint16(npcId) << uint16(swap16(0x022e)) << GetProgressiveByte();
		packet.append( make_shared<HexGenericMsg>("810cb6d90402008d0000")->toBuf() );
		packet << uint8(NPCrot) << float(NPCx_pve) << float(NPCy_pve) << float(NPCz_pve) << uint16(0);
	}

	SendPacketToCmd(Bin2Hex(packet));

	SetMood_NPC(npcId, "0c");
}
void PlayerObject::SetMood_NPC(uint16 a, string mood){
	ByteBuffer test;
	test << uint8(0x03) << uint16( a );
	test.append( make_shared<HexGenericMsg>("0201"+mood+"00000000")->toBuf() );
	SendPacketToCmd(Bin2Hex(test)); test.clear(); //stand pose - aggressive mood
}
void PlayerObject::SetMood(uint32 a, string mood){
	ByteBuffer packet;
	packet << uint8(0x03) << uint16(a);
	packet.append( make_shared<HexGenericMsg>("0301"+mood+"000000")->toBuf() );
	SendPacketToCmd(Bin2Hex(packet));
}
void PlayerObject::CastFXOn(uint32 a, string FX){
	//for my client
	ByteBuffer packet;
	packet << uint8(0x03) << uint16(a);
	/*packet.append( make_shared<HexGenericMsg>("028080808030"+FX)->toBuf() ); packet << uint8(GetProgressiveByte());
	packet.append( make_shared<HexGenericMsg>("0000000000")->toBuf() );*/
	packet.append( make_shared<HexGenericMsg>("0280808080b0"+FX)->toBuf() ); packet << uint32(GetProgressiveByte());
	SendPacketToCmd(Bin2Hex(packet)); packet.clear();

	//for all
	packet << uint8(0x03) << uint16(a);
	/*packet.append( make_shared<HexGenericMsg>("028080800c"+FX)->toBuf() ); packet << uint8(GetProgressiveByte());
	packet.append( make_shared<HexGenericMsg>("03000100047dfe88460080f743728c574500")->toBuf() );*/
	packet.append( make_shared<HexGenericMsg>("028080800c"+FX)->toBuf() ); packet << uint32(GetProgressiveByte());
	SendPacketToCmd(Bin2Hex(packet)); packet.clear();
}
void PlayerObject::CastFXOn_PRIVATE(uint32 a, string FX){
	//for all
	ByteBuffer packet;
	packet << uint8(0x03) << uint16(a);
	packet.append( make_shared<HexGenericMsg>("028080800c"+FX)->toBuf() ); packet << uint32(GetProgressiveByte());
	SendPacketToCmd(Bin2Hex(packet)); packet.clear();
}
void PlayerObject::SPAWN_FX_TARGET(double a, double b, double c, string FX, uint16 SCALE){
	//y-90 player*

	ByteBuffer packet;
	packet << uint16(swap16(0x8143));
	packet.append(make_shared<HexGenericMsg>(FX)->toBuf() );
	packet << double(a) << double(b) << double(c) << uint32(swap32(0x0000)) << uint16(swap16(SCALE));
	sGame.Broadcast(packet,true);
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//ATTRIBUTES & SKILLS/////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::RPC_SetAttributes(ByteBuffer &src){
	uint16 extra0 = src.read<uint16>();

	uint16 extra1 = 0;
	uint16 attribute1 = 0;
	uint16 extra2 = 0;
	uint8 quantity1 = 0; //end first block

	ByteBuffer packet;

	INFO_LOG(m_handle+" : RPC_SetAttributes@(..)");

	uint8 number_attributes = src.read<uint8>();
	switch (number_attributes){
		case 0x01:
			extra1 = src.read<uint16>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 1st block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();
			break;
		case 0x02:
			extra1 = src.read<uint16>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 1st block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();

			extra1 = src.read<uint8>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 2nd block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();
			break;
		case 0x03:
			extra1 = src.read<uint16>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 1st block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();

			extra1 = src.read<uint8>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 2nd block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();

			extra1 = src.read<uint8>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 3rd block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();
			break;
		case 0x04:
			extra1 = src.read<uint16>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 1st block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();

			extra1 = src.read<uint8>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 2nd block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();

			extra1 = src.read<uint8>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 3rd block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();

			extra1 = src.read<uint8>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 4th block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();
			break;
		case 0x05:
			extra1 = src.read<uint16>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 1st block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();

			extra1 = src.read<uint8>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 2nd block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();

			extra1 = src.read<uint8>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 3rd block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();

			extra1 = src.read<uint8>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 4th block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();
			
			extra1 = src.read<uint8>();
			attribute1 = src.read<uint16>();
			extra2 = src.read<uint16>();
			quantity1 = src.read<uint8>(); //end 5th block

			packet << uint16(swap16(0x80b2)) << uint16(attribute1) << uint16(TYPE_ATTRIBUTE(attribute1, quantity1)) << uint16(swap16(0x0802));
			SendRPC(Bin2Hex(packet)); packet.clear();
			break;
	}

	bool store_attributes = sDatabase.Execute(format("UPDATE `myattributes` SET `belief` = '%1%', `percepition` = '%2%', `reason` = '%3%', `focus` = '%4%', `vitality` = '%5%' , `total` = '72' WHERE `handle` = '%6%'") % belief % percepition % reason % focus % vitality % m_handle);
	SendMsg("","Your attributes were updated successfully!","SYSTEM");
}
uint16 PlayerObject::TYPE_ATTRIBUTE(uint16 attribute, uint16 quantity){
	uint16 def = 0;
	switch(attribute){
		case 0x52: //belief
			def = belief+quantity;
			belief = def;
			break;
		case 0x4f: //percepition
			def = percepition+quantity;
			percepition = def;
			break;
		case 0x51: //reason
			def = reason+quantity;
			reason = def;
			break;
		case 0x4e: //focus
			def = focus+quantity;
			focus = def;
			break;
		case 0x54: //vitality
			def = vitality+quantity;
			vitality = def;
			break;
	}
	return def;
}
	//IMPLEMENT SKILLS(...)
void PlayerObject::UpdateGUIonDuel(){
	ByteBuffer packet;
	packet << uint8(0x03) << uint16(m_goId) << uint8(0x02) << uint32(swap32(0x80808050)) << uint16(m_innerStrC) << uint16(m_healthC) << uint16(0);
	SendPacketToMe(Bin2Hex(packet)); packet.clear();
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//BUDDY LIST//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::UpdateBuddyList(bool reportMyAbsence){
	ByteBuffer packet;

	std::vector<std::string> list_buddy;
	boost::split(list_buddy, buddyList, boost::is_any_of(","));

	if(buddyList != "NONE"){
		string handles = "";
		vector<uint32> allObjects = sObjMgr.getAllGOIds();
		foreach(uint32 objId, allObjects){
			PlayerObject* playerObj = NULL;
			try{
				playerObj = sObjMgr.getGOPtr(objId);
			}catch (ObjectMgr::ObjectNotAvailable){
				continue;
			}
			string handle = playerObj->getHandle();
			//if( handle!=m_handle ){
				handles += handle+",";

				for(int i = 1; i<(int)list_buddy.size(); i++){
					if( iequals(list_buddy[i], handle) ){
						packet << uint32(swap32(0x80db0800)) << uint32(swap32(0x3b00008a));
						packet.writeString("SOE+MXO+Syntax+"+m_handle);
						playerObj->SendRPC(Bin2Hex(packet)); packet.clear(); //delete from list -- it can appear offline

						if(reportMyAbsence == false){
							packet << uint32(swap32(0x80d70800)) << uint32(swap32(0x3b00008e));
							packet.writeString("SOE+MXO+Syntax+"+m_handle);
							playerObj->SendRPC(Bin2Hex(packet)); packet.clear(); //add online
						}else if(reportMyAbsence == true){
							packet << uint32(swap32(0x80d70800)) << uint32(swap32(0x3b00008a));
							packet.writeString("SOE+MXO+Syntax+"+m_handle);
							playerObj->SendRPC(Bin2Hex(packet)); packet.clear(); //add offline
						}
					}
				}
			//}
		}

		if(reportMyAbsence == false){
			handles = handles.substr(0,handles.length()-1);
			if(handles.length()>2){
				std::vector<std::string> list_handles;
				boost::split(list_handles, handles, boost::is_any_of(","));

				for(int i = 1; i<(int)list_buddy.size(); i++){
					bool found = false;
					for(int j= 0; j<(int)list_handles.size(); j++){
						if(list_buddy[i] == list_handles[j] && found == false){
							found = true;	
						}
					}
					if(found == true){
						packet << uint32(swap32(0x80d70800)) << uint32(swap32(0x3b00008e));
						packet.writeString("SOE+MXO+Syntax+"+list_buddy[i]);
						SendRPC(Bin2Hex(packet)); packet.clear();
					}else{
						packet << uint32(swap32(0x80d70800)) << uint32(swap32(0x3b00008a));
						packet.writeString("SOE+MXO+Syntax+"+list_buddy[i]);
						SendRPC(Bin2Hex(packet)); packet.clear();

						packet.size();
					}
				}
			}
		}//
	}
}
void PlayerObject::RPC_AddBuddy(ByteBuffer &src){
	uint16 extra = src.read<uint16>();

	string buddy = src.readString();

	std::vector<std::string> def;
	boost::split(def, buddy, boost::is_any_of("+"));

	ByteBuffer packet;
	packet << uint32(swap32(0x80d70800)) << uint32(swap32(0x3b00008a));

	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `handle` FROM `characters` WHERE `handle` = '%1%' LIMIT 1") % def[3]) );
	if (value == NULL){ /*do nothing*/
		SendMsg("",def[3]+" doesn't exist.","SYSTEM");
		return;
	}else{
		Field *field_value = value->Fetch();
		def[3] = ""+format(field_value[0].GetString()).str();

		buddyList += ","+def[3];
		INFO_LOG(m_handle+" : RPC_AddBuddy@(handle)"+def[3]);
	}

	packet.writeString("SOE+MXO+Syntax+"+def[3]);
	SendRPC(Bin2Hex(packet)); packet.clear();

	SetValueDB("mybuddylist","buddyList",buddyList,"handle",m_handle);
}
void PlayerObject::RPC_RemoveBuddy(ByteBuffer &src){
	uint16 extra = src.read<uint16>();

	string buddy = src.readString();

	INFO_LOG(m_handle+" : RPC_RemoveBuddy@(handle)"+buddy);

	std::vector<std::string> def;
	boost::split(def, buddy, boost::is_any_of("+"));

	std::vector<std::string> list_buddy;
	boost::split(list_buddy, buddyList, boost::is_any_of(","));

	string new_buddyList = "";
	for(int i = 0; i<(int)list_buddy.size(); i++){
		if(list_buddy[i] == def[3]){
			ByteBuffer packet;
			packet << uint32(swap32(0x80db0800)) << uint32(swap32(0x3b00008a));
			packet.writeString("SOE+MXO+Syntax+"+def[3]);
			SendRPC(Bin2Hex(packet)); packet.clear();
		}else{
			new_buddyList += list_buddy[i]+",";
		}
	}
	buddyList = new_buddyList.substr(0,new_buddyList.length()-1);

	SetValueDB("mybuddylist","buddyList",buddyList,"handle",m_handle);
}
void PlayerObject::UpdateCrew(){
	/*std::vector<std::string> list_buddy;
	boost::split(list_buddy, buddyList, boost::is_any_of(","));*/

	if(m_lvl >= 16){
		ByteBuffer test;
		test.append( make_shared<HexGenericMsg>("8086ac53020000000000000000000000000000000000000000210000000000230000000000")->toBuf() );
		SendRPC(Bin2Hex(test)); test.clear(); //unlock CrewPanel if m_lvl>=16
	}

	//refer to PlayerObject()
	string increw = GetValueDB("mycrew","IN_CREW","handle",m_handle);
	if(increw == "0"){
		return;
	}else if(increw == "1"){
		string command = GetValueDB("mycrew","COMMAND","handle",m_handle);
		if(command == "CAPTAIN"){
			m_crew[0] = GetValueDB("mycrew","NAME_CREW","handle",m_handle);
			m_crew[1] = GetValueDB("mycrew","MEMBERS","handle",m_handle);
			m_crew[2] = "CAPTAIN";
			m_crew[4] = m_handle;
			m_crew[5] = GetValueDB("mycrew","ID_CREW","handle",m_handle);
		}else{
			string captain = GetValueDB("mycrew","CAPTAIN","handle",m_handle);
			m_crew[0] = GetValueDB("mycrew","NAME_CREW","handle",captain);
			m_crew[1] = GetValueDB("mycrew","MEMBERS","handle",captain);
			m_crew[2] = "RECRUIT";
			m_crew[4] = captain;
			m_crew[5] = GetValueDB("mycrew","ID_CREW","handle",captain);
		}

		InstanceCrew();
		//(...)
	}
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//CREW & TEAM/////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::SendInvitation(string msg, string sender){
	int start_msg = 12+sender.length()+3;

	ByteBuffer packet;
	packet << uint32(swap32(0x808f0c00)) << uint16(start_msg) << uint32(0) << uint16(0);
	packet.writeString(sender);
	packet.writeString(msg);

	SendRPC(Bin2Hex(packet));
}
void PlayerObject::RPC_AnswerInvitation(ByteBuffer &src){
	uint32 extra = src.read<uint16>();
	extra = src.read<uint32>();

	bool accepted = false;

	switch(extra){
	case 0:
		accepted = true;
		break;
	case 0x2e00000d:
		break;
	}

	PlayerObject* player = NULL;
	try{
		 player = sObjMgr.getGOPtr(sender_id);
	}catch (ObjectMgr::ObjectNotAvailable){ 
		SendMsg("","Player is not currently jacked in.","SYSTEM");

		m_team[0] = m_team[1] = m_team[2] = m_team[3] = m_team[4] = "NONE";
		sender_id = target_id = 0;
		updateState = "NONE";

		return;
	}

	if(accepted == true){
		player->SendMsg("",m_handle+" accepted your invitation.","SYSTEM");
		if(updateState == "RECRUITMENT_CREW"){

			INFO_LOG(m_handle+" : SendInvitation->(sub)RPC_AnswerInvitation@(Type : RECRUITMENT_CREW)"+player->getHandle()+" (id)"+player->m_ID);

			if(player->m_crew[1] == "NONE"){ //Crew is begin instanced for the first time
				player->m_crew[1] = player->getHandle()+","+m_handle;

				int new_id_crew = atoi( (GetValueDB("mytotalnpcnumber","npcNumber","command","CREWS")).c_str() )+1;
				player->m_crew[5] = (format("%1%") % new_id_crew).str();

				SetValueDB("mytotalnpcnumber","npcNumber",(format("%1%") % new_id_crew).str(),"command","CREWS");
			}else{
				player->m_crew[1] = player->m_crew[1]+","+m_handle;
			}
			player->m_crew[4] = player->getHandle();

			m_crew[0] = player->m_crew[0]; //update my name, Crew
			m_crew[1] = player->m_crew[1]; //update my member list, Crew
			m_crew[4] = player->m_crew[4]; //update my Captain, Crew
			m_crew[5] = player->m_crew[5]; //update my ID, Crew

			INFO_LOG( player->getHandle()+" invited "+ m_handle +" to join their crew.\n\tCrew name: "+ m_crew[0]+"\n\tMembers: "+ m_crew[1]+"\n\tCrew ID & Captain: "+ m_crew[5] + ", "+ m_crew[4]);

			sDatabase.Execute(format("UPDATE `mycrew` SET `IN_CREW` = '1', `COMMAND` = 'RECRUIT', `CAPTAIN` = '%2%' WHERE `handle` = '%1%'") % m_handle % m_crew[4] );

			string handle = player->getHandle();
			SetValueDB("mycrew","IN_CREW","1","handle", handle);
			SetValueDB("mycrew","COMMAND","CAPTAIN","handle", handle);
			SetValueDB("mycrew","MEMBERS",m_crew[1],"handle", handle);
			SetValueDB("mycrew","NAME_CREW",m_crew[0],"handle", handle);
			SetValueDB("mycrew","ID_CREW",m_crew[5],"handle", handle);

			player->SendMsg("","Restart your client!","SYSTEM");
			SendMsg("","Restart your client!","SYSTEM");

			player->InstanceCrew();
			InstanceCrew();
			//UpdateHumans(true);
		}
		if(updateState == "RECRUITMENT_TEAM"){
			if(player->m_team[0] == "0"){ //Crew is begin instanced for the first time
				player->m_team[1] = player->getHandle()+","+m_handle;
			}else{
				player->m_team[1] = player->m_team[1]+","+m_handle;

				//update other members
				std::vector<std::string> def;
				boost::split(def, player->m_team[1], boost::is_any_of(","));

				string data_chat = "";
				for(int i = 0; i<(int)def.size(); i++){
					data_chat+= def[i] + ", ";
				}
				data_chat = data_chat.substr(0,data_chat.length()-2)+".";

				vector<uint32> objectLists = sObjMgr.getAllGOIds();
				foreach (int currObj, objectLists){
					PlayerObject* member = NULL;
					try{
						member = sObjMgr.getGOPtr(currObj);
					}catch (ObjectMgr::ObjectNotAvailable){ continue; }
					if(member != NULL){
						for(int i = 0; i<(int)def.size(); i++){
							if(def[i] != m_handle && def[i] != player->getHandle()){
								member->SendMsg("",m_handle+" joined your team. Members:\n{c:ff0000}"+data_chat+"{/c}","SYSTEM");
								member->m_team[1] = player->m_team[1];
							}
						}
					}
				}
				//
			}
			player->m_team[0] = "1";
			player->m_team[4] = player->getHandle();

			m_team[0] = "1"; //update my name, Team
			m_team[1] = player->m_team[1]; //update my member list, Team
			m_team[4] = player->m_team[4]; //update my Captain, Team

			sDatabase.Execute(format("UPDATE `myteam` SET `IN_TEAM` = '1', `COMMAND` = 'RECRUIT', `CAPTAIN` = '%2%' WHERE `handle` = '%1%'") % m_handle % m_team[4] );

			string handle = player->getHandle();
			SetValueDB("myteam","IN_TEAM","1","handle", handle);
			SetValueDB("myteam","COMMAND","CAPTAIN","handle", handle);
			SetValueDB("myteam","MEMBERS",m_team[1],"handle", handle);

			INFO_LOG( player->m_handle+" invited "+ m_handle +" to join his Team.\n\tMembers: "+ m_team[1]+"\n\tCaptain: "+ m_team[4]);

			InstanceTeam(m_team[4]+"'s Mission Team",m_team[4]);
		}
	}else{
		player->SendMsg("",m_handle+" rejected your invitation.","SYSTEM");
		SendMsg("","You rejected "+ src.readString() +"'s invitation.","SYSTEM");

		if(updateState == "RECRUITMENT_CREW"){
			m_crew[0] = m_crew[1] = m_crew[2] = m_crew[3] = m_crew[4] = "NONE";
		}
		if(updateState == "RECRUITMENT_TEAM"){
			m_team[0] = m_team[1] = m_team[2] = m_team[3] = m_team[4] = "NONE";
		}
	}

	sender_id = target_id = 0;
	player->target_id = 0;
	updateState = "NONE";
}
void PlayerObject::RPC_CreateCrew(ByteBuffer &src){
	uint32 extra = src.read<uint32>();
	extra = src.read<uint8>();

	string crew_name = src.readString();
	erase_all(crew_name,"'");

	string recruit = src.readString();

	vector<uint32> allObjects = sObjMgr.getAllGOIds();
	foreach(uint32 objId, allObjects){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(objId);
		}catch (ObjectMgr::ObjectNotAvailable){
			continue;
		}
		if(iequals(player->getHandle(), recruit)){
			if( player->m_reputation == m_reputation ){
				if(player->m_crew[0] == "NONE"){
					m_crew[0] = crew_name;
					m_crew[2] = "CAPTAIN";
					target_id = objId;

					player->m_crew[0] = crew_name;
					player->m_crew[2] = "RECRUIT";
					player->m_crew[4] = m_handle;
					player->sender_id = m_goId;

					player->SendInvitation( m_handle+"'s Crew", m_handle );
					player->updateState = "RECRUITMENT_CREW";

					INFO_LOG(m_handle+" : RPC_CreateCrew->(sub)SendInvitation@(Type : Pending for acceptance)"+m_handle+" <> "+player->getHandle());
				}else{
					SendMsg("","{c:ff0000}"+player->getHandle()+" already joined a Crew.{/c}","SYSTEM");
				}
			}else{
				SendMsg("","{c:ff0000}"+player->getHandle()+"'s reputation is different from yours.{/c}","SYSTEM");
				player->SendMsg("","{c:ff0000}"+m_handle+"'s reputation is different from yours.{/c}","SYSTEM");
			}
			break;
		}
	}
}
void PlayerObject::RPC_CrewMsg(ByteBuffer &src){
	uint32 extra = src.read<uint16>();
	extra = src.read<uint32>();

	string msg = src.readString();

	std::vector<std::string> def;
	boost::split(def, m_crew[1], boost::is_any_of(","));

	vector<uint32> objectLists = sObjMgr.getAllGOIds();
	foreach (int currObj, objectLists){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(currObj);
		}catch (ObjectMgr::ObjectNotAvailable){ continue; }
		if(player != NULL){
			for(int i = 0; i<(int)def.size(); i++){
				if(def[i] != m_handle && def[i] == player->getHandle()){
					player->SendMsg(m_handle,msg,"CREW");
				}
			}
		}
	}
}
void PlayerObject::RPC_DisbandCrew(ByteBuffer &src){
	std::vector<std::string> def;
	boost::split(def, m_crew[1], boost::is_any_of(","));

	if(m_crew[2] == "CAPTAIN" ){
		INFO_LOG(m_handle+" : RPC_DisbandCrew@(Type : Partial, is Captain)"+m_handle);

		for(int i = 0; i<(int)def.size(); i++){
			if(def[i] != m_handle){
				sDatabase.Execute(format("UPDATE `mycrew` SET `IN_CREW` = '0', `COMMAND` = 'NONE', `CAPTAIN` = 'NONE', `ID_CREW` = '0' WHERE `handle` = '%1%'") % def[i] );
			}
		}

		SetValueDB("mycrew","IN_CREW","0","handle", m_handle);
		SetValueDB("mycrew","COMMAND","NONE","handle", m_handle);
		SetValueDB("mycrew","MEMBERS","NONE","handle", m_handle);
		SetValueDB("mycrew","NAME_CREW","NONE","handle", m_handle);
		SetValueDB("mycrew","ID_CREW","NONE","handle", m_handle);

		m_crew[0] = m_crew[1] = m_crew[2] = m_crew[3] = m_crew[4] = "NONE";
		m_crew[5] = "0";
	}else if(m_crew[2] == "RECRUIT" ){
		INFO_LOG(m_handle+" : RPC_DisbandCrew@(Type : Partial, is Recruit)"+m_handle);

		sDatabase.Execute(format("UPDATE `mycrew` SET `IN_CREW` = '0', `COMMAND` = 'NONE', `CAPTAIN` = 'NONE', `ID_CREW` = '0' WHERE `handle` = '%1%'") % m_handle );

		string new_member_list = "";
		for(int i = 0; i<(int)def.size(); i++){
			if(def[i] != m_handle){
				new_member_list+= def[i] + ",";
			}
		}
		new_member_list = new_member_list.substr(0,new_member_list.length()-1);

		SetValueDB("mycrew","MEMBERS",new_member_list,"handle",m_crew[4]); //update captain list

		m_crew[0] = m_crew[1] = m_crew[2] = m_crew[3] = m_crew[4] = "NONE";
		m_crew[5] = "0";
	}

	SendMsg("","Restart your client!","SYSTEM");
}
void PlayerObject::RPC_RecruitCrew(ByteBuffer &src){
	uint32 extra = src.read<uint32>();

	string crew_name = src.readString();
	erase_all(crew_name,"'");

	string recruit = src.readString();

	if(m_crew[2] == "RECRUIT"){
		SendMsg("","{c:ff0000}You are not the captain of your crew!{/c}","SYSTEM");
		return;
	}

	INFO_LOG(m_handle+" : RPC_RecruitCrew@("+recruit+")");

	vector<uint32> allObjects = sObjMgr.getAllGOIds();
	foreach(uint32 objId, allObjects){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(objId);
		}catch (ObjectMgr::ObjectNotAvailable){
			continue;
		}
		if(iequals(player->getHandle(), recruit)){
			if( player->m_reputation == m_reputation ){
				if(player->m_crew[0] == "NONE"){
					m_crew[0] = crew_name;
					m_crew[2] = "CAPTAIN";
					target_id = objId;

					player->m_crew[0] = crew_name;
					player->m_crew[2] = "RECRUIT";
					player->m_crew[4] = m_handle;
					player->sender_id = m_goId;

					player->SendInvitation( m_handle+"'s crew", m_handle );
					player->updateState = "RECRUITMENT_CREW";
				}else{
					SendMsg("","{c:ff0000}"+player->getHandle()+" already joined a crew.{/c}","SYSTEM");
				}
			}else{
				SendMsg("","{c:ff0000}"+player->getHandle()+"'s reputation is different from yours.{/c}","SYSTEM");
				player->SendMsg("","{c:ff0000}"+m_handle+"'s reputation is different from yours.{/c}","SYSTEM");
			}
			break;
		}
	}
}
void PlayerObject::RPC_CallCrewName(ByteBuffer &src){
	uint32 id_crew = src.read<uint32>();
	string crew = "";
	
	if(id_crew == 11000){
		crew = "{i}System{/i} - {c:33CC00}MOD{/c}";
	}else if(id_crew == 12000){
		crew = "{i}System{/i} - {c:FF0000}ADMIN{/c}";
	}else{
		crew = GetValueDB("mycrew","NAME_CREW","ID_CREW", (format("%1%")%id_crew).str() );
	}

	INFO_LOG(m_handle+" : RPC_CallCrewName@(pending since login or recruitment)");

	ByteBuffer packet;
	packet << uint16(swap16(0x80f5)) << uint32(id_crew);
	string crew_hex = parseStringToHex(crew,"PACKETSCHEME_NO");
	string voidStr = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000"; //42bytes
	crew_hex = crew_hex+voidStr.substr(crew_hex.length());
	packet.append( make_shared<HexGenericMsg>(crew_hex)->toBuf() );

	SendRPC(Bin2Hex(packet));
}
void PlayerObject::InstanceCrew(){

	INFO_LOG(m_handle+" : InstanceCrew@(pending since login or recruitment)");

	ByteBuffer packet;
	packet << uint16(swap16(0x8086)) << uint32(swap32(0xAE040A00)) << uint16(swap16(0x4bb1)) << uint16(0);

	if(m_reputation == "zion"){
		packet.append( make_shared<HexGenericMsg>("01210093f5010075d4010000000000")->toBuf() );
	}else if(m_reputation == "machinist"){
		packet.append( make_shared<HexGenericMsg>("02210093f5010075d4010000000000")->toBuf() );
	}else if(m_reputation == "merovingian"){
		packet.append( make_shared<HexGenericMsg>("03210093f5010075d4010000000000")->toBuf() );
	}else if(m_reputation == "niobesgroup"){
		packet.append( make_shared<HexGenericMsg>("04210093f5010075d4010000000000")->toBuf() );
	}else if(m_reputation == "epn"){
		packet.append( make_shared<HexGenericMsg>("05210093f5010075d4010000000000")->toBuf() );
	}else if(m_reputation == "maskedmen"){
		packet.append( make_shared<HexGenericMsg>("06210093f5010075d4010000000000")->toBuf() );
	}else if(m_reputation == "cyph"){
		packet.append( make_shared<HexGenericMsg>("07210093f5010075d4010000000000")->toBuf() );
	}else{
		packet.append( make_shared<HexGenericMsg>("01210093f5010075d4010000000000")->toBuf() );
	}

	int start_members = 33+(m_crew[0]).length()+4;
	packet << uint16(start_members) << uint16(swap16(0x1402)) << uint16(0) << uint16(swap16(0x3e01));
	packet.writeString(m_crew[0]);
	packet << uint8(0);

	std::vector<std::string> def;
	boost::split(def, m_crew[1], boost::is_any_of(","));
	packet << uint16(def.size());

	string voidStr = "0000000000000000000000000000000000000000000000000000000000000000"; //32bytes
	int k = 0;
	for(int i = 0; i<(int)def.size(); i++){
		string member = parseStringToHex( def[i], "PACKETSCHEME_NO");
		member = member+voidStr.substr(member.length());
		if(i == 0){
			k = i+1;
			packet << uint8(0) << uint16(swap16(0x93f5)) << uint16(k); //it's the captain
			packet.append( make_shared<HexGenericMsg>(member)->toBuf() );
		}else{
			k = i+1;
			packet << uint8(0) << uint16(k) << uint16(k);
			packet.append( make_shared<HexGenericMsg>(member)->toBuf() );
		}
		if(i == 0){
			packet << uint8(1); //it should show only the ONLINE state of member (-> 00 OFFLINE) - use it to show the CAPTAIN
		}else{
			packet << uint8(0);
		}
	}

	SendRPC(Bin2Hex(packet));
}
void PlayerObject::RPC_StartTeam(ByteBuffer &src){
	uint32 extra = src.read<uint32>();
	extra = src.read<uint8>();

	string recruit = src.readString();

	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `handle` FROM `characters` WHERE `handle` = '%1%' LIMIT 1") % recruit ) );
	if (value == NULL){ /*do nothing*/
		SendMsg("",recruit+" doesn't exist.","SYSTEM");
		return;
	}else{
		Field *field_value = value->Fetch();
		recruit = ""+format(field_value[0].GetString()).str();
	}

	vector<uint32> allObjects = sObjMgr.getAllGOIds();
	foreach(uint32 objId, allObjects){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(objId);
		}catch (ObjectMgr::ObjectNotAvailable){
			continue;
		}
		if(iequals(player->getHandle(), recruit)){
			if(player->m_team[0] == "0"){
				if(m_team[0] == "0"){
					//Captain's team panel is pending for answer
					/*string answer_1 = " 6f 02 08 00 00 00 00 00 "+parseStringToHex(m_handle,"PACKETSCHEME_YES");
					SendRPC(answer_1);*/

					//send answer invitation (ok)
					int start_2 = 12+m_handle.length()+3;
					ByteBuffer answer_2;
					answer_2 << uint32(swap32(0x808d0c00)) << uint16(start_2) << uint32(0) << uint16(0);
					answer_2.writeString( m_handle);
					answer_2.writeString( player->getHandle()+"'s Mission Team");
					SendRPC(Bin2Hex(answer_2));

					InstanceTeam(m_handle+"'s Mission Team",m_handle);

					target_id = objId;
					player->sender_id = m_goId;

					player->SendInvitation(m_handle+"'s Team",m_handle);
					player->updateState = "RECRUITMENT_TEAM";

					INFO_LOG(m_handle+" : RPC_StartTeam->(sub)SendInvitation@(Type : Pending for acceptance)"+m_handle+" <> "+player->getHandle());
				}
			}else{
				SendMsg("","{c:ff0000}"+player->getHandle()+" already joined a Team.{/c}","SYSTEM");
			}
			break;
		}
	}
}
void PlayerObject::RPC_TeamMsg(ByteBuffer &src){
	uint32 extra = src.read<uint32>();
	extra = src.read<uint16>();

	string msg = src.readString();

	std::vector<std::string> def;
	boost::split(def, m_team[1], boost::is_any_of(","));

	vector<uint32> objectLists = sObjMgr.getAllGOIds();
	foreach (int currObj, objectLists){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(currObj);
		}catch (ObjectMgr::ObjectNotAvailable){ continue; }
		if(player != NULL){
			for(int i = 0; i<(int)def.size(); i++){
				if(def[i] != m_handle && def[i] == player->getHandle()){
					player->SendMsg(m_handle,msg,"TEAM");
				}
			}
		}
	}
}
void PlayerObject::RPC_InviteInTeam(ByteBuffer &src){
	if(m_team[0] == "1" && m_team[4] != m_handle){
		SendMsg("","{c:ff0000}You are not the captain of your team.{/c}","SYSTEM");
		return;
	}

	uint32 extra = src.read<uint32>();

	string recruit = src.readString();

	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `handle` FROM `characters` WHERE `handle` = '%1%' LIMIT 1") % recruit ) );
	if (value == NULL){ /*do nothing*/
		SendMsg("",recruit+" doesn't exist.","SYSTEM");
		return;
	}else{
		Field *field_value = value->Fetch();
		recruit = ""+format(field_value[0].GetString()).str();
	}

	string team = src.readString();
	vector<uint32> allObjects = sObjMgr.getAllGOIds();
	foreach(uint32 objId, allObjects){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(objId);
		}catch (ObjectMgr::ObjectNotAvailable){
			continue;
		}
		if(iequals(player->getHandle(), recruit)){
			if(player->m_team[0] == "0"){
				if(m_team[0] == "1" && m_team[4] != m_handle){
					SendMsg("","{c:ff0000}You are not the captain of your team.{/c}","SYSTEM");
				}else if(m_team[0] == "1"){
					target_id = objId;
					player->sender_id = m_goId;

					player->SendInvitation(m_handle+"'s Team",m_handle);
					player->updateState = "RECRUITMENT_TEAM";

					INFO_LOG(m_handle+" : RPC_InviteInTeam->(sub)SendInvitation@(Type : Pending for acceptance)"+m_handle+" <> "+player->getHandle());
				}
			}else{
				SendMsg("","{c:ff0000}"+player->getHandle()+" already joined a team.{/c}","SYSTEM");
			}
			break;
		}
	}
}
void PlayerObject::RPC_DisbandTeam(ByteBuffer &src){
	std::vector<std::string> def;
	boost::split(def, m_team[1], boost::is_any_of(","));

	if(m_crew[2] == "CAPTAIN" ){
		INFO_LOG(m_handle+" : RPC_DisbandTeam@(Type : Complete, is Captain)"+m_handle);

		for(int i = 0; i<(int)def.size(); i++){
			if(def[i] != m_handle){
				sDatabase.Execute(format("UPDATE `myteam` SET `IN_TEAM` = '0', `COMMAND` = 'NONE', `CAPTAIN` = 'NONE' WHERE `handle` = '%1%'") % def[i] );
			}
		}

		SetValueDB("myteam","IN_TEAM","0","handle", m_handle);
		SetValueDB("myteam","COMMAND","NONE","handle", m_handle);
		SetValueDB("myteam","MEMBERS","NONE","handle", m_handle);

		//update other members
		vector<uint32> objectLists = sObjMgr.getAllGOIds();
		foreach (int currObj, objectLists){
			PlayerObject* member = NULL;
			try{
				member = sObjMgr.getGOPtr(currObj);
			}catch (ObjectMgr::ObjectNotAvailable){ continue; }
			if(member != NULL){
				for(int i = 0; i<(int)def.size(); i++){
					if(def[i] != m_handle){
						member->SendMsg("","This team was disbanded by your captain.","SYSTEM");
						member->m_team[0] = member->m_team[1] = member->m_team[2] = member->m_team[3] = member->m_team[4] = "NONE";
					}
				}
			}
		}
		//

		m_team[0] = m_team[1] = m_team[2] = m_team[3] = m_team[4] = "NONE";
	}else if(m_crew[2] == "RECRUIT" ){
		INFO_LOG(m_handle+" : RPC_DisbandTeam@(Type : Partial, is Recruit)"+m_handle);

		sDatabase.Execute(format("UPDATE `myteam` SET `IN_TEAM` = '0', `COMMAND` = 'NONE', `CAPTAIN` = 'NONE' WHERE `handle` = '%1%'") % m_handle );

		string new_member_list = "";
		string data_chat = "";
		for(int i = 0; i<(int)def.size(); i++){
			if(def[i] != m_handle){
				new_member_list+= def[i] + ",";
				data_chat+= def[i] + ", ";
			}
		}
		new_member_list = new_member_list.substr(0,new_member_list.length()-1);
		data_chat = data_chat.substr(0,data_chat.length()-2)+".";

		//update other members
		vector<uint32> objectLists = sObjMgr.getAllGOIds();
		foreach (int currObj, objectLists){
			PlayerObject* member = NULL;
			try{
				member = sObjMgr.getGOPtr(currObj);
			}catch (ObjectMgr::ObjectNotAvailable){ continue; }
			if(member != NULL){
				for(int i = 0; i<(int)def.size(); i++){
					if(def[i] != m_handle){
						member->SendMsg("",m_handle+" left your team. Members:\n{c:ff0000}"+data_chat+"{/c}","SYSTEM");
						member->m_team[1] = new_member_list;
					}
				}
			}
		}
		//

		SetValueDB("mycrew","MEMBERS",new_member_list,"handle",m_team[4]); //update captain list

		m_team[0] = m_team[1] = m_team[2] = m_team[3] = m_team[4] = "NONE";
	}

	SendMsg("","Press 'j', then click on 'Leave'.","SYSTEM");
}
void PlayerObject::InstanceTeam(string msg, string sender){
	int start_member = 19+msg.length()+3;

	ByteBuffer packet;
	packet.append( make_shared<HexGenericMsg>("808e2fd80100b800000313000100050004")->toBuf() );
	packet << uint16(start_member);
	packet.writeString(msg);
	packet << uint16(1) << uint8(0) << uint16(1) << uint16(5);
	
	string captain = parseStringToHex( sender, "PACKETSCHEME_NO");
	string voidStr = "0000000000000000000000000000000000000000000000000000000000000000";
	captain = captain+voidStr.substr(captain.length());
	packet.append( make_shared<HexGenericMsg>(captain)->toBuf() );
	packet << uint8(0);

	SendRPC(Bin2Hex(packet));
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//MISSION/////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::RPC_RequestMission(ByteBuffer &src){
	if(m_team[0] == "1"){
		SendMsg("","You cannot accept a mission until you exit or disband your team.","BROADCAST");
		return;
	}

	uint16 extra = src.read<uint16>();
	extra = src.read<uint8>();
	switch(extra){
		case 0x01:
			mission_org = "zion";
			break;
		case 0x02:
			mission_org = "machinist";
			break;
		case 0x03:
			mission_org = "merovingian";
			break;
	}

	//verify limit quests
	int my_quests = 0;
	int kO = 1;
	int kNPC = 1;
	my_quests = atoi( (GetValueDB("myquests","MISSION_ID","handle",m_handle)).c_str() )+1;
	string temp ;

	//rest variables
	mission_title = "NONE";
	mission_detail = "NONE";
	mission_desc = "NONE";
	mission_exp = 0;
	mission_info = 0;
	total_npc = total_obj = 0;
	curr_obj = 1;

	for(int i=0; i<100; i++){
		desc_obj[i] = command_obj[i] = NPC_obj[i] = NPC_name[i] = item_inMission[i] = dial_obj[i] = data_NPC_inMission[i] = "NONE";
	}//end reset

	name_file = "data/mission_"+mission_org+"_"+ (format("%1%")%my_quests).str() +".xml";

	INFO_LOG(m_handle+" : RPC_RequestMission@(reading XML)(file)"+ name_file );

	using namespace irr::io;
	IrrXMLReader* xml = createIrrXMLReader( name_file.c_str() );
	while(xml && xml->read()){
		switch(xml->getNodeType()){
			case EXN_ELEMENT:
				if (iequals("data", xml->getNodeName())){
					mission_title = xml->getAttributeValue("title");
					mission_desc = xml->getAttributeValue("description");
					temp = xml->getAttributeValue("exp");
					mission_exp = atoi( temp.c_str() );

					temp = xml->getAttributeValue("info");
					mission_info = atoi( (temp).c_str() );
				}
				if (!strcmp("total", xml->getNodeName())){
					temp = xml->getAttributeValue("objective");
					total_obj = atoi( (temp).c_str() );

					temp = xml->getAttributeValue("npc");
					total_npc = atoi( (temp).c_str() );

					mission_detail = xml->getAttributeValue("detail");
				}
				if(kO <= total_obj){
					string tag = "objective"+ lexical_cast<string>(kO);
					if (iequals(tag, xml->getNodeName())){
						desc_obj[kO] = xml->getAttributeValue( "description" );
						NPC_obj[kO] = xml->getAttributeValue( "idNpc" );
						command_obj[kO] = xml->getAttributeValue( "command" );

						if(command_obj[kO] == "LOOT" || command_obj[kO] == "GET" || command_obj[kO] == "GIVE"){
							item_inMission[kO] = xml->getAttributeValue( "item" );
						}

						dial_obj[kO] = xml->getAttributeValue( "dial" );

						string new_dial = "";
						std::vector<std::string> defA;
						boost::split(defA, dial_obj[kO], boost::is_any_of("%"));
						for(int i = 0; i<(int)defA.size(); i++){
							new_dial += defA[i]+m_handle;
						}
						new_dial = new_dial.substr(0, new_dial.length()-m_handle.length());
						dial_obj[kO] = new_dial;
						
						kO += 1;
					}
				}
				if(kNPC <= total_npc){
					string tag = "npc"+ lexical_cast<string>(kNPC);
					if (iequals(tag, xml->getNodeName())){
						string type = xml->getAttributeValue( "type" );
						data_NPC_inMission[kNPC] =  type+",";
						if(type == "FRIENDLY"){
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "idNpc" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "level" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "rsi" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "x" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "y" ) + string(",");;
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "z" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "handle" );
						}else if(type == "FRIENDLY_noInteraction"){
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "idNpc" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "level" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "rsi" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "x" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "y" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "z" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "handle" );
						}else if(type == "HOSTILE"){
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "idNpc" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "level" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "maxHP" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "rsi" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "x" )  + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "y" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "z" ) + string(",");
							data_NPC_inMission[kNPC] += xml->getAttributeValue( "handle" );
						}
						kNPC += 1;
					}
				}
				break;
		}
	}

	if (xml)
	{
		delete xml;
		xml = NULL;
	}

	StartTeam();

	ByteBuffer packet;
	packet << uint32(swap32(0x80950000)) << uint32(swap32(0)) << uint16(1) << uint8(0) << uint8(extra);
	packet.append( make_shared<HexGenericMsg>("d0060000310000b4c00c0028")->toBuf() );
	SendRPC(Bin2Hex(packet)); packet.clear();

	packet << uint32(swap32(0x80960000)) << uint32(swap32(0x00000f00)) << uint8(extra) << uint16(swap16(0xd006)) << uint32(0);
	packet.writeString(mission_title);
	SendRPC(Bin2Hex(packet)); packet.clear();

	packet << uint32(swap32(0x80970000));
	SendRPC(Bin2Hex(packet)); packet.clear();
}
void PlayerObject::StartTeam(){
	ByteBuffer packet;
	packet.append( make_shared<HexGenericMsg>("808d660000020e00040000000000")->toBuf() );
	packet.writeString(m_handle+"'s Mission Team");
	SendRPC(Bin2Hex(packet)); packet.clear();
}
void PlayerObject::RPC_LoadMissionInformation(ByteBuffer &srcCmd){
	INFO_LOG(m_handle+" : RPC_LoadMissionInformation->(sub)RPC_RequestMission@(..)");

	int start_title_description = 47+mission_title.length()+3;

	ByteBuffer packet;
	packet.append( make_shared<HexGenericMsg>("80990000000000002f000100000002a29f7e46a29f7e46000000000000000000d0060000310000b4c00c0028")->toBuf() );
	packet << uint16(start_title_description) << uint8(0);
	packet.writeString(mission_title);
	packet.writeString(mission_desc);
	SendRPC(Bin2Hex(packet)); packet.clear();
}
void PlayerObject::NameMission(string msg){
	ByteBuffer packet;
	packet.append( make_shared<HexGenericMsg>("809c0000000000002b00000057000200000000000000000000000014000026700a00280000ff3c00")->toBuf() );
	packet << uint16(43+msg.length()+3) << uint8(0);
	packet.writeString(msg);
	packet.append( make_shared<HexGenericMsg>("0100000100da030000")->toBuf() );
	SendRPC(Bin2Hex(packet)); packet.clear();
}
void PlayerObject::RPC_AcceptMission(ByteBuffer &srcCmd){

	INFO_LOG(m_handle+" : RPC_AcceptMission->(sub)RPC_LoadMissionInformation@(..)");

	NameMission(mission_title);

	for(int i = 1; i<=total_obj; i++){
		SetObjective(i, "REMAIN", desc_obj[i]); 
	}

	for(int i = 1; i<=total_npc; i++){
		//data_NPC_inMission
		std::vector<std::string> def;
		boost::split(def, data_NPC_inMission[i], boost::is_any_of(","));
		INFO_LOG(data_NPC_inMission[i]);

		//string type, int id, int level, (hpM), string RSI, double x, double y, double z, string handle, bool DEFEATED
		if(def[0] == "FRIENDLY"){
			SetNPC_inMission( def[0], atoi(def[1].c_str()), atoi(def[2].c_str()), 0, def[3], atoi(def[4].c_str()), atoi(def[5].c_str()), atoi(def[6].c_str()), def[7], false );
		}
		if(def[0] == "FRIENDLY_noInteraction"){
			SetNPC_inMission( def[0], atoi(def[1].c_str()), atoi(def[2].c_str()), 0, def[3], atoi(def[4].c_str()), atoi(def[5].c_str()), atoi(def[6].c_str()), def[7], false );
		}
		if(def[0] == "HOSTILE"){
			SetNPC_inMission( def[0], atoi(def[1].c_str()), atoi(def[2].c_str()), atoi(def[3].c_str()), def[4], atoi(def[5].c_str()), atoi(def[6].c_str()), atoi(def[7].c_str()), def[8], false );
		}
	}

	//FIX NONE BUG 
	for(int i = 1; i<=total_obj; i++){
		//get NPC ID listed for the MISSION
		int ID_NPC_IN_MISSION = 0 + atoi( NPC_obj[i].c_str() );

		//check if NPC ID is EQUAL to a specific OBJECTIVE
		for(int j = 1; j<=total_npc; j++){
			std::vector<std::string> def;
			boost::split(def, data_NPC_inMission[j], boost::is_any_of(","));

			int ID_NPC_FROM_OBJECTIVE = 0 + atoi( def[1].c_str() );

			//if EQUAL, save NPC NAME
			/**/if( ID_NPC_FROM_OBJECTIVE == ID_NPC_IN_MISSION ){
				if(def[0] == "FRIENDLY" || def[0] == "FRIENDLY_noInteraction"){
					NPC_name[i] = def[7];
				}else if(def[0] == "HOSTILE"){
					NPC_name[i] = def[8];
				}
			}
		}
	}

	NPC_CAN_TALK(true);

	inMission = true;

	SendMsg("",mission_detail,"BROADCAST");
}
void PlayerObject::UpdateMission(){
	if(curr_obj == total_obj){
		ByteBuffer test;
		RPC_AbortMission(test);
	}

	SetObjective(curr_obj,"CLEAR",desc_obj[curr_obj]);
	curr_obj += 1;
}
void PlayerObject::NPC_CAN_TALK(bool canTalk){
	//NPC can talk
	ByteBuffer packet;
	packet << uint8(0x03) << uint16(m_goId) << uint16(swap16(0x0308));
	
	LocationVector posFLOAT;
	posFLOAT.x = this->getPosition().x;
	posFLOAT.y = this->getPosition().y;
	posFLOAT.z = this->getPosition().z;
	posFLOAT.toFloatBuf(packet);

	packet.append( make_shared<HexGenericMsg>("2aec9022ff0102000000000000000000000000"+parseStringToHex( m_firstName ,"PACKETSCHEME_YES_TYPEB")+parseStringToHex( m_lastName ,"PACKETSCHEME_YES_TYPEB"))->toBuf() );

	packet << uint8(0xff);
	vector<byte> rsiBuf(15,0);
	this->getRsiData(&rsiBuf[0],rsiBuf.size());
	packet.append(rsiBuf);
	packet << uint16(1) << uint32(0);

	if(canTalk == true){
		packet.append( make_shared<HexGenericMsg>("ffbf01000200d70102ff")->toBuf() );
	}else if(canTalk == false){
		packet.append( make_shared<HexGenericMsg>("ffbf01000200d670102ff")->toBuf() );
	}
	packet << uint16( m_healthM ) << uint8( m_lvl );

	packet.append( make_shared<HexGenericMsg>("0000000000000000000000000000000000000000000000000000000000000000000000ff0000000000")->toBuf() );
	packet << uint16( m_innerStrM ) << uint16(0) << uint16( m_innerStrM ) << uint32(0) << uint16( m_healthM ) << uint8(0xff);

	packet.append( make_shared<HexGenericMsg>("00000000000000000000000000000000000000ff000000000100000000000000803f1100000000000000000000003f0000000022000000000000000000")->toBuf() );
	SendPacketToMe(Bin2Hex(packet));
}
void PlayerObject::SetObjective(int id, string state, string msg){
	ByteBuffer packet;
	packet << uint16(swap16(0x80a0)) << uint8(id) << uint16(6);

	if(state == "REMAIN"){
		packet << uint8(0);
	}else if(state == "CLEAR"){
		packet << uint8(1);
	}else if(state == "FAILED_REMAIN"){
		packet << uint8(2);
	}else if(state == "FAILED_CLEARED"){
		packet << uint8(3);
	}

	packet.writeString(msg);
	SendRPC(Bin2Hex(packet));
}
void PlayerObject::RPC_AbortMission(ByteBuffer &srcCmd){

	INFO_LOG(m_handle+" : RPC_AbortMission@(pending since loading or aborting)");

	//clean objectives
	string packet = "80a330002103000000000000000000003c00000000000000000000000000000000000000000000000000000000000000010000";
	SendRPC(packet);

	if(total_obj == curr_obj){
		ByteBuffer complete;
		complete.append( make_shared<HexGenericMsg>("80a20800000000b4658db5")->toBuf() );
		complete << uint32( mission_exp ) << uint32( mission_info ) << uint8(0) << uint16(0) << uint32(0);
		SendRPC(Bin2Hex(complete)); complete.clear(); //update mission panel

		GiveCash(mission_info);
		GiveEXP(mission_exp);

		UPDATE_REPUTATION();

		int my_quests = atoi( (GetValueDB("myquests","MISSION_ID","handle",m_handle)).c_str() )+1;
		SetValueDB("myquests","MISSION_ID", (format("%1%")%my_quests).str() ,"handle",m_handle);
	}else{
		packet = "80a20800000000b4658db5000000000000000000000000000000";
		SendRPC(packet);
	}

	NPC_CAN_TALK(false);

	for(int i = 1; i<=total_obj; i++){
		ByteBuffer deleteNPC;
		deleteNPC << uint8(0x03) << uint32(swap32(0x01000101)) << uint8(0) << uint16( atoi( NPC_obj[i].c_str() ) ) << uint16(0);
		SendPacketToMe(Bin2Hex(deleteNPC));
	}
	
	//rest variables
	mission_org, mission_title, mission_desc = "NONE";
	mission_exp, mission_info = 0;
	total_npc = total_obj = 0;

	for(int i=0; i<100; i++){
		desc_obj[i] = command_obj[i] = NPC_obj[i] = NPC_name[i] = item_inMission[i] = dial_obj[i] = data_NPC_inMission[i] = "NONE";
	}//end reset

	inMission = false;
}
void PlayerObject::UPDATE_REPUTATION(){
	if(inMission == true){
		if(mission_org == "zion"){
			m_reputation_zi++;
			if(m_reputation_ma > 0){
				m_reputation_ma--;
			}
			if(m_reputation_me > 0){
				m_reputation_me--;
			}
		}
		if(mission_org == "machinist"){
			m_reputation_ma++;
			if(m_reputation_zi > 0){
				m_reputation_zi--;
			}
			if(m_reputation_me > 0){
				m_reputation_me--;
			}
		}
		if(mission_org == "merovingian"){
			m_reputation_me++;
			if(m_reputation_ma > 0){
				m_reputation_ma--;
			}
			if(m_reputation_zi > 0){
				m_reputation_zi--;
			}
		}

		if( m_reputation_zi > m_reputation_ma && m_reputation_zi > m_reputation_me ){
			m_reputation = "ZION";
			sDatabase.Execute(format("UPDATE `myreputation` SET `%2%` = '%3%' WHERE `handle` = '%1%'") % m_handle % m_reputation % m_reputation_zi );
		}else if( m_reputation_ma > m_reputation_zi && m_reputation_ma > m_reputation_me ){
			m_reputation = "MACHINIST";
			sDatabase.Execute(format("UPDATE `myreputation` SET `%2%` = '%3%' WHERE `handle` = '%1%'") % m_handle % m_reputation % m_reputation_ma );
		}else if( m_reputation_me > m_reputation_zi && m_reputation_me > m_reputation_ma ){
			m_reputation = "MEROVINGIAN";
			sDatabase.Execute(format("UPDATE `myreputation` SET `%2%` = '%3%' WHERE `handle` = '%1%'") % m_handle % m_reputation % m_reputation_me );
		}
		if( m_reputation_me == m_reputation_zi && m_reputation_me == m_reputation_ma ){
			m_reputation = "NULL"; //unable to define
		}
	}
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//PVP & PVE///////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::RPC_SetPVPState(ByteBuffer &src){
	if(defeatedNPC == true){
		SendMsg("","Wait for server response...","SYSTEM");
		return;
	}

	switch (m_pvp){
	case 0:
		ENTER_DUEL();

		m_pvp = 1;
		opponentId = 0;
		SendMsg("","Your PVP state is active.\nPlease, type: ?duel <opponentHandle>","SYSTEM");
		SetMood(m_goId,"0c");
		break;
	case 1:
		LEAVE_DUEL();

		m_pvp = 0;
		opponentId = 0;
		SendMsg("","Your PVP state is not active.","SYSTEM");
		SetMood(m_goId,"00");
		break;
	}
}
void PlayerObject::InDuel_PVP(string animForMe, string animForOpponentPVP, string animForOpponentPVE, string CAST_FX, string FXForMe, string FXForOpponentPVP, int DAMAGE, double DISTANCE, string weaponForMe){

	if(m_pvp == 0){
		ENTER_DUEL();

		m_pve = 0;
		m_pvp = 1;
		SetMood(m_goId, "0c");
	}

	PlayerObject* player = sObjMgr.getGOPtr(opponentId);
	if(player->m_pvp == 1 && player->can_attack_pvp == false && m_pvp == 1 && can_attack_pvp == true && player->m_pve == 0 && m_pve == 0){
		//start duel
		uint16 damage = 0;
		damage = CastSkillWithSuccess(DAMAGE); //give bonus according with tactic + basic damage

		if(damage > 0){
			int new_playerHP = player->m_healthC - damage;
			if(new_playerHP > 0){
				player->m_healthC = new_playerHP; player->UpdateGUIonDuel(); //save&store new HP for opponent
				can_attack_pvp = false;
				
				DistanceWithPlayer(DISTANCE, animForMe, animForOpponentPVP, true); //continue, cast skill damage-effect

				player->SendMsg("","It's your turn!","BROADCAST");
				SendMsg("","Wait for opponent's answer!","BROADCAST");

				player->can_attack_pvp = true;
				UpdateGUIonDuel();
			}else{ 
				player->m_pvp = 0;
				player->opponentId = 0;
				player->m_healthC = player->m_healthM; player->UpdateGUIonDuel();
				player->can_attack_pvp = false;

				player->SendMsg("","You lost the duel : (opponent)"+m_handle,"SYSTEM");
				player->LEAVE_DUEL();
				//player->SetMood(opponentId,"0d"); //lie on ground (looks like dead)
				player->ReconstructionFrame();

				SendMsg("","You won the duel : (opponent)"+ player->m_handle ,"SYSTEM");

				//restore my data
				opponentId = 0;
				can_attack_pvp = false;
				m_healthC = m_healthM;

				LEAVE_DUEL();
				UpdateGUIonDuel();
				//

				GiveEXP( rand()%5000 );
			}
		}else{
			SendMsg("","{c:00ff00}Your attack missed "+ player->m_handle +".{/c}" ,"SYSTEM");
			player->SendMsg("","{c:00ff00}"+m_handle+"'s attack missed you.{/c}","SYSTEM");
		}
	}else{
		if(player->m_pvp == 0){
			SendMsg("","Your opponent's state is not PVP.","SYSTEM");
			player->SendMsg("","{c:ff0000}"+m_handle+" is trying to attack you.{/c}","SYSTEM");
		}else if(m_pvp == 0){
			SendMsg("","Your state is not PVP.","SYSTEM");
		}else if(can_attack_pvp = false){
			SendMsg("","Wait for your turn!","BROADCAST");
		}
	}
}
void PlayerObject::DistanceWithPlayer(double MIN_DISTANCE, string my_anim, string opponent_anim, bool LOAD_ANIM){
	double my_x = abs(this->getPosition().x);
	double my_z = abs(this->getPosition().z);
	double my_y = abs(this->getPosition().y);

	double my_x_PLAN = this->getPosition().x;
	double my_z_PLAN = this->getPosition().z;
	double my_y_PLAN = this->getPosition().y;

	int my_rot = (int)this->getPosition().getMxoRot();
	LocationVector pos0;

	vector<uint32> objectLists = sObjMgr.getAllGOIds();
	foreach(uint32 objId, objectLists){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(objId);
		}catch (ObjectMgr::ObjectNotAvailable){
			continue;
		}
		if( objId == opponentId ){
			double player_x = abs(player->getPosition().x);
			double player_z = abs(player->getPosition().z);
			double player_y = abs(player->getPosition().y);

			double diff_x = 0;
			if( my_x < player_x){
				diff_x = abs(player_x-my_x);
			}else if( my_x >= player_x){
				diff_x = abs(my_x-player_x);
			}

			double diff_z = 0;
			if( my_z < player_z){
				diff_z = abs(player_z-my_z);
			}else if( my_z >= player_z){
				diff_z = abs(my_z-player_z);
			}

			double diff_y = 0;
			if( my_y < player_y){
				diff_y = abs(player_y-my_y);
			}else if( my_z >= player_y){
				diff_y = abs(my_y-player_y);
			}
			double reduced_y = 0+(diff_y/100);
			diff_y = reduced_y;

			double distance_pit = ( sqrt( (diff_x*diff_x) + (diff_z*diff_z) )/100 );
			double distance_pit_y = sqrt( (distance_pit*distance_pit) + (diff_y*diff_y) );

			double player_x_PLAN = player->getPosition().x;
			double player_z_PLAN = player->getPosition().z;
			double player_y_PLAN = player->getPosition().y;

			pos0.y = player_y_PLAN; //same plan
			pos0.x = player_x_PLAN;

			if(distance_pit > MIN_DISTANCE || distance_pit_y > 0 || distance_pit_y > MIN_DISTANCE){
				if( my_z_PLAN >= 0 && my_z_PLAN < player_z_PLAN){
					pos0.z = player_z_PLAN-MIN_DISTANCE;
				}else if( my_z_PLAN >= 0 && my_z_PLAN >= player_z_PLAN){
					pos0.z = player_z_PLAN+MIN_DISTANCE;
				}
				if( my_z_PLAN < 0 && my_z_PLAN >= player_z_PLAN){
					pos0.z = player_z_PLAN+MIN_DISTANCE;
				}else if( my_z_PLAN < 0 && my_z_PLAN < player_z_PLAN){
					pos0.z = player_z_PLAN-MIN_DISTANCE;
				}

				int player_rot = (int)player->getPosition().getMxoRot();

				int my_new_rot = 0;
				int opponent_rot = 0;
				if( my_z_PLAN < player_z_PLAN){
					opponent_rot = 127;
					my_new_rot = 255;
				}else if( my_z_PLAN >= player_z_PLAN ){
					opponent_rot = 255;
					my_new_rot = 127;
				}

				ByteBuffer packet;

				packet << uint8(0x03) << uint16(m_goId) << uint16(swap16(0x0104)) << uint8(my_new_rot) << uint16(0);
				SendPacketToCmd(Bin2Hex(packet)); packet.clear(); //set my_new_rot
				
				packet << uint8(0x03) << uint16(objId) << uint16(swap16(0x0104)) << uint8(opponent_rot) << uint16(0);
				player->SendPacketToCmd(Bin2Hex(packet)); packet.clear(); //set opponent_rot
			}
		}
	}

	if(LOAD_ANIM == true){
		x_pvp = pos0.x;
		y_pvp = pos0.y;
		z_pvp = pos0.z;
		this->setPosition(pos0);
		sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));

		LoadAnimation(m_goId, my_anim, false);
		LoadAnimation(opponentId, opponent_anim, false);
	}
}
uint16 PlayerObject::CastSkillWithSuccess(int BASIC_DAMAGE){
	PlayerObject* opponent = sObjMgr.getGOPtr(opponentId);
	string opponent_tactic = opponent->m_tactic;

	SPAWN_FX_TARGET(this->getPosition().x,this->getPosition().y-90,this->getPosition().z,"7b314612", 0x803f);
	SPAWN_FX_TARGET(opponent->getPosition().x,opponent->getPosition().y-90,opponent->getPosition().z,"79314612", 0x803f);

	/*
	SPEED -> -15% ACCURACY, -10% DEFENSE
	POWER -> -10% ACCURACY, +20% DAMAGE!
	GRAB -> +5% ACCURACY, -30% DEFENCE, -15% DAMAGE
	*/

	int MY_MELEE_ACCURACY = belief*2;
	int MY_MELEE_RESISTANCE = MY_MELEE_ACCURACY;

	int MY_MELEE_DEFENSE = focus*2;
	int MY_BALLISTIC_ACCURACY = MY_MELEE_DEFENSE ;

	int MY_BALLISTIC_DEFENSE = percepition *2;
	
	if(m_tactic=="SPEED"){
		MY_MELEE_ACCURACY -= (int)(MY_MELEE_ACCURACY)/(100/15);
		MY_MELEE_DEFENSE -= (int)(MY_MELEE_DEFENSE)/(100/10);
	}else if(m_tactic=="POWER"){
		MY_MELEE_ACCURACY -= (int)(MY_MELEE_ACCURACY)/(100/10);
		BASIC_DAMAGE += (int)(BASIC_DAMAGE)/(100/20);
	}else if(m_tactic=="GRAB"){
		MY_MELEE_ACCURACY += (int)(MY_MELEE_ACCURACY)/(100/5);
		MY_MELEE_DEFENSE -= (int)(MY_MELEE_DEFENSE)/(100/30);
		BASIC_DAMAGE -= (int)(BASIC_DAMAGE)/(100/15);
	}

	int OPP_MELEE_ACCURACY = opponent->belief *2;
	int OPP_MELEE_RESISTANCE = OPP_MELEE_ACCURACY;

	int OPP_MELEE_DEFENSE = opponent->focus *2;
	int OPP_BALLISTIC_ACCURACY = OPP_MELEE_DEFENSE;

	int OPP_BALLISTIC_DEFENSE = opponent->percepition *2;

	if(opponent_tactic=="SPEED"){
		OPP_MELEE_ACCURACY -= (int)(OPP_MELEE_ACCURACY)/(100/15);
		OPP_MELEE_DEFENSE -= (int)(OPP_MELEE_DEFENSE)/(100/10);
	}else if(opponent_tactic=="POWER"){
		OPP_MELEE_ACCURACY -= (int)(OPP_MELEE_ACCURACY)/(100/10);
	}else if(opponent_tactic=="GRAB"){
		OPP_MELEE_ACCURACY += (int)(OPP_MELEE_ACCURACY)/(100/5);
		OPP_MELEE_DEFENSE -= (int)(OPP_MELEE_DEFENSE)/(100/30);
	}

	if(OPP_MELEE_DEFENSE > MY_MELEE_ACCURACY || BASIC_DAMAGE < 0){
		BASIC_DAMAGE = 0;
	}

	SendMsg("","Basic Damage <?> "+ (format("%1%")% BASIC_DAMAGE).str(),"BROADCAST");

	int BONUS_DAMAGE = BASIC_DAMAGE-OPP_MELEE_RESISTANCE;
	if(BONUS_DAMAGE>0){
		int perc_amount_bonus = (int)( (BONUS_DAMAGE*100)/BASIC_DAMAGE );
		SendMsg("","Bonus <?> (+)"+ (format("%1%")%perc_amount_bonus).str()+"(%)","BROADCAST");
		
		BASIC_DAMAGE += BONUS_DAMAGE;
	}

	opponent->SendMsg("",m_handle+" hit for "+ (format("%1%")%BASIC_DAMAGE).str() +"damage","SYSTEM");
	SendMsg("","{c:ff0000}OPPONENT HP = "+ (format("%1% of %2%") % opponent->m_healthC % opponent->m_healthM ).str() +"{/c}","SYSTEM" );

	return BASIC_DAMAGE;
}

void PlayerObject::InDuel_PVE(uint16 id, string animForMe, string animForOpponentPVE, int DAMAGE, bool Automatic){

		// Automatic is a periodic reattack triggered by GameSocket::ProcessWorld
	if ( Automatic )
	{
		SendMsg("","Repeating attack.","SYSTEM");
	}

	if(inMission == true){
		if( command_obj[curr_obj] != "DEFEAT" ){
			SendMsg("","{c:ff0000}It's not time for this action!{/c}","SYSTEM");
			LEAVE_DUEL();
			return;
		}
	}

	INFO_LOG(m_handle+" : InDuel_PVE->(sub)RPC_UseCloseCombatStyle@(Type : Attack NPC (starting PVE) )"+ (format("%1%")%id).str() );

	int NPCM = 0; //marker
	for(int i = 0; i<=NPC_number; i++){
		if(NPC_id[i] == id){
			NPCM = i;
			handle_NPC = NPC_HANDLE[i];
			break;
		}
	}

	NPCy_pve -= 90;

	// Disabled restriction on attacking powerful NPCs.
	/*
	if(NPC_level[NPCM]-m_lvl >= 5){
		if(handle_NPC == "Slum Corrupted" && weapon_FLYMAN == true){
			DAMAGE += 450;
		}else{
			LoadAnimation(m_goId,"296e11", true);

			//restore
			m_healthC = m_healthM;
			LEAVE_DUEL();

			m_pve = 0;
			x_pve = x_pve = x_pve = 0;
			UpdateGUIonDuel();

			//SetMood(m_goId, "00");
			ReconstructionFrame();
			//

			SendMsg("","Your level is insufficient to attack this enemy.","FRAMEMODAL");
			return;
		}
	}
	*/
	if(NPC_type[NPCM] == "HOSTILE" || NPC_type[NPCM] == "BIGMAN"){
		opponentId = id;
	
		if(m_pve == 0){
			ENTER_DUEL();

			m_pvp = 0;

			x_pve = this->getPosition().x;
			y_pve = this->getPosition().y;
			z_pve = this->getPosition().z;

			m_pve = 1;
			SetMood(m_goId, "0c");
		}

		if( handle_NPC != "Slum Corrupted" ){
			DAMAGE += CastSkillWithSuccess_NPC(DAMAGE, NPCM);
		}

		int npcHP = NPC_hpC[NPCM] - DAMAGE;

		if( handle_NPC == "Slum Corrupted" ){
			DAMAGE -= 450;
		}
		
		//opponent miss your attack ?
		int possibility = rand()%10;
		if(possibility > 5){ //..yes

			if(handle_NPC == "Slum Corrupted"){
				m_healthC -= DAMAGE; //return damage to you
			}else{
				m_healthC -= DAMAGE;
			}

			if(m_healthC <= 0){ //stop duel, you lose
				//click to attack > fighting > he fall and is dead
				LoadAnimation(m_goId, animForMe, true);
				
				NPC_hpC[NPCM] = NPC_hpM[NPCM];
				//LoadAnimation_NPC(id, 0, animForOpponentPVE);

				//restore
				m_healthC = m_healthM;
				LEAVE_DUEL();

				m_pve = 0;
				x_pve = x_pve = x_pve = 0;
				UpdateGUIonDuel();

				//SetMood(m_goId, "00");
				ReconstructionFrame();
				//
			}else{ //you have 1 possibility to miss opponent attack too, can you?
				LoadAnimation_NPC(id, npcHP, "5d0000");

				SPAWN_FX_TARGET(this->getPosition().x,this->getPosition().y-90,this->getPosition().z,"79314612", 0x803f);

				possibility = rand()%10;
				if(possibility < 5){ //yes
					LoadAnimation(m_goId,"29000e",false);
					m_healthC += DAMAGE; //heal
					SPAWN_FX_TARGET(NPCx_pve,NPCy_pve,NPCz_pve,"78314612", 0x803f);
				}else{
					SPAWN_FX_TARGET(NPCx_pve,NPCy_pve,NPCz_pve,"7B314612", 0x803f);
					LoadAnimation(m_goId,"29df10",false);
					LoadAnimation(m_goId,"298403",false);
				}
			}
			UpdateGUIonDuel();
			return;
		}

		if(npcHP > 0){
			SPAWN_FX_TARGET(this->getPosition().x,this->getPosition().y-90,this->getPosition().z,"7B314612", 0x803f);
			SPAWN_FX_TARGET(NPCx_pve,NPCy_pve,NPCz_pve,"79314612", 0x803f);

			//change to defeat NPC?
			//UpdateGUIonDuel();

			LoadAnimation(m_goId, animForMe, false);

			NPC_hpC[NPCM] = npcHP;
			LoadAnimation_NPC(id, npcHP, animForOpponentPVE);

			SendMsg("","{c:ff0000}NPC HP = "+ (format("%1% of %2%") % NPC_hpC[NPCM] % NPC_hpM[NPCM] ).str() +"{/c}","SYSTEM" );
		}else{
			defeatedNPC = true;

			if( handle_NPC == "Slum Corrupted" ){
				NPCy_pve += 90;
				SPAWN_FX_TARGET(NPCx_pve,NPCy_pve,NPCz_pve,"4a314612", 0xf03f);
				SPAWN_FX_TARGET(NPCx_pve,NPCy_pve,NPCz_pve,"4b314612", 0xf03f);
				SPAWN_FX_TARGET(NPCx_pve,NPCy_pve,NPCz_pve,"7f314612", 0xf03f);
			}

			DefeatNPC(id);

			NPC_hpC[NPCM] = NPC_hpM[NPCM];

			SetMood(m_goId, "00");
			m_healthC = m_healthM; UpdateGUIonDuel();

			SendMsg("","{c:ff0000}You won the duel.{/c}","SYSTEM" );

			//verify total EXP to close the current level
			int raptor = (NPC_level[NPCM]-m_lvl);
			INFO_LOG(m_handle + " : EXP EXPONENT : " + (format("%1%")%raptor).str() );

			if(raptor > 0){
				GiveEXP( (DIFFERENCE_EXP_PER_LVL(false)*raptor) );
			}else{
				GiveEXP( DIFFERENCE_EXP_PER_LVL(false) );
			}
		}
	}
}
uint16 PlayerObject::CastSkillWithSuccess_NPC(int BASIC_DAMAGE, int MARKER_NPC){
	uint16 npc_lvl = 0;
	npc_lvl = NPC_level[MARKER_NPC];

	//SPAWN_FX_TARGET(NPCx_pve,NPCy_pve-90,NPCz_pve,"78314612");
	/*
	SPEED -> -15% ACCURACY, -10% DEFENSE
	POWER -> -10% ACCURACY, +20% DAMAGE!
	GRAB -> +5% ACCURACY, -30% DEFENCE, -15% DAMAGE
	*/

	int MY_MELEE_ACCURACY = belief*2;

	int MY_MELEE_DEFENSE = focus*2;

	int MY_BALLISTIC_DEFENSE = percepition *2;
	
	if(m_tactic=="SPEED"){
		MY_MELEE_ACCURACY -= (int)(MY_MELEE_ACCURACY)/(100/15);
	}else if(m_tactic=="POWER"){
		MY_MELEE_ACCURACY -= (int)(MY_MELEE_ACCURACY)/(100/10);
		BASIC_DAMAGE += (int)(BASIC_DAMAGE)/(100/20);
	}else if(m_tactic=="GRAB"){
		MY_MELEE_ACCURACY += (int)(MY_MELEE_ACCURACY)/(100/5);
		BASIC_DAMAGE -= (int)(BASIC_DAMAGE)/(100/15);
	}

	int OPP_DEFENSE = (int)( (MY_MELEE_ACCURACY*2)/npc_lvl );

	int BONUS_DAMAGE = BASIC_DAMAGE-OPP_DEFENSE;
	if(OPP_DEFENSE == 0 || BASIC_DAMAGE < 0){
		BONUS_DAMAGE = 0;
	}

	SendMsg("","Basic Damage <?> "+ (format("%1%")% BASIC_DAMAGE).str(),"BROADCAST");

	if(BONUS_DAMAGE>0){
		int perc_amount_bonus = (int)( (BONUS_DAMAGE*100)/BASIC_DAMAGE );
		SendMsg("","Bonus <?> (+)"+ (format("%1%")%perc_amount_bonus).str()+"(%)","BROADCAST");
	}

	return BONUS_DAMAGE;
}
void PlayerObject::DefeatNPC(uint16 npcId){
	ByteBuffer packet;

	packet << uint8(0x03) << uint16(npcId) << uint32(swap32(0x02010d00)) << uint32(0);
	SendPacketToCmd(Bin2Hex(packet)); packet.clear(); //lie on the ground

	packet << uint8(0x03) << uint16(npcId);
	packet.append( make_shared<HexGenericMsg>("0501c20200808100000000e0010000c0830000de810303fd070000000000000000")->toBuf() );
	SendPacketToCmd(Bin2Hex(packet)); packet.clear(); //activate loot+fx

	packet << uint8(0x03) << uint32(swap32(0x01000206)) << uint8(0) << uint16(npcId) << uint32(swap32(0x01010000)) << uint16(0);
	SendPacketToCmd(Bin2Hex(packet)); packet.clear(); //loot effect on

	if(inMission == true){
		LEAVE_DUEL();
		if( command_obj[curr_obj] == "DEFEAT" ){
			SendMsg(NPC_name[curr_obj],dial_obj[curr_obj],"TEAM");
			UpdateMission();
		}
	}
}

void PlayerObject::ENTER_DUEL(){
	ByteBuffer packet;
	packet << uint8(0x03) << uint16(m_goId) << uint32(swap32(0x03010c0f)) << uint32(swap32(0x808080c0));
	packet << uint16(m_healthC) << uint32(swap32(0x80800100)) << uint32(swap32(0x00100000)) << uint32(0);
	SendPacketToMe(Bin2Hex(packet));
}
void PlayerObject::LEAVE_DUEL(){
	ByteBuffer packet;
	packet << uint8(0x03) << uint16(m_goId) << uint32(swap32(0x03010c0f)) << uint32(swap32(0x808080c0));
	packet << uint16(m_healthC) << uint32(swap32(0x80800100)) << uint32(swap32(0x00000000)) << uint32(0);
	SendPacketToMe(Bin2Hex(packet));

	SetMood(m_goId, "00");
}

void PlayerObject::RPC_UseFreeAttackStyle(ByteBuffer &src)
{
	string BYTE_PACKET = ""+(format("%1%") % Bin2Hex(src)).str();
	std::vector<std::string> def;
	boost::split(def, BYTE_PACKET, boost::is_any_of(" "));

	if(def[3] == "2f"){ //karate+aikido
		uint16 target = src.read<uint16>();

		if(target > 32000) { //opponent = human
			PlayerObject* player = sObjMgr.getGOPtr(target);
			INFO_LOG(m_handle+" : RPC_UseFreeAttackStyle@(Type : Attack Human)"+ player->m_handle);
		}else{
			//can't happen, opponent = npc
			INFO_LOG(m_handle+" : RPC_UseFreeAttackStyle@(Type : ERROR)");
		}
		return;
	}
}
void PlayerObject::RPC_UseCloseCombatStyle(ByteBuffer &src)
{
	uint16 target = src.read<uint16>();

	if(target < 32000) { //opponent = npc
		INFO_LOG(m_handle+" : RPC_UseCloseCombatStyle@(Type : Attack NPC (?) )"+ (format("%1%")%target).str() );
		if(m_pvp == 0 && defeatedNPC == false){
			if(m_tactic == "BLOCK"){
				SendMsg("","{c:ff0000}You cannot perform this action. Change tactic.{/c}","SYSTEM");
			}else{
				opponentId = target;
				opponentType = src.read<uint16>();

				InDuel_PVE(opponentId, "290401", "cc0000", 100, false);
				//(or) InDuel_PVE(opponentId, "29eb05", "c40000", 50);
			}
			INFO_LOG("TARGET_"+ (format("%1%")%opponentId).str() );
		}else{
			SendMsg("","{c:ff0000}Use 'Block' to unmount your PVP state.{/c}","SYSTEM");
		}
		if(defeatedNPC == true){
			SendMsg("","Wait for server response...","SYSTEM");
		}
	}
	return;
}
void PlayerObject::RPC_ChangeFreeAttackStyle(ByteBuffer &src){
	//when change style: src.read<uint8>() -> 00(or)01
	//do nothing
}

	//////////////////////////////////////////////////////////////////////////////////////////
	//TRADE///////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

//this is a shit, nothing to care about
void PlayerObject::RPC_TradeItem(ByteBuffer &src){
	if(inMission == true){
		SendMsg("","You cannot trade items during a mission.","SYSTEM");
		return;
	}

	uint16 target_id = src.read<uint16>();

	uint8 slot;
	for(int i = 0; i<=7; i++){
		slot = src.read<uint8>();
	}

	string itemId = this->inventory[slot];

	if(target_id > 32000){
		PlayerObject* player = sObjMgr.getGOPtr(target_id);
		for(int i=0; i<96; i++){
			if(player->inventory[i] == "00000000"){
				player->inventory[i] = itemId;

				INFO_LOG(m_handle+" : RPC_TradeItem@"+ player->m_handle + " - "+itemId);

				ByteBuffer packet;
				packet << uint8(0x5e) << uint8(i) << uint8(00);
				packet.append( make_shared<HexGenericMsg>(itemId+"000000001401")->toBuf() );
				player->SendRPC(Bin2Hex(packet)); packet.clear(); //give item to player, immediately to [i=slot]

				player->SendMsg("",m_handle+" gave you an item. Check your inventory.","SYSTEM");

				packet << uint8(0x5e) << uint8(slot) << uint8(00);
				packet.append( make_shared<HexGenericMsg>("00000000000000001401")->toBuf() );
				SendRPC(Bin2Hex(packet)); packet.clear(); //remove item from my slot

				string new_inventory = "";
				for(int j = 0; j<96; j++){
					new_inventory += player->inventory[j]+",";
				}
				new_inventory = new_inventory.substr(0, new_inventory.length()-1);
				SetValueDB("myinventory","inventory",new_inventory,"handle", player->m_handle); //save&store player inventory

				this->inventory[slot] = "00000000";

				new_inventory = "";
				for(int j = 0; j<96; j++){
					new_inventory += this->inventory[j]+",";
				}
				new_inventory = new_inventory.substr(0, new_inventory.length()-1);
				SetValueDB("myinventory","inventory",new_inventory,"handle", m_handle); //save&store my inventory

				break;
			}
		}
	}
}
void PlayerObject::UnstuckClient(){
	string answer = "80 b2 00 00 00 00 08 02";
	SendRPC(answer); //unstuck client
}
void PlayerObject::UpdateHumans(bool onlyCrew){
	ByteBuffer packet;

	bool reveal_me = false;
	if(m_crew[0] != "NONE"){
		reveal_me = true;
	}

	vector<uint32> objectLists = sObjMgr.getAllGOIds();
	foreach (int currObj, objectLists){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(currObj);
		}catch (ObjectMgr::ObjectNotAvailable){ continue; }
		if(player->m_handle == m_handle){
			if(m_lvl == 0){
				m_lvl = CHECK_LEVEL_GAMESOCKET();

				uint16 new_level = 0;
				new_level = m_lvl;
				SetValueDB("characters","level", (format("%1%")%new_level).str() ,"handle", m_handle);
			}

			UpdateCrew(); //unlock mycrew
		}

		if(player != NULL && player->m_district == m_district && player->m_handle != m_handle){

			if(onlyCrew == false){
				if(player->myRSImask != "NULL"){ //check RSI Mask state
					packet << uint8(0x03) << uint16(m_goId);
					packet.append( make_shared<HexGenericMsg>("0280808050")->toBuf() );
					packet << uint16( m_innerStrC ) << uint16( m_healthC ) << uint16(currObj);
					packet.append( make_shared<HexGenericMsg>("0281008090530e8001"+player->myRSImask)->toBuf() );
					SendPacketToCmd(Bin2Hex(packet)); packet.clear(); //set rsi for my client
				}
				if(player->SIT_DOWN == 1){ //check if sit
					packet << uint8(0x03) << uint16(currObj);
					packet.append( make_shared<HexGenericMsg>("030101000000000000")->toBuf() );
					SendPacketToMe(Bin2Hex(packet)); packet.clear();
				}
			}

			if(player->m_crew[0] != "NONE"){
				int id_crew = atoi( (player->m_crew[5]).c_str() );

				if(player->PAL == "1"){ //moderator
					id_crew = 11000;
				}else if(player->PAL == "2"){ //administrator
					id_crew = 12000;
				}

				packet << uint8(0x03) << uint16(currObj) << uint8(0x02) << uint32(swap32(0x81008070)) << uint16(player->m_healthC) << uint8(0) << uint32(id_crew) << uint16(0);
				SendPacketToMe(Bin2Hex(packet)); packet.clear();
			}

			if(reveal_me == true){
				int id_crew = atoi( (m_crew[5]).c_str() );

				if(PAL == "1"){ //moderator
					id_crew = 11000;
				}else if(PAL == "2"){ //administrator
					id_crew = 12000;
				}

				packet << uint8(0x03) << uint16(m_goId) << uint8(0x02) << uint32(swap32(0x81008070)) << uint16(m_healthC) << uint8(0) << uint32(id_crew) << uint16(0);
				player->SendPacketToMe(Bin2Hex(packet)); packet.clear();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void PlayerObject::RPC_NullHandle( ByteBuffer &srcCmd )
{
	return;
}

void PlayerObject::RPC_HandleReadyForSpawn( ByteBuffer &srcCmd )
{
	if (!m_spawnedInWorld)
	{
		this->SpawnSelf();
	}
}

void PlayerObject::ParseAdminCommand( string theCmd )
{
	stringstream cmdStream;
	cmdStream.str(theCmd);

	string command;
	cmdStream >> command;

	if (cmdStream.fail()){
		return;
	}

	using boost::erase_all;

	if(iequals(command, "test90")){
		int type = 0;
		cmdStream >> type;
		if (cmdStream.fail()){
			return;
		}
		if(type == 0 || type > 8){
			type = 1;
		}
		db_testAnim_FOWARD(type, m_goId);
		return;
	}else if(iequals(command, "test91")){
		int type = 0;
		cmdStream >> type;
		if (cmdStream.fail()){
			return;
		}
		if(type == 0 || type > 8){
			type = 1;
		}
		db_testAnim_BACK(type);
		return;
	}

	if(iequals(command, "kick") || iequals(command, "gtfo")){
		string handle;
		cmdStream >> handle;
		if (cmdStream.fail()){
			return;
		}
		string scan_right_handle = GetValueDB("characters","handle","handle",handle);

		vector<uint32> objectLists = sObjMgr.getAllGOIds();
		foreach (int currObj, objectLists){
			PlayerObject* player = NULL;
			try{
				player = sObjMgr.getGOPtr(currObj);
			}catch (ObjectMgr::ObjectNotAvailable){ continue; }
			string handle = player->getHandle();
			if( handle == scan_right_handle ){
				LoadAnimation(currObj,"290f14", true);
				player->SendMsg(m_handle,"You have been disconnected by an administrator.","FRAMEMODAL");
				player->m_parent.Invalidate();
				break;
			}
		}

	}

	if(iequals(command, "spawnFX")){
		vector<uint32> objectLists = sObjMgr.getAllGOIds();

		string fx;
		cmdStream >> fx;
		if (cmdStream.fail()){
			return;
		}

		string curr_pos;
		cmdStream >> curr_pos;
		if (cmdStream.fail()){
			return;
		}
		if(curr_pos == "0"){
			//78-79-7A-7B +314612 YELLOW\RED\PURPLE\GREEN circle
			SPAWN_FX_TARGET(this->getPosition().x,this->getPosition().y-90,this->getPosition().z,fx, 0x803f);
			return;
		}
	}

	if(iequals(command, "give")){
		string type;
		cmdStream >> type;
		if (cmdStream.fail()){
			return;
		}

		for(int j = 0; j<96; j++){
			if(inventory[j] == "00000000"){ //evade double storing
				inventory[j] = type;

				GiveItemToSlot(j,type);
				break;
			}
		}

		SaveInventory();
	}

	if(iequals(command, "spawnNPCs")){
		double distance = 0;
		cmdStream >> distance;
		if (cmdStream.fail()){
			return;
		}

		double quantity = 0;
		cmdStream >> quantity;
		if (cmdStream.fail()){
			return;
		}

		string mode;
		cmdStream >> mode;
		if (cmdStream.fail()){
			return;
		}

		string res;
		cmdStream >> res;
		if (cmdStream.fail()){
			return;
		}

		string rsi;
		cmdStream >> rsi;
		if (cmdStream.fail()){
			return;
		}

		//rsi = 24030058
		if(res == "0"){
			SetNPC_SERIE(distance,quantity,mode,false,rsi,"Tactical Security");
		}else if(res == "1"){
			SetNPC_SERIE(distance,quantity,mode,true,rsi,"Tactical Security");
		}
	}

	if(iequals(command, "setANIMPlayer")){
		string animHEX;
		cmdStream >> animHEX;
		if (cmdStream.fail()){
			return;
		}

		string handle;
		cmdStream >> handle;
		if (cmdStream.fail()){
			return;
		}

		string scan_right_handle = GetValueDB("characters","handle","handle",handle);

		vector<uint32> objectLists = sObjMgr.getAllGOIds();
		foreach (int currObj, objectLists){
			PlayerObject* player = NULL;
			try{
				player = sObjMgr.getGOPtr(currObj);
			}catch (ObjectMgr::ObjectNotAvailable){ continue; }
			string handle = player->getHandle();
			if( handle == scan_right_handle ){
				if(player->m_pvp == 1 || player->m_pvp == 1 ){
					LoadAnimation(currObj,animHEX,false);
				}else{
					LoadAnimation(currObj,animHEX,true);
				}
				break;
			}
		}
	}
	
	if(iequals(command, "setFXPlayer")){
		string fxHEX;
		cmdStream >> fxHEX;
		if (cmdStream.fail()){
			return;
		}

		string handle;
		cmdStream >> handle;
		if (cmdStream.fail()){
			return;
		}

		string scan_right_handle = GetValueDB("characters","handle","handle",handle);

		vector<uint32> objectLists = sObjMgr.getAllGOIds();
		foreach (int currObj, objectLists){
			PlayerObject* player = NULL;
			try{
				player = sObjMgr.getGOPtr(currObj);
			}catch (ObjectMgr::ObjectNotAvailable){ continue; }
			string handle = player->getHandle();
			if( handle == scan_right_handle ){
				CastFXOn(currObj,fxHEX);
				break;
			}
		}
	}

	if(iequals(command, "setFXPlayers")){
		string fxHEX;
		cmdStream >> fxHEX;
		if (cmdStream.fail()){
			return;
		}

		vector<uint32> objectLists = sObjMgr.getAllGOIds();
		foreach (int currObj, objectLists){
			PlayerObject* player = NULL;
			try{
				player = sObjMgr.getGOPtr(currObj);
			}catch (ObjectMgr::ObjectNotAvailable){ continue; }
			if( player != NULL ){
				CastFXOn(currObj,fxHEX);
				break;
			}
		}
	}

	if(iequals(command, "setRSIPlayer")){
		string rsiName;
		cmdStream >> rsiName;
		if (cmdStream.fail()){
			return;
		}

		scoped_ptr<QueryResult> value(sDatabase.Query( format("SELECT `rsiHexPacket` FROM `myrsimask` WHERE `rsiName` = '%1%' LIMIT 1") % rsiName) );
		if (value == NULL){
			SendMsg("","Failed! RSI "+rsiName+" doesn't exist.","SYSTEM");
			return;
		}else{
			Field *field_value = value->Fetch();
			rsiName = (format("%1%")%field_value[0].GetString()).str();
		}

		string handle;
		cmdStream >> handle;
		if (cmdStream.fail()){
			return;
		}

		string scan_right_handle = GetValueDB("characters","handle","handle",handle);

		vector<uint32> objectLists = sObjMgr.getAllGOIds();
		foreach (int currObj, objectLists){
			PlayerObject* player = NULL;
			try{
				player = sObjMgr.getGOPtr(currObj);
			}catch (ObjectMgr::ObjectNotAvailable){ continue; }
			string handle = player->getHandle();
			if( handle == scan_right_handle ){
				player->myRSImask = rsiName;
				player->SetRSIMask( rsiName );
				break;
			}
		}
	}

	if(iequals(command, "setRSIPlayers")){
		string rsiName;
		cmdStream >> rsiName;
		if (cmdStream.fail()){
			return;
		}

		scoped_ptr<QueryResult> value(sDatabase.Query( format("SELECT `rsiHexPacket` FROM `myrsimask` WHERE `rsiName` = '%1%' LIMIT 1") % rsiName) );
		if (value == NULL){
			SendMsg("","Failed! RSI "+rsiName+" doesn't exist.","SYSTEM");
			return;
		}else{
			Field *field_value = value->Fetch();
			rsiName = (format("%1%")%field_value[0].GetString()).str();
		}

		vector<uint32> objectLists = sObjMgr.getAllGOIds();
		foreach (int currObj, objectLists){
			PlayerObject* player = NULL;
			try{
				player = sObjMgr.getGOPtr(currObj);
			}catch (ObjectMgr::ObjectNotAvailable){ continue; }
			string handle = player->getHandle();
			if( player != NULL ){
				player->myRSImask = rsiName;
				player->SetRSIMask( rsiName );
				break;
			}
		}
	}

	if(iequals(command, "ban")){
		string handle;
		cmdStream >> handle;
		if (cmdStream.fail()){
			return;
		}

		string scan_right_handle = GetValueDB("characters","handle","handle",handle);
		if(scan_right_handle == m_handle){
			SendMsg("","You cannot perform this action on yourself.","BROADCAST");
			return;
		}

		bool ban_character = sDatabase.Execute(format("UPDATE `characters` SET `x` = '12223', `y` = '-705', `z` = '59707', `state` = '2' WHERE `handle` = '%1%'") % scan_right_handle);
	}

	if(iequals(command, "allow")){
		string handle;
		cmdStream >> handle;
		if (cmdStream.fail()){
			return;
		}

		string scan_right_handle = GetValueDB("characters","handle","handle",handle);
		if(scan_right_handle == m_handle){
			SendMsg("","You cannot perform this action on yourself.","BROADCAST");
			return;
		}

		bool ban_character = sDatabase.Execute(format("UPDATE `characters` SET `x` = '12223', `y` = '-705', `z` = '59707', `state` = '2' WHERE `handle` = '%1%'") % scan_right_handle);
	}

	if((iequals(command, "who")) || (iequals(command, "count")))
	{
		int users = 1;
		string handles = "You, ";
		vector<uint32> objectLists = sObjMgr.getAllGOIds();
		foreach (int currObj, objectLists){
			PlayerObject* player = NULL;
			try{
				player = sObjMgr.getGOPtr(currObj);
			}catch (ObjectMgr::ObjectNotAvailable){ continue; }
			string handle = player->getHandle();
			if( handle != m_handle ){
				users++;
				handles += handle+", ";
			}
		}
		handles = handles.substr(0, handles.length()-2)+".";

		SendMsg("","Clients connected: " + handles ,"SYSTEM");
		SendMsg("","Total: "+ (format("%1%")%users).str() ,"SYSTEM");
	}

	if (iequals(command, "teleportAll") || iequals(command, "bringAll"))
	{
		LocationVector derp;

		if (iequals(command, "teleportAll"))
		{
			double x,y,z;
			cmdStream >> x;
			if (cmdStream.eof() || cmdStream.fail())
				return;
			cmdStream >> y;
			if (cmdStream.eof() || cmdStream.fail())
				return;
			cmdStream >> z;
			if (cmdStream.fail())
				return;

			x*=100;
			y*=100;
			z*=100;

			derp.ChangeCoords(x,y,z);
		}
		else if (iequals(command, "bringAll"))
		{
			LocationVector newPos = this->getPosition();
			derp.ChangeCoords(newPos.x,newPos.y,newPos.z,newPos.getMxoRot());
		}

		vector<uint32> allObjects = sObjMgr.getAllGOIds();
		foreach(uint32 objId, allObjects)
		{
			PlayerObject* playerObj = NULL;
			try
			{
				playerObj = sObjMgr.getGOPtr(objId);
			}
			catch (ObjectMgr::ObjectNotAvailable)
			{
				continue;
			}

			if (playerObj == NULL)
				continue;

			playerObj->setPosition(derp);
			sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(objId));
			playerObj->PopulateWorld();

		}
		return;
	}
}

void PlayerObject::ParsePlayerCommand( string theCmd )
{
	stringstream cmdStream;
	cmdStream.str(theCmd);

	string command;
	cmdStream >> command;

	if (cmdStream.fail()){
		return;
	}

	using boost::erase_all;

	if(iequals(command, "help")){
		string notes;
		notes = "You can use the following commands:\n?duel <handle>\n?fixCharacter <handle>\n?restorepos\n?up (or) ?down\n?goToPlayer <handle>\n?undo\n?fixGender female(or)male\n?rank\n?fixEXP\n\n?tapping\n?lastmission";
		SendMsg("",notes,"FRAMEMODAL");
	}

	if(iequals(command,"lastmission")){
		int m_mission_id_inThisCase = 0 + atoi( ( GetValueDB("myquests","MISSION_ID","handle",m_handle) ).c_str() );
		if( m_mission_id_inThisCase == 0 ){
			SendMsg("","{c:ff0000}You didn't close any mission yet!{/c}","SYSTEM");
		}else if( m_mission_id_inThisCase > 0 ){
			m_mission_id_inThisCase--;
			string m_mission_name_inThisCase = GetValueDB( "mymissions", "MISSION_NAME", "MISSION_ID", (format("%1%")%m_mission_id_inThisCase).str() );
			SendMsg("","{i}You can reload the mission{/i}: {c:ff0000}" + m_mission_name_inThisCase + " {/c}","SYSTEM");

			SetValueDB( "myquests", "MISSION_ID", (format("%1%")%m_mission_id_inThisCase).str(), "handle", m_handle );
		}
	}

	if(iequals(command,"tapping")){
		if(m_pve == 0 && m_pvp == 0){
			if(tapping == 0){
				tapping = 1;
			}else{
				tapping = 3;
			}
		}else{
			SendMsg("Arcady","Please, be sure to unmount PVP/PVE states and don't run any mission.\n\nThank you.","POPUP_FRIENDLY");
		}
	}

	if(iequals(command,"slot")){
		uint16 slot;
		cmdStream >> slot;
		if ( !cmdStream.eof() || cmdStream.fail()){
			return;
		}
		if(slot >= 1 && slot <= 96){
			slot--;
			string item = inventory[slot];
			if(item != "00000000"){
				slot++;
				SendMsg("","Slot number : "+ (format("%1%") % slot).str() + " @ Item ID : " + item, "SYSTEM" );
			}else{
				SendMsg("","Slot number : "+ (format("%1%") % slot).str() + " @ No item", "SYSTEM" );
			}
		}
		return;
	}

	if(iequals(command, "rank")){
		if(PAL == "ADMIN"){
			SendMsg("","Your rank is : ADMINISTRATOR", "SYSTEM" );
		}else if(PAL == "MOD"){
			SendMsg("","Your rank is : MODERATOR", "SYSTEM" );
		}else if(PAL == "LESIG"){
			SendMsg("","Your rank is : LESIG", "SYSTEM" );
		}else{
			SendMsg("","Your rank is : USER", "SYSTEM" );
		}
	}

	if(iequals(command, "testA")){
		//nothing to care about
	}

	if(iequals(command, "decToHex")){
		uint16 id;
		cmdStream >> id;
		if (cmdStream.fail() || !cmdStream.eof()){
			return;
		}

		ByteBuffer packet;
		packet << uint16(id);
		SendMsg("",Bin2Hex(packet),"SYSTEM");
	}

	//DIFFERENCE_EXP_PER_LVL
	if(iequals(command, "fixEXP")){
		uint32 fix_current_exp = 0;
		fix_current_exp = DIFFERENCE_EXP_PER_LVL(true);
		
		m_exp = fix_current_exp;
		ByteBuffer complete;
		complete << uint16(swap16(0x80e5)) << uint64(m_exp) << uint8(0) << uint32(swap32(0x11000000));
		SendRPC(Bin2Hex(complete)); complete.clear(); //update GUI, exp bar

		CHECK_LEVEL(); //send FX, if you close a level
		SetValueDB( "characters","exp", (format("%1%")%m_exp).str(), "handle", m_handle); //save data

		uint16 new_lvl  = 0+m_lvl;
		SendMsg("","Your EXP is restored to :\n" + (format("%1%")%fix_current_exp).str() + "\n\nYour level is :\n"+ (format("%1%")%new_lvl).str(),"FRAMEMODAL");
	}

	if(iequals(command, "fixGender")){
		string gender;
		cmdStream >> gender;
		if (cmdStream.fail()){
			return;
		}

		string char_id = (format("%1%")%m_characterUID).str();
		if(gender == "female"){
			SetValueDB("rsivalues","sex","1","charId",char_id);
			SetValueDB("rsivalues","body","1","charId",char_id);
		}else if(gender == "male"){
			SetValueDB("rsivalues","sex","0","charId",char_id);
			SetValueDB("rsivalues","body","0","charId",char_id);
		}

		SendMsg("","Please, restart your client.","FRAMEMODAL");
	}

	if(iequals(command, "undo")){

		if(item_to_delete == "NULL"){
			SendMsg("","You cannot perform this action.","SYSTEM");
		}else{
			for(int i = 0; i<96; i++){
				if(inventory[i] == "00000000"){ //evade double storing
					inventory[i] = item_to_delete;
					GiveItemToSlot(i,item_to_delete);
					item_to_delete = "NULL";
					break;
				}
			}
			SaveInventory();

			if(item_to_delete != "NULL"){
				SendMsg("","Inventory full?","SYSTEM"); //usually users is informed by game, directly
			}else{
				SendMsg("","Done!","SYSTEM"); //(..)
			}
		}

	}

	if(iequals(command, "fixCharacter" ))
	{
		string handle;
		cmdStream >> handle;
		if (cmdStream.fail())
			return;
				
		string scan_right_handle = GetValueDB("characters","handle","handle",handle);
		if(scan_right_handle == m_handle)
		{
			SendMsg("","You cannot perform this action on yourself.","BROADCAST");
			return;
		}

		string char_id;
		scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `userId` FROM `characters` WHERE `handle` = '%1%' LIMIT 1") % scan_right_handle) );
		if (value == NULL){
			SendMsg("","Error. This character doesn't exist.","BROADCAST");
			return;
		}else{
			Field *field_value = value->Fetch();
			char_id = ""+format(field_value[0].GetString()).str();
		}

		string curr_char_id = GetValueDB("characters","userId","handle",m_handle);
		if(char_id != curr_char_id){
			SendMsg("","Error. Character exists but does not belong to you.","BROADCAST");
			SendMsg("","If the problem persists, please contact an administrator.","BROADCAST");
			return;
		}
			string advice = "REPORT ASSISTANCE for <?> "+scan_right_handle+"\nSTATE <?> OffLine\n\n";

			SetValueDB("characters","x", "12223" ,"handle",scan_right_handle);
			SetValueDB("characters","y", "-705" ,"handle",scan_right_handle);
			SetValueDB("characters","z", "59707" ,"handle",scan_right_handle);
			SetValueDB("characters","worldId", "1" ,"handle",scan_right_handle);

			advice += "New Coords: (X)12223 (Y)-705 (Z)59707\n";

		SendMsg("",advice,"FRAMEMODAL");
	}
	
	if (iequals(command, "restorepos" ) )
	{
		LocationVector new_pos;
		new_pos.x = 12223.2;
		new_pos.y = -705.0;
		new_pos.z = 59707.3;

		this->setPosition(new_pos);
		sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));

		ByteBuffer packet;
		packet << uint8(0x03) << uint16(m_goId) << uint16(swap16(0x0104)) << uint8(191) << uint16(0); 
		SendPacketToCmd(Bin2Hex(packet));

		LoadAnimation(m_goId, "296f11", true);
	}

	if (iequals(command, "duel"))
	{
		string handle;
		cmdStream >> handle;
		if (cmdStream.fail()){
			return;
		}

		if(handle == m_handle){
			SendMsg("","You can't attack yourself, Mr. Durden.","SYSTEM");
			return;
		}else{
			if(m_pvp == 1){
				vector<uint32> allObjects = sObjMgr.getAllGOIds();
				foreach(uint32 objId, allObjects){
					PlayerObject* player = NULL;
					try{
						player = sObjMgr.getGOPtr(objId);
					}catch (ObjectMgr::ObjectNotAvailable){
						continue;
					}
					if(iequals(player->m_handle,handle)){
						if(player->m_pvp == 1){
							opponentId = objId;
							player->SendMsg("","{c:ff0000}You are " + m_handle + "'s target.{/c}","SYSTEM");
							player->opponentId = m_goId;

							player->can_attack_pvp = false;
							can_attack_pvp = true;

							player->SendMsg("","Wait for opponent's answer!","BROADCAST");
							SendMsg("","It's your turn!","BROADCAST");

							break;
						}else{
							SendMsg("","Your opponent's state is not PVP.","SYSTEM");
							player->SendMsg("","{c:ff0000}" + m_handle + "is trying to attack you.{/c}","SYSTEM");

							break;
						}
					}
				}
			}else{
				SendMsg("","Your state is not PVP.","SYSTEM");
			}
		}
		return;
	}

	if(iequals(command, "district")) // Lame, q&d district change function -bb
	{
	string targetRegion;
	cmdStream >> targetRegion;
	SetRegion(targetRegion);
	}
	
	
	if( iequals(command, "sixxthrr55ttrr") ){
		SendMsg("","Refer to me(Arcady)!","BROADCAST");
		CastFXOn(m_goId, "36314612");

		int random_anim = rand()%5;
		if(random_anim <2){
			LoadAnimation(m_goId, "294d0f", true);
		}else if(random_anim >2){
			LoadAnimation(m_goId, "294e0f", true);
		}

		if(m_currentFX != "CONFETTO"){
			m_currentFX = "CONFETTO";
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	if (iequals(command, "send") || iequals(command, "sendCmd"))
	{
		SendMsg("","Deprecated function.","SYSTEM");
	}
	else if (iequals(command, "UP") )
	{
		LocationVector new_pos;
		new_pos.x = this->getPosition().x;
		new_pos.y = (this->getPosition().y)+400;
		new_pos.z = this->getPosition().z;

		if( abs(new_pos.y/100) <= 300 ){
			this->setPosition(new_pos);
			sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
		}else{
			SendMsg("","You cannot go any further.","BROADCAST");
		}
	}
	else if (iequals(command, "DOWN") )
	{
		LocationVector new_pos;
		new_pos.x = this->getPosition().x;
		new_pos.y = (this->getPosition().y)-400;
		new_pos.z = this->getPosition().z;

		if( abs(new_pos.y/100) <= 300 ){
			this->setPosition(new_pos);
			sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
		}else{
			SendMsg("","You cannot go any further.","BROADCAST");
		}
	}
	else if (iequals(command, "gotoPlayer"))
	{
		string playerName;
		cmdStream >> playerName;

		if (cmdStream.fail()){
			return;
		}

		PlayerObject* theTargetPlayer = NULL;
		{
			vector<uint32> allObjects = sObjMgr.getAllGOIds();
			foreach(uint32 objId, allObjects)
			{
				PlayerObject* playerObj = NULL;
				try
				{
					playerObj = sObjMgr.getGOPtr(objId);
				}
				catch (ObjectMgr::ObjectNotAvailable)
				{
					continue;
				}

				if (iequals(playerName,playerObj->getHandle()))
				{
					theTargetPlayer = playerObj;
					break;
				}
			}
		}

		if (theTargetPlayer == NULL)
		{
			m_parent.QueueCommand(make_shared<SystemChatMsg>((format("Player %1% is not online!")%playerName).str()));
			return;
		}

		this->setPosition(theTargetPlayer->getPosition());		
		sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
		return;
	}
}

// -----------------------------------------------------------------------------------------
// Lame region change function (bitbomb)
// This function does not yet specify coordinates. Once hardlines are synced in all districts,
// the player should arrive at a specific hardline for each district (Array of hardlines)
// -----------------------------------------------------------------------------------------

void PlayerObject::SetRegion(string targetRegion)
{
	// Flag for a handled region change
	bool transitionComplete = false;
	
	struct regionName_t 
	{
		string name1;
		string name2;
	};

	regionName_t regionName[regioncount] = 
				{{"tutorial" , "tut"} , {"richland" , "slums"} ,
				 {"downtown" , "dt"} , {"international","int"} ,
				 {"zeroone" , "01"} , {"onezero" , "10"} ,
				 {"ashencourt" , "ash"} , {"datamine" , "dm"} ,
				 {"sakura" , "sak"} , {"satisplayground" , "sati"} ,
				 {"widowsmoor" , "wm"} , {"yuki" , "yuki"} ,
				 {"large1" , "l1"} , {"large2" , "l2"} ,
				 {"medium1" , "m1"} , {"medium2" , "m2"} ,
				 {"medium3" , "m3"} , {"small3" , "s3"} };
	
	for(int currRegion = 0; currRegion < regioncount; currRegion++)
	{
		// Check for region name match
		if((targetRegion == regionName[currRegion].name1) || (targetRegion == regionName[currRegion].name2))
		{
			// Check if target is current region
			if(currRegion != m_district)
			{
				// Grab the region number as a string
				std::stringstream out;
				out << currRegion;														
				targetRegion = out.str(); 												
				
				SetValueDB("characters","district", targetRegion,"handle",m_handle); 	
				SendMsg("","{i}District changed...{/i}\n\nYou are now disconnected. Your client should close momentarily. If you're suffering the 'chat bug' you will have to close your client manually.","FRAMEMODAL");
				//m_parent.Invalidate();
				this->addEvent(EVENT_JACKOUT,boost::bind(&PlayerObject::jackoutEvent,this),1.0f);
			}
			else
			{
				SendMsg("","You are already in that district.","SYSTEM");
			}
		transitionComplete = true;
		break;
		}
	}
	if(!transitionComplete) 
		SendMsg("","{c:FF0000}Invalid district.{/c}","SYSTEM");
}


void PlayerObject::RPC_HandleChat( ByteBuffer &srcCmd )
{
	uint16 stringLenPos = srcCmd.read<uint16>();
	stringLenPos = swap16(stringLenPos);

	if (stringLenPos != 8){
		//WARNING_LOG(format("(%1%) Chat packet stringLenPos not 8 but %2%, packet %3%") % "|PRIVATE|" % stringLenPos % Bin2Hex(srcCmd));
	}

	srcCmd.rpos(stringLenPos);
	string theMessage = srcCmd.readString();

	if (!theMessage.length()){
		return;
	}

	if (theMessage[0] == '?')
	{
		ParsePlayerCommand(theMessage.substr(1));

		if(PAL == "ADMIN" || PAL == "MOD" || PAL == "LESIG"){
			ParseAdminCommand(theMessage.substr(1));
		}
		return;
	}

	int LIMIT = 50;

	LocationVector my_pos;
	my_pos.x = this->getPosition().x/100;
	my_pos.y = this->getPosition().y/100;
	my_pos.z = this->getPosition().z/100;

	bool sent_message = false;

	vector<uint32> objectLists = sObjMgr.getAllGOIds();
	foreach (int currObj, objectLists){
		PlayerObject* player = NULL;
		try{
			player = sObjMgr.getGOPtr(currObj);
		}catch (ObjectMgr::ObjectNotAvailable){ continue; }
		string handle = player->getHandle();

		if( handle != m_handle && player != NULL && player->m_district == m_district ){
			
			LocationVector player_pos;
			player_pos.x = this->getPosition().x/100;
			player_pos.y = this->getPosition().y/100;
			player_pos.z = this->getPosition().z/100;

			if(my_pos.Distance2D(player_pos) <= LIMIT){
				player->SendMsg(m_handle,theMessage,"AREA");
				sent_message = true;
			}

		}
	}

	if(sent_message == true){
		SendMsg("","Your message has been sent.","SYSTEM");
	}
	//
}

void PlayerObject::RPC_HandleWhisper( ByteBuffer &srcCmd )
{
	uint8 thirdByte = srcCmd.read<uint8>();

	if (thirdByte != 0)
		WARNING_LOG(format("(%1%) Whisper packet third byte not 0 but %2%, packet %3%") % "|PRIVATE|" % uint32(thirdByte) % Bin2Hex(srcCmd));

	uint16 messageLenPos = srcCmd.read<uint16>();
	uint16 whisperCount = srcCmd.read<uint16>();
	whisperCount = swap16(whisperCount); //big endian in packet

	string theRecipient = srcCmd.readString();
	string serverPrefix = sGame.GetChatPrefix() + string("+");
	string::size_type prefixPos = theRecipient.find_first_of(serverPrefix);
	if (prefixPos != string::npos)
		theRecipient = theRecipient.substr(prefixPos+serverPrefix.length());

	if (srcCmd.rpos() != messageLenPos)
		WARNING_LOG(format("(%1%) Whisper packet size byte mismatch, packet %2%") % "|PRIVATE|" % Bin2Hex(srcCmd));

	string theMessage = srcCmd.readString();

	bool sentProperly=false;
	vector<uint32> objectLists = sObjMgr.getAllGOIds();
	foreach (int currObj, objectLists)
	{
		PlayerObject* targetPlayer = NULL;
		try
		{
			targetPlayer = sObjMgr.getGOPtr(currObj);
		}
		catch (ObjectMgr::ObjectNotAvailable)
		{
			continue;
		}

		if (targetPlayer == NULL)
			continue;

		if (iequals(targetPlayer->getHandle(),theRecipient) && targetPlayer->m_district == m_district)
		{
			targetPlayer->getClient().QueueCommand(make_shared<WhisperMsg>(m_handle,theMessage));
			sentProperly=true;
		}
	}
	if (sentProperly)
	{
		INFO_LOG(format("%1% whispered to %2%: %3%") % m_handle % theRecipient % theMessage);
		m_parent.QueueCommand(make_shared<SystemChatMsg>((format("You whispered %1% to %2%")%theMessage%theRecipient).str()));
	}
	else
	{
		INFO_LOG(format("%1% sent whisper to disconnected player %2%: %3%") % m_handle % theRecipient % theMessage);
		m_parent.QueueCommand(make_shared<SystemChatMsg>((format("%1% is not online")%theRecipient).str()));
	}
}

void PlayerObject::RPC_HandleStopAnimation( ByteBuffer &srcCmd )
{
	m_currAnimation = 0;
	sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
}

void PlayerObject::RPC_HandleStartAnimtion( ByteBuffer &srcCmd )
{
	uint8 newAnimation = srcCmd.read<uint8>();
	m_currAnimation = newAnimation;
	sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
}

void PlayerObject::RPC_HandleChangeMood( ByteBuffer &srcCmd )
{
	uint8 newMood = srcCmd.read<uint8>();
	m_currMood = newMood;	
	sGame.AnnounceStateUpdate(NULL,make_shared<AnimationStateMsg>(m_goId));
	return;
}

void PlayerObject::RPC_HandlePerformEmote( ByteBuffer &srcCmd )
{
	uint32 emoteId = srcCmd.read<uint32>();
	uint32 emoteTarget = srcCmd.read<uint32>();

	m_emoteCounter++;
	sGame.AnnounceStateUpdate(NULL,make_shared<EmoteMsg>(m_goId,emoteId,m_emoteCounter));

	DEBUG_LOG(format("(%1%) %2%:%3% doing emote %4% on target %5% at coords %6%,%7%,%8%")
		% "|PRIVATE|"
		% m_handle
		% m_goId
		% Bin2Hex((const byte*)&emoteId,sizeof(emoteId),0)
		% Bin2Hex((const byte*)&emoteTarget,sizeof(emoteTarget),0)
		% m_pos.x % m_pos.y % m_pos.z );
}

void PlayerObject::RPC_HandleDynamicObjInteraction( ByteBuffer &srcCmd )
{
	uint16 viewId = srcCmd.read<uint16>();
	uint16 objType = srcCmd.read<uint16>();
	uint16 interaction = srcCmd.read<uint16>();

	format debugStr = 
		format("(%s) %s:%d interacting with dynamic view id 0x%04x object type 0x%04x interaction %d")
		% "|PRIVATE|"
		% m_handle
		% m_goId
		% viewId
		% objType
		% int(interaction);

	INFO_LOG( debugStr );
	//m_parent.QueueCommand(make_shared<SystemChatMsg>(debugStr.str()));

	string BYTE_PACKET = ""+(format("%1%") % Bin2Hex(srcCmd)).str();
	std::vector<std::string> def;
	boost::split(def, BYTE_PACKET, boost::is_any_of(" "));

	if(def[6] == "05"){ //looting
		INFO_LOG("\t" + m_handle+" : RPC_HandleDynamicObjInteraction@(Type : NPC - Looting)" + (format("%1%") % viewId_loot ).str() );
		
		m_loot = ""; //clean

		//start loot
		ByteBuffer packet;
		packet.append( make_shared<HexGenericMsg>("81190000000010000000000000000000")->toBuf() );

		int quantity = (rand()%4) +1; //if 0 = error, thank +1 = safe

		//check if is missioning -- start loot
		if(inMission == true){
			ByteBuffer testt;
			testt << uint16(atoi(NPC_obj[curr_obj].c_str()));

			if( Bin2Hex(testt) == (def[2]+" "+def[3])){
				if( command_obj[curr_obj] == "LOOT"){
					m_loot = item_inMission[curr_obj]+",";
				}

				if(dial_obj[curr_obj] != "NONE"){
					SendMsg(NPC_name[curr_obj],dial_obj[curr_obj],"TEAM");
				}

				viewId_loot = viewId;
			}
		}//end check loot during mission

		bool extra_item = false; string extra_item_Str;
		if(handle_NPC == "Blackwoods"){
			m_loot+= "d9260000,"; //OnyxRing
		}else if(handle_NPC == "Demon Army"){
			m_loot+= "86bb0000,"; //DemonHeadSouvenir
		}else if(handle_NPC == "Slashers"){
			m_loot+= "db260000,"; //RazorBladePendant
		}else if(handle_NPC == "Silver Bullets"){
			m_loot+= "dc260000,"; //EngravedSilverRing
		}else if(handle_NPC == "Bricks"){
			m_loot+= "fb260000,"; //RustRedHandkerchief
		}

		string char_id = (format("%1%")%m_characterUID).str();
		string gender = GetValueDB("rsivalues","sex","charId",char_id);

		for(int i = 0; i<quantity; i++){
			uint32 rand_turn = 0;

			if(gender == "1"){
				rand_turn = MTRand::getSingleton().randInt(965);
			}else{
				rand_turn = MTRand::getSingleton().randInt(1461)+966;
			}

			string random_item = GetValueDB("myclothes","clothId","clothNumber",(format("%1%") % rand_turn ).str());
			m_loot += random_item + ",";
		}

		m_loot = m_loot.substr(0, m_loot.length()-1);

		std::vector<std::string> DEF_LOOT;
		boost::split(DEF_LOOT, m_loot, boost::is_any_of(","));

		packet << uint16( DEF_LOOT.size() );
		for(int i = 0; i<(int)DEF_LOOT.size(); i++){
			packet.append( make_shared<HexGenericMsg>("00000000" + DEF_LOOT[i] )->toBuf() );
		}

		INFO_LOG("\t" + m_handle + "::(var:myloot)" + m_loot);
		SendRPC(Bin2Hex(packet)); 
		//INFO_LOG(Bin2Hex(packet)); 
		packet.clear();

		string msg1 = "80 a1 a0 11 00 28 00 00 00 00 14 00 1a 00 00 88 17 00 01 00 01 00 00 01 00 00"; //operator: 'take all'
		SendRPC(msg1);
	}else{
		//check if is missioning -- TALK\GET\GIVE
		if(inMission == true){
			ByteBuffer testt;
			testt << uint16(atoi(NPC_obj[curr_obj].c_str()));

			bool proceed_mission = false;

			if( Bin2Hex(testt) == (def[2]+" "+def[3]) && def[4]== "2e" ){
				if( command_obj[curr_obj] == "TALK"){
					proceed_mission = true;
				}else if( command_obj[curr_obj] == "GET"){
					for(int i = 0; i<96; i++){
						if(inventory[i] == "00000000"){ //evade double storing
							inventory[i] = item_inMission[curr_obj];
							GiveItemToSlot(i,item_inMission[curr_obj]);
							break;
						}
					}
					SaveInventory();

					proceed_mission = true;
				}else if( command_obj[curr_obj] == "GIVE"){
					for(int i = 0; i<96; i++){
						if(inventory[i] == item_inMission[curr_obj]){ //evade double storing
							proceed_mission = true;

							inventory[i] = "00000000";
							RemoveItem(i);
							break;
						}
					}
				}

				if(proceed_mission == true){
					SendMsg(NPC_name[curr_obj],dial_obj[curr_obj],"TEAM");
					UpdateMission();
				}else{
					SendMsg("","{c:ff0000}Something is wrong! Try to reload this Mission by aborting it first. If problem persists, contact ADMIN.{/c}","SYSTEM");
				}

				return;
			}else{
				SendMsg("","{c:ff0000}It's not time for this action!{/c}","SYSTEM");
				return;
			}
		}//end check loot during mission

		//check if datanode
		if(objType == 0xf5){
			INFO_LOG(m_handle+" : RPC_HandleDynamicObjInteraction@(Type : Interacting with DataNode )");

			//datanode can be strong to violate
			//calculate level of your character and compare it with REASON(attributes)
			int perc_violation = reason - percepition;
			if(perc_violation > 2){
				LoadAnimation(m_goId,"291514",true);
				SetNPC_DATANODE(viewId_loot,0,0,0,true);
				uint32 lower_exp = rand()%DIFFERENCE_EXP_PER_LVL(false)+1; //evade NULL value
				GiveEXP( lower_exp );
			}else{
				//give a chance to violate
				int chance_violation = rand()%10;
				if(chance_violation < 6){
					SendMsg("The Architect","Haven't you got anything better to do?\n\nIt's the best you'll get if you don't care about your {i}52 65 61 73 6f 6e{/i}","POPUP_FRIENDLY");
					LoadAnimation(m_goId,"291514",true);
				}else{
					SetNPC_DATANODE(viewId_loot,0,0,0,true);
					uint32 lower_exp = rand()%DIFFERENCE_EXP_PER_LVL(false)+1; //evade NULL value
					GiveEXP( lower_exp*2 );
					SendMsg("The Architect","You violated the protocol {c:ff0000}56/X-21{/c}.\n\nIt's only a coincidence. But you won't miss my System again.\n\nBwahah!","POPUP_HOSTILE");
					
					CastFXOn_PRIVATE(m_goId, "1A000028");
					LoadAnimation(m_goId,"296e11", true);
					ReconstructionFrame();
				}
			}
		}else{
			INFO_LOG(m_handle+" : RPC_HandleDynamicObjInteraction@(Type : UNKNOWN )");
		}//check if datanode
	}

}

void PlayerObject::RPC_HandleStaticObjInteraction( ByteBuffer &srcCmd )
{
	uint32 staticObjId = srcCmd.read<uint32>();
	uint16 interaction = srcCmd.read<uint16>();

	vector<std::string> def = GetBytes(srcCmd);

	format debugStr = 
		format("(%s) %s:%d interacting with object id 0x%08x interaction %d")
		% "|PRIVATE|"
		% m_handle
		% m_goId
		% staticObjId
		% int(interaction);

	INFO_LOG( debugStr );

	//m_parent.QueueCommand(make_shared<SystemChatMsg>(debugStr.str()));

	if(interaction == 0x00){ //panel locator-benches
		ByteBuffer packet;

		LocationVector my_new_pos;
		my_new_pos.x = this->getPosition().x;
		my_new_pos.y = this->getPosition().y;
		my_new_pos.z = this->getPosition().z;

		int my_rot = ((int)this->getPosition().getMxoRot())+255;
		if( my_rot > 255){
			my_rot -= 255;
		}

		if( my_rot < 31 || my_rot >= 224){
			my_rot = 127;
			//z+1
			my_new_pos.z += 40;
		}else if( my_rot >= 31 && my_rot < 94){
			my_rot = 190;
			my_new_pos.x += 40;
		}else if( my_rot >= 94 && my_rot < 158){
			my_rot = 0;
			//z-1
			my_new_pos.z -= 40;
		}else if( my_rot >= 158 && my_rot < 224){
			my_rot = 63;
			my_new_pos.x -= 40;
		}

		this->setPosition(my_new_pos);
		sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));

		packet << uint8(0x03) << uint16(m_goId) << uint16(swap16(0x0104)) << uint8(my_rot) << uint16(0); 
		SendPacketToCmd(Bin2Hex(packet)); packet.clear();

		packet << uint8(0x03) << uint16(m_goId);
		packet.append( make_shared<HexGenericMsg>("030101000000000000")->toBuf() );
		SendPacketToCmd(Bin2Hex(packet)); packet.clear();

		SIT_DOWN = 1;
		return;
	}else if( interaction == 0x01 || interaction == 0x02 ){ //vendor\collector
		string id_vendor = def[2]+def[3]+def[4]+def[5];

		INFO_LOG(m_handle+" : RPC_HandleStaticObjInteraction@(Type : Vendor(or)Collector)(id)"+ id_vendor);

		string def, npcName, items;
		int quantity = 0;

		scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `type`, `npcName`, `quantity`, `items` FROM `myvendorsandcollectors` WHERE `npcId` = '%1%' LIMIT 1") % id_vendor ) );
		if (value == NULL){
			def = "NULL";
		}else{
			Field *field_value = value->Fetch();
			def = ""+format(field_value[0].GetString()).str();
			npcName = ""+format(field_value[1].GetString()).str();
			quantity = atoi( ((format(field_value[2].GetString())).str()).c_str() );
			items = ""+format(field_value[3].GetString()).str();
		}

		string packet;

		if(def == "RSI_PILLS"){
			packet = "81 0d 7c ad d9 43 00 00 00 00 80 4e f8 40 00 00 00 00 00 c0 57 40 00 00 00 00 00 62 d6 40 20 00"+parseIntegerToHexBIG(235);
			packet += "a9b60000 fdb60000 03b70000 02b70000 01b70000 00b70000 ffb60000 feb60000 04b70000 0bb70000 0ab70000 09b70000 08b70000 07b70000 06b70000 05b70000 b0b60000 1ab70000 20b70000 1fb70000 1eb70000 1db70000 1cb70000 1bb70000 21b70000 28b70000 27b70000 26b70000 25b70000 24b70000 23b70000 22b70000 aab60000 0cb70000 12b70000 11b70000 10b70000 0fb70000 0eb70000 0db70000 b1b60000 13b70000 19b70000 18b70000 17b70000 16b70000 15b70000 14b70000 a8b60000 eab60000 f2b60000 f1b60000 f0b60000 efb60000 eeb60000 edb60000 ecb60000 ebb60000 f3b60000 fcb60000 fbb60000 fab60000 f9b60000 f8b60000 f7b60000 f6b60000 f5b60000 f4b60000 afb60000 29b70000 32b70000 31b70000 30b70000 2fb70000 2eb70000 2db70000 2cb70000 2bb70000 2ab70000 3ab70000 39b70000 38b70000 37b70000 36b70000 35b70000 34b70000 33b70000 a7b60000 cbb60000 d3b60000 d2b60000 d1b60000 d0b60000 cfb60000 ceb60000 cdb60000 ccb60000 d4b60000 ddb60000 dcb60000 dbb60000 dab60000 d9b60000 d8b60000 d7b60000 d6b60000 d5b60000 deb60000 e9b60000 e8b60000 e7b60000 e6b60000 e5b60000 e4b60000 e3b60000 e2b60000 dfb60000 e1b60000 e0b60000 aeb60000 3bb70000 43b70000 42b70000 41b70000 40b70000 3fb70000 3eb70000 3db70000 3cb70000 44b70000 4db70000 4cb70000 4bb70000 4ab70000 49b70000 48b70000 47b70000 46b70000 45b70000 53b70000 52b70000 51b70000 50b70000 4fb70000 4eb70000 59b70000 58b70000 57b70000 56b70000 55b70000 54b70000 93b60000 94b60000 95b60000 96b60000 97b60000 98b60000 a6b60000 b9b60000 cab60000 c9b60000 c8b60000 c7b60000 c6b60000 c5b60000 c4b60000 c3b60000 c2b60000 c1b60000 c0b60000 bfb60000 beb60000 bdb60000 bcb60000 bbb60000 bab60000 adb60000 5ab70000 6ab70000 69b70000 68b70000 67b70000 66b70000 64b70000 63b70000 62b70000 61b70000 60b70000 5fb70000 5eb70000 5db70000 5cb70000 5bb70000 65b70000 a5b60000 b2b60000 b3b60000 b4b60000 b5b60000 b8b60000 b6b60000 b7b60000 acb60000 6bb70000 71b70000 70b70000 6fb70000 6eb70000 6db70000 6cb70000 99b60000 9ab60000 9bb60000 9cb60000 9db60000 9eb60000 9fb60000 a0b60000 a1b60000 a2b60000 a3b60000 a4b60000 abb60000 72b70000 7db70000 7cb70000 7bb70000 7ab70000 79b70000 78b70000 77b70000 76b70000 75b70000 74b70000 73b70000 ";
		}else if(def == "CLOTHES"){
			packet = "810d7cadd943000000209ac4e34000000000003c91c0000000802e5bbdc02000b60096030000a5030000a6030000a7030000a9030000aa030000fe0300000204000007040000080400003704000038040000390400003a0400003b0400003c040000b4040000b7040000b8040000b9040000ba040000f00400007717000078170000791700007a1700007b1700007c1700007d1700007e1700007f170000e0170000e1170000e2170000e3170000e4170000e5170000eb170000ec170000ed170000ee170000ef170000f0170000f1170000f2170000f3170000f4170000f5170000f6170000f717000032190000331900003419000039190000631900006419000065190000661900006719000068190000691900006a190000f6190000f7190000f8190000f9190000fa190000fb190000fc190000fd190000fe190000ff190000001a0000011a0000021a0000031a0000041a0000051a0000451a0000461a0000c7020000d4020000db020000e4020000e8020000e9020000f50200001c0300001e0300001f030000240300007f030000800300008103000082030000830300008403000085030000860300008d0300007c1300007d1300007e1300007f130000801300008113000082130000831300008413000085130000861300008713000088130000891300008a1300008b1300008c1300008d1300008e1300008f13000090130000911300009213000093130000e0130000e1130000e2130000e3130000e5130000e6130000e7130000e8130000e9130000db140000dc140000dd140000de140000df140000e0140000e1140000e2140000e3140000e4140000e9140000ec140000ed140000ee140000ef140000f0140000f1140000f2140000f3140000f4140000f5140000f6140000f7140000f814000024160000261600002716000028160000291600002a160000fe160000ff1600000017000001170000021700000317000004170000051700006c1a00006d1a00006e1a00006f1a0000701a0000711a0000721a000063b3000065b300005eb3000060b30000";
		}else if(def == "PROP"){
			packet = "810d44b3d74300000000807eefc0000000800abc57400000000000add1402000700067b5000068b5000069b500006ab500006bb500006cb500006db500006eb500006fb5000070b5000071b5000072b5000073b5000074b5000075b5000076b5000077b5000078b5000079b500007ab50000971a00007cb500007db500007eb500007fb5000080b5000081b5000082b5000083b5000084b5000085b5000086b5000087b500004eb6000088b5000089b500008ab500008bb500008cb500008db500008eb500008fb5000090b5000091b5000092b5000093b5000094b5000095b5000096b5000097b5000098b5000099b500009ab500009bb500009cb500009db500009eb500009fb50000a0b50000a1b50000a2b50000a3b50000a4b50000a5b50000a6b50000a7b50000a8b50000a9b50000aab50000abb50000acb50000adb50000aeb50000afb50000b0b50000b1b50000b2b50000b3b50000b4b50000b6b50000b6b50000b7b50000b8b50000b9b50000bab50000bbb50000bcb50000bdb50000beb50000bfb50000c0b50000c1b50000c2b50000c3b50000c4b50000c5b50000c6b50000c7b50000c8b50000c9b50000cab50000cbb50000ccb50000cdb50000ceb50000cfb50000d0b50000d1b50000d2b50000d3b50000d4b50000d5b50000";
		}else if(def == "CONSUMABLES"){
			packet = "810d5cc10044000000804d5ad640000000c0f5e47fc000000000a0ccdb402000010003a80000";
		}else if(def == "BOOKS"){
			packet = "81 0d 7c ad d9 43 00 00 00 00 80 4e f8 40 00 00 00 00 00 c0 57 40 00 00 00 00 00 62 d6 40 20 00"+parseIntegerToHexBIG(7);
			packet += "8A960000 a6b80000 89960000 6eba0000 88960000 87960000 35b30000";
		}else if(def == "RSI_CAPTURES"){
			packet = "81 0d 7c ad d9 43 00 00 00 00 80 4e f8 40 00 00 00 00 00 c0 57 40 00 00 00 00 00 62 d6 40 20 00"+parseIntegerToHexBIG(91);
			 packet += "35bc0000 36bc0000 37bc0000 38bc0000 39bc0000 3abc0000 3bbc0000 3cbc0000 3dbc0000 3ebc0000 3fbc0000 40bc0000 41bc0000 42bc0000 43bc0000 44bc0000 45bc0000 46bc0000 47bc0000 48bc0000 4abc0000 4bbc0000 4cbc0000 4dbc0000 4ebc0000 4fbc0000 50bc0000 51bc0000 52bc0000 53bc0000 54bc0000 55bc0000 56bb0000 56bc0000 57bb0000 57bc0000 58bb0000 58bc0000 59bb0000 59bc0000 5abb0000 5abc0000 5bbb0000 5bbc0000 5cbb0000 5cbc0000 5dbb0000 5dbc0000 5ebb0000 5ebc0000 5fbb0000 5fbc0000 60bb0000 60bc0000 61bb0000 62bb0000 63bb0000 64bb0000 65bb0000 66bb0000 67bb0000 68bb0000 69bb0000 6abb0000 6bbb0000 6cbb0000 6dbb0000 6ebb0000 6fbb0000 70bb0000 71bb0000 72bb0000 73bb0000 74bb0000 75bb0000 76bb0000 77bb0000 78bb0000 79bb0000 7abb0000 7bbb0000 7cbb0000 7dbb0000 7ebb0000 7fbb0000 80bb0000 81bb0000 82bb0000 83bb0000 84bb0000 85bb0000";
		}else if(def == "RSI_DISGUISES"){
			packet = "81 0d 7c ad d9 43 00 00 00 00 80 4e f8 40 00 00 00 00 00 c0 57 40 00 00 00 00 00 62 d6 40 20 00"+parseIntegerToHexBIG(73);
			 packet += " 001f0000 011f0000 021f0000 031f0000 21a80000 22a80000 23a80000 24a80000 27a80000 289e0000 28a80000 2ba80000 2ca80000 2e990000 2f990000 3fa80000 3fb30000 40a80000 40b30000 41a80000 41b30000 42a80000 42b30000 43b30000 44b30000 45b30000 46b30000 47b30000 48b30000 49b30000 4ab30000 4bb30000 4cb30000 4db30000 4eb30000 659e0000 67020000 6aa70000 6b9b0000 6b9e0000 6ba70000 839a0000 949b0000 b09d0000 bba60000 bd9d0000 be9d0000 bf9d0000 c1990000 c2990000 c3990000 c39d0000 c4990000 c49d0000 c5990000 c6990000 cfa70000 d29d0000 e9a70000 f4040000 f5040000 f51e0000 f6040000 f61e0000 f71e0000 f81e0000 f91e0000 fa1e0000 fb1e0000 fc1e0000 fd1e0000 fe1e0000 ff1e0000";
		}else if(def == "RARE"){
			packet = "81 0d 7c ad d9 43 00 00 00 00 80 4e f8 40 00 00 00 00 00 c0 57 40 00 00 00 00 00 62 d6 40 20 00"+parseIntegerToHexBIG(quantity);
			packet += items;
		}

		if(def == "NULL"){
			SendMsg("","UNSUPPORTED VENDOR OR COLLECTOR -> (id)"+id_vendor,"BROADCAST");
		}else{
			SendMsg(npcName,"May I help you?","TEAM");
			SendRPC(packet);
		}
		return;
	}else if (interaction == 0x03){ //open door
		this->GoAhead(1);
		return;
	}
}

void PlayerObject::RPC_HandleJump( ByteBuffer &srcCmd )
{
	LocationVector endPos;
	if (endPos.fromDoubleBuf(srcCmd) == false)
	{
		WARNING_LOG(format("(%1%) %2%:%3% jump packet doesn't have endPos: %4%")
			% "|PRIVATE|"
			% m_handle
			% m_goId
			% Bin2Hex(srcCmd) );
		return;
	}

	vector<byte> extraData(0x0B);
	srcCmd.read(extraData);

	uint32 theTimeStamp = srcCmd.read<uint32>();

/*	DEBUG_LOG(format("(%1%) %2%:%3% jumping to %4%,%5%,%6% extra data %7% timestamp %8%")
		% "|PRIVATE|"
		% m_handle
		% m_goId
		% endPos.x % endPos.y % endPos.z
		% Bin2Hex(&extraData[0],extraData.size())
		% theTimeStamp );*/

	this->setPosition(endPos);
	sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
}
void PlayerObject::RPC_UseFreeHyperJump( ByteBuffer &srcCmd )
{
	LocationVector endPos;
	if (endPos.fromDoubleBuf(srcCmd) == false){
		return;
	}

	string anim = "296b13";
	SendMsg("","CTRL+SPACEBAR","BROADCAST");

	this->setPosition(endPos);
	sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));

	LoadAnimation(m_goId,anim,true);
}

void PlayerObject::RPC_HandleRegionLoadedNotification( ByteBuffer &srcCmd )
{
	vector<byte> fourBytes(4);
	srcCmd.read(fourBytes);
	LocationVector loc;
	if (!loc.fromFloatBuf(srcCmd))
		throw ByteBuffer::out_of_range();

/*	DEBUG_LOG(format("(%1%) %2%:%3% loaded region X:%4% Y:%5% Z:%6% extra: %7%")
		% "|PRIVATE|"
		% m_handle
		% m_goId
		% loc.x % loc.y % loc.z
		% Bin2Hex(fourBytes));
*/
}

void PlayerObject::RPC_HandleReadyForWorldChange( ByteBuffer &srcCmd )
{
	uint32 shouldBeZero = srcCmd.read<uint32>();

	if (shouldBeZero != 0)
		DEBUG_LOG(format("ReadyForWorldChange uint32 is %1%")%shouldBeZero);

	m_district = 0x03;
	InitializeWorld();
	SpawnSelf();
}

void PlayerObject::RPC_HandleWho( ByteBuffer &srcCmd )
{
	stringstream playerList;
	vector<uint32> objects = sObjMgr.getAllGOIds();
	playerList << "Players(" << objects.size() << ")";
	playerList << " [ ";
	foreach (uint32 objId, objects)
	{
		PlayerObject* pObj = NULL;
		try
		{
			pObj = sObjMgr.getGOPtr(objId);
		}
		catch (ObjectMgr::ObjectNotAvailable)
		{
			continue;
		}

		if (pObj->getDistrict() == this->getDistrict())
		{
			playerList << pObj->getHandle() << " ";
		}
	}
	playerList << "]";

	m_parent.QueueCommand(make_shared<WhisperMsg>("PlayersInYourDistrict",playerList.str()));
}

void PlayerObject::RPC_HandleWhereAmI( ByteBuffer &srcCmd )
{
	LocationVector clientSidePos;
	if (clientSidePos.fromFloatBuf(srcCmd) == false)
		throw ByteBuffer::out_of_range();

	bool byte1Valid = false;
	bool byte2Valid = false;
	bool byte3Valid = false;
	uint8 byte1,byte2,byte3;

	if (srcCmd.remaining() >= sizeof(byte1))
	{
		byte1Valid=true;
		srcCmd >> byte1;
	}
	if (srcCmd.remaining() >= sizeof(byte2))
	{
		byte2Valid=true;
		srcCmd >> byte2;
	}
	if (srcCmd.remaining() >= sizeof(byte3))
	{
		byte3Valid=true;
		srcCmd >> byte3;
	}

	INFO_LOG(format("(%1%) %2%:%3% requesting whereami clientPos %4%,%5%,%6% %7%:%8% %9%:%10% %11%:%12%")
		% "|PRIVATE|"
		% m_handle
		% m_goId
		% clientSidePos.x % clientSidePos.y % clientSidePos.z
		% byte1Valid % uint32(byte1)
		% byte2Valid % uint32(byte2)
		% byte3Valid % uint32(byte3) );

	m_parent.QueueCommand(make_shared<WhereAmIResponse>(m_pos));
	//m_parent.QueueCommand(make_shared<HexGenericMsg>("8107"));
}

void PlayerObject::RPC_HandleGetPlayerDetails( ByteBuffer &srcCmd )
{
	uint32 zeroInt = srcCmd.read<uint32>();

	if (zeroInt != 0)
		WARNING_LOG(format("Get player details zero int is %1%") % zeroInt );

	uint16 playerNameStrLenPos = srcCmd.read<uint16>();

	if (playerNameStrLenPos != srcCmd.rpos())
	{
		WARNING_LOG(format("Get player details strlenpos not %1% but %2%") % (int)srcCmd.rpos() % playerNameStrLenPos );
		return;
	}

	srcCmd.rpos(playerNameStrLenPos);
	string thePlayerName = srcCmd.readString();

	vector<uint32> objectList = sObjMgr.getAllGOIds();
	foreach(uint32 objId, objectList)
	{
		PlayerObject* targetPlayer = NULL;
		try
		{
			targetPlayer = sObjMgr.getGOPtr(objId);
		}
		catch (ObjectMgr::ObjectNotAvailable)
		{
			continue;
		}

		if (targetPlayer == NULL)
			continue;

		if (targetPlayer->getHandle() == thePlayerName)
		{
			m_parent.QueueCommand(make_shared<PlayerDetailsMsg>(targetPlayer));
			m_parent.QueueCommand(make_shared<PlayerBackgroundMsg>(targetPlayer->getBackground()));
			break;
		}
	}
}

void PlayerObject::RPC_HandleGetBackground( ByteBuffer &srcCmd )
{
	m_parent.QueueCommand(make_shared<BackgroundResponseMsg>(this->getBackground()));
}

void PlayerObject::RPC_HandleSetBackground( ByteBuffer &srcCmd )
{
	uint16 backgroundStrLenPos = srcCmd.read<uint16>();

	srcCmd.rpos(backgroundStrLenPos);
	string theNewBackground = srcCmd.readString();

	bool success = this->setBackground(theNewBackground);
	if (success)
	{
		INFO_LOG(format("(%1%) %2%:%3% changed background to |%4%|")
			% "|PRIVATE|"
			% m_handle
			% m_goId
			% this->getBackground() );
	}
	else
	{
		WARNING_LOG(format("(%1%) %2%:%3% background sql query update failed")
			% "|PRIVATE|"
			% m_handle
			% m_goId );
	}
}

void PlayerObject::RPC_HandleHardlineTeleport( ByteBuffer &srcCmd )
{
	uint8 hardlineYouAreUsing;
	uint8 districtYouAreIn;
	uint8 hardlineLocation;
	uint8 hardlineDistrict;

	//Get hardlineYouAreUsing
	srcCmd >> hardlineYouAreUsing;

	srcCmd.rpos(6);
	//Get District you are currently in
	srcCmd >> districtYouAreIn;

	srcCmd.rpos(10);
	//Get Hardline location
	srcCmd >> hardlineLocation;

	srcCmd.rpos(14);
	//Get Hardline District
	srcCmd >> hardlineDistrict;

	//See if we need to add this HL to the DB
	LocationVector loc = this->getPosition();

	format sqlHLExists = 
		format("SELECT * FROM `hardlines` WHERE `DistrictId`='%1%' AND `HardlineId`='%2%' LIMIT 1")
		% (int)districtYouAreIn 
		% (int)hardlineYouAreUsing;

	scoped_ptr<QueryResult> resultHLExists(sDatabase.Query(sqlHLExists));
	if (resultHLExists == NULL)
	{
		m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FFFF00}Hardline synchronized :){/c}"));	
		format sqlHLInsert = 
			format("INSERT INTO `hardlines` SET `DistrictId` = '%1%', `HardlineId` = '%2%', X = '%3%', Y = '%4%', Z = '%5%', ROT = '%6%', HardlineName = 'Tagged By %7%'")
			% (int)districtYouAreIn 
			% (int)hardlineYouAreUsing 
			% loc.x % loc.y % loc.z % loc.rot
			% this->getHandle();

		if (sDatabase.Execute(sqlHLInsert))
		{
			format msg1 = 
				format("{c:00FF00}HardlineId:%1% in District %7% Set to Tagged By %2% at X:%3% Y:%4% Z:%5% O:%6%{/c}")
				% (int)hardlineYouAreUsing 
				% this->getHandle() 
				% loc.x % loc.y % loc.z % loc.rot
				% (int)districtYouAreIn;

			m_parent.QueueCommand(make_shared<SystemChatMsg>(msg1.str()));
		}
		else
		{
			m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF00FF}New hardline set, FAILED On INSERT.{/c}"));
		}

	}

	format sql = 
		format("SELECT `X`,`Y`,`Z`, `ROT`, `HardlineName` FROM `hardlines` Where `DistrictId` = '%1%' And `HardlineId` = '%2%' LIMIT 1")
		% (int)hardlineDistrict 
		% (int)hardlineLocation;

	scoped_ptr<QueryResult> result(sDatabase.Query(sql));
	if (result == NULL)
	{
		m_parent.QueueCommand(make_shared<SystemChatMsg>("{c:FF0000}The hardline you selected is not synchronized. You may synch it by travelling FROM that hardline.{/c}"));	
	}
	else
	{
		Field *field = result->Fetch();
		double newX = field[0].GetDouble();
		double newY = field[1].GetDouble();
		double newZ = field[2].GetDouble();
		double newRot = field[3].GetDouble();
		string newlocationName = field[4].GetString();

		format message1 = format("{c:00FFFF}Welcome to %1%.{/c}") % newlocationName;
		m_parent.QueueCommand(make_shared<SystemChatMsg>(message1.str()));

		LocationVector newLoc(newX, newY, newZ);
		newLoc.rot = newRot;
		this->setPosition(newLoc);
		sGame.AnnounceStateUpdate(NULL,make_shared<PositionStateMsg>(m_goId));
	}
}

void PlayerObject::RPC_HandleObjectSelected( ByteBuffer &srcCmd )
{
	uint16 viewId = srcCmd.read<uint16>();
	uint16 objType = srcCmd.read<uint16>();
	/*if (viewId || objType)
	{
		format msg = 
			format("(%s) %s:%d selected dynamic object view id %04x objType %04x")
			% "|PRIVATE|" % m_handle	% m_goId
			% viewId % objType;

		DEBUG_LOG(msg);
		m_parent.QueueCommand(make_shared<SystemChatMsg>(msg.str()));
	}*/
}

void PlayerObject::RPC_HandleJackoutRequest( ByteBuffer &srcCmd )
{
	UpdateBuddyList(true);

	ByteBuffer extraData = ByteBuffer(&srcCmd.contents()[srcCmd.rpos()],srcCmd.remaining());
	format msg = 
		format("(%s) %s:%d wants to jackout with extra data %s")
		% "|PRIVATE|"
		% m_handle
		% m_goId
		% Bin2Hex(extraData,0);

	DEBUG_LOG(msg);
	//effect
	m_parent.QueueState(make_shared<JackoutEffectMsg>(m_goId));
	//chat msg
	m_parent.QueueCommand(make_shared<HexGenericMsg>("2E0700000000000000000000002300002E00000000000000000000000000000000000000"));
	this->addEvent(EVENT_JACKOUT,boost::bind(&PlayerObject::jackoutEvent,this),10.0f); //schedule jackout in 10 seconds
}


void PlayerObject::jackoutEvent()
{
	m_parent.QueueCommand(make_shared<HexGenericMsg>("80fd000000000000"));
	m_parent.FlushQueue();
	//hack, should see why client doesnt send jackout complete msg, instead of invalidating here
	//m_parent.Invalidate();
}

void PlayerObject::RPC_HandleJackoutFinished( ByteBuffer &srcCmd )
{
	//this doesn't get sent by client, even though it does on real server

	ByteBuffer extraData = ByteBuffer(&srcCmd.contents()[srcCmd.rpos()],srcCmd.remaining());
	format msg = 
		format("(%s) %s:%d jackout complete extra data %s")
		% "|PRIVATE|"
		% m_handle
		% m_goId
		% Bin2Hex(extraData,0);

	DEBUG_LOG(msg);
	m_parent.Invalidate();
}
