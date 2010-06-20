#include "ui.h"
#include "window.h"
#include "listwidget.h"
#include "tablewidget.h"
#include "layout.h"
#include "forward.h"

string toString(int i)
{
	char *buf = new char[4];
	_itoa(i, buf, 10);

	return buf;
}

CUI::CUI(uint width, uint height, uint splitSID, bool splitOn)
{
	window = new CWindow();
	window->setTitle("GHost++ 17.1 CursesMod-2.0");

	mainWidget = new CTabWidget();
	mainWidget->setBackgroundColor(Cyan);
	mainWidget->setForegroundColor(White);
	mainWidget->setFixedSize(0, 1);

	CListWidget *log0 = new CListWidget();
	log0->setName("Raw");
	log0->setCustomID(-3);
	log0->setBackgroundColor(Black);
	log0->setForegroundColor(White);
	mainWidget->addTab(log0);

	games = new CTabWidget();
	games->setName("Games");
	games->setCustomID(-2);
	games->setBackgroundColor(Cyan);
	games->setForegroundColor(White);
	//mainWidget->addTab(games);

	CWidget *splitWidget = new CWidget();
	splitWidget->setName("Games");
	splitWidget->setCustomID(-1);
	splitWidget->setLayout(new CVBoxLayout(splitWidget));
	mainWidget->addTab(splitWidget);

	window->setWidget(mainWidget);
	window->show();

	resize(width, height);

	_forceQuit = false;
	_splitOn = splitOn;
	_splitSID = splitSID;

	split();

	// Initialize replytargets
	for(uint i = 0; i < 20; i++)
		_replyTargets.push_back("");
}

CUI::~CUI()
{
	SafeDelete(window);
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
		window->setSize(_newSize.width(), _newSize.height());
		_resize = false;
	}

	window->update();

	if(currentServerID() >= 0)
	{
		CTextEdit *edit = (CTextEdit *)getWidget(currentServerID() + ",1");

		if(edit)
		{
			if(edit->text() == "/r ")
				edit->setText("/w " + _replyTargets[currentServerID()] + " ");
		}
	}

	return _forceQuit;
}

