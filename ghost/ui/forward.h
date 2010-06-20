#ifndef FORWARD_H
#define FORWARD_H

/*
	We use CFwdData to transfer data between GHost++ and the user interface.
*/

enum FwdType
{
	FWD_GENERAL = 0,
	FWD_REALM,
	FWD_RAW,
	FWD_SERVER_ADD,
	FWD_CHANNEL_CHANGE,
	FWD_CHANNEL_ADD,
	FWD_CHANNEL_UPDATE,
	FWD_CHANNEL_REMOVE,
	FWD_CHANNEL_CLEAR,
	FWD_FRIENDS_ADD,
	FWD_FRIENDS_CLEAR,
	FWD_CLAN_ADD,
	FWD_CLAN_CLEAR,
	FWD_BANS_ADD,
	FWD_BANS_REMOVE,
	FWD_BANS_CLEAR,
	FWD_ADMINS_ADD,
	FWD_ADMINS_REMOVE,
	FWD_ADMINS_CLEAR,
	FWD_REPLYTARGET,
	FWD_GAME_ADD,
	FWD_GAME_UPDATE,
	FWD_GAME_REMOVE,
	FWD_GAME_CHAT,
	FWD_GAME_SLOT_ADD,
	FWD_GAME_SLOT_UPDATE,
	FWD_GAME_SLOT_REMOVE,
	FWD_GAME_STATS_ADD,
	FWD_GAME_STATS_REMOVE,
	FWD_GAME_DOTA_DB_ADD,
	FWD_GAME_DOTA_DB_REMOVE,
	FWD_GAME_DOTA_RT_ADD,
	FWD_GAME_DOTA_RT_UPDATE,
	FWD_GAME_DOTA_RT_REMOVE,
	FWD_GAME_MAP_INFO_ADD,
	FWD_GAME_MAP_INFO_UPDATE,
	FWD_OUT_BANS,
	FWD_OUT_ADMINS,
	FWD_OUT_MESSAGE,
	FWD_OUT_GAME
};

class CFwdData
{
public:
	CFwdData(const FwdType &type, int id)
	{
		_type = type;
		_text = "";
		_flag = 0;
		_id = id;
	}

	CFwdData(const FwdType &type, const string &text, int id)
	{
		_type = type;
		_text = text;
		_flag = 0;
		_id = id;
	}

	CFwdData(const FwdType &type, const string &text, int flag, int id)
	{
		_type = type;
		_text = text;
		_flag = flag;
		_id = id;
	}

	CFwdData(const FwdType &type, vector<string> data, int id)
	{
		_type = type;
		_text = "";
		_flag = 0;
		_data = data;
		_id = id;
	}

	~CFwdData()
	{
	}

	vector<string> _data;

	FwdType _type;
	string _text;
	int _flag;
	int _id;
};


#endif
