#ifndef UI_H
#define UI_H

#include "window.h"

class CFwdData;

// TODO: Fix some small memory leaks

class CUI
{
public:
	CUI(uint width, uint height, uint splitSID, bool splitOn);
	~CUI();

	// Forward data to UI
	void forward(CFwdData *data);

	// Get current server ID
	uint currentServerID();

	// Get current game ID
	uint currentGameID();

	// Force quit
	void forceQuit();

	// Resize UI
	void resize(uint width, uint height);

	// Update UI
	bool update();

	// Set split server ID
	void setSplitServerID(int id);

	void setSplit(bool enabled);

private:
	// Add server to UI
	CWidget* addServer(const string &name, int id);

	// Add game to UI
	CWidget* addGame(const string &name, int id);

	// Update game in UI
	void updateGame(const string &name, int id);

	// Remove game from UI
	void removeGame(int id);

	// Get widget from IDs. Example: "0,0,1,0,0"
	CWidget *getWidget(const string &IDs);

	// Print general message
	void printToGeneral(const string &message, int flag, int id);

	// Print realm message
	void printToServer(const string &message, int flag, int id);

	// Print raw message
	void printToRaw(const string &message);

	// Print game message
	void printToGame(const string &message, int flag, int id);

	// Get raw flag
	int rawFlag(const string &message);

	// Set channel name
	void setChannelName(const string &name, int id);

	// Add/Update channel user
	void updateChannelUser(const string &name, int flag, int id);

	// Remove channel user
	void removeChannelUser(const string &name, int id);

	// Remove channel users
	void removeChannelUsers(int id);

	// Add friend
	void addFriend(const string &name, int flag, int id);

	// Remove friends
	void removeFriends(int id);

	// Add clan member
	void addClanMember(const string &name, int flag, int id);

	// Remove clan
	void removeClan(int id);

	// Add ban
	void addBan(const vector<string> &row, int id);

	// Remove ban
	void removeBan(const string &name, int id);

	// Remove bans
	void removeBans(int id);

	// Add admin
	void addAdmin(const string &name, int flag, int id);

	// Remove admin
	void removeAdmin(const string &name, int id);

	// Remove admins
	void removeAdmins(int id);

	// Add player
	void addPlayer(const vector<string> &row, int id);

	// Update player
	void updatePlayer(const vector<string> &row, int id);

	// Remove player
	void removePlayer(const string &name, int id);

	// Add stats
	void addStats(const vector<string> &row, int id);

	// Remove stats
	void removeStats(const string &name, int id);

	// Add dota db
	void addDotaDB(const vector<string> &row, int id);

	// Remove dota db
	void removeDotaDB(const string &name, int id);

	// Add game info
	void addGameInfo(const vector<string> &row, int id);

	// Update game info
	void updateGameInfo(const vector<string> &row, int id);

	// Splits Games-tab
	void split();

	CWindow *window;
	CTabWidget *mainWidget;

	CTabWidget *games;

	vector<string> _replyTargets;

	bool _forceQuit;

	bool _resize;
	CSize _newSize;

	bool _splitOn;
	uint _splitSID;
};

#endif

