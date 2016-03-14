// Vip core engine
// By slp13at420 of EmuDevs.com

#include "AccountMgr.h"
#include "chat.h"
#include "Config.h"
#include "Grumboz_VIP_Core.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "player.h"
#include "RBAC.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include <unordered_map>
#include "World.h"

// color definitions since i hate yellow..
std::string green = "|cff00cc00";
std::string red = "|cffFF0000";
std::string white = "|cffFFFFFF";
std::string blue = "|cff3333FF";
std::string black = "|cff000000";

float ver = 2.05f;

uint8 VIP_MAX;
uint8 VIP_LEVEL_BONUS;
uint8 VIP_TP_BONUS;
uint32 VIP_STONE_ID;
uint32 VIP_COIN_ID;
uint32 VIP_MG_ID;
uint32 VIP_VOTE_COUNT;
float VIP_OFFSET;
bool VIP_VOTE_ENABLE;
bool VIP_LEVEL_BONUS_ENABLE;
std::string VIP_COIN_NAME;

std::unordered_map<uint32, VipElements> Vip;
std::unordered_map<uint32, ItemVIP> ItemVip;
std::unordered_map<uint8, VipMallGPS> MALL;
std::unordered_map<uint8, VipHomeGPS> HOME;
std::unordered_map<uint32, VipHearthStoneGPS> HearthStone;

VIP::VIP() { }

VIP::~VIP()
{
}

std::string ConvertNumberToString(uint64 numberX)
{
	auto number = numberX;
	std::stringstream convert;
	std::string number32_to_string;
	convert << number;
	number32_to_string = convert.str();

	return number32_to_string;
};

void AnnounceLoggingToWorld(Player* player, uint8 type)
{
	std::string pName = player->GetName();
	uint32 acct_id = player->GetSession()->GetAccountId();
	uint8 PlayerLogInVip = VIP::GetVIP(acct_id);

	SessionMap sessions = sWorld->GetAllSessions(); // GetPlayersInWorld

	for (SessionMap::iterator itr = sessions.begin(); itr != sessions.end(); ++itr)
	{
		if (!itr->second)
			continue;

		uint8 ItrVip = VIP::GetVIP(itr->second->GetAccountId());

		if (PlayerLogInVip <= ItrVip) // if target is same as or higher. won't announce to lower vip's.
		{
			std::string msg = "[" + green + "VIP" + ConvertNumberToString(PlayerLogInVip) + "|r]:";
			msg = msg + pName + green + " has logged";

			if (type == 0) { msg = msg + " out.|r"; };
			if (type == 1) { msg = msg + " in.|r"; };

			ChatHandler(itr->second->GetPlayer()->GetSession()).PSendSysMessage(msg.c_str());
		}
	}
};

class VIP_Load_Conf : public WorldScript
{
public: VIP_Load_Conf() : WorldScript("VIP_Load_Conf"){ };

