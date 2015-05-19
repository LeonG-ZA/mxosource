//Arcady's module: expansion to test animation

#pragma once

#include "Common.h"
#include "PlayerObject.h"
#include "Log.h"
#include <string>
#include <cstdlib>

#include <boost/algorithm/string.hpp>
using boost::iequals; 
using boost::erase_all;

class ArcadyClass{
	public:
		ArcadyClass(void){}
		virtual ~ArcadyClass(void){}
};

void PlayerObject::db_testAnim_FOWARD(int type, uint32 obj){
	string str_type = "";
	string hex_anim = "";
	switch(type){
		case 1:
			str_type = "MA_ATTACK";
			if(db_INDEX_MA_ATTACK < 253){
				hex_anim = GetValueDB("myanimations", "HEX", "animId", (format("%1%")%db_INDEX_MA_ATTACK).str() );
				db_INDEX_MA_ATTACK++;
				INFO_LOG("MA ATTACK HEX::" + hex_anim);
			}else{
				db_INDEX_MA_ATTACK = 0;
				INFO_LOG("LIMIT MA ATTACK -- RESTORED");
			}
			break;
		case 2:
			str_type = "MA_DEFENCE";
			if(db_INDEX_MA_DEFENCE < 686){
				hex_anim = GetValueDB("myanimations", "HEX", "animId", (format("%1%")%db_INDEX_MA_DEFENCE).str() );
				db_INDEX_MA_DEFENCE++;
				INFO_LOG("MA DEFENCE HEX::" + hex_anim);
			}else{
				db_INDEX_MA_DEFENCE = 253;
				INFO_LOG("LIMIT MA DEFENCE -- RESTORED");
			}
			break;
		case 3:
			str_type = "MA_REVERSE";
			if(db_INDEX_MA_REVERSE < 961){
				hex_anim = GetValueDB("myanimations", "HEX", "animId", (format("%1%")%db_INDEX_MA_REVERSE).str() );
				db_INDEX_MA_REVERSE++;
				INFO_LOG("MA REVERSE HEX::" + hex_anim);
			}else{
				db_INDEX_MA_REVERSE = 686;
				INFO_LOG("LIMIT MA REVERSE -- RESTORED");
			}
			break;
		case 4:
			str_type = "GM ATTACK";
			if(db_INDEX_GM_ATTACK < 1172){
				hex_anim = GetValueDB("myanimations", "HEX", "animId", (format("%1%")%db_INDEX_GM_ATTACK).str() );
				db_INDEX_GM_ATTACK++;
				INFO_LOG("GM ATTACK HEX::" + hex_anim);
			}else{
				db_INDEX_GM_ATTACK = 961;
				INFO_LOG("LIMIT GM ATTACK -- RESTORED");
			}
			break;
		case 5:
			str_type = "GM DEFENCE";
			if(db_INDEX_GM_DEFENCE < 1275){
				hex_anim = GetValueDB("myanimations", "HEX", "animId", (format("%1%")%db_INDEX_GM_DEFENCE).str() );
				db_INDEX_GM_DEFENCE++;
				INFO_LOG("GM DEFENCE HEX::" + hex_anim);
			}else{
				db_INDEX_GM_DEFENCE = 1172;
				INFO_LOG("LIMIT GM DEFENCE -- RESTORED");
			}
			break;
		case 6:
			str_type = "GM REVERSE";
			if(db_INDEX_GM_REVERSE < 1456){
				hex_anim = GetValueDB("myanimations", "HEX", "animId", (format("%1%")%db_INDEX_GM_REVERSE).str() );
				db_INDEX_GM_REVERSE++;
				INFO_LOG("GM REVERSE HEX::" + hex_anim);
			}else{
				db_INDEX_GM_REVERSE = 1275;
				INFO_LOG("LIMIT GM REVERSE -- RESTORED");
			}
			break;
		case 7:
			str_type = "OTHER";
			if(db_INDEX_OTHER < 1632){
				hex_anim = GetValueDB("myanimations", "HEX", "animId", (format("%1%")%db_INDEX_OTHER).str() );
				db_INDEX_OTHER++;
				INFO_LOG("OTHER HEX::" + hex_anim);
			}else{
				db_INDEX_OTHER = 1456;
				INFO_LOG("LIMIT OTHER -- RESTORED");
			}
			break;
	}
	LoadAnimation(obj, hex_anim, true);
}
void PlayerObject::db_testAnim_BACK(int type){
	switch(type){
		case 1:
			if(db_INDEX_MA_ATTACK > 0){
				db_INDEX_MA_ATTACK--;
			}
			break;
		case 2:
			if(db_INDEX_MA_DEFENCE > 253){
				db_INDEX_MA_DEFENCE--;
			}
			break;
		case 3:
			if(db_INDEX_MA_REVERSE > 686){
				db_INDEX_MA_REVERSE--;
			}
			break;
		case 4:
			if(db_INDEX_GM_ATTACK > 961){
				db_INDEX_GM_ATTACK--;
			}
			break;
		case 5:
			if(db_INDEX_GM_DEFENCE > 1172){
				db_INDEX_GM_DEFENCE--;
			}
			break;
		case 6:
			if(db_INDEX_GM_REVERSE > 1275){
				db_INDEX_GM_REVERSE--;
			}
			break;
		case 7:
			if(db_INDEX_OTHER > 1456){
				db_INDEX_OTHER--;
			}
			break;
	}
}