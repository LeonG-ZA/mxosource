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

#ifndef MXOEMU_PLAYEROBJECT_H
#define MXOEMU_PLAYEROBJECT_H

#include "LocationVector.h"
#include "MessageTypes.h"

class PlayerObject
{
public:
	class CharacterNotFound {};

	PlayerObject(class GameClient &parent,uint64 charUID);
	~PlayerObject();

	void InitializeWorld();
	void SpawnSelf();
	void PopulateWorld();

	void initGoId(uint32 theGoId);
	void HandleStateUpdate(ByteBuffer &srcData);
	void HandleCommand(ByteBuffer &srcCmd);

	string getHandle() const {return m_handle;}
	string getFirstName() const {return m_firstName;}
	string getLastName() const {return m_lastName;}
	string getBackground() const {return m_background;}
	bool setBackground(string newBackground);

	uint64 getExperience() const {return m_exp;}
	uint64 getInformation() const {return m_cash;}
	LocationVector getPosition() const {return m_pos;}
	void setPosition(const LocationVector& newPos) {m_pos = newPos;}
	uint8 getDistrict() const {return m_district;}
	void setDistrict(uint8 newDistrict) {m_district = newDistrict;}
	uint8 getRsiData(byte* outputBuf,size_t maxBufLen) const;
	uint16 getCurrentHealth() const {return m_healthC;}
	uint16 getMaximumHealth() const {return m_healthM;}
	uint16 getCurrentIS() const {return m_innerStrC;}
	uint16 getMaximumIS() const {return m_innerStrM;}
	uint32 getProfession() const {return m_prof;}
	uint8 getLevel() const {return m_lvl;}
	uint8 getAlignment() const {return m_alignment;}
	bool getPvpFlag() const {return m_pvpflag;}

	uint8 getCurrentAnimation() const {return m_currAnimation;}
	uint8 getCurrentMood() const {return m_currMood;}

	class GameClient& getClient() { return m_parent; }
	vector<msgBaseClassPtr> getCurrentStatePackets();

	void Update();
private: 
	//RPC handler type
	typedef void (PlayerObject::*RPCHandler)( ByteBuffer &srcCmd );

	//RPC handlers
	void RPC_NullHandle(ByteBuffer &srcCmd);
	void RPC_HandleReadyForSpawn(ByteBuffer &srcCmd);
	void RPC_HandleChat( ByteBuffer &srcCmd );
	void RPC_HandleWhisper( ByteBuffer &srcCmd );
	void RPC_HandleStopAnimation( ByteBuffer &srcCmd );
	void RPC_HandleStartAnimtion( ByteBuffer &srcCmd );
	void RPC_HandleChangeMood( ByteBuffer &srcCmd );
	void RPC_HandlePerformEmote( ByteBuffer &srcCmd );
	void RPC_HandleDynamicObjInteraction( ByteBuffer &srcCmd );
	void RPC_HandleStaticObjInteraction( ByteBuffer &srcCmd );
	void RPC_HandleJump( ByteBuffer &srcCmd );
	void RPC_HandleRegionLoadedNotification( ByteBuffer &srcCmd );
	void RPC_HandleReadyForWorldChange( ByteBuffer &srcCmd );
	void RPC_HandleWho( ByteBuffer &srcCmd );
	void RPC_HandleWhereAmI( ByteBuffer &srcCmd );
	void RPC_HandleGetPlayerDetails( ByteBuffer &srcCmd );
	void RPC_HandleGetBackground( ByteBuffer &srcCmd );
	void RPC_HandleSetBackground( ByteBuffer &srcCmd );
	void RPC_HandleHardlineTeleport( ByteBuffer &srcCmd );
	void RPC_HandleObjectSelected( ByteBuffer &srcCmd );
	void RPC_HandleJackoutRequest( ByteBuffer &srcCmd );
	void RPC_HandleJackoutFinished( ByteBuffer &srcCmd );