	virtual void OnConfigLoad(bool /*reload*/)
	{
		TC_LOG_INFO("server.loading", "___________________________________");
		TC_LOG_INFO("server.loading", "| Grumbo'z  VIP  Engine : Loading |");
		TC_LOG_INFO("server.loading", "|__________By_Slp13at420__________|");
		TC_LOG_INFO("server.loading", "|__________of_EmuDev.com__________|");
		TC_LOG_INFO("server.loading", "|-                               -|");
		TC_LOG_INFO("server.loading", "|____________Ver:%.2f_____________|", ver);

		QueryResult VIPItemQery = WorldDatabase.Query("SELECT entry, vip FROM item_template;");

		if (VIPItemQery)
		{
			do
			{
				Field* fields = VIPItemQery->Fetch();
				uint32 item_id = fields[0].GetUInt32();
				uint32 vip = fields[1].GetUInt8();

				ItemVIP& data1 = ItemVip[item_id];
				// Save the DB values to the MyData object
				data1.item_id = item_id;
				data1.vip = vip;

			} while(VIPItemQery->NextRow());
		}
		
		QueryResult gpsQery = WorldDatabase.Query("SELECT * FROM hearthstone;");

		if (gpsQery)
		{
			do
			{
				// unpacks the results of `result` into fields and appoint data to variable.
				Field* fields = gpsQery->Fetch();
				uint32 guid = fields[0].GetUInt32();
				uint32 map_id = fields[1].GetUInt32();
				float x = fields[2].GetFloat();
				float y = fields[3].GetFloat();
				float z = fields[4].GetFloat();
				float o = fields[5].GetFloat();

				VipHearthStoneGPS& data2 = HearthStone[guid];
				// Save the DB values to the MyData object
				data2.guid = guid;
				data2.map_id = map_id;
				data2.x = x;
				data2.y = y;
				data2.z = z;
				data2.o = o;

			} while (gpsQery->NextRow());
		}

		VipMallGPS& data3 = MALL[0];
		// Save the DB values to the MyData object
		data3.map_id = 530;
		data3.x = -1800.3104f;
		data3.y = 5315.0424f;
		data3.z = -12.4276f;
		data3.o = 2.1062f;

		VipMallGPS& data4 = MALL[1]; // like Lua table VIP[acctId].vip
		// Save the DB values to the MyData object
		data4.map_id = 530;
		data4.x = -1921.8005f;
		data4.y = 5546.6264f;
		data4.z = -12.4278f;
		data4.o = 5.2321f;

		VipHomeGPS& data5 = HOME[0]; // like Lua table VIP[acctId].vip
		// Save the DB values to the MyData object
		data5.map_id = 0;
		data5.x = -4906.3911f;
		data5.y = -970.9063f;
		data5.z = 501.4540f;
		data5.o = 2.3338f;

		VipHomeGPS& data6 = HOME[1]; // like Lua table VIP[acctId].vip
		// Save the DB values to the MyData object
		data6.map_id = 1;
		data6.x = 1604.4882f;
		data6.y = -4394.3603f;
		data6.z = 9.9671f;
		data6.o = 3.5517f;

		TC_LOG_INFO("server.loading", "___________________________________");
		TC_LOG_INFO("server.loading", "|   VIP Teleport GPS's : Loaded   |");

		VIP_MAX					= sConfigMgr->GetIntDefault("VIP.MAX", 6);
		VIP_OFFSET				= sConfigMgr->GetFloatDefault("VIP.OFFSET", 0.05f);
		VIP_VOTE_ENABLE			= sConfigMgr->GetBoolDefault("VIP.VOTE_ENABLE", true);
		VIP_VOTE_COUNT			= sConfigMgr->GetIntDefault("VIP.VOTE_COUNT", 125);
		VIP_COIN_ID				= sConfigMgr->GetIntDefault("VIP.COIN", 63020);
		VIP_STONE_ID			= sConfigMgr->GetIntDefault("VIP.STONE", 63021);
		VIP_MG_ID				= sConfigMgr->GetIntDefault("VIP.MAGIC_GOLD", 44209);
		VIP_TP_BONUS			= sConfigMgr->GetIntDefault("VIP.TP_BONUS", 14);
		VIP_LEVEL_BONUS_ENABLE	= sConfigMgr->GetBoolDefault("VIP.LEVEL_BONUS_ENABLE", true);
		VIP_LEVEL_BONUS			= sConfigMgr->GetIntDefault("VIP.LEVEL_BONUS", 1);

		TC_LOG_INFO("server.loading", "___________________________________");
		TC_LOG_INFO("server.loading", "|  VIP MAX_VIP : %u", VIP_MAX);

		if (VIP_VOTE_ENABLE)
		{
			TC_LOG_INFO("server.loading", "___________________________________");
			TC_LOG_INFO("server.loading", "|  VIP VOTES : ENABLED            |");
		};

		TC_LOG_INFO("server.loading", "___________________________________");
		TC_LOG_INFO("server.loading", "|  VIP TP BONUS:%u", VIP_TP_BONUS);

		if (VIP_LEVEL_BONUS_ENABLE)
		{ 
			TC_LOG_INFO("server.loading", "___________________________________");
			TC_LOG_INFO("server.loading", "|  VIP LEVEL BONUS : ENABLED      |");
			TC_LOG_INFO("server.loading", "|  VIP LEVEL BONUS:%u", VIP_LEVEL_BONUS);
		};

		if (!sObjectMgr->GetItemTemplate(VIP_COIN_ID)){ TC_LOG_INFO("server.loading", "! VIP COIN %u MISSING FROM DB ! SERVER CRASHING !", VIP_COIN_ID); };

		VIP_COIN_NAME			= sObjectMgr->GetItemTemplate(VIP_COIN_ID)->Name1;

		TC_LOG_INFO("server.loading", "___________________________________");
		TC_LOG_INFO("server.loading", "|  VIP Config  : Loaded           |");
		TC_LOG_INFO("server.loading", "| Grumbo'z  VIP Engine  : Loaded  |");
		TC_LOG_INFO("server.loading", "|_________________________________|");
	};

};

