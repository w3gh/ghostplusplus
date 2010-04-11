
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
// CCurses
//

class CGHost;

class CCurses
{
private:
	// Pointer to CGHost
	CGHost *m_GHost;

	// Buffers
	vector<string> m_MainBuffer;
	vector<vector<string> > m_RealmBuffers;
	string m_InputBuffer;

	// Channels, Friends, Clan
	string m_ChannelName;
	vector<pair<string, int> > m_ChannelUsers;
	vector<pair<string, char> > m_Friends;
	vector<pair<string, char> > m_Clan;
	uint32_t m_ListCycleTimer;
	bool m_ShowFriends;

	// Windows
	WINDOW *m_MainWindow;
	WINDOW *m_InputWindow;
	WINDOW *m_ChannelWindow;
	WINDOW *m_CustomListWindow;
	WINDOW *m_RealmWindow;
	WINDOW *m_MiddleBorder;
	WINDOW *m_BottomBorder;
	WINDOW *m_RightBorder;
	bool m_MainWindowChanged;
	bool m_InputWindowChanged;
	bool m_ChannelWindowChanged;
	bool m_CustomListWindowChanged;
	bool m_RealmWindowChanged;

	// Realm
	string m_RealmAlias;
	uint32_t m_RealmId;

	// Drawing
	void SetMessageAttribute( WINDOW* window, string message, bool on, bool errorEnabled = false );
	void SetNameAttribute( WINDOW* window, int flags, bool on, bool isChannelUser = false );

	void Draw( );
	void DrawMainWindow( );
	void DrawInputWindow( );
	void DrawChannelWindow( );
	void DrawCustomListWindow( );
	void DrawRealmWindow( );
	void Resize( );

	// Misc
	uint32_t GetRealmId( string &realmAlias );
	uint32_t GetNextRealm( uint32_t realmId );
	bool IsConnected( uint32_t realmId, bool entry );

public:
	CCurses( int nTermWidth, int nTermHeight );
	~CCurses( );

	void SetGHost( CGHost * nGHost );
	void Print( string message, string realmAlias, bool toMainBuffer );
	bool Update( );

	// Channel / Custom list
	void ChangeChannel( string channel, string realmAlias );
	void AddChannelUser( string name, string realmAlias, int userFlags );
	void UpdateChannelUser( string name, string realmAlias, int userFlags );
	void RemoveChannelUser( string name, string realmAlias );
	void RemoveChannelUsers( string realmAlias );
	void UpdateCustomList( string realmAlias );
};

#endif