	//RPC Handler maps
	map<uint8,RPCHandler> m_RPCbyte;
	map<uint16,RPCHandler> m_RPCshort;
private:
	void loadFromDB(bool updatePos=false);
	void checkAndStore();
	void saveDataToDB();
	void setOnlineStatus( bool isOnline );

	typedef enum
	{
		EVENT_JACKOUT
	} eventType;

	typedef boost::function < void (void) > eventFunc;

	void addEvent(eventType type, eventFunc func, float activationTime);
	size_t cancelEvents(eventType type);

	struct eventStruct
	{
		eventStruct(eventType _type, eventFunc _func, float _fireTime) : type(_type), func(_func), fireTime(_fireTime) {}
		eventType type;
		eventFunc func;
		float fireTime;
	};

	list<eventStruct> m_events;

	void jackoutEvent();

	void ParseAdminCommand(string theCmd);
	void ParsePlayerCommand(string theCmd);
	void GoAhead(double distanceToGo);
	void UpdateAppearance();
	class GameClient &m_parent;
	
	//Player info
	uint64 m_characterUID;
	string m_firstName;
	string m_lastName;
	string m_background;

	uint32 m_goId;
	uint64 m_exp,m_cash;
	LocationVector m_pos,m_savedPos;
	shared_ptr<class RsiData> m_rsi;
	uint16 m_healthC,m_healthM,m_innerStrC,m_innerStrM;
	uint32 m_prof;
	uint8 m_alignment;
	bool m_pvpflag;
	uint32 testCount;

	bool m_spawnedInWorld;
	queue<msgBaseClassPtr> m_sendAfterSpawn;
	bool m_worldPopulated;

	uint32 m_lastStore;
	uint32 m_storeCntr;

	uint8 m_currMood;

	bool m_isAdmin;

	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

public:

	//general
	uint8 m_district, m_lvl;
	uint8 m_currAnimation;
	uint8 m_emoteCounter;

	uint8 INDEX_ANIM_AND_FX;

	//ANIM DATABASE
		int db_INDEX_MA_ATTACK , db_INDEX_MA_DEFENCE, db_INDEX_MA_REVERSE, db_INDEX_GM_ATTACK, db_INDEX_GM_DEFENCE, db_INDEX_GM_REVERSE, db_INDEX_OTHER;
		void db_testAnim_FOWARD(int type, uint32 obj);
		void db_testAnim_BACK(int type);
	//

	string m_handle, m_ID, myRSImask, m_reputation, m_currentFX;
	int update_me, update_NPCs, SIT_DOWN;
	uint16 belief, percepition, reason, focus, vitality, total;
	string inventory[96];
	string appearence[9];
	string m_crew[10];
	string buddyList;
	string PAL;
	uint16 show_recycling_msg_once;

	//bin
	string item_to_delete;

	//reputation
	uint16 m_reputation_points, m_reputation_zi, m_reputation_ma, m_reputation_me, m_reputation_epn, m_reputation_cyph;

	//team
	string m_team[10];
	int MEMBERS_TEAM;

	//mission
	string mission_org, mission_title, mission_desc, name_file, mission_detail;
	uint32 mission_exp, mission_info;
	int total_obj, total_npc, curr_obj;
	string desc_obj[100]; string command_obj[100]; string NPC_obj[100]; string NPC_name[100]; int NPC_idOnMission[100]; string dial_obj[100]; string item_inMission[100]; 
	void SetNPC_inMission(string type, int id, int level, int hpM, string RSI, double x, double y, double z, string handle, bool DEFEATED);
	void SetNPC_ContactMission(string type, int id, string RSI, double x, double y, double z, string handle, uint8 root);
	string data_NPC_inMission[100];
	int HP_hostileNPC_inMission[100];
	bool inMission;

	//on sending invitation
	uint16 sender_id, target_id;
	string updateState;

	//tapping
	uint16 tapping;
	bool vioating_protocol56x21;

	//pve
	uint16 m_pve;
	bool defeatedNPC;
		double NPCx_pve, NPCy_pve, NPCz_pve;
		uint8 NPCrot;
		bool weapon_FLYMAN;
	double x_pve, y_pve, z_pve;
	string handle_NPC;
	void SPAWN_FX_TARGET(double a, double b, double c, string FX, uint16 SCALE);