uint8 VIP::GetVIPMAX()
{
	return VIP_MAX;
};

bool VIP::GetVIPVOTE_ENABLE()
{
	return VIP_VOTE_ENABLE;
}

uint32 VIP::GetVIPVOTECOUNT()
{
	return VIP_VOTE_COUNT;
};

uint32 VIP::GetVIPCOINID()
{
	return VIP_COIN_ID;
};

uint32 VIP::GetVIPSTONEID()
{
	return VIP_STONE_ID;
};

uint32 VIP::GetVIPMGID()
{
	return VIP_MG_ID;
};

float VIP::GetVIPOFFSET()
{
	return VIP_OFFSET;
};

uint8 VIP::GetTALENTBONUS()
{
	return VIP_TP_BONUS;
};

bool VIP::GetLEVELBONUS_ENABLE()
{
	return VIP_LEVEL_BONUS_ENABLE;
};

uint8 VIP::GetLEVELBONUS()
{
	return VIP_LEVEL_BONUS;
};

void VIP::SetHearthStone(uint32 guid, uint32 map_id, float x, float y, float z, float o)
{
	WorldDatabase.PExecute("UPDATE `hearthstone` SET `map_id`='%u', `x`='%f', `y`='%f', `z`='%f', `o`='%f' WHERE guid=%u;", map_id, x, y, z, o, guid);

	VipHearthStoneGPS& data = HearthStone[guid];
	// Save the DB values to the MyData object
	data.guid = guid;
	data.map_id = map_id;
	data.x = x;
	data.y = y;
	data.z = z;
	data.o = o;
}

void VIP::SetVIP(uint32 acct_id, uint8 pvip)
{ // you must update votes first for the dead mans check

	if (VIP_VOTE_ENABLE)
	{
		uint32 pvotes = Vip[acct_id].votes;

		pvip = uint8(pvotes / VIP_VOTE_COUNT); // dead mans check auto-calibrater

		if (pvotes < VIP_VOTE_COUNT)
		{
			pvip = 1;
		}

		if (pvotes >(VIP_VOTE_COUNT * VIP_MAX))
		{
			pvip = VIP_MAX;
		}
	}

	Vip[acct_id].vip = pvip;
	
	PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SET_VIP);
	stmt->setUInt8(0, pvip);
	stmt->setUInt32(1, acct_id);
	LoginDatabase.Execute(stmt);
};

void VIP::SetMG(uint32 acct_id, uint32 pmg)
{
	PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SET_MG);
	stmt->setUInt32(0, pmg);
	stmt->setUInt32(1, acct_id);
	LoginDatabase.Execute(stmt);

	Vip[acct_id].mg = pmg;
};

void VIP::SetVOTES(uint32 acct_id, uint32 pvotes)
{
	PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SET_VOTES);
	stmt->setUInt32(0, pvotes);
	stmt->setUInt32(1, acct_id);
	LoginDatabase.Execute(stmt);

	Vip[acct_id].votes = pvotes;
};

uint8 VIP::GetVIP(uint32 acct_id)
{
	return Vip[acct_id].vip;
};

uint32 VIP::GetMG(uint32 acct_id)
{
	return Vip[acct_id].mg;
};

uint32 VIP::GetVOTES(uint32 acct_id)
{
	return Vip[acct_id].votes;
};

uint8 VIP::GetItemVIP(uint32 item_id)
{
	return ItemVip[item_id].vip;
};

void VIP::SetItemVIP(uint32 item_id, uint8 item_vip)
{
	if (item_vip < 1) { item_vip = 1; };
	if (item_vip > VIP_MAX) { item_vip = VIP_MAX; };

	PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SET_ITEM_VIP);
	stmt->setUInt8(0, item_vip);
	stmt->setUInt32(1, item_id);
	WorldDatabase.Execute(stmt);

	ItemVip[item_id].vip = item_vip;
};

