#include "ui.h"
#include "window.h"
#include "listwidget.h"
#include "tablewidget.h"
#include "layout.h"
#include "forward.h"

CUI::CUI(uint width, uint height, uint splitSID, bool splitOn, bool gameinfotab)
{
	_window = new CWindow();
	_window->setTitle("GHost++ 17.1 CursesMod-2.0");

	_mainWidget = new CTabWidget("", 0, White, Cyan);
	_games = new CTabWidget("", -2, White, Cyan);

	CListWidget *log = new CListWidget("Log", -3, White, Black);
	log->listenKeys(true);

	CWidget *splitWidget = new CWidget("Games", -1);
	splitWidget->setLayout(new CVBoxLayout(splitWidget));

	_mainWidget->addTab(log);
	_mainWidget->addTab(splitWidget);

	_window->setWidget(_mainWidget);
	_window->show();

#ifdef WIN32
	resize(width, height);
#endif

	_forceQuit = false;
	_splitOn = splitOn;
	_splitSID = splitSID;
	_gameinfotab = gameinfotab;

	split();

	// Initialize replytargets
	for(uint i = 0; i < 20; i++)
		_replyTargets.push_back("");

	_scrollablewidgets.push_back(log);

	_tabwidgets.push_back(_mainWidget);
	_tabwidgets.push_back(_games);

	_selectedTabWidget = _mainWidget;
	_selectedTabWidget->listenKeys(true);
}

CUI::~CUI()
{
	SafeDelete(_window);
}

void CUI::forceQuit()
{
	_forceQuit = true;
}

void CUI::resize(uint width, uint height)
{
	_resize = true;
	_newSize.set(width, height);
}

bool CUI::update()
{
	if(_resize)
	{
		_window->setSize(_newSize.width(), _newSize.height());
		_resize = false;
	}

	_window->update();

	if(_window->key() == KEY_RESIZE)
		resize(COLS, LINES);

	if(_window->key() == 9 || !_selectedTabWidget->visible()) // TAB
	{
		_selectedTabWidget->listenKeys(false);
		bool next = false;
		for(vector<CTabWidget *>::iterator i = _tabwidgets.begin();; i++)
		{
			if(i == _tabwidgets.end())
				i = _tabwidgets.begin();

			if(next)
			{
				if(((*i)->visible() && (*i)->count() > 1) || *i == _selectedTabWidget)
				{
					_selectedTabWidget = *i;
					break;
				}
			}
			else if(*i == _selectedTabWidget)
				next = true;
		}
		_selectedTabWidget->listenKeys(true);
	}
	else if(_window->key() == KEY_BTAB) // SHIFT-TAB
	{
		_selectedTabWidget->listenKeys(false);
		bool next = false;
		for(vector<CTabWidget *>::iterator i = _tabwidgets.end() - 1;; i--)
		{
			if(next)
			{
				if(((*i)->visible() && (*i)->count() > 1) || *i == _selectedTabWidget)
				{
					_selectedTabWidget = *i;
					break;
				}
			}
			else if(*i == _selectedTabWidget)
				next = true;

			if(i == _tabwidgets.begin())
				i = _tabwidgets.end() - 1;
		}
		_selectedTabWidget->listenKeys(true);
	}

	if(_window->key() == KEY_LEFT || _window->key() == KEY_RIGHT)
	{
		for(vector<CWidget *>::iterator i = _scrollablewidgets.begin(); i != _scrollablewidgets.end(); i++)
			(*i)->listenKeys((*i)->visible());
	}

	if(currentServerID() >= 0)
	{
		for(vector<PairedWidget>::iterator i = _edit.begin(); i != _edit.end(); i++)
		{
			if((*i).first == currentServerID())
			{
				CTextEdit *edit = static_cast<CTextEdit *>((*i).second);

				if(edit->text() == "/r ")
				{
					if(_replyTargets[(*i).first].empty())
						edit->setText("/w ");
					else
						edit->setText("/w " + _replyTargets[(*i).first] + " ");
				}
				break;
			}
		}
	}

	return _forceQuit;
}

