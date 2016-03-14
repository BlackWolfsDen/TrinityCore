
#ifndef GRUMBOZ_VIP_CORE_H
#define GRUMBOZ_VIP_CORE_H

struct VipElements
{
	uint8 vip;
	uint32 mg;
	uint32 votes;
};

struct ItemVIP
{
	uint32 item_id;
	uint8 vip;
};

struct VipMallGPS
{
	uint32 map_id;
	float x;
	float y;
	float z;
	float o;
};

struct VipHomeGPS
{
	uint32 map_id;
	float x;
	float y;
	float z;
	float o;
};

struct VipHearthStoneGPS
{
	uint32 guid;
	uint32 map_id;
	float x;
	float y;
	float z;
	float o;
};

class VIP
{

public:
	VIP();
	~VIP();

	// Getterz
	static uint8 GetVIPMAX();
	static bool GetVIPVOTE_ENABLE();
	static uint32 GetVIPVOTECOUNT();
	static uint32 GetVIPCOINID();
	static uint32 GetVIPSTONEID();
	static uint32 GetVIPMGID();
	static float GetVIPOFFSET();
	static uint8 GetTALENTBONUS();
	static bool GetLEVELBONUS_ENABLE();
	static uint8 GetLEVELBONUS();

	static uint8 GetItemVIP(uint32 item_id);
	static uint8 GetVIP(uint32 acct_id);
	static uint32 GetMG(uint32 acct_id);
	static uint32 GetVOTES(uint32 acct_id);

	// Setterz
	static void SetVIP(uint32 acct_id, uint8 pvip);
	static void SetMG(uint32 acct_id, uint32 pmg);
	static void SetVOTES(uint32 acct_id, uint32 pvotes);
	static void SetHearthStone(uint32 guid, uint32 map_id, float x, float y, float z, float o);
	static void SetItemVIP(uint32 item_id, uint8 item_vip);

private:
	// tools
	void RemoveItem(uint32 id, Player* player);
};

#endif // GRUMBOZ_GUILD_WARZ_H_INCLUDED