class Grumboz_VIP_Account_Engine : public AccountScript
{
public: Grumboz_VIP_Account_Engine() : AccountScript("Grumboz_VIP_Account_Engine"){ };

		virtual void OnAccountLogout(uint32 accountId)
		{
			TC_LOG_INFO("server.loading", "ACCOUNT::LOGOUT ID:%u VIP:%u", accountId, Vip[accountId].vip);

			Vip.erase(accountId);
		};

		virtual void OnAccountLogin(uint32 accountId)
		{
			if (accountId > 0)
			{
				PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_LOAD_VIP);
				stmt->setUInt32(0, accountId);
				PreparedQueryResult result = LoginDatabase.Query(stmt);

				if (!result)
				{
					TC_LOG_INFO("server.loading", "XX ERROR Loading a VIP table ID %u XX", accountId);
				};

				if (result)
				{
					// unpacks the results of `result` into fields and appoint data to variable.
					Field* fields = result->Fetch();
					uint8 pvip = fields[0].GetUInt8();
					uint32 pmg = fields[1].GetUInt32();
					uint32 pvotes = fields[2].GetUInt32();

					VipElements& data = Vip[accountId]; // like Lua table VIP[acctId].vip
					// Save the DB values to the MyData object
					data.vip = pvip;
					data.mg = pmg;
					data.votes = pvotes;

					VIP::SetVIP(accountId, pvip);

					TC_LOG_INFO("server.loading", "ACCOUNT::LOGIN ID:%u VIP:%u", accountId, Vip[accountId].vip);
				}
			}
		}
};

class Grumboz_VIP_Player_Engine : public PlayerScript
{
public: Grumboz_VIP_Player_Engine() : PlayerScript("Grumboz_VIP_Player_Engine"){ };

	virtual void OnLogout(Player* player)
	{
		AnnounceLoggingToWorld(player, 0);
	};

	virtual void OnLogin(Player* player, bool firstLogin)
	{
		AnnounceLoggingToWorld(player, 1);

		uint32 guid = player->GetGUID();
		uint32 acct_id = player->GetSession()->GetAccountId();
		uint8 Pvip = VIP::GetVIP(acct_id);
		bool lvl_enable = VIP::GetLEVELBONUS_ENABLE();
		uint8 xtra_levels = VIP::GetLEVELBONUS();
		uint8 Plvl = player->getLevel();
		uint8 max_level = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

		uint8 VIP_level_cap = (max_level + (xtra_levels * Pvip)) - xtra_levels; // has to compensate for base VIP 1

		ChatHandler(player->GetSession()).PSendSysMessage("Welcome %s, you are VIP %u.", player->GetName().c_str(), Vip[acct_id].vip);
		ChatHandler(player->GetSession()).PSendSysMessage("%stype `.vip` for a list of VIP commands.", green.c_str());

		if (HearthStone[guid].guid != guid)
		{
			WorldDatabase.PExecute("REPLACE INTO hearthstone SET `guid`='%u';", guid);

			VIP::SetHearthStone(guid, player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation());
		}

		if (Plvl != VIP_level_cap)
		{
			if (lvl_enable)
			{
				player->SetLevel(VIP_level_cap);
			}
		}
	};
};

class VIP_commands : public CommandScript
{
public:
	VIP_commands() : CommandScript("VIP_commands") { }