void CUI::addServer(const string &name, int id)
{
	CWidget *server = new CWidget(name, id);
	CWidget *sub = new CWidget;
	CWidget *channel = new CWidget("Channel", 0);
	CWidget *friends = new CWidget("Friends", 1);
	CWidget *clan = new CWidget("Clan", 2);
	CWidget *admins = new CWidget("Admins", 3);
	admins->forwardOnSelect(new CFwdData(FWD_OUT_ADMINS, id));

	CListWidget *log1 = new CListWidget("All", 0, White, Black);
	_allLog.push_back(PairedWidget(id, log1));
	_scrollablewidgets.push_back(log1);

	CListWidget *log2 = new CListWidget("Server", 1, White, Black);
	_serverLog.push_back(PairedWidget(id, log2));
	_scrollablewidgets.push_back(log2);

	vector<PairedColumnHeader> headers;
	headers.push_back(PairedColumnHeader("Name", 15));
	headers.push_back(PairedColumnHeader("IP", 15));
	headers.push_back(PairedColumnHeader("Date", 15));
	headers.push_back(PairedColumnHeader("GameName", 20));
	headers.push_back(PairedColumnHeader("Admin", 15));
	headers.push_back(PairedColumnHeader("Reason", 25));

	CTableWidget *bans = new CTableWidget("Bans", 2, White, Black);
	bans->setColumnHeaders(headers);
	bans->setAutoScroll(false);
	bans->forwardOnSelect(new CFwdData(FWD_OUT_BANS, id));
	_bans.push_back(PairedWidget(id, bans));
	_scrollablewidgets.push_back(bans);

	CLabel *channelName = new CLabel("", 0, White, Black);
	channelName->setText("Channel");
	channelName->setFixedSize(0, 3);
	_channelName.push_back(PairedWidget(id, channelName));

	CLabel *friendsLabel = new CLabel("", 0, White, Black);
	friendsLabel->setText("Friends");
	friendsLabel->setFixedSize(0, 3);

	CLabel *clanName = new CLabel("", 0, White, Black);
	clanName->setText("Clan");
	clanName->setFixedSize(0, 3);

	CLabel *adminsLabel = new CLabel("", 0, White, Black);
	adminsLabel->setText("Admins");
	adminsLabel->setFixedSize(0, 3);

	CTextEdit *edit = new CTextEdit("", 1, White, Cyan);
	edit->setFixedSize(0, 2);
	edit->setForwardTypeOnEnter(FWD_OUT_MESSAGE);
	_edit.push_back(PairedWidget(id, edit));

	CListWidget *channelList = new CListWidget("", 1, White, Black);
	channelList->setAutoScroll(false);
	_channel.push_back(PairedWidget(id, channelList));

	CListWidget *friendsList = new CListWidget("", 1, White, Black);
	friendsList->setAutoScroll(false);
	_friends.push_back(PairedWidget(id, friendsList));

	CListWidget *clanList = new CListWidget("", 1, White, Black);
	clanList->setAutoScroll(false);
	_clan.push_back(PairedWidget(id, clanList));

	CListWidget *adminsList = new CListWidget("", 1, White, Black);
	adminsList->setAutoScroll(false);
	_admins.push_back(PairedWidget(id, adminsList));
	
	CLayout *layout0a = new CVBoxLayout(channel);
	layout0a->addWidget(channelName);
	layout0a->addWidget(channelList);

	CLayout *layout0b = new CVBoxLayout(friends);
	layout0b->addWidget(friendsLabel);
	layout0b->addWidget(friendsList);

	CLayout *layout0c = new CVBoxLayout(clan);
	layout0c->addWidget(clanName);
	layout0c->addWidget(clanList);

	CLayout *layout0d = new CVBoxLayout(admins);
	layout0d->addWidget(adminsLabel);
	layout0d->addWidget(adminsList);
	
	CTabWidget *tab1 = new CTabWidget("", 0, White, Cyan);
	tab1->addTab(log1);
	tab1->addTab(log2);
	tab1->addTab(bans);

	CTabWidget *tab2 = new CTabWidget("", 1, White, Cyan);
	tab2->setFixedSize(29, 0);
	tab2->addTab(channel);
	tab2->addTab(friends);
	tab2->addTab(clan);
	tab2->addTab(admins);
	
	CLayout *layout1 = new CHBoxLayout(sub);
	layout1->addWidget(tab1);
	layout1->addWidget(tab2);

	CLayout *layout2 = new CVBoxLayout(server);
	layout2->addWidget(sub);
	layout2->addWidget(edit);

	_mainWidget->addTab(server);

	_tabwidgets.push_back(tab1);
	_tabwidgets.push_back(tab2);
}