CWidget* CUI::addServer(const string &name, int id)
{
	CWidget *srvmain = new CWidget();
	srvmain->setName(name);
	srvmain->setCustomID(id);

	CWidget *sub = new CWidget();
	sub->setCustomID(0);

	CWidget *channel = new CWidget();
	channel->setName("Channel");
	channel->setCustomID(0);

	CWidget *friends = new CWidget();
	friends->setName("Friends");
	friends->setCustomID(1);

	CWidget *clan = new CWidget();
	clan->setName("Clan");
	clan->setCustomID(2);

	CListWidget *log1 = new CListWidget();
	log1->setName("All");
	log1->setCustomID(0);
	log1->setBackgroundColor(Black);
	log1->setForegroundColor(White);

	CListWidget *log2 = new CListWidget();
	log2->setName("Server");
	log2->setCustomID(1);
	log2->setBackgroundColor(Black);
	log2->setForegroundColor(White);

	CListWidget *admins = new CListWidget();
	admins->setName("Admins");
	admins->setCustomID(2);
	admins->setBackgroundColor(Black);
	admins->setForegroundColor(White);
	admins->setAutoScroll(false);
	admins->forwardOnSelect(new CFwdData(FWD_OUT_ADMINS, id));

	vector<string> bansHeader;			vector<uint> bansWidths;
	bansHeader.push_back("Name");		bansWidths.push_back(15);
	bansHeader.push_back("IP");			bansWidths.push_back(15);
	bansHeader.push_back("Date");		bansWidths.push_back(15);
	bansHeader.push_back("GameName");	bansWidths.push_back(20);
	bansHeader.push_back("Admin");		bansWidths.push_back(15);
	bansHeader.push_back("Reason");		bansWidths.push_back(25);

	CTableWidget *bans = new CTableWidget();
	bans->setName("Bans");
	bans->setCustomID(3);
	bans->setBackgroundColor(Black);
	bans->setForegroundColor(White);
	bans->setColumnHeaders(bansHeader);
	bans->setColumnWidths(bansWidths);
	bans->setAutoScroll(false);
	bans->forwardOnSelect(new CFwdData(FWD_OUT_BANS, id));

	CLabel *channelName = new CLabel();
	channelName->setCustomID(0);
	channelName->setText("Channel");
	channelName->setBackgroundColor(Black);
	channelName->setForegroundColor(White);
	channelName->setFixedSize(0, 3);

	CLabel *friendsLabel = new CLabel();
	friendsLabel->setCustomID(0);
	friendsLabel->setText("Friends");
	friendsLabel->setBackgroundColor(Black);
	friendsLabel->setForegroundColor(White);
	friendsLabel->setFixedSize(0, 3);

	CLabel *clanName = new CLabel();
	clanName->setCustomID(0);
	clanName->setText("Clan");
	clanName->setBackgroundColor(Black);
	clanName->setForegroundColor(White);
	clanName->setFixedSize(0, 3);

	CTextEdit *edit = new CTextEdit(srvmain);
	edit->setCustomID(1);
	edit->setBackgroundColor(Cyan);
	edit->setFixedSize(0, 2);
	edit->setForwardTypeOnEnter(FWD_OUT_MESSAGE);

	CListWidget *channelList = new CListWidget();
	channelList->setCustomID(1);
	channelList->setBackgroundColor(Black);
	channelList->setForegroundColor(White);
	channelList->setAutoScroll(false);

	CListWidget *friendsList = new CListWidget();
	friendsList->setCustomID(1);
	friendsList->setBackgroundColor(Black);
	friendsList->setForegroundColor(White);
	friendsList->setAutoScroll(false);

	CListWidget *clanList = new CListWidget();
	clanList->setCustomID(1);
	clanList->setBackgroundColor(Black);
	clanList->setForegroundColor(White);
	clanList->setAutoScroll(false);

	CLayout *layout0a = new CVBoxLayout(channel);
	layout0a->addWidget(channelName);
	layout0a->addWidget(channelList);

	CLayout *layout0b = new CVBoxLayout(friends);
	layout0b->addWidget(friendsLabel);
	layout0b->addWidget(friendsList);

	CLayout *layout0c = new CVBoxLayout(clan);
	layout0c->addWidget(clanName);
	layout0c->addWidget(clanList);
	
	CTabWidget *tab1 = new CTabWidget();
	tab1->setCustomID(0);
	tab1->setBackgroundColor(Cyan);
	tab1->setForegroundColor(White);
	tab1->addTab(log1);
	tab1->addTab(log2);
	tab1->addTab(admins);
	tab1->addTab(bans);

	CTabWidget *tab2 = new CTabWidget();
	tab2->setCustomID(1);
	tab2->setBackgroundColor(Cyan);
	tab2->setForegroundColor(White);
	tab2->setFixedSize(22, 0);
	tab2->addTab(channel);
	tab2->addTab(friends);
	tab2->addTab(clan);
	
	CLayout *layout1 = new CHBoxLayout(sub);
	layout1->addWidget(tab1);
	layout1->addWidget(tab2);

	CLayout *layout2 = new CVBoxLayout(srvmain);
	layout2->addWidget(sub);
	layout2->addWidget(edit);

	return srvmain;
}

