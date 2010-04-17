
#include "ghost.h"
#include "bnet.h"
#include "userinterface.h"
#include "util.h"
#include "game_base.h"

#include <math.h>

int ceili( float f )
{
	return int( ceilf( f ) );
}

CCurses :: CCurses( int nTermWidth, int nTermHeight, bool nSplitView )
{
#ifdef __PDCURSES__
	PDC_set_title("GHost++ CursesMod");
#endif

	// Initialize vectors
	SRealmData temp1;
	for ( uint32_t i = 0; i < 10; ++i )
		m_RealmData.push_back( temp1 );

	SWindowData temp2;
	for ( uint32_t i = 0; i < 7; ++i )
		m_WindowData.push_back( temp2 );

	Buffer *temp3;
	for ( uint32_t i = 0; i < 30; ++i )
	{
		temp3 = new Buffer( );
		m_Buffers.push_back( temp3 );
	}

	// Initialize variables
	m_RealmId = 0;
	m_RealmId2 = 0;
	m_ListUpdateTimer = 0;
	m_SelectedTab = 0;
	m_SelectedInput = 0;
	m_GHost = 0;
	m_SplitView = nSplitView;
	
	// Initialize curses and windows
	initscr( );
	clear( );
	noecho( );
	cbreak( );

	m_WindowData[W_TAB].Window = newwin( 1, COLS, 0, 0 );
	m_WindowData[W_FULL].Window = newwin( LINES - 3, COLS - 21, 1, 0 );
	m_WindowData[W_FULL2].Window = newwin( LINES - 3, COLS, 1, 0 );
	m_WindowData[W_UPPER].Window = newwin( LINES / 2 - 2, COLS - 21, 1, 0 );
	m_WindowData[W_LOWER].Window = newwin( LINES / 2 - 2, COLS - 21, LINES / 2, 0 );
	m_WindowData[W_CHANNEL].Window = newwin( LINES - 3, 21, 1, COLS - 21 );
	m_WindowData[W_INPUT].Window = newwin( 2, COLS, LINES - 2, 0 );

	scrollok( m_WindowData[W_FULL].Window, TRUE );
	scrollok( m_WindowData[W_FULL2].Window, TRUE );
	scrollok( m_WindowData[W_UPPER].Window, TRUE );
	scrollok( m_WindowData[W_LOWER].Window, TRUE );
	keypad( m_WindowData[W_INPUT].Window, TRUE );
	scrollok( m_WindowData[W_INPUT].Window, TRUE );
	nodelay( m_WindowData[W_INPUT].Window, TRUE );

	// Initialize colors
	start_color();
	init_pair( 0, COLOR_WHITE, COLOR_BLACK );
	init_pair( 1, COLOR_GREEN, COLOR_BLACK );
	init_pair( 2, COLOR_RED, COLOR_BLACK );
	init_pair( 3, COLOR_CYAN, COLOR_BLACK );
	init_pair( 4, COLOR_YELLOW, COLOR_BLACK );
	init_pair( 5, COLOR_BLUE, COLOR_BLACK );
	init_pair( 6, COLOR_WHITE, COLOR_CYAN );

	// make this an option/config:
	wbkgdset( m_WindowData[W_INPUT].Window, ' ' | COLOR_PAIR(6)  );
	wattr_set( m_WindowData[W_INPUT].Window, A_NORMAL, 6, 0 );
	wbkgdset( m_WindowData[W_TAB].Window, ' ' | COLOR_PAIR(6)  );
	wattr_set( m_WindowData[W_TAB].Window, A_NORMAL, 6, 0 );
	
	// Change terminal size
	resize_term( nTermHeight, nTermWidth );
	Resize( );

	// Tabs
	AddTab( "ALL", T_MAIN, 0, B_ALL );
	AddTab( "MAIN", T_MAIN, 0, B_MAIN );
	AddTab( "FRIENDS", T_LIST, 0, B_FRIENDS);
	AddTab( "CLAN", T_LIST, 0, B_CLAN );
	AddTab( "BANS", T_LIST, 0, B_BANS );
	AddTab( "ADMINS", T_LIST, 0, B_ADMINS );
	AddTab( "GAMES", T_LIST, 0, B_GAMES );
	SelectTab( 0 );
	
	// Initialize Input-buffer
	m_Buffers[B_INPUT]->push_back( pair<string, int>("", 0) );

	// Mouse cursor
	mouse_on( ALL_MOUSE_EVENTS );
	mouseinterval( 30 );
	curs_set( 1 );	// 0 = nothing or 1 = underline or 2 = block
}