void CUI::addGame(const string &name, int id)
{
	CWidget *game = new CWidget(name, id);
	CWidget *sub1 = new CWidget;
	CWidget *sub2 = new CWidget;
	CWidget *sub3 = new CWidget;
	CListWidget *chat = new CListWidget("", 0, White, Black);
	_gamechat.push_back(PairedWidget(id, chat));

	vector<PairedColumnHeader> headers;
	headers.push_back(PairedColumnHeader("", 10));
	headers.push_back(PairedColumnHeader("", 30));

	CTableWidget *info = new CTableWidget("Info", 1, White, Black);
	info->setFixedSize(40, 0);
	info->setColumnHeaders(headers);
	_gameinfo.push_back(PairedWidget(id, info));

	headers.clear();
	headers.push_back(PairedColumnHeader("Name", 15));
	headers.push_back(PairedColumnHeader("Slot", 5));
	headers.push_back(PairedColumnHeader("From", 5));
	headers.push_back(PairedColumnHeader("Ping", 8));
	headers.push_back(PairedColumnHeader("Race", 5));
	headers.push_back(PairedColumnHeader("Team", 5));
	headers.push_back(PairedColumnHeader("Color", 10));
	headers.push_back(PairedColumnHeader("Handicap", 9));
	headers.push_back(PairedColumnHeader("GProxy++", 9));

	CTableWidget *players = new CTableWidget("Players", 0, White, Black);
	players->setColumnHeaders(headers);
	players->sortByColumn(1);
	_players.push_back(PairedWidget(id, players));

	headers.clear();
	headers.push_back(PairedColumnHeader("Name", 15));
	headers.push_back(PairedColumnHeader("Total Games", 12));
	headers.push_back(PairedColumnHeader("Last Gamedate", 20));
	headers.push_back(PairedColumnHeader("AVG Stay%", 10));
	headers.push_back(PairedColumnHeader("AVG Duration", 13));
	headers.push_back(PairedColumnHeader("AVG Loading time", 20));

	CTableWidget *stats = new CTableWidget("Stats", 1, White, Black);
	stats->setColumnHeaders(headers);
	_stats.push_back(PairedWidget(id, stats));

	headers.clear();
	headers.push_back(PairedColumnHeader("Name", 15));
	headers.push_back(PairedColumnHeader("Wins/Losses", 15));
	headers.push_back(PairedColumnHeader("K/D/A", 20));
	headers.push_back(PairedColumnHeader("AVG K/D/A", 15));
	headers.push_back(PairedColumnHeader("CS K/D/N", 20));
	headers.push_back(PairedColumnHeader("AVG CS K/D/N", 15));
	headers.push_back(PairedColumnHeader("T/R/C", 10));

	CTableWidget *dotadb = new CTableWidget("DotA/DB", 2, White, Black);
	dotadb->setColumnHeaders(headers);
	_dotadb.push_back(PairedWidget(id, dotadb));

	/*
	headers.clear();
	headers.push_back(PairedColumnHeader("Name", 15));
	headers.push_back(PairedColumnHeader("Hero", 15));
	headers.push_back(PairedColumnHeader("Kills", 10));
	headers.push_back(PairedColumnHeader("Deaths", 10));
	headers.push_back(PairedColumnHeader("Assists", 10));
	headers.push_back(PairedColumnHeader("CS K/D/N", 10));
	headers.push_back(PairedColumnHeader("T/R/C", 10));

	CTableWidget *dotart = new CTableWidget("DotA/RT", 3, White, Black);
	dotart->setColumnHeaders(headers);
	_dotart.push_back(PairedWidget(id, dotart));
	*/

	CTabWidget *tabs = new CTabWidget("", id, White, Cyan);
	tabs->addTab(players);
	tabs->addTab(stats);
	tabs->addTab(dotadb);
	//tabs->addTab(dotart);
	if(_gameinfotab) tabs->addTab(info);

	CTextEdit *edit = new CTextEdit("", 1, White, Cyan);
	edit->setFixedSize(0, 2);
	edit->setForwardTypeOnEnter(FWD_OUT_GAME);

	CSeparatorWidget *separator = new CSeparatorWidget;
	separator->setFixedSize(0, 1);

	CLayout *layout0 = new CVBoxLayout(game);
	layout0->addWidget(sub1);
	layout0->addWidget(edit);

	CLayout *layout1 = new CVBoxLayout(sub1);
	layout1->addWidget(tabs);
	layout1->addWidget(sub2);

	// Because of the bug in layout.cpp recursive resizing functions, we have to make another layout.
	// Otherwise we would just have done layout1->addWidget(separator) between tabs and sub2 (now sub3).
	CLayout *layout2 = new CVBoxLayout(sub2);
	layout2->addWidget(separator);
	layout2->addWidget(sub3);

	CLayout *layout3 = new CHBoxLayout(sub3);
	layout3->addWidget(chat);
	if(!_gameinfotab) layout3->addWidget(info);

	_games->addTab(game);

	_tabwidgets.push_back(tabs);
}