CWidget* CUI::addGame(const string &name, int id)
{
	CWidget *game = new CWidget();
	game->setName(name);
	game->setCustomID(id);

	CWidget *sub1 = new CWidget();
	sub1->setCustomID(0);

	CWidget *sub2 = new CWidget();
	sub2->setCustomID(0);

	CWidget *sub3 = new CWidget();
	sub3->setFixedSize(40, 0);
	sub3->setCustomID(1);

	CListWidget *chat = new CListWidget();
	chat->setCustomID(1);
	chat->setBackgroundColor(Black);
	chat->setForegroundColor(White);

	vector<string> mapHeaders;
	mapHeaders.push_back("");
	mapHeaders.push_back("");

	vector<uint> mapWidths;
	mapWidths.push_back(10);
	mapWidths.push_back(30);

	CTableWidget *map = new CTableWidget();
	map->setCustomID(0);
	map->setBackgroundColor(Black);
	map->setForegroundColor(White);
	map->setColumnHeaders(mapHeaders);
	map->setColumnWidths(mapWidths);

	vector<string> slotHeaders;
	slotHeaders.push_back("Name");
	slotHeaders.push_back("Slot");
	slotHeaders.push_back("From");
	slotHeaders.push_back("Ping");
	slotHeaders.push_back("Race");
	slotHeaders.push_back("Team");
	slotHeaders.push_back("Color");
	slotHeaders.push_back("Handicap");
	slotHeaders.push_back("GProxy++");

	vector<uint> slotWidths;
	slotWidths.push_back(15);
	slotWidths.push_back(5);
	slotWidths.push_back(5);
	slotWidths.push_back(8);
	slotWidths.push_back(5);
	slotWidths.push_back(5);
	slotWidths.push_back(10);
	slotWidths.push_back(9);
	slotWidths.push_back(9);

	CTableWidget *slots = new CTableWidget();
	slots->setName("Slots");
	slots->setCustomID(0);
	slots->setBackgroundColor(Black);
	slots->setForegroundColor(White);
	slots->setColumnHeaders(slotHeaders);
	slots->setColumnWidths(slotWidths);
	slots->sortByColumn(1);

	vector<string> statHeaders;
	statHeaders.push_back("Name");
	statHeaders.push_back("Total Games");
	statHeaders.push_back("Last Gamedate");
	statHeaders.push_back("AVG Stay%");
	statHeaders.push_back("AVG Duration");
	statHeaders.push_back("AVG Loading time");

	vector<uint> statWidths;
	statWidths.push_back(15);
	statWidths.push_back(12);
	statWidths.push_back(20);
	statWidths.push_back(10);
	statWidths.push_back(13);
	statWidths.push_back(20);

	CTableWidget *stats = new CTableWidget();
	stats->setName("Stats");
	stats->setCustomID(1);
	stats->setBackgroundColor(Black);
	stats->setForegroundColor(White);
	stats->setColumnHeaders(statHeaders);
	stats->setColumnWidths(statWidths);

	vector<string> dotaHeaders;
	dotaHeaders.push_back("Name");
	dotaHeaders.push_back("Wins/Losses");
	dotaHeaders.push_back("K/D/A");
	dotaHeaders.push_back("AVG K/D/A");
	dotaHeaders.push_back("CS K/D/N");
	dotaHeaders.push_back("AVG CS K/D/N");
	dotaHeaders.push_back("T/R/C");

	vector<uint> dotaWidths;
	dotaWidths.push_back(15);
	dotaWidths.push_back(12);
	dotaWidths.push_back(8);
	dotaWidths.push_back(12);
	dotaWidths.push_back(15);
	dotaWidths.push_back(15);
	dotaWidths.push_back(10);

	CTableWidget *dota = new CTableWidget();
	dota->setName("DotA/DB");
	dota->setCustomID(2);
	dota->setBackgroundColor(Black);
	dota->setForegroundColor(White);
	dota->setColumnHeaders(dotaHeaders);
	dota->setColumnWidths(dotaWidths);

	/*
	vector<string> dotaCurrentHeaders;
	dotaCurrentHeaders.push_back("Name");
	dotaCurrentHeaders.push_back("Hero");
	dotaCurrentHeaders.push_back("Kills");
	dotaCurrentHeaders.push_back("Deaths");
	dotaCurrentHeaders.push_back("Assists");
	dotaCurrentHeaders.push_back("CS K/D/N");
	dotaCurrentHeaders.push_back("T/R/C");

	vector<uint> dotaCurrentWidths;
	dotaCurrentWidths.push_back(15);
	dotaCurrentWidths.push_back(15);
	dotaCurrentWidths.push_back(10);
	dotaCurrentWidths.push_back(10);
	dotaCurrentWidths.push_back(10);
	dotaCurrentWidths.push_back(10);
	dotaCurrentWidths.push_back(10);
	dotaCurrentWidths.push_back(10);

	CTableWidget *dotaCurrent = new CTableWidget();
	dotaCurrent->setName("DotA/RT");
	dotaCurrent->setCustomID(3);
	dotaCurrent->setBackgroundColor(Black);
	dotaCurrent->setForegroundColor(White);
	dotaCurrent->setColumnHeaders(dotaCurrentHeaders);
	dotaCurrent->setColumnWidths(dotaCurrentWidths);*/

	CTabWidget *tabs = new CTabWidget();
	tabs->setBackgroundColor(Cyan);
	tabs->setForegroundColor(White);
	tabs->addTab(slots);
	tabs->addTab(stats);
	tabs->addTab(dota);
	//tabs->addTab(dotaCurrent);

	CTextEdit *edit = new CTextEdit(game);
	edit->setCustomID(1);
	edit->setBackgroundColor(Cyan);
	edit->setFixedSize(0, 2);
	edit->setForwardTypeOnEnter(FWD_OUT_GAME);

	CLayout *layout0 = new CVBoxLayout(game);
	layout0->addWidget(sub1);
	layout0->addWidget(edit);

	CLayout *layout1 = new CHBoxLayout(sub1);
	layout1->addWidget(sub2);
	layout1->addWidget(sub3);
	
	CLayout *layout2 = new CVBoxLayout(sub2);
	layout2->addWidget(tabs);
	layout2->addWidget(chat);

	CLayout *layout3 = new CVBoxLayout(sub3);
	layout3->addWidget(map);

	return game;
}