CCurses :: ~CCurses( )
{
	endwin( );
}

void CCurses :: AddTab( string nName, TabType nType, uint32_t nId, BufferType nBufferType )
{
	STabData data;
	data.name = nName;
	data.type = nType;
	data.id = nId;
	data.bufferType  = nBufferType;
	data.IsTabSelected = false;

	for( vector<STabData> :: iterator i = m_TabData.begin( ); i != m_TabData.end( ); i++ )
	{
		if( (*i).type == nType && (*i).id == nId - 1 )
		{
			m_TabData.insert( i + 1, data );
			m_WindowData[W_TAB].IsWindowChanged = true;
			return;
		}
	}

	m_WindowData[W_TAB].IsWindowChanged = true;
	m_TabData.push_back( data );
}

void CCurses :: RemoveTab( TabType type, uint32_t id )
{
	// todo
	m_WindowData[W_TAB].IsWindowChanged = true;
}

void CCurses :: SelectTab( uint32_t n )
{
	if( n >= 0 && n < m_TabData.size( ) )
	{
		if( m_SelectedTab < m_TabData.size( ) )
			m_TabData[m_SelectedTab].IsTabSelected = false;

		m_SelectedTab = n;
		m_TabData[m_SelectedTab].IsTabSelected = true;

		m_RealmId = m_TabData[m_SelectedTab].id;

		m_Buffers[B_REALM] = &m_RealmData[m_RealmId].Messages;
		m_Buffers[B_CHANNEL] = &m_RealmData[m_RealmId].ChannelUsers;

		UpdateCustomLists( m_TabData[m_SelectedTab].bufferType );
		CompileList( m_TabData[m_SelectedTab].bufferType );

		if( m_TabData[m_SelectedTab].type == T_REALM && m_SplitView )
		{
			mvhline( LINES / 2 - 1, 0, 0, COLS - 21 );
			refresh( );
		}

		UpdateWindows( );
	}
}

void CCurses :: UpdateWindow( WindowType type )
{
	int y1 = 0, x1 = 0;
	int y2 = 0, x2 = 0;

	switch( type )
	{
	case W_TAB:		y1 = 0; x1 = 0; y2 = 1; x2 = COLS; break;
	case W_FULL:	y1 = 1; x1 = 0; y2 = LINES - 3; x2 = COLS - 21; break;
	case W_FULL2:	y1 = 1; x1 = 0; y2 = LINES - 3; x2 = COLS; break;
	case W_UPPER:	y1 = 1; x1 = 0; y2 = LINES / 2 - 2; x2 = COLS - 21; break;
	case W_LOWER:	y1 = LINES / 2; x1 = 0; y2 = ceili(LINES * 1.0f / 2) - 2; x2 = COLS - 21; break;
	case W_CHANNEL:	y1 = 1; x1 = COLS - 21; y2 = LINES - 3; x2 = 21; break;
	case W_INPUT:	y1 = LINES - 2; x1 = 0; y2 = 2; x2 = COLS; break;
	}

	wresize( m_WindowData[type].Window, y2, x2 );
	mvwin( m_WindowData[type].Window, y1, x1 );
	m_WindowData[type].IsWindowChanged = true;
}

void CCurses :: UpdateWindows( )
{
	for( uint32_t i = 0; i < 7; ++i )
	{
		UpdateWindow( WindowType( i ) );
	}
}

void CCurses :: CompileList( BufferType type )
{
	switch( type )
	{
	case B_BANS:	CompileBans( );		break;
	case B_ADMINS:	CompileAdmins( );	break;
	case B_FRIENDS:	CompileFriends( );	break;
	case B_CLAN:	CompileClan( );		break;
	case B_GAMES:	CompileGames( );	break;
	}
}

void CCurses :: CompileLists( )
{
	CompileBans( );
	CompileAdmins( );
	CompileFriends( );
	CompileClan( );
	CompileGames( );
}

void CCurses :: CompileFriends( )
{
	m_Buffers[B_FRIENDS]->clear( );

	for( uint32_t i = 0; i < 10; ++i )
	{
		if( !m_RealmData[i].Friends.empty( ) )
		{
			m_Buffers[B_FRIENDS]->push_back( pair<string, int>( "[" + m_RealmData[i].RealmAlias + "]", 1 ) );
			
			for( uint32_t j = 0; j < m_RealmData[i].Friends.size( ); ++j )
			{
				m_Buffers[B_FRIENDS]->push_back( pair<string, int>( " " + m_RealmData[i].Friends[j].first, m_RealmData[i].Friends[j].second ) );
			}

			m_Buffers[B_FRIENDS]->push_back( pair<string, int>( "\n", 1 ) );
		}
	}
}

