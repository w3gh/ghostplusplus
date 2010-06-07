#include "window.h"
#include "listwidget.h"
#include "layout.h"

//fix: layout resizing (s-f-s, f-s-f, ...)

CTabWidget *mainWidget = 0;

string toString(int i)
{
	char *buf = new char[sizeof(int)];
	_itoa(i, buf, 10);

	return buf;
}

CWidget* addServer(const string &name, int id)
{
	CWidget *srvmain = new CWidget();
	srvmain->setName(name);
	srvmain->setCustomID(id);

	CWidget *sub = new CWidget();
	sub->setCustomID(0);

	CWidget *channel = new CWidget();
	channel->setName("Channel");
	channel->setCustomID(0);

	CListWidget *log1 = new CListWidget();
	log1->setName("All");
	log1->setCustomID(0);
	log1->setBackgroundColor(White);
	log1->setForegroundColor(Black);

	CListWidget *log2 = new CListWidget();
	log2->setName("Server");
	log2->setCustomID(1);
	log2->setBackgroundColor(White);
	log2->setForegroundColor(Black);

	CLabel *channelName = new CLabel();
	channelName->setCustomID(0);
	channelName->setBackgroundColor(White);
	channelName->setForegroundColor(Black);
	channelName->setFixedSize(0, 3);	

	CTextEdit *edit = new CTextEdit();
	edit->setCustomID(1);
	edit->setBackgroundColor(Cyan);
	edit->setFixedSize(0, 2);

	CListWidget *channelList = new CListWidget();
	channelList->setCustomID(1);
	channelList->setBackgroundColor(White);
	channelList->setForegroundColor(Black);

	CListWidget *friendsList = new CListWidget();
	friendsList->setName("Friends");
	friendsList->setCustomID(1);
	friendsList->setBackgroundColor(White);
	friendsList->setForegroundColor(Black);

	CListWidget *clanList = new CListWidget();
	clanList->setName("Clan");
	clanList->setCustomID(1);
	clanList->setBackgroundColor(White);
	clanList->setForegroundColor(Black);

	CLayout *layout0a = new CVBoxLayout(channel);
	layout0a->addWidget(channelName);
	layout0a->addWidget(channelList);
	
	CTabWidget *tab1 = new CTabWidget();
	tab1->setCustomID(0);
	tab1->setBackgroundColor(White);
	tab1->setForegroundColor(Blue);
	tab1->addTab(log1);
	tab1->addTab(log2);

	CTabWidget *tab2 = new CTabWidget();
	tab2->setCustomID(1);
	tab2->setBackgroundColor(White);
	tab2->setForegroundColor(Blue);
	tab2->setFixedSize(22, 0);
	tab2->addTab(channel);
	tab2->addTab(friendsList);
	tab2->addTab(clanList);
	
	CLayout *layout1 = new CHBoxLayout(sub);
	layout1->addWidget(tab1);
	layout1->addWidget(tab2);

	CLayout *layout2 = new CVBoxLayout(srvmain);
	layout2->addWidget(sub);
	layout2->addWidget(edit);

	return srvmain;
}

// Get widget from IDs. Example: "0,0,1,0,0".
CWidget *getWidget(const string &IDs)
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

	layout = mainWidget->at(mainWidget->indexOf(_IDs[0]))->layout();

	for(uint i = 1; i < _IDs.size() - 1; i++)
		layout = layout->at(layout->indexOf(_IDs[i]))->layout();

	return layout->at(layout->indexOf(_IDs[_IDs.size() - 1]));
}

void setChannelName(const string &name, int id)
{
	CLabel *channelName = (CLabel *)getWidget(toString(id) + ",0,1,0,0");
	channelName->setText(name);
}

void addChannelUser(const string &name, int flag, int id)
{
	Color color;

	switch(flag)
	{
	case 1: color = Blue;	break;	// BLIZZARD REP
	case 2: color = Yellow;	break;	// CHANNEL OP
	case 4: color = Cyan;	break;	// SPEAKER
	case 8: color = Blue;	break;	// BNET ADMIN
	case 32: color = Red;	break;	// SQUELCHED
	default: color = Null;
	}

	CListWidget *channelList = (CListWidget *)getWidget(toString(id) + ",0,1,0,1");
	channelList->addItem(name, color);
}

int main()
{
	CWindow *window = new CWindow();
	window->setTitle("CursesMod-2.0");

	CWidget *sub1a = addServer("Europe", 0);
	CWidget *sub1b = addServer("USWest", 1);
	CWidget *sub1c = addServer("USEast", 2);
	CWidget *sub1d = addServer("Asia", 3);
	CWidget *sub1e = addServer("PVPGN", 4);

	mainWidget = new CTabWidget();
	mainWidget->setBackgroundColor(Cyan);
	mainWidget->setFixedSize(0, 1);

	mainWidget->addTab(sub1a);
	mainWidget->addTab(sub1b);
	mainWidget->addTab(sub1c);
	mainWidget->addTab(sub1d);
	mainWidget->addTab(sub1e);

	window->setWidget(mainWidget);
	window->show();

	window->setSize(135, 43);

	setChannelName("The Void", 0);
	addChannelUser("Testuser", 0, 0);

	CTextEdit *edit = (CTextEdit *)getWidget("0,1");

	string replyTarget = "Testuser";

	while(true)
	{
		window->update();
		
		if(edit->text() == "/r ")
			edit->setText("/w " + replyTarget + " ");
	}

	SafeDelete(window);

	return 0;
}