void CUI::updateGame(const string &name, int id)
{
	CWidget *widget = games->at(games->indexOf(id));

	widget->setName(name);
}

void CUI::removeGame(int id)
{
	games->removeTab(games->at(games->indexOf(id)));
}

// Get widget from IDs. Example: "0,0,1,0,0".
CWidget* CUI::getWidget(const string &IDs)
{
	CWidget *result = 0;
	if(mainWidget)
	{
		vector<int> _IDs;
		CLayout *layout = 0;

		for(uint i = 0; i < IDs.size(); i++)
		{
			int j = atoi(IDs.substr(i, IDs.find_first_of(",", i) - i).c_str());
			_IDs.push_back(j);

			if(IDs.find_first_of(",", i) != -1)
				i = IDs.find_first_of(",", i);
		}

		if(!_IDs.empty() && mainWidget->indexOf(_IDs[0]) != -1)
		{
			layout = mainWidget->at(mainWidget->indexOf(_IDs[0]))->layout();

			if(layout)
			{
				for(uint i = 1; i < _IDs.size() - 1; i++)
				{
					if(layout)
					{
						if(layout->indexOf(_IDs[i]) != -1)
						{
							if(layout->at(layout->indexOf(_IDs[i]))->layout())
								layout = layout->at(layout->indexOf(_IDs[i]))->layout();
						}
						else
							return 0;
					}
				}

				if(layout)
					result = layout->at(layout->indexOf(_IDs[_IDs.size() - 1]));
			}
		}
	}

	return result;
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

	CTabWidget *tabs = 0;
	CListWidget *log1 = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,0");
	log1 = (CListWidget *)tabs->at(0);

	log1->addItem(message, color);
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

	CTabWidget *tabs = 0;
	CListWidget *log2 = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,0");
	log2 = (CListWidget *)tabs->at(1);

	log2->addItem(message, color);
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
	
	CListWidget *log0 = (CListWidget *)mainWidget->at(mainWidget->indexOf(-3));
	log0->addItem(message, color);
}

void CUI::printToGame(const string &message, int flag, int id)
{
	Color color = Null;

	switch(flag)
	{
	case 1: color = Cyan;    break; //
	}
	
	CListWidget *widget = static_cast<CListWidget *>(games->at(games->indexOf(id))->layout()->at(0)->layout()->at(0)->layout()->at(1));

	widget->addItem(message, color);
}

int CUI::rawFlag(const string &message)
{
	if ( message.size( ) > 4 && message[0] == '[' )
	{
		if ( message.compare(1, 4, "INFO") == 0 )			return 1;
		else if ( message.compare(1, 4, "BNET") == 0 )		return 4;
		else if ( message.compare(1, 5, "GHOST") == 0 )		return 2;
		else if ( message.compare(1, 5, "BNLSC") == 0 )		return 4;
		else if ( message.compare(1, 5, "ERROR") == 0 )		return 3;
		else if ( message.compare(1, 5, "EMOTE") == 0 )		return 1;
		else if ( message.compare(1, 7, "WHISPER") == 0 )	return 5;
	}
	return 0;
}