void CCurses :: CompileClan( )
{
	m_Buffers[B_CLAN]->clear( );

	for( uint32_t i = 0; i < 10; ++i )
	{
		if( !m_RealmData[i].Clan.empty( ) )
		{
			m_Buffers[B_CLAN]->push_back( pair<string, int>( "[" + m_RealmData[i].RealmAlias + "]", 1 ) );
			
			for( uint32_t j = 0; j < m_RealmData[i].Clan.size( ); ++j )
			{
				m_Buffers[B_CLAN]->push_back( pair<string, int>( " " + m_RealmData[i].Clan[j].first, m_RealmData[i].Clan[j].second ) );
			}

			m_Buffers[B_CLAN]->push_back( pair<string, int>( "\n", 1 ) );
		}
	}
}

void CCurses :: CompileBans( )
{
	m_Buffers[B_BANS]->clear( );

	for( uint32_t i = 0; i < 10; ++i )
	{
		if( !m_RealmData[i].Bans.empty( ) )
		{
			m_Buffers[B_BANS]->push_back( pair<string, int>( "[" + m_RealmData[i].RealmAlias + "]", 1 ) );
			
			for( uint32_t j = 0; j < m_RealmData[i].Bans.size( ); ++j )
			{
				m_Buffers[B_BANS]->push_back( pair<string, int>( " " + m_RealmData[i].Bans[j].first, m_RealmData[i].Bans[j].second ) );
			}

			m_Buffers[B_BANS]->push_back( pair<string, int>( "\n", 1 ) );
		}
	}
}

void CCurses :: CompileAdmins( )
{
	m_Buffers[B_ADMINS]->clear( );

	for( uint32_t i = 0; i < 10; ++i )
	{
		if( !m_RealmData[i].Admins.empty( ) )
		{
			m_Buffers[B_ADMINS]->push_back( pair<string, int>( "[" + m_RealmData[i].RealmAlias + "]", 1 ) );
			
			for( uint32_t j = 0; j < m_RealmData[i].Admins.size( ); ++j )
			{
				m_Buffers[B_ADMINS]->push_back( pair<string, int>( " " + m_RealmData[i].Admins[j].first, m_RealmData[i].Admins[j].second ) );
			}

			m_Buffers[B_ADMINS]->push_back( pair<string, int>( "\n", 1 ) );
		}
	}
}

void CCurses :: CompileGames( )
{
	m_Buffers[B_GAMES]->clear( );

	if( m_GHost->m_Games.empty( ) && m_GHost->m_CurrentGame )
		m_Buffers[B_GAMES]->push_back( pair<string, int>( "0. " + m_GHost->m_CurrentGame->GetDescription( ) + "\n", 0 ) );
	else
	{
		if (m_GHost->m_CurrentGame )
			m_Buffers[B_GAMES]->push_back( pair<string, int>( "0. " + m_GHost->m_CurrentGame->GetDescription( ) + "\n", 0 ) );

		for( uint32_t i = 0; i < m_GHost->m_Games.size( ); ++i )
		{
			m_Buffers[B_GAMES]->push_back( pair<string, int>( UTIL_ToString( i + 1 ) + ". " + m_GHost->m_Games[i]->GetDescription( ) + "\n", 0 ) );
		}
	}
}

void CCurses :: SetGHost( CGHost* nGHost )
{
	m_GHost = nGHost;

	// Realms
	for ( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		m_RealmData[(*i)->GetRealmId( )].RealmAlias = (*i)->GetServerAlias( );
		AddTab( (*i)->GetServerAlias( ), T_REALM, (*i)->GetRealmId( ), B_REALM );
	}
}