	std::vector<ChatCommand> GetCommands() const
	{
		static std::vector<ChatCommand> vipCommandChangeTable =
		{
			{ "race", rbac::RBAC_IN_GRANTED_LIST, true, &HandleChangeRaceCommand, "allows the player to change there race during next login." },
			{ "faction", rbac::RBAC_IN_GRANTED_LIST, true, &HandleChangeFactionCommand, "allows the player to change there faction during next login." },
			{ "custom", rbac::RBAC_IN_GRANTED_LIST, true, &HandleCustomizeCommand, "allows the player to re-costumize there character during next login." },
		};

		static std::vector<ChatCommand> vipCommandSetTable =
		{
			{ "hearthstone", rbac::RBAC_IN_GRANTED_LIST, true, &HandleVipSetHearthstoneCommand, "stores players current gps to VIP hearthstone command." },
		};

		std::string repair_info = "repairs all the players items. Requires the player to possess a " + VIP_COIN_NAME + ".";

		static std::vector<ChatCommand> vipCommandTable =

		{
			{ "mall",		rbac::RBAC_IN_GRANTED_LIST, true, &HandleVipMallCommand, "Teleports the player to a VIP mall." },
			{ "home",		rbac::RBAC_IN_GRANTED_LIST, true, &HandleHomeCommand, "Teleports the player to there faction home mall." },
			{ "repair",		rbac::RBAC_IN_GRANTED_LIST, true, &HandleRepairCommand, repair_info },
			{ "hearthstone",rbac::RBAC_IN_GRANTED_LIST, true, &HandleHearthStoneCommand, "Teleports a player to there custom pre-set location." },
			{ "set",		rbac::RBAC_IN_GRANTED_LIST, true, NULL, "Player customizable commands.", vipCommandSetTable },
			{ "change",		rbac::RBAC_IN_GRANTED_LIST, true, NULL, "Character customizing commands.", vipCommandChangeTable },
		};

		static std::vector<ChatCommand> commandTable =
		{
			{ "vip", rbac::RBAC_IN_GRANTED_LIST, true, NULL, "custom VIP commands by Grumbo. Some commands may require player has an item.", vipCommandTable },
		};
		return commandTable;
	}

	static bool HandleVipMallCommand(ChatHandler* handler, const char* args)
	{

		Player* player = handler->GetSession()->GetPlayer();

		auto team_id = player->GetTeamId();

		if (player->IsInCombat())
		{
			handler->SendSysMessage(LANG_YOU_IN_COMBAT);
			handler->SetSentErrorMessage(true);
			return false;
		}

		// stop flight if need
		if (player->IsInFlight())
		{
			player->GetMotionMaster()->MovementExpired();
			player->CleanupAfterTaxiFlight();
		}
		// save only in non-flight case
		else
			player->SaveRecallPosition();

		player->TeleportTo(MALL[team_id].map_id, MALL[team_id].x, MALL[team_id].y, MALL[team_id].z, MALL[team_id].o);
		return true;
	}

	static bool HandleChangeRaceCommand(ChatHandler* handler, const char* args)
	{
		Player* player = handler->GetSession()->GetPlayer();
		player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
		handler->PSendSysMessage("Relog to change race of your character.");
		return true;
	}

	static bool HandleChangeFactionCommand(ChatHandler* handler, const char* args)
	{
		Player* player = handler->GetSession()->GetPlayer();
		player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
		handler->PSendSysMessage("Relog to change faction of your character.");
		return true;
	}

	static bool HandleCustomizeCommand(ChatHandler* handler, const char* args)
	{
		Player* player = handler->GetSession()->GetPlayer();
		player->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
		handler->PSendSysMessage("Relog to customize your character.");
		return true;
	}

	static bool HandleHomeCommand(ChatHandler* handler, const char* args)
	{

		Player* player = handler->GetSession()->GetPlayer();

		auto team_id = player->GetTeamId();

		if (player->IsInCombat())
		{
			handler->SendSysMessage(LANG_YOU_IN_COMBAT);
			handler->SetSentErrorMessage(true);
			return false;
		}

		// stop flight if need
		if (player->IsInFlight())
		{
			player->GetMotionMaster()->MovementExpired();
			player->CleanupAfterTaxiFlight();
		}
		// save only in non-flight case
		else
			player->SaveRecallPosition();

		player->TeleportTo(HOME[team_id].map_id, HOME[team_id].x, HOME[team_id].y, HOME[team_id].z, HOME[team_id].o);
		return true;
	}

	static bool HandleRepairCommand(ChatHandler* handler, const char* args)
	{
		Player* player = handler->GetSession()->GetPlayer();

		if (!player->HasItemCount(VIP_COIN_ID, 1, false))
		{
			handler->PSendSysMessage("You must have a %s to use this command.", VIP_COIN_NAME.c_str());
			return false;
		}

		if (player->HasItemCount(VIP_COIN_ID, 1, false))
		{
			player->DurabilityRepairAll(0, 0, false);
			handler->PSendSysMessage("Done.");
			return true;
		}
		return true;
	}

