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
#include "GameSocket.h"
#include "Log.h"
#include "GameClient.h"
#include "Timer.h"
#include "Database/DatabaseEnv.h"
#include "GameServer.h"
#include <Sockets/Ipv4Address.h>

#include <boost/algorithm/string.hpp>
using boost::iequals;
using boost::erase_all;

#include "base64.h"
#include "irrxml/irrXML.h"
#include "irrxml/irrTypes.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

uint16 spawnFX_atMara = 0;

void GameSocket::SendPacketToCmd(string packet){
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
void GameSocket::SetNPC(int district, string type, int id, int level, int hpM, int hpC, string RSI, double x, double y, double z, string handle, int buffer){
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
	NPC_buffer[NPC_number] = buffer;

	NPC_number++;
}
void GameSocket::InstanceNPC(){
	string name_file, temp;
	uint16 proper_district = 0;
	int kNPC = 1;

	name_file = "npc/NPC_COLLECTION.xml";
	INFO_LOG("");

	using namespace irr::io;
	IrrXMLReader* xml = createIrrXMLReader( name_file.c_str() );
	while(xml && xml->read())
	{
		switch(xml->getNodeType())
		{
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
						string itype, ihandle, iRSI, iIS_DEFEATED, iID, imood;
						float ilevel, ihpM, ihpC, ix_pos, iy_pos, iz_pos, irotation;
						
						itype = xml->getAttributeValue( "type" );
						proper_district = xml->getAttributeValueAsInt( "DISTRICT" );

						if(itype == "HOSTILE")
						{
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
						}

						if(itype == "DERBY_COURT_EFFECT"){
							spawnFX_atMara += kNPC;
						}

						if(itype == "DATANODE"){
							ix_pos = xml->getAttributeValueAsFloat( "x_pos" );
							iy_pos = xml->getAttributeValueAsFloat( "y_pos" );
							iz_pos = xml->getAttributeValueAsFloat( "z_pos" );

							uint16 i_THISid = kNPC+100;

							SetNPC(proper_district, "DATANODE", i_THISid, 0, 0, 0, "NULL", ix_pos, iy_pos, iz_pos, "NULL", false);
						}

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

	INFO_LOG("NPC WORLD ready.");
}

void GameSocket::UpdateClients(){

	m_currTime = getTime();

	if(loadNPC <= 0){		
		InstanceNPC();
		loadNPC = 1;
	}

	if( (m_currTime - updateNPCs) > 30 ){
		//---
		//update NPC world every 120sec
		for(int i = 0; i<NPC_number; i++){
			if(NPC_buffer[i] > 0){
				NPC_buffer[i] = 0;

				ByteBuffer test;

				test.append( make_shared<HexGenericMsg>("030100020600")->toBuf() );
				test << uint16( NPC_id[i] );
				test.append( make_shared<HexGenericMsg>("010000000000")->toBuf() ); 
				SendPacketToCmd(Bin2Hex(test)); test.clear(); //loof fx off

				INFO_LOG("\t(Update NPC)(id)"+ (format("%1%")%NPC_id[i]).str() +" -> Loot FX=off" );

				test << uint8(0x03) << uint16( NPC_id[i] );
				test.append( make_shared<HexGenericMsg>("04808080600000000000")->toBuf() );
				SendPacketToCmd(Bin2Hex(test)); test.clear(); //available on click

				INFO_LOG("\t(Update NPC)(id)"+ (format("%1%")%NPC_id[i]).str() +" -> Available on click=on" );

				test << uint8(0x03) << uint16( NPC_id[i] ) << uint16(swap16(0x0627)) << uint8( rand()%255 );
				test.append( make_shared<HexGenericMsg>("010c000010020088000c00008080808040")->toBuf() );
				test << uint16( NPC_hpM[i] ) << uint8(0);
				SendPacketToCmd(Bin2Hex(test)); test.clear(); //set max HP

				INFO_LOG("\t(Update NPC)(id)"+ (format("%1%")%NPC_id[i]).str() +" -> Restore max HP: " + (format("%1%")%NPC_hpM[i]).str() );

				test << uint8(0x03) << uint16( NPC_id[i] );
				test.append( make_shared<HexGenericMsg>("02010c000000000000000000000000000000")->toBuf() );
				SendPacketToCmd(Bin2Hex(test)); test.clear(); //stand pose

				INFO_LOG("\t(Update NPC)(id)"+ (format("%1%")%NPC_id[i]).str() +" -> Set mood=Aggressive" );

				test << uint8(0x03) << uint16( NPC_id[i] ) << uint16(swap16(0x0208)) << float(NPC_X[i]*100) << float(NPC_Y[i]*100) << float(NPC_Z[i]*100) << uint8(0);
				SendPacketToCmd(Bin2Hex(test)); test.clear(); //respawn at original location

				INFO_LOG("\t(Update NPC)(id)"+ (format("%1%")%NPC_id[i]).str() +" -> Respawn at original location" );

				INFO_LOG("GameSocket::UpdateClients()@(Type : Update NPC)(id)"+ (format("%1%")%NPC_id[i]).str() );
			}
		}

		updateNPCs = m_currTime;
	}

	if( (m_currTime - updateClients) > 2 ){
		vector<uint32> allObjects = sObjMgr.getAllGOIds();
		foreach(uint32 objId, allObjects)
		{
			PlayerObject* player = NULL;
			try{
				player = sObjMgr.getGOPtr(objId);
			}catch (ObjectMgr::ObjectNotAvailable){
				continue;
			}
			player->UnstuckClient(); //unstuck clients

			ByteBuffer test;
			test<<uint16(objId);
			player->m_ID = Bin2Hex(test); test.clear(); //store ID, reversed in twos

			//---
			//start 1st update
			if(player->update_me == 0){
				string handle_player = player->m_handle;

				//---
				//set Access Rank
				player->PAL = player->GetValueDB("myplayers","PAL","handle",handle_player);
				if(player->PAL == "0"){
					player->PAL = "USER";
				}else if(player->PAL == "1"){
					player->PAL = "LESIG";
				}else if(player->PAL == "2"){
					player->PAL = "MOD";
				}else if(player->PAL == "3"){
					player->PAL = "ADMIN";
				}//end set Access Rank

				//---
				//set Attributes
				scoped_ptr<QueryResult> value(sDatabase.Query( format("SELECT `belief`, `percepition`, `reason`, `focus`, `vitality`, `total` FROM `myattributes` WHERE `handle` = '%1%' LIMIT 1") % handle_player) );
				if (value == NULL){ /*do nothing*/ }else{
					Field *field_value = value->Fetch();

					ByteBuffer setAttribute;
					for(int i = 0; i<5; i++){
						if(i == 0){
							setAttribute << uint32(swap32(0x80b25200)) << uint16(field_value[i].GetUInt8()) << uint16(swap16(0x0802));
							player->SendRPC(Bin2Hex(setAttribute)); setAttribute.clear();

							player->belief = field_value[i].GetUInt8();
						}else if(i == 1){
							setAttribute << uint32(swap32(0x80b24f00)) << uint16(field_value[i].GetUInt8()) << uint16(swap16(0x0802));
							player->SendRPC(Bin2Hex(setAttribute)); setAttribute.clear();

							player->percepition = field_value[i].GetUInt8();
						}else if(i == 2){
							setAttribute << uint32(swap32(0x80b25100)) << uint16(field_value[i].GetUInt8()) << uint16(swap16(0x0802));
							player->SendRPC(Bin2Hex(setAttribute)); setAttribute.clear();

							player->reason = field_value[i].GetUInt8();
						}else if(i == 3){
							setAttribute << uint32(swap32(0x80b24e00)) << uint16(field_value[i].GetUInt8()) << uint16(swap16(0x0802));
							player->SendRPC(Bin2Hex(setAttribute)); setAttribute.clear();

							player->focus = field_value[i].GetUInt8();
						}else if(i == 4){
							setAttribute << uint32(swap32(0x80b25400)) << uint16(field_value[i].GetUInt8()) << uint16(swap16(0x0802));
							player->SendRPC(Bin2Hex(setAttribute)); setAttribute.clear();

							player->vitality = field_value[i].GetUInt8();
						}else if(i == 5){
							player->total = 72;
						}
					}
				}//end set attributes

				scoped_ptr<QueryResult> value2(sDatabase.Query( format("SELECT `inventory` FROM `myinventory` WHERE `handle` = '%1%' LIMIT 1") % handle_player) );
				if (value2 == NULL){ /*do nothing*/ }else{
					//protip: this is an incredibly stupid way to store a mostly sparse array of items
					//tables can have more than one row that references a specific key, you know ?

					Field *field_value = value2->Fetch();

					std::vector<std::string> def;
					string inventoryCommaSeparatedList = field_value[0].GetString();
					inventoryCommaSeparatedList += ",00000000";
					boost::split(def, inventoryCommaSeparatedList, boost::is_any_of(","));

					for(int i = 0; i<96; i++){
						def[i] = def[i].substr(0, 8);
						player->inventory[i] = def[i];
						
						//SEE MARGINSOCKET -> (ByteBuffer)inventory
					}
				}//end save&store inventory

				scoped_ptr<QueryResult> value3(sDatabase.Query( format("SELECT `hat`, `glasses`, `shirt`, `gloves`, `coat`, `pants`, `shoes`, `weapon`, `tool` FROM `myappearence` WHERE `handle` = '%1%' LIMIT 1") % handle_player) );
				if (value3 == NULL){ /*do nothing*/ }else{
					Field *field_value = value3->Fetch();

					ByteBuffer storeItem;
					//61 Hat, 62 Glasses, 63 Shirt, 64 Gloves, 65 Coat, 66 Pant, 68 Shoes |69 Weapon, 6a Tool|
					for(int i = 0; i<9; i++){
						string itemId = (format("%1%")%field_value[i].GetString()).str();
						player->appearence[i] = itemId;

						//SEE MARGINSOCKET -> (ByteBuffer)inventory
					}
				}//end save&store appearence

				scoped_ptr<QueryResult> value4(sDatabase.Query( format("SELECT `ZION`, `MACHINIST`, `MEROVINGIAN`, `EPN`, `CYPH` FROM `myreputation` WHERE `handle` = '%1%' LIMIT 1") % handle_player) );
				if (value4 == NULL){ /*do nothing*/ }else{
					Field *field_value = value4->Fetch();
					uint8 rep_zion = field_value[0].GetUInt8();
					uint8 rep_ma = field_value[1].GetUInt8();
					uint8 rep_me = field_value[2].GetUInt8();
					uint8 rep_epn = field_value[3].GetUInt8();
					uint8 rep_cyph = field_value[4].GetUInt8();

					player->m_reputation_zi = rep_zion;
					player->m_reputation_ma = rep_ma;
					player->m_reputation_me = rep_me;
					player->m_reputation_epn = rep_epn;
					player->m_reputation_cyph = rep_cyph;

					if( rep_zion > rep_ma && rep_zion > rep_me ){
						player->m_reputation = "ZION";
					}else if( rep_ma > rep_zion && rep_ma > rep_me ){
						player->m_reputation = "MACHINIST";
					}else if( rep_me > rep_zion && rep_me > rep_ma ){
						player->m_reputation = "MEROVINGIAN";
					}
					if( rep_me == rep_zion && rep_me == rep_ma ){
						player->m_reputation = "NULL"; //unable to define
					}
					if(rep_epn>0){
						player->m_reputation = "EPN";
						if(rep_epn >= 2){
							player->m_reputation = "NIOBESGROUP";
						}
					}else if(rep_cyph>0){
						player->m_reputation = "CYPH";
						if(rep_cyph >= 2){
							player->m_reputation = "MASKEDMEN";
						}
					}
				}//end save reputation

				//
				Sleep(100);
				//

				scoped_ptr<QueryResult> value5(sDatabase.Query( format("SELECT `buddyList` FROM `mybuddylist` WHERE `handle` = '%1%' LIMIT 1") % handle_player) );
				if (value5 == NULL){ /*do nothing*/ }else{
					Field *field_value = value5->Fetch();
					player->buddyList = (format("%1%")%field_value[0].GetString()).str();
				}//end save&store buddylist

				//
				Sleep(100);
				//

				for(int i = 0; i<NPC_number; i++){
					if(NPC_type[i] != "NONE" && NPC_district[i] == player->m_district){
						bool STATE = false;
						if(NPC_buffer[i] > 0){
							STATE = true; //is defeated (?)
						}
						if(STATE == true){
							ByteBuffer spawn_NPC;
							spawn_NPC << uint8(0x03) << uint16(i);
							spawn_NPC.append( make_shared<HexGenericMsg>("02010d000000000000000000000000000000")->toBuf() );
							player->SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear(); //lie on ground

							spawn_NPC << uint8(0x03) << uint16(i);
							spawn_NPC.append( make_shared<HexGenericMsg>("04808080600100000000")->toBuf() );
							player->SendPacketToMe(Bin2Hex(spawn_NPC)); spawn_NPC.clear(); //unavailable on click
						}
					}
				}
				
				//
				Sleep(100);
				//

				//rest TEAM references inside database
				sDatabase.Execute(format("UPDATE `myteam` SET `IN_TEAM` = '0', `COMMAND` = 'NONE', `CAPTAIN` = 'NONE', `MEMBERS` = 'NONE' WHERE `handle` = '%1%'") % player->m_handle);
				player->UpdateBuddyList(false); //give me info BuddyList
				player->UpdateHumans(false); //give me info about other clients (moods etc.)

				player->update_me = 1;
			}//end 1st basic update
			

			if(player->update_me >= 1){
				if(player->update_me == 1){
					//MOTD should be in reality.conf
					player->SendMsg("","{c:8470FF}--{/c} {c:FFFFFF}Welcome to {/c}{c:00FF00}MxO{/c} {c:FFFFFF}Source {/c}{c:8470FF}--{/c}\n\n{c:FFFFFF}Type {/c}{c:00FF00}?help{/c} {c:FFFFFF}to list server commands, or {/c}{c:00FF00}/commands{/c}{c:FFFFFF} to list client commands. Many client commands are unsupported, please check the FAQ first.{/c}\n\nBy continuing, you agree to direct your questions to the FAQ before bringing them anywhere else.\nLink: http://mxosource.com/forum/index.php?topic=259.0","FRAMEMODAL");
					//player->SendMsg("","STOP UPDATE","SYSTEM");

					player->update_me = 2;
				}

				if( player->m_currAnimation >= 100){
					player->m_currAnimation = 1;
				}
				if( player->m_emoteCounter >= 100){
					player->m_emoteCounter = 1;
				}

				//---
				//check if pvp(or)pve is on
				if(player->m_pvp == 1 || player->m_pve == 1){
					string player_tactic = player->m_tactic;
					if(player_tactic == "SPEED"){
						player->CastFXOn(objId, "20070028");
					}else if(player_tactic == "POWER"){
						player->CastFXOn(objId, "1B070028");
					}else if(player_tactic == "GRAB"){
						player->CastFXOn(objId, "25070028");
					}
					player->SetMood(objId, "0c");

					if(player->tapping == 1 || player->tapping == 4 ){
						player->tapping = 0;
					}
				}else if(player->m_pvp == 0 && player->m_pve == 0){
					//check if fx is on (example -> HyperSpeed)
					string fx = player->m_currentFX;
					if(player->tapping == 1 || player->tapping == 4 ){
						player->CastFXOn(objId, "c8060028");
					}else{
						if(fx == "HYPER_SPEED"){
							//player->CastFXOn(objId, "56060028");
						}else if(fx == "CONFETTO"){
							player->CastFXOn(objId, "3f0000de");
						}else if(fx == "AFK"){
							player->CastFXOn(objId, "a8060028");
						}else if(fx == "KUNODOLL"){
							player->CastFXOn(objId,"ec040028");
						}
					}
					//player->SetMood(objId, "00");
					//player->LEAVE_DUEL();
				}

				//---
				//check if tapping
				if(player->tapping == 1){
					player->SendMsg("Arcady","Spawning DataNodes.","POPUP_FRIENDLY");
					for(int i = 0; i<NPC_number; i++){
						if(NPC_type[i] == "DATANODE" && player->m_district == NPC_district[i] ){
							double spawn_DN_y ;
							spawn_DN_y = NPC_Y[i] - (rand()%4) ;
							player->SetNPC_DATANODE(NPC_id[i], NPC_X[i], spawn_DN_y, NPC_Z[i],false);
						}
					}
					player->tapping = 4;
				}else if(player->tapping == 3){
					player->SendMsg("Arcady","DELETING DataNodes.","POPUP_FRIENDLY");
					for(int i = 0; i<NPC_number; i++){
						if(NPC_type[i] == "DATANODE" && player->m_district == NPC_district[i] ){
							player->SetNPC_DATANODE(NPC_id[i], 0, 0, 0,true);
						}
					}
					player->tapping = 0;
				}

				//---
				//check if PVE (update NPC pos!)
				//PVE
				if(player->m_pve == 1 && player->opponentId != 0 && player->defeatedNPC == false){
					//180� = 127�
					test << uint8(0x03) << uint16(objId) << uint16(swap16(0x0104)) << uint8(63) << uint16(0); 
					player->SendPacketToCmd(Bin2Hex(test)); test.clear(); //set my direction (EAST)

					test << uint8(0x03) << uint16(player->opponentId) << uint16(swap16(0x0204)) << uint8(190) << uint32(0);
					player->SendPacketToCmd(Bin2Hex(test)); test.clear(); //set direction NPC, opposite (WEST)

					if(player->getPosition().x >= 0){
						test << uint8(0x03) << uint16(player->opponentId) << uint16(swap16(0x0208)) << float(player->getPosition().x +100);
						if(player->opponentType == 0xf4){
							test << float(player->getPosition().y+200);
							player->NPCrot = uint8(190);
							player->NPCx_pve = (player->getPosition().x +100);
							player->NPCy_pve = (player->getPosition().y+200);
							player->NPCz_pve = (player->getPosition().z);
						}else{
							test << float(player->getPosition().y);
							player->NPCx_pve = (player->getPosition().x +100);
							player->NPCy_pve = (player->getPosition().y);
							player->NPCz_pve = (player->getPosition().z);
						}
						test << float(player->getPosition().z) << uint32(0);
						player->SendPacketToCmd(Bin2Hex(test)); test.clear(); //set coords NPC, 100 units (1 m.) from mine

						test << uint8(0x03) << uint16(objId) << uint16(swap16(0x0104)) << uint8(63) << uint16(0); 
						player->SendPacketToCmd(Bin2Hex(test)); test.clear(); //set my direction (EAST)

						test << uint8(0x03) << uint16(player->opponentId) << uint16(swap16(0x0204)) << uint8(190) << uint32(0);
						player->SendPacketToCmd(Bin2Hex(test)); test.clear(); //set direction NPC, opposite (WEST)
					}else{
						test << uint8(0x03) << uint16(player->opponentId) << uint16(swap16(0x0208)) << float(player->getPosition().x -100);
						if(player->opponentType == 0xf4){
							test << float(player->getPosition().y+200);
							player->NPCrot = uint8(63);
							player->NPCx_pve = (player->getPosition().x -100);
							player->NPCy_pve = (player->getPosition().y+200);
							player->NPCz_pve = (player->getPosition().z);
						}else{
							test << float(player->getPosition().y);
							player->NPCx_pve = (player->getPosition().x -100);
							player->NPCy_pve = (player->getPosition().y);
							player->NPCz_pve = (player->getPosition().z);
						}
						test << float(player->getPosition().z) << uint32(0);
						player->SendPacketToCmd(Bin2Hex(test)); test.clear(); //set coords NPC, 100 units (1 m.) from mine

						test << uint8(0x03) << uint16(objId) << uint16(swap16(0x0104)) << uint8(190) << uint16(0); 
						player->SendPacketToCmd(Bin2Hex(test)); test.clear(); //set my direction (WEST)

						test << uint8(0x03) << uint16(player->opponentId) << uint16(swap16(0x0204)) << uint8(63) << uint32(0);
						player->SendPacketToCmd(Bin2Hex(test)); test.clear(); //set direction NPC, opposite (EAST)
					}
				}

				//---
				//check if Player defeated 1 NPC
				if(player->defeatedNPC == true){
					for(int i = 0; i<=NPC_number; i++){
						if(NPC_id[i] == player->opponentId && NPC_type[i] != "NONE"){
							NPC_buffer[i] = 1;
							INFO_LOG("GameSocket::UpdateClients()@(Type : NPC defeated)(id)"+ (format("%1%")%NPC_id[i]).str() +" (buffer)"+ (format("%1%")%NPC_buffer[i]).str() );
						}
					}

					player->defeatedNPC = false;
					player->opponentId = 0;
					player->m_pve = 0;
					//player->SetMood(objId, "00");

					player->SendMsg("","Server response received.","SYSTEM");
				}//end if 1 NPC is KO

				//---
				//FX at DerbyCourt -- Richland.Mara Central
				if(player->m_district == 1){
					int cast_fx = rand()%10;
					if(cast_fx >= 5 && cast_fx < 7){
						player->CastFXOn_PRIVATE(spawnFX_atMara, "0900004a");
					}else if(cast_fx >= 7){
						player->CastFXOn_PRIVATE(spawnFX_atMara, "aa000004");
					}
				}//end FX at DerbyCourt

				//---
				//check NPC in area
				if(player->m_pve == 0 && player->m_pvp == 0){
					for(int i = 0; i< player->NPC_number; i++){
						if(player->NPC_type[i] == "HOSTILE" && player->NPC_level[i] > player->m_lvl){
							LocationVector my_pos;
							my_pos.x = player->getPosition().x/100;
							my_pos.y = player->getPosition().y/100;
							my_pos.z = player->getPosition().z/100;

							LocationVector NPC_pos;
							NPC_pos.x = player->NPC_X[i];
							NPC_pos.y = player->NPC_Y[i];
							NPC_pos.z = player->NPC_Z[i];

							if( my_pos.Distance2D(NPC_pos) <= 4 ){
								//INFO_LOG("NPC IN AREA NEAR YOU");
								double evv_root = player->getPosition().getMxoRot()+127;
								if(evv_root>255){
									evv_root -= 255;
								}
								test << uint8(0x03) << uint16(player->NPC_id[i]) << uint16(swap16(0x0204)) << uint8( NPC_pos.CalcAngToMxo(my_pos) ) << uint32(0);
								player->SendPacketToCmd(Bin2Hex(test)); test.clear();

								if( rand()%10 < 2){
									//player->SendMsg(NPC_HANDLE[i],"Kill him!","POPUP_EVIL");
									player->LoadAnimation_NPC(player->NPC_id[i],player->NPC_hpC[i],"5d0000");
									player->LoadAnimation(objId, "29cf0b", false);
								}
							}
						}
					}
				}//end check NPC in area
			}//

		}

		updateClients = m_currTime;
	}

}

/*
 * Handles world periodic processes including combat.
 */
void GameSocket::ProcessWorld(){
	GClientList::iterator it ;

		// Check all clients to see if they are in combat and repeat their attack if they are.
	for ( it = m_clients.begin() ; it != m_clients.end() ; it++ )
	{
		GameClient *Client = it->second;

		if ( Client->GetPlayerGOID () >= 32768 )
		{

			PlayerObject *Player = sObjMgr.getGOPtr(Client->GetPlayerGOID()) ;

			if ( Player->m_pve == 1 )
			{
				Player->InDuel_PVE(Player->opponentId, "290401", "cc0000", 100, true);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

GameSocket::GameSocket( ISocketHandler& theHandler ) : UdpSocket(theHandler)
{
	updateClients = getTime();
	updateNPCs = getTime();
	loadNPC = 0;
	NPC_number = 0;

	m_lastCleanupTime = getTime();
	// set player count to 0
	{
		sDatabase.Execute(format("UPDATE `worlds` SET `numPlayers`='0' WHERE `name`='%1%' LIMIT 1")
			% sGame.GetName() );
		m_lastPlayerCount = 0;
	}

}

GameSocket::~GameSocket()
{

}

void GameSocket::OnRawData( const char *pData,size_t len,struct sockaddr *sa_from,socklen_t sa_len )
{
	struct sockaddr_in inc_addr;
	memcpy(&inc_addr,sa_from,sa_len);
	Ipv4Address theAddr(inc_addr);

	TIMING_LOG ( "OnRawData Start" ) ;

	if (theAddr.IsValid() == false)
		return;

	string IPStr = theAddr.Convert(true);
	GClientList::iterator it = m_clients.find(IPStr);
	if (it != m_clients.end())
	{
		GameClient *Client = it->second;
		if (Client->IsValid() == false)
		{
			DEBUG_LOG( format("Removing dead client [%1%]") % IPStr );
			m_clients.erase(it);
			delete Client;
		}
		else
		{
			Client->HandlePacket(pData, len);
		}
	}
	else
	{
		m_clients[IPStr] = new GameClient(inc_addr, this);
		DEBUG_LOG(format ("Client connected [%1%], now have [%2%] clients")
			% IPStr % Clients_Connected());

		m_clients[IPStr]->HandlePacket(pData, len);
	}

	TIMING_LOG ( "OnRawData End" ) ;
}

void GameSocket::PruneDeadClients()
{
	m_currTime = getTime();

	// Do client cleanup
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();)
	{
		GameClient *Client = it->second;
		if (!Client->IsValid() || (m_currTime - Client->LastActive()) >= 20)
		{
			if (!Client->IsValid())
				DEBUG_LOG( format("Removing invalidated client [%1%]") % Client->Address() );
			else
				DEBUG_LOG( format("Removing client due to time-out [%1%]") % Client->Address() );

			m_clients.erase(it++);
			delete Client;
		}
		else
		{
			++it;
		}
	}

	if ((m_currTime - m_lastCleanupTime) >= 5)
	{
		// Update player count
		if (m_lastPlayerCount != this->Clients_Connected())
		{
			sDatabase.Execute(format("UPDATE `worlds` SET `numPlayers`='%1%' WHERE `name`='%2%' LIMIT 1")
				% this->Clients_Connected()
				% sGame.GetName() );

			m_lastPlayerCount = this->Clients_Connected();
		}

		m_lastCleanupTime = m_currTime;
	}
}

void GameSocket::RemoveCharacter(string IPAddr)
{
	
	GClientList::iterator it = m_clients.find(IPAddr);
	if (it != m_clients.end())
	{
		GameClient *Client = it->second;
		DEBUG_LOG( format("Removing XXX dead client [%1%]") % IPAddr );
		m_clients.erase(it);
		delete Client;		
	}

}

GameClient * GameSocket::GetClientWithSessionId( uint32 sessionId )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second->GetSessionId() == sessionId)
		{
			return it->second;
		}
	}
	return NULL;
}

vector<GameClient*> GameSocket::GetClientsWithCharacterId( uint64 charId )
{
	vector<GameClient*> returns;
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second->GetCharacterId() == charId)
		{
			returns.push_back(it->second);
		}
	}
	return returns;
}

void GameSocket::CheckAndResend()
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		it->second->CheckAndResend();
	}
}

void GameSocket::Broadcast( const ByteBuffer &message, bool command )
{
	if (command)
		return AnnounceCommand(NULL,make_shared<StaticMsg>(message));
	else
		return AnnounceStateUpdate(NULL,make_shared<StaticMsg>(message),true);
}

void GameSocket::AnnounceStateUpdate( GameClient* clFrom, msgBaseClassPtr theMsg, bool immediateOnly, GameClient::packetAckFunc callFunc )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second!=clFrom)
		{
			it->second->QueueState(theMsg,immediateOnly,callFunc);
		}
	}
}

void GameSocket::AnnounceCommand( GameClient* clFrom,msgBaseClassPtr theCmd, GameClient::packetAckFunc callFunc )
{
	for (GClientList::iterator it=m_clients.begin();it!=m_clients.end();++it)
	{
		if (it->second!=clFrom)
		{
			it->second->QueueCommand(theCmd,callFunc);
		}
	}
}
