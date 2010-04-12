
#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "includes.h"

#ifdef WIN32
 #include "curses.h"
#else
 #include <curses.h> // NOTE: Change this to xcurses.h if necessary
#endif

#define A_WHITE		( COLOR_PAIR(0) )
#define A_GREEN		( COLOR_PAIR(1) )
#define A_RED		( COLOR_PAIR(2) )
#define A_CYAN		( COLOR_PAIR(3) )
#define A_YELLOW	( COLOR_PAIR(4) )
#define A_BLUE		( COLOR_PAIR(5) )

#define A_BWHITE	( A_BOLD | COLOR_PAIR(0) )
#define A_BGREEN	( A_BOLD | COLOR_PAIR(1) )
#define A_BRED		( A_BOLD | COLOR_PAIR(2) )
#define A_BCYAN		( A_BOLD | COLOR_PAIR(3) )
#define A_BYELLOW	( A_BOLD | COLOR_PAIR(4) )
#define A_BBLUE		( A_BOLD | COLOR_PAIR(5) )

//
// Buffer
//

typedef vector<pair<string, int> > Buffer;
// buffers except channel, friends and clan get automatically integer value of 0

enum BufferType
{
	B_ALL = 0,
	B_MAIN,
	B_FRIENDS,
	B_CLAN,
	B_REALM,
	B_CHANNEL = 15,
	B_INPUT,
	B_TAB,
	B_GAME = 18
};

// All-Buffer:          m_Buffers[B_ALL]
// Main-Buffer:         m_Buffers[B_MAIN]
// Realm-Buffer:        m_Buffers[B_REALM] = m_RealmData[m_RealmId].Messages
// Channel-Buffer:      m_Buffers[B_CHANNEL] = m_RealmData[m_RealmId].ChannelUsers
// Friends-Buffer:      m_Buffers[B_FRIENDS] = m_RealmData[m_RealmId].Friends
// Clan-Buffer:         m_Buffers[B_CLAN] = m_RealmData[m_RealmId].Clan

// Normally max username length is 15 characters, but it can be 48 characters in some cases.

// Highlight currently selected tab

struct SRealmData
{
	string RealmAlias;
	string ChannelName;
	Buffer ChannelUsers;
	Buffer Messages;
	Buffer Friends;
	Buffer Clan;
};

enum WindowType
{
	W_TAB = 0,
	W_FULL,
	W_FULL2,
	W_UPPER,
	W_LOWER,
	W_CHANNEL,
	W_INPUT
};

struct SWindowData
{
	WINDOW *Window;
	string title;
	bool IsWindowChanged;
};

enum TabType
{
	T_MAIN = 0,
	T_LIST,
	T_REALM,
	T_GAME
};

struct STabData
{
	string name;
	TabType type;

	// realmId or gameId
	uint32_t id;

	BufferType bufferType;

	bool IsTabSelected;
};

//
// CCurses
//

class CGHost;

class CCurses
{
private:
	// Pointer to CGHost
	CGHost *m_GHost;

	// Buffers
	vector<Buffer *> m_Buffers;

	// RealmData
	vector<SRealmData> m_RealmData;

	// Windows
	vector<SWindowData> m_WindowData;

	// Tabs
	vector<STabData> m_TabData;

	// Input
	string m_InputBuffer;

	// Realm
	uint32_t m_RealmId;
	uint32_t m_RealmId2;	// for ghost commands and IsConnected

	uint32_t m_ListUpdateTimer;

	// Drawing
	void SetAttribute( SWindowData &data, string message, int something, BufferType type, bool on );

	void Draw( );
	void DrawTabs( );
	void DrawWindow( SWindowData &data, BufferType type );
	void DrawListWindow( SWindowData &data, BufferType type );
	void Resize( );
	
	// Tab
	void AddTab( string nName, TabType nType, uint32_t nId, BufferType nBufferType );
	void RemoveTab( TabType type, uint32_t id );
	void SelectTab( uint32_t n );

	// Window
	void ShowWindow( WindowType type );
	void HideWindow( WindowType type );
	void UpdateWindows( );

	void CompileFriends( );
	void CompileClan( );

	// Misc
	uint32_t GetRealmId( string &realmAlias );
	uint32_t GetNextRealm( uint32_t realmId );
	bool IsConnected( uint32_t realmId, bool entry );
	uint32_t GetMessageFlag( string &message );

	uint32_t m_SelectedTab;
	uint32_t m_SelectedInput;

public:
	CCurses( int nTermWidth, int nTermHeight );
	~CCurses( );

	void SetGHost( CGHost * nGHost );
	void Print( string message, uint32_t realmId, bool toMainBuffer );
	bool Update( );

	// Channel / Custom list
	void ChangeChannel( string channel, uint32_t realmId );
	void AddChannelUser( string name, uint32_t realmId, int flag );
	void UpdateChannelUser( string name, uint32_t realmId, int flag );
	void RemoveChannelUser( string name, uint32_t realmId );
	void RemoveChannelUsers( uint32_t realmId );
	void UpdateCustomLists( uint32_t realmId );

};

#endif