void CUI::updateGame(const string &name, int id)
{
	CWidget *widget = _games->at(_games->indexOf(id));

	widget->setName(name);
}

void CUI::removeGame(int id)
{
	for(vector<CTabWidget *>::iterator i = _tabwidgets.begin(); i != _tabwidgets.end(); i++)
	{
		if((*i)->customID() == id)
		{
			_tabwidgets.erase(i);
			break;
		}
	}

	for(vector<PairedWidget>::iterator i = _players.begin(); i != _players.end(); i++)
	{
		if((*i).first == id)
		{
			_players.erase(i);
			break;
		}
	}

	for(vector<PairedWidget>::iterator i = _stats.begin(); i != _stats.end(); i++)
	{
		if((*i).first == id)
		{
			_stats.erase(i);
			break;
		}
	}

	for(vector<PairedWidget>::iterator i = _dotadb.begin(); i != _dotadb.end(); i++)
	{
		if((*i).first == id)
		{
			_dotadb.erase(i);
			break;
		}
	}

	for(vector<PairedWidget>::iterator i = _gamechat.begin(); i != _gamechat.end(); i++)
	{
		if((*i).first == id)
		{
			_gamechat.erase(i);
			break;
		}
	}

	for(vector<PairedWidget>::iterator i = _gameinfo.begin(); i != _gameinfo.end(); i++)
	{
		if((*i).first == id)
		{
			_gameinfo.erase(i);
			break;
		}
	}

	_games->removeTab(_games->at(_games->indexOf(id)));
}

void CUI::printToGeneral(const string &message, int flag, int id)
{
	Color color = Null;

	switch(flag)
	{
	case 1: color = Cyan;    break; // INFO, EMOTE
	case 2: color = Cyan;    break; // GHOST
	case 3: color = Yellow;  break; // 
	case 4: color = Yellow;  break; // BNET, BNLSC
	case 5: color = Green;   break; // WHISPER
	case 6: color = Red;     break; // ERROR
	}

	for(vector<PairedWidget>::iterator i = _allLog.begin(); i != _allLog.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CListWidget *>((*i).second)->addItem(message, color);
			return;
		}
	}
}

void CUI::printToServer(const string &message, int flag, int id)
{
	Color color = Null;

	switch(flag)
	{
	case 1: color = Cyan;    break; // INFO, EMOTE
	case 2: color = Cyan;    break; // GHOST
	case 3: color = Yellow;  break; // 
	case 4: color = Yellow;  break; // BNET, BNLSC
	case 5: color = Green;   break; // WHISPER
	case 6: color = Red;     break; // ERROR
	}

	for(vector<PairedWidget>::iterator i = _serverLog.begin(); i != _serverLog.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CListWidget *>((*i).second)->addItem(message, color);
			return;
		}
	}
}

void CUI::printToRaw(const string &message)
{
	Color color = Null;
	int flag = rawFlag(message);

	switch(flag)
	{
	case 1: color = Cyan;    break; // INFO, EMOTE
	case 2: color = Cyan;    break; // GHOST
	case 3: color = Yellow;  break; // 
	case 4: color = Yellow;  break; // BNET, BNLSC
	case 5: color = Green;   break; // WHISPER
	case 6: color = Red;     break; // ERROR
	}
	
	static_cast<CListWidget *>(_mainWidget->at(_mainWidget->indexOf(-3)))->addItem(message, color);
}