void CCurses :: SetAttribute( SWindowData &data, string message, int flag, BufferType type, bool on )
{
	attr_t attribute = A_WHITE;

	transform( message.begin(), message.end(), message.begin(), ::tolower );
	
	switch( type )
	{
	case B_TAB:
		if( flag == 1 )	attribute = COLOR_PAIR( 6 ) | A_BOLD;
		break;
	case B_ALL:
	case B_MAIN:
		if ( message.find("error") != -1 )
		{
			attribute = A_RED;
			break;
		}
	case B_REALM:
		flag = GetMessageFlag( message );
		switch( flag )
		{
		case 0: attribute = A_WHITE;	break;	// NORMAL, GAME, MAP, CONFIG, UTIL
		case 1: attribute = A_CYAN;		break;	// INFO, EMOTE
		case 2: attribute = A_CYAN;		break;	// GHOST
		case 3: attribute = A_RED;		break;	// ERROR
		case 4: attribute = A_YELLOW;	break;	// BNET, BNLSC
		case 5: attribute = A_GREEN;	break;	// WHISPER
		}
		break;
	case B_CHANNEL:
		switch( flag )
		{
		case 1: attribute = A_BBLUE;	break;	// BLIZZARD REP
		case 2: attribute = A_YELLOW;	break;	// CHANNEL OP
		case 4: attribute = A_CYAN;		break;	// SPEAKER
		case 8: attribute = A_BBLUE;	break;	// BNET ADMIN
		case 32: attribute = A_RED;		break;	// SQUELCHED
		}
		break;
	case B_FRIENDS:
		switch ( flag )
		{
		case 0: attribute = A_RED;		break;	// OFFLINE
		case 1: attribute = A_WHITE;	break;	// NOT IN CHAT
		case 2: attribute = A_BWHITE;	break;	// IN CHAT
		case 3: attribute = A_GREEN;	break;	// IN PUBLIC GAME
		case 4: attribute = A_YELLOW;	break;	// IN PRIVATE GAME; NOT MUTUAL
		case 5: attribute = A_CYAN;		break;	// IN PRIVATE GAME; MUTUAL
		}
		break;
	case B_ADMINS:
		if( flag == 2 ) attribute = A_YELLOW; break;	// ROOTADMIN
	case B_CLAN:
		// todo: add flags
		break;
	}

	if( on )
		wattr_on( data.Window, attribute, 0 );
	else
		wattr_off( data.Window, attribute, 0 );
}

void CCurses :: Draw ( )
{
	// view 1: all, no channel
	// view 2: main only, no channel
	// view 3-7: list, no channel
	// accept only inner commands "!", "/commands", "/quit" and "/resize"
	// send inner commands to the first realm that user is logged in
	// view 7-x: realm, channel, input, (splitview: main)

	// draw tabs always
	DrawTabs( );

	if( !m_TabData.empty( ) )
		switch( m_TabData[m_SelectedTab].type )
		{
		case T_MAIN:
			DrawWindow( W_FULL2, m_TabData[m_SelectedTab].bufferType );
			DrawWindow( W_INPUT, B_INPUT );
			break;
		case T_LIST:
			DrawListWindow( W_FULL2, m_TabData[m_SelectedTab].bufferType );
			DrawWindow( W_INPUT, B_INPUT );
			break;
		case T_REALM:
			m_WindowData[W_CHANNEL].title = m_RealmData[m_TabData[m_SelectedTab].id].ChannelName;
			if( m_SplitView )
			{
				mvhline( LINES / 2 - 1, 0, 0, COLS - 21 );
				DrawWindow( W_UPPER, B_MAIN );
				DrawWindow( W_LOWER, m_TabData[m_SelectedTab].bufferType );
			}
			else
			{
				DrawWindow( W_FULL, m_TabData[m_SelectedTab].bufferType );
			}
			DrawWindow( W_CHANNEL, B_CHANNEL );
			DrawWindow( W_INPUT, B_INPUT );
			break;
		case T_GAME:
			DrawWindow( W_FULL, m_TabData[m_SelectedTab].bufferType );
			DrawWindow( W_INPUT, B_INPUT );
			break;
		}
}

void CCurses :: DrawTabs( )
{
	SWindowData &data = m_WindowData[W_TAB];
	BufferType type = B_TAB;

	if ( data.IsWindowChanged )
	{
		wclear( data.Window );
		wmove( data.Window, 0, 0 );

		for( vector<STabData> :: iterator i = m_TabData.begin( ); i != m_TabData.end( ); i++ )
		{
			string &name = (*i).name;
			int flag = (*i).IsTabSelected ? 1 : 0;

			name = UTIL_UTF8ToLatin1( name );

			SetAttribute( data, name, flag, type, true );

			for( string :: iterator j = name.begin( ); j != name.end( ); j++ )
				waddch( data.Window, UTIL_ToULong( *j ) );

			SetAttribute( data, name, flag, type, false );

			waddch( data.Window, ' ' );
		}

		wrefresh( data.Window );
		data.IsWindowChanged = false;
	}
}