	static bool HandleVipSetHearthstoneCommand(ChatHandler* handler, const char* args)
	{

		Player* player = handler->GetSession()->GetPlayer();

		auto team_id = player->GetTeamId();

		if (player->IsInCombat())
		{
			handler->SendSysMessage(LANG_YOU_IN_COMBAT);
			handler->SetSentErrorMessage(true);
			return false;
		}

		// stop flight if need
		if (player->IsInFlight())
		{
			player->GetMotionMaster()->MovementExpired();
			player->CleanupAfterTaxiFlight();
		}
		// save only in non-flight case
		else
			player->SaveRecallPosition();

		VIP::SetHearthStone(player->GetGUID(), player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation());
		handler->PSendSysMessage("%s, your location has been stored.", player->GetName().c_str());
		return true;
	}

	static bool HandleHearthStoneCommand(ChatHandler* handler, const char* args)
	{

		Player* player = handler->GetSession()->GetPlayer();

		uint32 guid = player->GetGUID();

		if (player->IsInCombat())
		{
			handler->SendSysMessage(LANG_YOU_IN_COMBAT);
			handler->SetSentErrorMessage(true);
			return false;
		}

		// stop flight if need
		if (player->IsInFlight())
		{
			player->GetMotionMaster()->MovementExpired();
			player->CleanupAfterTaxiFlight();
		}

		if (HearthStone[guid].guid != guid)
		{
			WorldDatabase.PExecute("REPLACE INTO hearthstone SET `guid`='%u';", guid);

			VIP::SetHearthStone(guid, player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation());

			handler->PSendSysMessage("You must store a location first to be able to use this command.");
			handler->PSendSysMessage("Default location is your current location.");
			return false;
		}

		if (HearthStone[guid].guid == guid)
		{
			// save only in non-flight case and a location is stored
			player->SaveRecallPosition();

			player->TeleportTo(HearthStone[guid].map_id, HearthStone[guid].x, HearthStone[guid].y, HearthStone[guid].z, HearthStone[guid].o);
			return true;
		}
		return true;
	}
};

class VIP_Coin_Script : public ItemScript
{
public: VIP_Coin_Script() : ItemScript("VIP_Coin_Script"){ };


		virtual bool OnUse(Player* player, Item* item, SpellCastTargets const& targets)
		{
			uint32 acct_id = player->GetSession()->GetAccountId();
			uint8 pVip = VIP::GetVIP(acct_id);
			uint32 pMg = VIP::GetMG(acct_id);
			uint32 pVotes = VIP::GetVOTES(acct_id);
			std::string Votes_Required_Ann;
			bool voting = VIP::GetVIPVOTE_ENABLE();

			ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************", green.c_str());
			ChatHandler(player->GetSession()).PSendSysMessage("%sYou are VIP:%s%u%s of %s%u.", green.c_str(), white.c_str(), pVip, green.c_str(), white.c_str(), VIP_MAX);
			ChatHandler(player->GetSession()).PSendSysMessage("%sYou have %s%u %smg's", green.c_str(), white.c_str(), pMg, green.c_str());

			if (pVotes <= 10){ ChatHandler(player->GetSession()).PSendSysMessage("%sYou have Voted %s%u%s time's.", green.c_str(), white.c_str(), pVotes, green.c_str()); };
			if (pVotes > 10){ ChatHandler(player->GetSession()).PSendSysMessage("%sThank you for voting %s%u%s time's.", green.c_str(), white.c_str(), pVotes, green.c_str()); };

			ChatHandler(player->GetSession()).PSendSysMessage("%sYou recieve a %s%u%s %sstat increase.", green.c_str(), white.c_str(), uint8(VIP_OFFSET * 100)*pVip, "%", green.c_str());

			if (voting)
			{
				if (pVip < VIP_MAX)
				{
					uint32 Votes_Required = ((pVip + 1) * VIP_VOTE_COUNT) - pVotes;

					ChatHandler(player->GetSession()).PSendSysMessage("%sYou need %s%u%s more votes to reach the next VIP rank:%s%u%s.", green.c_str(), white.c_str(), Votes_Required, green.c_str(), white.c_str(), (pVip + 1), green.c_str());

					ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************", green.c_str());

					return true;
				}
			}
			ChatHandler(player->GetSession()).PSendSysMessage("%s**********************************", green.c_str());
			return true;
		}
};