void CUI::setChannelName(const string &name, int id)
{
	CTabWidget *tabs = 0;
	CLabel *channelName = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,1");
	channelName = (CLabel *)tabs->at(0)->layout()->at(0);

	channelName->setText(name);
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

	CTabWidget *tabs = 0;
	CListWidget *channelList = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,1");
	channelList = (CListWidget *)tabs->at(0)->layout()->at(1);

	if(channelList->indexOf(name) == -1)
		channelList->addItem(name, color);
	else
		channelList->updateItem(name, color);
}

void CUI::removeChannelUser(const string &name, int id)
{
	CTabWidget *tabs = 0;
	CListWidget *channelList = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,1");
	channelList = (CListWidget *)tabs->at(0)->layout()->at(1);

	channelList->removeItem(name);
}

void CUI::removeChannelUsers(int id)
{
	CTabWidget *tabs = 0;
	CListWidget *channelList = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,1");
	channelList = (CListWidget *)tabs->at(0)->layout()->at(1);

	channelList->removeItems();
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

	CTabWidget *tabs = 0;
	CListWidget *friends = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,1");
	friends = (CListWidget *)tabs->at(1)->layout()->at(1);

	if(friends->indexOf(name) == -1)
		friends->addItem(name, color);
}

void CUI::removeFriends(int id)
{
	CTabWidget *tabs = 0;
	CListWidget *friends = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,1");
	friends = (CListWidget *)tabs->at(1)->layout()->at(1);

	friends->removeItems();
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

	CTabWidget *tabs = 0;
	CListWidget *clan = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,1");
	clan = (CListWidget *)tabs->at(2)->layout()->at(1);

	if(clan->indexOf(name) == -1)
		clan->addItem(name, color);
}
void CUI::removeClan(int id)
{
	CTabWidget *tabs = 0;
	CListWidget *clan = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,1");
	clan = (CListWidget *)tabs->at(2)->layout()->at(1);

	clan->removeItems();
}

void CUI::addBan(const vector<string> &row, int id)
{
	CTabWidget *tabs = 0;
	CTableWidget *bans = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,0");
	bans = (CTableWidget *)tabs->at(3);

	bans->addRow(row);
}

void CUI::removeBan(const string &name, int id)
{
	CTabWidget *tabs = 0;
	CTableWidget *bans = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,0");
	bans = (CTableWidget *)tabs->at(3);

	bans->removeRow(name);
}

void CUI::removeBans(int id)
{
	CTabWidget *tabs = 0;
	CTableWidget *bans = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,0");
	bans = (CTableWidget *)tabs->at(3);

	bans->removeRows();
}

void CUI::addAdmin(const string &name, int flag, int id)
{
	Color color = Null;

	switch(flag)
	{
	case 0: color = White;     break; // NORMAL
	case 1: color = Yellow;    break; // ROOT
	}

	CTabWidget *tabs = 0;
	CListWidget *admins = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,0");
	admins = (CListWidget *)tabs->at(2);

	admins->addItem(name, color);
}

void CUI::removeAdmin(const string &name, int id)
{
	CTabWidget *tabs = 0;
	CListWidget *admins = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,0");
	admins = (CListWidget *)tabs->at(2);

	admins->removeItem(name);
}

void CUI::removeAdmins(int id)
{
	CTabWidget *tabs = 0;
	CListWidget *admins = 0;
	tabs = (CTabWidget *)getWidget(toString(id) + ",0,0");
	admins = (CListWidget *)tabs->at(2);

	admins->removeItems();
}

void CUI::addPlayer(const vector<string> &row, int id)
{
	CTabWidget *tabs = static_cast<CTabWidget *>(games->at(games->indexOf(id))->layout()->at(0)->layout()->at(0)->layout()->at(0));

	CTableWidget *slots = static_cast<CTableWidget *>(tabs->at(tabs->indexOf(0)));

	slots->addRow(row);
}