void CCurses :: DrawWindow( WindowType wType, BufferType bType )
{
	SWindowData& data = m_WindowData[wType];

	if ( data.IsWindowChanged )
	{
		bool onlyLast = bType == B_INPUT && m_Buffers[bType]->size( ) > 1;

		wclear( data.Window );

		if( !data.title.empty( ) )
		{
			mvwhline( data.Window, 0, 0, 0, 20 );
			mvwaddnstr( data.Window, 0, data.title.size( ) < 20 ? ( 20 - data.title.size( ) ) / 2 : 0, data.title.c_str( ), 20 );
			wmove( data.Window, 1, 0 );
		}

		for( Buffer :: iterator i = onlyLast ? m_Buffers[bType]->end( ) - 1 : m_Buffers[bType]->begin( ); i != m_Buffers[bType]->end( ); i++ )
		{
			string message = (*i).first;
			int &flag = (*i).second;
			
			message = UTIL_UTF8ToLatin1( message );

			SetAttribute( data, message, flag, bType, true );

			for( string :: iterator j = message.begin( ); j != message.end( ); j++ )
				waddch( data.Window, UTIL_ToULong( *j ) );

			SetAttribute( data, message, flag, bType, false );

			if( i != m_Buffers[bType]->end( ) - 1 )
				waddch( data.Window, '\n' );
		}

		wrefresh( data.Window );
		data.IsWindowChanged = false;
	}
}

void CCurses :: DrawListWindow( WindowType wType, BufferType bType )
{
	SWindowData& data = m_WindowData[wType];

	if ( data.IsWindowChanged )
	{
		wclear( data.Window );

		for( Buffer :: iterator i = m_Buffers[bType]->begin( ); i != m_Buffers[bType]->end( ); i++ )
		{
			string message = (*i).first;
			int &flag = (*i).second;
			
			message = UTIL_UTF8ToLatin1( message );

			if( message[0] == '[' )
				waddch( data.Window, '\n' );

			SetAttribute( data, message, flag, bType, true );

			for( string :: iterator j = message.begin( ); j != message.end( ); j++ )
				waddch( data.Window, UTIL_ToULong( *j ) );

			SetAttribute( data, message, flag, bType, false );
		}

		wrefresh( data.Window );
		data.IsWindowChanged = false;
	}
}

void CCurses :: Resize( int y, int x )
{
	if ( y > 5 && x > 5 )
	{
		resize_term( y, x );
		clear();
		refresh();

		UpdateWindows( );

		Draw( );
	}
}

void CCurses :: Resize( )
{
	resize_term( 0, 0 );
	clear();
    refresh();

	UpdateWindows( );

	Draw( );
}

void CCurses :: Print( string message, uint32_t realmId, bool toMainBuffer )
{
	message = UTIL_UTF8ToLatin1( message );
	pair<string, int> temp = pair<string, int>( message, 0 );
	if ( toMainBuffer )
	{
		m_Buffers[B_MAIN]->push_back( temp );

		if( m_Buffers[B_MAIN]->size( ) > 100 )
			m_Buffers[B_MAIN]->erase( m_Buffers[B_MAIN]->begin( ) );

		m_WindowData[W_UPPER].IsWindowChanged = true;
	}
	else
	{
		m_RealmData[realmId].Messages.push_back( temp );

		if( m_RealmData[realmId].Messages.size( ) > 100 )
			m_RealmData[realmId].Messages.erase( m_RealmData[realmId].Messages.begin( ) );

		m_WindowData[W_LOWER].IsWindowChanged = true;
	}

	m_Buffers[B_ALL]->push_back( temp );

	if( m_Buffers[B_ALL]->size( ) > 100 )
		m_Buffers[B_ALL]->erase( m_Buffers[B_ALL]->begin( ) );

	m_WindowData[W_FULL].IsWindowChanged = true;
	m_WindowData[W_FULL2].IsWindowChanged = true;

	Draw( );
}

void CCurses :: UpdateMouse( )
{
	// Mouse position update
	request_mouse_pos( );
	move( MOUSE_Y_POS, MOUSE_X_POS );
	refresh( );

	// Is cursor over tabs?
	if( MOUSE_Y_POS <= 1 )
	{
		// Is left button pressed?
		if( Mouse_status.button[0] == BUTTON_PRESSED )
		{
			int x1 = 0, x2 = 0;
			// Where it is pressed?
			for( uint32_t i = 0; i < m_TabData.size( ); ++i )
			{
				x1 = x2;
				x2 = x1 + m_TabData[i].name.size( ) + 1;

				if ( MOUSE_X_POS >= x1 && MOUSE_X_POS < x2 - 1 )
				{
					SelectTab( i );
					return;
				}
			}
		}
	}
}