void RemoveItem(uint32 id, Player* player)
{
	player->DestroyItemCount(uint32(id), 1, true);

	ChatHandler(player->GetSession()).PSendSysMessage("%s+1 VIP.", green.c_str());
};

class VIP_Stone_Script : public ItemScript
{
public: VIP_Stone_Script() : ItemScript("VIP_Stone_Script"){ };


		virtual bool OnUse(Player* player, Item* item, SpellCastTargets const& targets)
		{
			uint32 acct_id = player->GetSession()->GetAccountId();
			uint8 pVip = VIP::GetVIP(acct_id);
			uint32 pMg = VIP::GetMG(acct_id);
			uint32 pVotes = VIP::GetVOTES(acct_id);

			if (pVip >= VIP_MAX)
			{
				ChatHandler(player->GetSession()).PSendSysMessage("%sYou are allready the maximum VIP rank:%s%u.", red.c_str(), white.c_str(), VIP_MAX);

			}

			if (pVip < VIP_MAX)
			{
				VIP::SetVOTES(acct_id, pVotes + VIP_VOTE_COUNT); // must be first for the dead mans check

				VIP::SetVIP(acct_id, pVip + 1);

				RemoveItem(VIP_COIN_ID, player);

				return true;
			}
			return true;
		}
};

class VIP_MG_BANKER : public CreatureScript
{
public: VIP_MG_BANKER() : CreatureScript("VIP_MG_BANKER"){ }