void CUI::printToGame(const string &message, int flag, int id)
{
	Color color = Null;

	switch(flag)
	{
	case 1: color = Cyan;    break; //
	}

	for(vector<PairedWidget>::const_iterator i = _gamechat.begin(); i != _gamechat.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CListWidget *>((*i).second)->addItem(message, color);
			return;
		}
	}
}

int CUI::rawFlag(const string &message)
{
	if ( message.size( ) > 4 && message[0] == '[' )
	{
		if ( message.compare(1, 4, "INFO") == 0 )			return 1;
		else if ( message.compare(1, 4, "BNET") == 0 )		return 4;
		else if ( message.compare(1, 5, "GHOST") == 0 )		return 2;
		else if ( message.compare(1, 5, "BNLSC") == 0 )		return 4;
		else if ( message.compare(1, 5, "ERROR") == 0 )		return 6;
		else if ( message.compare(1, 5, "EMOTE") == 0 )		return 1;
		else if ( message.compare(1, 7, "WHISPER") == 0 )	return 5;
	}
	return 0;
}

void CUI::setChannelName(const string &name, int id)
{
	for(vector<PairedWidget>::const_iterator i = _channelName.begin(); i != _channelName.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CLabel *>((*i).second)->setText(name);
			return;
		}
	}
}

void CUI::updateChannelUser(const string &name, int flag, int id)
{
	Color color = Null;

	switch(flag)
	{
	case 1: color = Blue;	break;	// BLIZZARD REP
	case 2: color = Yellow;	break;	// CHANNEL OP
	case 4: color = Cyan;	break;	// SPEAKER
	case 8: color = Blue;	break;	// BNET ADMIN
	case 32: color = Red;	break;	// SQUELCHED
	}

	for(vector<PairedWidget>::const_iterator i = _channel.begin(); i != _channel.end(); i++)
	{
		if((*i).first == id)
		{
			CListWidget *channelList = static_cast<CListWidget *>((*i).second);

			if(channelList->indexOf(name) == -1)
				channelList->addItem(name, color);
			else
				channelList->updateItem(name, color);

			return;
		}
	}
}

void CUI::removeChannelUser(const string &name, int id)
{
	for(vector<PairedWidget>::const_iterator i = _channel.begin(); i != _channel.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CListWidget *>((*i).second)->removeItem(name);
			return;
		}
	}
}

void CUI::removeChannelUsers(int id)
{
	for(vector<PairedWidget>::const_iterator i = _channel.begin(); i != _channel.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CListWidget *>((*i).second)->removeItems();
			return;
		}
	}
}

void CUI::addFriend(const string &name, int flag, int id)
{
	Color color = Null;

	switch(flag)
	{
	case 0: color = Red;     break; // OFFLINE
	case 1: color = Yellow;  break;	// NOT IN CHAT
	case 2: color = Cyan;    break;	// IN CHAT
	case 3: color = Green;   break;	// IN PUBLIC GAME
	case 4: color = Blue;    break;	// IN PRIVATE GAME; NOT MUTUAL
	case 5: color = Magenta; break;	// IN PRIVATE GAME; MUTUAL
	}

	for(vector<PairedWidget>::const_iterator i = _friends.begin(); i != _friends.end(); i++)
	{
		if((*i).first == id)
		{
			CListWidget *friends = static_cast<CListWidget *>((*i).second);

			if(friends->indexOf(name) == -1)
				friends->addItem(name, color);
			else
				friends->updateItem(name, color);

			return;
		}
	}
}

void CUI::removeFriends(int id)
{
	for(vector<PairedWidget>::const_iterator i = _friends.begin(); i != _friends.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CListWidget *>((*i).second)->removeItems();
			return;
		}
	}
}

void CUI::addClanMember(const string &name, int flag, int id)
{
	Color color = Null;

	switch(flag)
	{
	case 0: color = Yellow;  break; // INITIATE
	case 1: color = Yellow;  break;	// PARTIAL MEMBER
	case 2: color = Green;   break;	// MEMBER
	case 3: color = Blue;    break;	// OFFICER
	case 4: color = Magenta; break;	// LEADER
	}

	for(vector<PairedWidget>::const_iterator i = _clan.begin(); i != _clan.end(); i++)
	{
		if((*i).first == id)
		{
			CListWidget *clan = static_cast<CListWidget *>((*i).second);

			if(clan->indexOf(name) == -1)
				clan->addItem(name, color);
			else
				clan->updateItem(name, color);

			return;
		}
	}
}
void CUI::removeClan(int id)
{
	for(vector<PairedWidget>::const_iterator i = _clan.begin(); i != _clan.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CListWidget *>((*i).second)->removeItems();
			return;
		}
	}
}