	//pvp
	uint16 m_pvp, m_turn;
	bool can_attack_pvp;
	double x_pvp, y_pvp, z_pvp;

	//pvp+pve
	string m_tactic;
	uint16 opponentId, opponentType;

	//loot
	string m_loot;
	uint16 viewId_loot;

	//npc part
	int NPC_number; 
	int NPC_district[3000]; string NPC_type[3000]; int NPC_id[3000]; int NPC_level[3000]; int NPC_hpM[3000]; int NPC_hpC[3000]; string NPC_RSI[3000]; string NPC_HANDLE[3000];
	double NPC_X[3000];  double NPC_Y[3000];  double NPC_Z[3000];
	void SetNPC_SERIE(double distance, double number, string spawnMode,bool deleteAll,string RSI,string handle); bool lesig_is_spawning;
	void SetNPC_Free(string type, int id, int level, string RSI, double x, double y, double z, string handle, bool DEFEATED, int rotation, uint16 mood);
	void SetNPC_DATANODE(int id, double x, double y, double z, bool deleteDataNode);
	void SetNPC(int district, string type, int id, int level, int hpM, int hpC, string RSI, double x, double y, double z, string handle, bool DEFEATED);
	void SetMood_NPC(uint16 a, string mood);
	void InstanceNPC(); //refer to PopulateWorld()

	//functions
	string GetValueDB(string a, string b, string c, string d);
	void SetValueDB(string a, string b, string c, string d, string e);

	vector<std::string> GetBytes(ByteBuffer &srcCmd);
	int parseInteger(string a);
	string parseIntegerToHexBIG(int a);
	string parseIntegerToHexSMALL(int a);
	string parseStringToHex(string msg, string param);

	void SendRPC(string packet);
	void SendPacketToMe(string packet);
	void SendPacketToCmd(string packet);
	void SendMsg(string handle, string msg, string TYPE);
	void UnstuckClient();

	void DestroyChar();

	uint8 GetProgressiveByte();
	void CastFXOn(uint32 a, string FX);
	void CastFXOn_PRIVATE(uint32 a, string FX);

	void RPC_LoadItem(ByteBuffer &src);
	void LoadItem(string itemId);
	void LoadWeapon(uint32 a, string weapon);
	void RPC_UnmountItem(ByteBuffer &src);
	void RPC_SwapItems(ByteBuffer &src);
	void RPC_RecycleItem(ByteBuffer &src);
	void RPC_UseItem(ByteBuffer &src);
	void UseItem(uint16 slot);
	void RPC_ActivateItem(ByteBuffer &src);
	void SetRSIMask(string itemId);
	void UnmountRSIMask();

	void RPC_LoadTactic(ByteBuffer &src);
	void RPC_CheckReputation(ByteBuffer &src);
	void RPC_TradeItem(ByteBuffer &src);

	void InDuel_PVP(string animForMe, string animForOpponentPVP, string animForOpponentPVE, string CAST_FX, string FXForMe, string FXForOpponentPVP, int DAMAGE, double DISTANCE, string weaponForMe);
	void InDuel_PVE(uint16 id, string animForMe, string animForOpponentPVE, int DAMAGE, bool Automatic);

	void RPC_SetPVPState(ByteBuffer &src);
	void SetMood(uint32 a, string mood);
	void DistanceWithPlayer(double MIN_DISTANCE, string my_anim, string opponent_anim, bool LOAD_ANIM);
	uint16 CastSkillWithSuccess(int BASIC_DAMAGE); //get basic skill damage + bonus(if > 0)
	uint16 CastSkillWithSuccess_NPC(int BASIC_DAMAGE, int MARKER_NPC);
	void LoadAnimation(uint32 a, string animation, bool GOOD);
	void UpdateGUIonDuel();
	void LoadAnimation_NPC(uint16 npcId, int npcHP, string npcAnim);

	void DefeatNPC(uint16 npcId);

