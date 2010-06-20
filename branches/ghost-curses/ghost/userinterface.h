
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

#define MAX_BUFFER_SIZE 512
#define SCROLL_VALUE 2

typedef vector<pair<string, int> > Buffer;

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
	B_BANS,
	B_ADMINS,
	B_GAMES,
	B_GAME = 18
};

struct SRealmData
{
	string RealmAlias;
	string ChannelName;
	Buffer ChannelUsers;
	Buffer Messages;
	Buffer Friends;
	Buffer Clan;
	Buffer Bans;
	Buffer Admins;
};

enum WindowType
{
	W_TAB = 0,
	W_TAB2,
	W_FULL,
	W_FULL2,
	W_UPPER,
	W_LOWER,
	W_CHANNEL,
	W_INPUT,
	W_HLINE,
	W_VLINE
};

struct SWindowData
{
	WINDOW *Window;
	string Title;
	bool IsWindowChanged;
	int Scroll;
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
	uint32_t id;	// realmId or gameId
	BufferType bufferType;
	bool IsTabSelected;
	WindowType windowType;
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
	vector<Buffer *> m_Buffers;		// pointers so updating is "automatic" and there is no useless copying

	// RealmData
	vector<SRealmData> m_RealmData;

	// Windows
	vector<SWindowData> m_WindowData;

	// Tabs
	vector<STabData> m_TabData;

	// Input
	string m_InputBuffer;	// current input

	// Realm
	uint32_t m_RealmId;		// currently selected realm
	uint32_t m_RealmId2;	// for ghost commands and IsConnected

	// Drawing
	void SetAttribute( SWindowData &data, string message, int flag, BufferType type, bool on );
			// on==true (before message), on==false (after message)

	void Draw( );												// draws everything
	void DrawTabs( WindowType type );							// draws tabs
	void DrawWindow( WindowType wType, BufferType bType );		// draws a window
	void DrawListWindow( WindowType wType, BufferType bType );	// draws a list (horizontal)
	void DrawListWindow2( WindowType wType, BufferType bType );	// draws a list (vertical)
	void DrawHorizontalLine( WindowType type );					// draws a horizontal line
	void DrawVerticalLine( WindowType type );					// draws a vertical line

	void Resize( int y, int x );								// resizes term
	void Resize( );												//
	
	// Tab
	void AddTab( string nName, TabType nType, uint32_t nId, BufferType nBufferType, WindowType nWindowType );
	void RemoveTab( TabType type, uint32_t id );				// *not used currently*
	void SelectTab( uint32_t n );								// unselects old tab and selects new tab

	// Window
	void UpdateWindow( WindowType type );						// updates window
	void ClearWindow( WindowType type );						// clears window
	void UpdateWindows( );										// updates windows

	void CompileList( BufferType type );						// compiles list
	void CompileLists( );										// compiles lists
	void CompileFriends( );										// compiles friends-list
	void CompileClan( );										// compiles clan-list
	void CompileBans( );										// compiles ban-list
	void CompileAdmins( );										// compiles admin-list
	void CompileGames( );										// compiles gamelist

	// Mouse
	void UpdateMouse( int c );									// updates mouse clicks/scrolls

	// Misc
	uint32_t GetRealmId( string &realmAlias );					// id from alias
	uint32_t GetNextRealm( uint32_t realmId );					// next realm id
	bool IsConnected( uint32_t realmId, bool entry );			// are we connected at all?

	uint32_t GetMessageFlag( string &message );					// returns message flag, e.g. "bnet" = 4 which makes it colored yellow
	void UpdateCustomLists( BufferType type );					// updates friends/clan list etc.

	uint32_t m_SelectedTab;										// currently selected tab
	uint32_t m_SelectedInput;									// for command history

	uint32_t m_ListUpdateTimer;									// timer for updating friends/clan list
	bool m_SplitView;											// is split view enabled for realm tab?
	int m_ListType;												// list type (horizontal == 0 / vertical != 0)

	void ScrollDown( );											// scrolls down window
	void ScrollUp( );											// scrolls up window

	uint32_t NextTab( );										// selects next tab (using arrow keys)
	uint32_t PreviousTab( );									// selects previous tab (using arrow keys)

	int exY, exX;												// fixes scrolling

public:
	CCurses( int nTermWidth, int nTermHeight, bool nSplitView, int nListType );
	~CCurses( );

	void SetGHost( CGHost * nGHost );									// important in accessing ghost
	void Print( string message, uint32_t realmId, bool toMainBuffer );	// prints messages to buffers
	bool Update( );														// updates user interface, returns true when quitting

	// Channel / Custom list
	void ChangeChannel( string channel, uint32_t realmId );
	void AddChannelUser( string name, uint32_t realmId, int flag );
	void UpdateChannelUser( string name, uint32_t realmId, int flag );
	void RemoveChannelUser( string name, uint32_t realmId );
	void RemoveChannelUsers( uint32_t realmId );

};

#endif