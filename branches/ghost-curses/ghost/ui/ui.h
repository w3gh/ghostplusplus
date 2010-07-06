#ifndef UI_H
#define UI_H

#include "window.h"

class CListWidget;
class CFwdData;

typedef pair<int, CWidget *> PairedWidget; // serverID, widget

class CUI
{
public:
	CUI(uint width, uint height, uint splitSID, bool splitOn, bool gameinfotab, bool debugOn);
	~CUI();
	
	void forward(CFwdData *data);										// Forward data to UI
	uint currentServerID();												// Get current server ID
	uint currentGameID();												// Get current game ID
	void forceQuit();													// Force quit
	void resize(uint width, uint height);								// Resize UI
	bool update();														// Update UI
	void setWindowTitle(const string &text);							// Set window title

	void debug(const string &text);

	bool debugOn() { return _debug; }

private:
	void addServer(const string &name, int id);							// Add server to UI
	void addGame(const string &name, int id);							// Add game to UI
	void updateGame(const string &name, int id);						// Update game in UI
	void removeGame(int id);											// Remove game from UI
	void printToGeneral(const string &message, int flag, int id);		// Print general message
	void printToServer(const string &message, int flag, int id);		// Print realm message
	void printToRaw(const string &message);								// Print raw message
	void printToGame(const string &message, int flag, int id);			// Print game message
	int rawFlag(const string &message);									// Get raw flag
	void setChannelName(const string &name, int id);					// Set channel name
	void updateChannelUser(const string &name, int flag, int id);		// Add/Update channel user
	void removeChannelUser(const string &name, int id);					// Remove channel user
	void removeChannelUsers(int id);									// Remove channel users
	void addFriend(const string &name, int flag, int id);				// Add friend
	void removeFriends(int id);											// Remove friends
	void addClanMember(const string &name, int flag, int id);			// Add clan member
	void removeClan(int id);											// Remove clan
	void addBan(const vector<string> &row, int id);						// Add ban
	void removeBan(const string &name, int id);							// Remove ban
	void removeBans(int id);											// Remove bans
	void addAdmin(const string &name, int flag, int id);				// Add admin
	void removeAdmin(const string &name, int id);						// Remove admin
	void removeAdmins(int id);											// Remove admins
	void addPlayer(const vector<string> &row, int id);					// Add player
	void updatePlayer(const vector<string> &row, int id);				// Update player
	void removePlayer(const string &name, int id);						// Remove player
	void addStats(const vector<string> &row, int id);					// Add stats
	void removeStats(const string &name, int id);						// Remove stats
	void addDotaDB(const vector<string> &row, int id);					// Add dota db
	void removeDotaDB(const string &name, int id);						// Remove dota db
	void addGameInfo(const vector<string> &row, int id);				// Add game info
	void updateGameInfo(const vector<string> &row, int id);				// Update game info
	void split();														// Splits Games-tab

	CWindow *_window;
	CTabWidget *_mainWidget;
	CListWidget *_debugWidget;
	CTabWidget *_games;

	vector<PairedWidget> _edit;
	vector<PairedWidget> _allLog;
	vector<PairedWidget> _serverLog;
	vector<PairedWidget> _bans;
	vector<PairedWidget> _channelName;
	vector<PairedWidget> _channel;
	vector<PairedWidget> _friends;
	vector<PairedWidget> _clan;
	vector<PairedWidget> _admins;

	vector<PairedWidget> _players;
	vector<PairedWidget> _stats;
	vector<PairedWidget> _dotadb;
	vector<PairedWidget> _gamechat;
	vector<PairedWidget> _gameinfo;

	vector<CWidget *> _scrollablewidgets;

	vector<CTabWidget *> _tabwidgets;
	CTabWidget *_selectedTabWidget;

	vector<string> _replyTargets;

	bool _forceQuit;

	bool _resize;
	CSize _newSize;

	bool _splitOn;
	uint _splitSID;

	bool _gameinfotab;
	bool _debug;
	int _debugCounter;
};

#endif