void CUI::updatePlayer(const vector<string> &row, int id)
{
	CTabWidget *tabs = static_cast<CTabWidget *>(games->at(games->indexOf(id))->layout()->at(0)->layout()->at(0)->layout()->at(0));

	CTableWidget *slots = static_cast<CTableWidget *>(tabs->at(tabs->indexOf(0)));

	slots->updateRow(row);
}

void CUI::removePlayer(const string &name, int id)
{
	CTabWidget *tabs = static_cast<CTabWidget *>(games->at(games->indexOf(id))->layout()->at(0)->layout()->at(0)->layout()->at(0));

	CTableWidget *slots = static_cast<CTableWidget *>(tabs->at(tabs->indexOf(0)));

	slots->removeRow(name);
}

void CUI::addStats(const vector<string> &row, int id)
{
	CTabWidget *tabs = static_cast<CTabWidget *>(games->at(games->indexOf(id))->layout()->at(0)->layout()->at(0)->layout()->at(0));

	CTableWidget *stats = static_cast<CTableWidget *>(tabs->at(tabs->indexOf(1)));

	stats->addRow(row);
}

void CUI::removeStats(const string &name, int id)
{
	CTabWidget *tabs = static_cast<CTabWidget *>(games->at(games->indexOf(id))->layout()->at(0)->layout()->at(0)->layout()->at(0));

	CTableWidget *stats = static_cast<CTableWidget *>(tabs->at(tabs->indexOf(1)));

	stats->removeRow(name);
}

void CUI::addDotaDB(const vector<string> &row, int id)
{
	CTabWidget *tabs = static_cast<CTabWidget *>(games->at(games->indexOf(id))->layout()->at(0)->layout()->at(0)->layout()->at(0));

	CTableWidget *stats = static_cast<CTableWidget *>(tabs->at(tabs->indexOf(2)));

	stats->addRow(row);
}

void CUI::removeDotaDB(const string &name, int id)
{
	CTabWidget *tabs = static_cast<CTabWidget *>(games->at(games->indexOf(id))->layout()->at(0)->layout()->at(0)->layout()->at(0));

	CTableWidget *stats = static_cast<CTableWidget *>(tabs->at(tabs->indexOf(2)));

	stats->removeRow(name);
}

void CUI::addGameInfo(const vector<string> &row, int id)
{
	CTableWidget *info = static_cast<CTableWidget *>(games->at(games->indexOf(id))->layout()->at(0)->layout()->at(1)->layout()->at(0));

	info->addRow(row);
}

void CUI::updateGameInfo(const vector<string> &row, int id)
{
	CTableWidget *info = static_cast<CTableWidget *>(games->at(games->indexOf(id))->layout()->at(0)->layout()->at(1)->layout()->at(0));

	info->updateRow(row);
}

void CUI::forward(CFwdData *data)
{
	switch(data->_type)
	{
	case FWD_GENERAL:
		for(uint i = 0; i < mainWidget->count(); i++)
		{
			int cID = mainWidget->at(i)->customID();
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
		mainWidget->addTab(addServer(data->_text, data->_id));
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
		games->addTab(addGame(data->_text, data->_id));
		mainWidget->update(31); // Refresh tab
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
	if(mainWidget->at(mainWidget->currentIndex())->customID() == -1)
		return _splitSID;
	else
		return mainWidget->currentIndex() >= 0 ? mainWidget->at(mainWidget->currentIndex())->customID() : 0;
}

uint CUI::currentGameID()
{
	return games->currentIndex() >= 0 ? games->at(games->currentIndex())->customID() : 0;
}

void CUI::setSplitServerID(int id)
{
	_splitSID = id;
}

void CUI::setSplit(bool enabled)
{
	_splitOn = enabled;

	split();
}

void CUI::split()
{
	CLayout *splitLayout = mainWidget->at(mainWidget->indexOf(-1))->layout();

	if(splitLayout->count() == 2)
		splitLayout->removeWidget(splitLayout->at(1));

	if(splitLayout->count() == 1)
		splitLayout->removeWidget(splitLayout->at(0));
	
	if(_splitOn && mainWidget->indexOf(_splitSID) != -1)
	{
		splitLayout->addWidget(mainWidget->at(mainWidget->indexOf(_splitSID)));
	}

	splitLayout->addWidget(games);
}