bool CCurses :: Update( )
{
	bool Quit = false;

	bool Connected = !m_GHost->m_BNETs.empty();
	if( Connected ) Connected = IsConnected( 0, true );

	if( GetTime( ) > m_ListUpdateTimer && m_TabData[m_SelectedTab].type == T_LIST )
	{
		m_ListUpdateTimer = GetTime( ) + 7;

		for( uint32_t i = 0; i < m_GHost->m_BNETs.size( ); ++i )
			m_GHost->m_BNETs[i]->RequestListUpdates( );

		UpdateCustomLists( m_TabData[m_SelectedTab].bufferType );
		CompileList( m_TabData[m_SelectedTab].bufferType );
	}

	int c = wgetch( m_WindowData[W_INPUT].Window );

	UpdateMouse( );

	if( c == KEY_LEFT )	// LEFT
	{
		SelectTab( m_SelectedTab - 1 );
		return false;
	}
	else if( c == KEY_RIGHT )	// RIGHT
	{
		SelectTab( m_SelectedTab + 1 );
		return false;
	}

	if( m_Buffers[B_INPUT]->size( ) > 1 )
	{
		if( c == KEY_UP )
		{
			m_SelectedInput = m_SelectedInput > 0 ? m_SelectedInput - 1 : 0;
			if( m_SelectedInput != m_Buffers[B_INPUT]->size( ) - 1 )
			{
				m_InputBuffer = m_Buffers[B_INPUT]->at( m_SelectedInput ).first;

				m_WindowData[W_INPUT].IsWindowChanged = true;

				m_Buffers[B_INPUT]->pop_back( );
				m_Buffers[B_INPUT]->push_back( pair<string, int>( m_InputBuffer, 0 ) );
			}
			return false;
		}
		else if( c == KEY_DOWN )
		{
			m_SelectedInput = m_SelectedInput < m_Buffers[B_INPUT]->size( ) - 1 ? m_SelectedInput + 1 : m_Buffers[B_INPUT]->size( ) - 1;
			if( m_SelectedInput != m_Buffers[B_INPUT]->size( ) - 1 )
			{
				m_InputBuffer = m_Buffers[B_INPUT]->at( m_SelectedInput ).first;

				m_WindowData[W_INPUT].IsWindowChanged = true;

				m_Buffers[B_INPUT]->pop_back( );
				m_Buffers[B_INPUT]->push_back( pair<string, int>( m_InputBuffer, 0 ) );
			}
			return false;
		}
	}

	while( c != ERR && Connected )
	{
		if( c == 8 || c == 127 || c == KEY_BACKSPACE || c == KEY_DC )
		{
			// backspace, delete
			if( !m_InputBuffer.empty( ) )
				m_InputBuffer.erase( m_InputBuffer.size( ) - 1, 1 );
		}
		else if( c == 9 )
		{
			// tab = 9
			// shift-tab = KEY_BTAB
		}
#ifdef WIN32
		else if( c == 10 || c == 13 || c == PADENTER )
#else
		else if( c == 10 || c == 13 )
#endif
		{
			// cr, lf
			// process input buffer now

			string Command = m_InputBuffer;
			transform( Command.begin( ), Command.end( ), Command.begin( ), (int(*)(int))tolower );

			if( Command.size( ) >= 9 && Command.substr( 0, 8 ) == "/resize " )
			{
				string temp;
				int j = 0;
				int* dimensions = new int[2];

				for ( uint32_t i = 8; i < m_InputBuffer.size( ); ++i )
				{
					if ( m_InputBuffer[i] == ' ' )
					{
						dimensions[j++] = UTIL_ToInt32( temp );
						temp.clear( );
					}

					if ( m_InputBuffer[i] >= 48 && m_InputBuffer[i] <= 57 )
						temp += m_InputBuffer[i];
				}

				dimensions[j++] = UTIL_ToInt32( temp );
				temp.clear( );

				if ( j == 2 )
				{
					Resize( dimensions[1] , dimensions[0] );
				}
			}
			else if( Command == "/split" )
			{
				m_SplitView = !m_SplitView;
				UpdateWindows( );
			}
			else if( Command == "/commands" )
			{
				CONSOLE_Print( ">>> /commands", true );
				CONSOLE_Print( "", true );
				CONSOLE_Print( "  In the GHost++ console:", true );
				CONSOLE_Print( "   !<command>               : GHost command. Replace '!' with defined trigger character", true );
				CONSOLE_Print( "   /<bnet-command> <...>    : Battle.net command", true );
				CONSOLE_Print( "   /resize <width> <height> : Resizes console", true );
				CONSOLE_Print( "   /split                   : Toggles split view in realm tabs", true );
				CONSOLE_Print( "   /exit or /quit           : Close GHost++", true );
				CONSOLE_Print( "", true );
			}
			else if( Command == "/exit" || Command == "/quit" )
			{
				Quit = true;
				break;
			}
			else if( Command.size( ) >= 2 && m_TabData[m_SelectedTab].type == T_MAIN && Command[0] == m_GHost->m_BNETs[m_RealmId2]->GetCommandTrigger( ) )
			{
				m_InputBuffer = UTIL_Latin1ToUTF8( m_InputBuffer );
				m_GHost->m_BNETs[m_RealmId2]->HiddenGhostCommand( m_InputBuffer );
			}
			else if( Command.size( ) >= 2 && m_GHost->m_BNETs[m_RealmId]->GetLoggedIn( ) && Command[0] == m_GHost->m_BNETs[m_RealmId]->GetCommandTrigger( ) )
			{
				m_InputBuffer = UTIL_Latin1ToUTF8( m_InputBuffer );
				m_GHost->m_BNETs[m_RealmId]->HiddenGhostCommand( m_InputBuffer );
			}
			else if( m_TabData[m_SelectedTab].type == T_REALM )
			{
				m_InputBuffer = UTIL_Latin1ToUTF8( m_InputBuffer );
				m_GHost->m_BNETs[m_RealmId]->QueueChatCommand( m_InputBuffer, m_InputBuffer[0] == '/' );
			}
			else return false; // don't clear the m_InputBuffer

			m_InputBuffer.clear( );
			m_Buffers[B_INPUT]->push_back( pair<string, int>( "", 0 ) );
			m_SelectedInput = m_Buffers[B_INPUT]->size( ) - 1;
		}
#ifdef WIN32
		else if( c == 22 )
		{
			// paste
			char *clipboard = NULL;
			long length = 0;

			if( PDC_getclipboard( &clipboard, &length ) == PDC_CLIP_SUCCESS )
			{
				m_InputBuffer += string( clipboard, length );
				PDC_freeclipboard( clipboard );
			}
		}
		else if( c == 3 )
		{
			// copy
			string clipboard = UTIL_Latin1ToUTF8(m_InputBuffer);
			PDC_setclipboard( clipboard.c_str(), clipboard.length() );
		}
#endif
		else if( c == 27 )
		{
			// esc
			m_InputBuffer.clear( );
		}
		else if( c >= 32 && c <= 255 )
		{
			// printable characters
			m_InputBuffer.push_back( c );
		}
#ifdef WIN32
		else if( c == PADSLASH )
			m_InputBuffer.push_back( '/' );
		else if( c == PADSTAR )
			m_InputBuffer.push_back( '*' );
		else if( c == PADMINUS )
			m_InputBuffer.push_back( '-' );
		else if( c == PADPLUS )
			m_InputBuffer.push_back( '+' );
#endif
		else if( c == KEY_RESIZE )
			Resize( );

		// clamp input buffer size
		if( m_InputBuffer.size( ) > 200 )
			m_InputBuffer.erase( 200 );

		c = wgetch( m_WindowData[W_INPUT].Window );
		m_WindowData[W_INPUT].IsWindowChanged = true;

		// "/r " -> "/w <username> " just like in wc3 client and it works like that for a reason.
		if( m_TabData[m_SelectedTab].type != T_MAIN &&
			m_InputBuffer.size( ) >= 3 &&	( m_InputBuffer.substr( 0, 3 ) == "/r " || m_InputBuffer.substr( 0, 3 ) == "/R " ) )
		{
			if( m_GHost->m_BNETs[m_RealmId]->GetReplyTarget( ).empty( ) )
				m_InputBuffer = "/w ";
			else
				m_InputBuffer = "/w " + m_GHost->m_BNETs[m_RealmId]->GetReplyTarget( ) + " ";
		}

		m_Buffers[B_INPUT]->pop_back( );
		m_Buffers[B_INPUT]->push_back( pair<string, int>( m_InputBuffer, 0 ) );
	}

	Draw( );

	return Quit;
}