void CUI::addBan(const vector<string> &row, int id)
{
	for(vector<PairedWidget>::const_iterator i = _bans.begin(); i != _bans.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->addRow(row);
			return;
		}
	}
}

void CUI::removeBan(const string &name, int id)
{
	for(vector<PairedWidget>::const_iterator i = _bans.begin(); i != _bans.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->removeRow(name);
			return;
		}
	}
}

void CUI::removeBans(int id)
{
	for(vector<PairedWidget>::const_iterator i = _bans.begin(); i != _bans.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->removeRows();
			return;
		}
	}
}

void CUI::addAdmin(const string &name, int flag, int id)
{
	Color color = Null;

	switch(flag)
	{
	case 0: color = White;     break; // NORMAL
	case 1: color = Yellow;    break; // ROOT
	}

	for(vector<PairedWidget>::const_iterator i = _admins.begin(); i != _admins.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CListWidget *>((*i).second)->addItem(name, color);
			return;
		}
	}
}

void CUI::removeAdmin(const string &name, int id)
{
	for(vector<PairedWidget>::const_iterator i = _admins.begin(); i != _admins.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CListWidget *>((*i).second)->removeItem(name, White);
			return;
		}
	}
}

void CUI::removeAdmins(int id)
{
	for(vector<PairedWidget>::const_iterator i = _admins.begin(); i != _admins.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CListWidget *>((*i).second)->removeItems();
			return;
		}
	}
}

void CUI::addPlayer(const vector<string> &row, int id)
{
	for(vector<PairedWidget>::const_iterator i = _players.begin(); i != _players.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->addRow(row);
			return;
		}
	}
}

void CUI::updatePlayer(const vector<string> &row, int id)
{
	for(vector<PairedWidget>::const_iterator i = _players.begin(); i != _players.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->updateRow(row);
			return;
		}
	}
}

void CUI::removePlayer(const string &name, int id)
{
	for(vector<PairedWidget>::const_iterator i = _players.begin(); i != _players.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->removeRow(name);
			return;
		}
	}
}

void CUI::addStats(const vector<string> &row, int id)
{
	for(vector<PairedWidget>::const_iterator i = _stats.begin(); i != _stats.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->addRow(row);
			return;
		}
	}
}

void CUI::removeStats(const string &name, int id)
{
	for(vector<PairedWidget>::const_iterator i = _stats.begin(); i != _stats.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->removeRow(name);
			return;
		}
	}
}

void CUI::addDotaDB(const vector<string> &row, int id)
{
	for(vector<PairedWidget>::const_iterator i = _dotadb.begin(); i != _dotadb.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->addRow(row);
			return;
		}
	}
}

void CUI::removeDotaDB(const string &name, int id)
{
	for(vector<PairedWidget>::const_iterator i = _dotadb.begin(); i != _dotadb.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->removeRow(name);
			return;
		}
	}
}

void CUI::addGameInfo(const vector<string> &row, int id)
{
	for(vector<PairedWidget>::const_iterator i = _gameinfo.begin(); i != _gameinfo.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->addRow(row);
			return;
		}
	}
}

void CUI::updateGameInfo(const vector<string> &row, int id)
{
	for(vector<PairedWidget>::const_iterator i = _gameinfo.begin(); i != _gameinfo.end(); i++)
	{
		if((*i).first == id)
		{
			static_cast<CTableWidget *>((*i).second)->updateRow(row);
			return;
		}
	}
}