	void RPC_AFKMounted(ByteBuffer &src);
	void RPC_AFKUnmounted(ByteBuffer &src);

	void RPC_UseFreeAttackStyle(ByteBuffer &src);
	void RPC_ChangeFreeAttackStyle(ByteBuffer &src);
	void RPC_UseCloseCombatStyle(ByteBuffer &src);

	void RPC_LootAllowed(ByteBuffer &src);
	void RPC_LootRejected(ByteBuffer &src);
	void GiveItemToSlot( uint8 slot, string item );
	
	void RPC_BuyItem(ByteBuffer &src);
	void RPC_SellItem(ByteBuffer &src);
	void RemoveItem(uint16 slot);
	void RPC_CloseVendorFrame(ByteBuffer &src);

	void SaveInventory();

	void UpdateHumans(bool onlyCrew);
	void UpdateConstruct(string itemId);
	void UpdateBuddyList(bool reportMyAbsence); //refer to RPC_HandleJackoutRequest() , PopulateWorld()
	void UpdateCrew();

	void RPC_SetAttributes(ByteBuffer &src);
	uint16 TYPE_ATTRIBUTE(uint16 attribute, uint16 quantity);

	void RPC_AddBuddy(ByteBuffer &src);
	void RPC_RemoveBuddy(ByteBuffer &src);

	void SendInvitation(string msg, string sender);
	void RPC_AnswerInvitation(ByteBuffer &src);
	void InstanceTeam(string msg, string sender);
	void InstanceCrew();

	void RPC_CreateCrew(ByteBuffer &src);
	void RPC_CrewMsg(ByteBuffer &src);
	void RPC_DisbandCrew(ByteBuffer &src);
	void RPC_RecruitCrew(ByteBuffer &src);
	void RPC_CallCrewName(ByteBuffer &src);

	void RPC_StartTeam(ByteBuffer &src);
	void RPC_TeamMsg(ByteBuffer &src);
	void RPC_InviteInTeam(ByteBuffer &src);
	void RPC_DisbandTeam(ByteBuffer &src);

	void RPC_RequestMission(ByteBuffer &src);
	void RPC_LoadMissionInformation(ByteBuffer &src);
	void RPC_AbortMission(ByteBuffer &src);
	void RPC_AcceptMission(ByteBuffer &src);
	void UpdateMission();

	void RPC_UseFreeHyperJump(ByteBuffer &src);

	void RPC_Suicide(ByteBuffer &src);
	void ReconstructionFrame();
	void RPC_Reconstruction(ByteBuffer &src);

	void RPC_UnmountBuff(ByteBuffer &src);
	void RPC_CmdRandom(ByteBuffer &src);

	void RPC_StartInteractionMarket(ByteBuffer &src);
	void RPC_OpenMarket(ByteBuffer &src);
	void RPC_MarketBuy(ByteBuffer &src);
	void RPC_MarketSell(ByteBuffer &src);

	void RPC_OpenEmailAccount(ByteBuffer &src);
	void RPC_LoadEmail(ByteBuffer &src);
	void RPC_SendEmail(ByteBuffer &src);
	void RPC_DeleteEmail(ByteBuffer &src);
	void RPC_CloseEmailAccount(ByteBuffer &src);
	
	void StartTeam();
	void NameMission(string msg);
	void SetObjective(int id, string state, string msg);
	void ReadXML(string nFile, string tag, string TYPE, int turn);
	void NPC_CAN_TALK(bool canTalk);

	void GiveCash(uint64 new_cash);
	void GiveEXP(uint32 new_exp);
	void CHECK_LEVEL();
	uint8 CHECK_LEVEL_GAMESOCKET();
	uint32 DIFFERENCE_EXP_PER_LVL(bool getBasicEXP);
	void UPDATE_REPUTATION();
	void ENTER_DUEL();
	void LEAVE_DUEL();
	void CREW_READER(int idCrew);
	void BUFFER(uint8 a, bool loadA);
	void SetRegion(string targetRegion);

	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

};

#endif