uint32_t CCurses :: GetRealmId( string &realmAlias )
{
	// Get id of a realm.
	for (uint32_t i = 0; i < m_GHost->m_BNETs.size(); ++i)
		if (m_GHost->m_BNETs[i]->GetServerAlias() == realmAlias)
			return i;

	return 0;
}

uint32_t CCurses :: GetNextRealm( uint32_t realmId )
{
	// Returns next realm in the vector.
	uint32_t result = realmId + 1;

	if ( result == m_GHost->m_BNETs.size( ) )
		result = 0;

	return result;
}

bool CCurses :: IsConnected( uint32_t realmId, bool entry )
{
	// This will check if we are logged in some realm.
	if ( m_GHost->m_BNETs[realmId]->GetLoggedIn() == false )
	{
		if ( realmId == m_RealmId2 && entry == false)
			return false;
		else
		{
			return IsConnected( GetNextRealm( realmId ), false );
		}
	}

	m_RealmId2 = realmId;

	return true;
}

uint32_t CCurses :: GetMessageFlag( string &message )
{
	if ( message.size( ) > 4 && message[0] == '[' )
	{
		if ( message.compare(1, 4, "info") == 0 )			return 1;
		else if ( message.compare(1, 4, "bnet") == 0 )		return 4;
		else if ( message.compare(1, 5, "ghost") == 0 )		return 2;
		else if ( message.compare(1, 5, "bnlsc") == 0 )		return 4;
		else if ( message.compare(1, 5, "error") == 0 )		return 3;
		else if ( message.compare(1, 5, "emote") == 0 )		return 1;
		else if ( message.compare(1, 7, "whisper") == 0 )	return 5;
	}

	return 0;
}