void CUI::forward(CFwdData *data)
{
	switch(data->_type)
	{
	case FWD_GENERAL:
		for(uint i = 0; i < _mainWidget->count(); i++)
		{
			int cID = _mainWidget->at(i)->customID();
			if(cID >= 0 && cID < 50) // only servers
				printToGeneral(data->_text, data->_flag, cID);
		}
		break;
	case FWD_REALM:
		printToGeneral(data->_text, data->_flag, data->_id);
		printToServer(data->_text, data->_flag, data->_id);
		break;
	case FWD_RAW:
		printToRaw(data->_text);
		break;
	case FWD_SERVER_ADD:
		addServer(data->_text, data->_id);
		split();
		break;
	case FWD_CHANNEL_CHANGE:
		setChannelName(data->_text, data->_id);
		break;
	case FWD_CHANNEL_ADD:
	case FWD_CHANNEL_UPDATE:
		updateChannelUser(data->_text, data->_flag, data->_id);
		break;
	case FWD_CHANNEL_REMOVE:
		removeChannelUser(data->_text, data->_id);
		break;
	case FWD_CHANNEL_CLEAR:
		removeChannelUsers(data->_id);
		break;
	case FWD_FRIENDS_ADD:
		addFriend(data->_text, data->_flag, data->_id);
		break;
	case FWD_FRIENDS_CLEAR:
		removeFriends(data->_id);
		break;
	case FWD_CLAN_ADD:
		addClanMember(data->_text, data->_flag, data->_id);
		break;
	case FWD_CLAN_CLEAR:
		removeClan(data->_id);
		break;
	case FWD_BANS_ADD:
		addBan(data->_data, data->_id);
		break;
	case FWD_BANS_REMOVE:
		removeBan(data->_text, data->_id);
		break;
	case FWD_BANS_CLEAR:
		removeBans(data->_id);
		break;
	case FWD_ADMINS_ADD:
		addAdmin(data->_text, data->_flag, data->_id);
		break;
	case FWD_ADMINS_REMOVE:
		removeAdmin(data->_text, data->_id);
		break;
	case FWD_ADMINS_CLEAR:
		removeAdmins(data->_id);
		break;
	case FWD_REPLYTARGET:
		_replyTargets[data->_id] = data->_text;
		break;
	case FWD_GAME_ADD:
		addGame(data->_text, data->_id);
		_mainWidget->update(31); // Refresh
		break;
	case FWD_GAME_UPDATE:
		updateGame(data->_text, data->_id);
		break;
	case FWD_GAME_REMOVE:
		removeGame(data->_id);
		break;
	case FWD_GAME_SLOT_ADD:
		addPlayer(data->_data, data->_id);
		break;
	case FWD_GAME_SLOT_UPDATE:
		updatePlayer(data->_data, data->_id);
		break;
	case FWD_GAME_SLOT_REMOVE:
		removePlayer(data->_text, data->_id);
		break;
	case FWD_GAME_STATS_ADD:
		addStats(data->_data, data->_id);
		break;
	case FWD_GAME_STATS_REMOVE:
		removeStats(data->_text, data->_id);
		break;
	case FWD_GAME_DOTA_DB_ADD:
		addDotaDB(data->_data, data->_id);
		break;
	case FWD_GAME_DOTA_DB_REMOVE:
		removeDotaDB(data->_text, data->_id);
		break;
	case FWD_GAME_CHAT:
		printToGame(data->_text, data->_flag, data->_id);
		break;
	case FWD_GAME_MAP_INFO_ADD:
		addGameInfo(data->_data, data->_id);
		break;
	case FWD_GAME_MAP_INFO_UPDATE:
		updateGameInfo(data->_data, data->_id);
		break;
	}

	delete data;
}

uint CUI::currentServerID()
{
	if(_mainWidget->at(_mainWidget->currentIndex())->customID() == -1)
		return _splitSID;
	else
		return _mainWidget->currentIndex() >= 0 ? _mainWidget->at(_mainWidget->currentIndex())->customID() : 0;
}

uint CUI::currentGameID()
{
	return _games->currentIndex() >= 0 ? _games->at(_games->currentIndex())->customID() : 0;
}

void CUI::split()
{
	CLayout *splitLayout = _mainWidget->at(_mainWidget->indexOf(-1))->layout();

	if(splitLayout->count() == 2)
		splitLayout->removeWidget(splitLayout->at(1));

	if(splitLayout->count() == 1)
		splitLayout->removeWidget(splitLayout->at(0));
	
	if(_splitOn && _mainWidget->indexOf(_splitSID) != -1)
		splitLayout->addWidget(_mainWidget->at(_mainWidget->indexOf(_splitSID)));

	splitLayout->addWidget(_games);
}