		bool OnGossipHello(Player* player, Creature* creature)
		{

			uint32 accountId = player->GetSession()->GetAccountId();
			uint8 pVIP = VIP::GetVIP(accountId);
			uint32 MG = VIP::GetMG(accountId);
			uint32 itemId = VIP::GetVIPMGID();
			uint32 pMg = player->GetItemCount(itemId);
			uint32 pVotes = VIP::GetVOTES(accountId);
			std::string itemName = sObjectMgr->GetItemTemplate(itemId)->Name1;
			std::string currency_inBank;
			std::string deposit_amt;

			if (pMg == 1){ deposit_amt = "Total:" + ConvertNumberToString(pMg) + " " + itemName; };
			if (pMg == 0 || pMg > 1){ deposit_amt = "Total:" + ConvertNumberToString(pMg) + " " + itemName + "'s"; };

			std::string withdraw10 = "Withdraw 10 " + itemName + "'s. Fee:0 " + itemName + "'s.";
			std::string withdraw100 = "Withdraw 100 " + itemName + "'s. Fee:1 " + itemName + ".";
			std::string withdraw1000 = "Withdraw 1,000 " + itemName + "'s. Fee:10 " + itemName + "'s.";
			std::string withdraw10000 = "Withdraw 10,000 " + itemName + "'s. Fee:100 " + itemName + "'s.";
			std::string withdraw100000 = "Withdraw 100,000 " + itemName + "'s. Fee:1,000 " + itemName + "'s.";

			if (MG == 1)
				currency_inBank = "Balance:" + ConvertNumberToString(MG) + " " + itemName;
			else
			{
				currency_inBank = "Balance:" + ConvertNumberToString(MG) + " " + itemName + "'s.";
			};

			std::string current_VOTES = "Votes:" + ConvertNumberToString(pVotes);
			std::string current_VIP = "VIP:" + ConvertNumberToString(pVIP);

			if (pMg > 0)
			{
				player->ADD_GOSSIP_ITEM(10, "-----------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
				player->ADD_GOSSIP_ITEM(10, "-Deposit-", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
				player->ADD_GOSSIP_ITEM(10, "-----------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
				player->ADD_GOSSIP_ITEM(10, "Deposit all my custom currency.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2001);
				player->ADD_GOSSIP_ITEM(10, deposit_amt.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2001);
				player->ADD_GOSSIP_ITEM(10, "-----------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
			}

			if (MG >= 10)
			{
				player->ADD_GOSSIP_ITEM(10, "-----------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
				player->ADD_GOSSIP_ITEM(10, "-WithDrawl-", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
				player->ADD_GOSSIP_ITEM(10, "-----------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
				player->ADD_GOSSIP_ITEM(10, withdraw10.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2002);

				if (MG >= 101){ player->ADD_GOSSIP_ITEM(10, withdraw100.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2003); };
				if (MG >= 1010){ player->ADD_GOSSIP_ITEM(10, withdraw1000.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2004); };
				if (MG >= 10100){ player->ADD_GOSSIP_ITEM(10, withdraw10000.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2005); };
				if (MG >= 101000){ player->ADD_GOSSIP_ITEM(10, withdraw100000.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2006); };
			}

			player->ADD_GOSSIP_ITEM(10, "-----------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
			player->ADD_GOSSIP_ITEM(10, "-----------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
			player->ADD_GOSSIP_ITEM(10, "-Bank Balance-", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
			player->ADD_GOSSIP_ITEM(10, currency_inBank.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
			player->ADD_GOSSIP_ITEM(10, current_VOTES.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
			player->ADD_GOSSIP_ITEM(10, current_VIP.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);
			player->ADD_GOSSIP_ITEM(10, "-----------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2000);

			player->SEND_GOSSIP_MENU(1, creature->GetGUID());

			return true;
		};

		bool OnGossipSelect(Player* player, Creature* creature, uint32 /* sender */, uint32 actions)
		{
			TC_LOG_INFO("server.loading", "MG_BANKER::OnSelect :%u", actions);

			uint32 accountId = player->GetSession()->GetAccountId();
			uint8 pVIP = VIP::GetVIP(accountId);
			uint32 MG = VIP::GetMG(accountId);
			uint32 itemId = VIP::GetVIPMGID();
			uint32 pMg = player->GetItemCount(itemId);
			uint32 pVotes = VIP::GetVOTES(accountId);

			switch (actions)
			{
			case GOSSIP_ACTION_INFO_DEF + 2000: // loopbacks

				player->PlayerTalkClass->ClearMenus();
				player->CLOSE_GOSSIP_MENU();

				OnGossipHello(player, creature);
				break;

			case GOSSIP_ACTION_INFO_DEF + 2001: // Deposit all

				player->DestroyItemCount(itemId, pMg, true);

				if (player->GetItemCount(itemId) == 0)
				{
					VIP::SetMG(accountId, MG + pMg);

				};

				player->PlayerTalkClass->ClearMenus();
				player->CLOSE_GOSSIP_MENU();

				OnGossipHello(player, creature);
				break;

			case GOSSIP_ACTION_INFO_DEF + 2002: // Withdraw 10

				player->PlayerTalkClass->ClearMenus();

				if (player->AddItem(itemId, 10))
				{
					VIP::SetMG(accountId, MG - 10);
					player->CLOSE_GOSSIP_MENU();
				}
				break;

			case GOSSIP_ACTION_INFO_DEF + 2003: // Withdraw 100

				player->PlayerTalkClass->ClearMenus();

				if (player->AddItem(itemId, 100))
				{
					VIP::SetMG(accountId, MG - 101);
					player->CLOSE_GOSSIP_MENU();
				}

				OnGossipHello(player, creature);
				break;

			case GOSSIP_ACTION_INFO_DEF + 2004: // Withdraw 1,000
				player->PlayerTalkClass->ClearMenus();

				if (player->AddItem(itemId, 1000))
				{
					VIP::SetMG(accountId, MG - 1010);
					player->CLOSE_GOSSIP_MENU();
				}

				OnGossipHello(player, creature);
				break;

			case GOSSIP_ACTION_INFO_DEF + 2005: // Withdraw 10,000
				player->PlayerTalkClass->ClearMenus();

				if (player->AddItem(itemId, 10000))
				{
					VIP::SetMG(accountId, MG - 10100);
					player->CLOSE_GOSSIP_MENU();
				}

				OnGossipHello(player, creature);
				break;

			case GOSSIP_ACTION_INFO_DEF + 2006: // Withdraw 100,000
				player->PlayerTalkClass->ClearMenus();

				if (player->AddItem(itemId, 100000))
				{
					VIP::SetMG(accountId, MG - 101000);
					player->CLOSE_GOSSIP_MENU();
				}

				OnGossipHello(player, creature);
				break;
			}
			return true;
		};
};

void AddSC_Grumboz_VIP_Core()
{
	new VIP_Load_Conf;
	new Grumboz_VIP_Account_Engine;
	new Grumboz_VIP_Player_Engine;
	new VIP_commands;
	new VIP_Coin_Script;
	new VIP_Stone_Script;
	new VIP_MG_BANKER;
}