void CCurses :: ChangeChannel( string channel, uint32_t realmId )
{
	m_RealmData[realmId].ChannelName = channel;
	m_WindowData[W_FULL].IsWindowChanged = true;
	m_WindowData[W_CHANNEL].IsWindowChanged = true;
}

void CCurses :: AddChannelUser( string name, uint32_t realmId, int flag )
{
	for( Buffer :: iterator i = m_RealmData[realmId].ChannelUsers.begin( ); i != m_RealmData[realmId].ChannelUsers.end( ); i++ )
	{
		if( (*i).first == name )
			return;
	}

	m_RealmData[realmId].ChannelUsers.push_back( pair<string, int>( name, flag ) );
	m_WindowData[W_CHANNEL].IsWindowChanged = true;
}

void CCurses :: UpdateChannelUser( string name, uint32_t realmId, int flag )
{
	for( Buffer :: iterator i = m_RealmData[realmId].ChannelUsers.begin( ); i != m_RealmData[realmId].ChannelUsers.end( ); i++ )
	{
		if( (*i).first == name )
		{
			(*i).second = flag;
			m_WindowData[W_CHANNEL].IsWindowChanged = true;
			return;
		}
	}
}

void CCurses :: RemoveChannelUser( string name, uint32_t realmId )
{
	for( Buffer :: iterator i = m_RealmData[realmId].ChannelUsers.begin( ); i != m_RealmData[realmId].ChannelUsers.end( ); i++ )
	{
		if( (*i).first == name )
		{
			i = m_RealmData[realmId].ChannelUsers.erase( i );
			break;
		}
	}

	m_WindowData[W_CHANNEL].IsWindowChanged = true;
}

void CCurses :: RemoveChannelUsers( uint32_t realmId )
{
	m_RealmData[realmId].ChannelUsers.clear( );
	m_WindowData[W_CHANNEL].IsWindowChanged = true;
}

void CCurses :: UpdateCustomLists( BufferType type )
{
	if( m_GHost )
	{
		for( uint32_t i = 0; i < m_GHost->m_BNETs.size( ); ++i )
		{
			switch( type )
			{
			case B_FRIENDS:	m_RealmData[i].Friends = m_GHost->m_BNETs[i]->GetFriends( );
			case B_CLAN:	m_RealmData[i].Clan = m_GHost->m_BNETs[i]->GetClan( );
			case B_BANS:	m_RealmData[i].Bans = m_GHost->m_BNETs[i]->GetBans( );
			case B_ADMINS:	m_RealmData[i].Admins = m_GHost->m_BNETs[i]->GetAdmins( );
			}
		}
		CompileGames( );
		m_WindowData[W_FULL2].IsWindowChanged = true;
	}
}