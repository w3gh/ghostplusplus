
#include "ghost.h"
#include "bnet.h"
#include "userinterface.h"
#include "util.h"

CCurses :: CCurses( int nTermWidth, int nTermHeight )
{
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
	
	// Initialize curses and windows
	initscr( );
	clear( );
	noecho( );
	cbreak( );

	m_WindowData[W_TAB].Window = newwin( 1, COLS, 0, 0 );
	m_WindowData[W_FULL].Window = newwin( LINES - 3, COLS - 21, 1, 0 );
	m_WindowData[W_FULL2].Window = newwin( LINES - 3, COLS, 1, 0 );
	m_WindowData[W_UPPER].Window = newwin( LINES / 2 - 1, COLS - 21, 1, 0 );
	m_WindowData[W_LOWER].Window = newwin( LINES / 2 - 3, COLS - 21, LINES / 2, 0 );
	m_WindowData[W_CHANNEL].Window = newwin( LINES - 3, 20, 1, COLS - 20 );
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
	SelectTab( 0 );
	
	// Initialize Input-buffer
	m_Buffers[B_INPUT]->push_back( pair<string, int>("", 0) );
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

		m_Buffers[B_REALM] = &m_RealmData[m_TabData[m_SelectedTab].id].Messages;
		m_Buffers[B_CHANNEL] = &m_RealmData[m_TabData[m_SelectedTab].id].ChannelUsers;

		m_RealmId = m_TabData[m_SelectedTab].id;

		UpdateWindows( );
	}
}

void CCurses :: ShowWindow( WindowType type )
{
	int y = 0, x = 0;
	switch( type )
	{
	case W_TAB:		y = 0; x = 0; break;
	case W_FULL:	y = 1; x = 0; break;
	case W_FULL2:	y = 1; x = 0; break;
	case W_UPPER:	y = 1; x = 0; break;
	case W_LOWER:	y = LINES / 2; x = 0; break;
	case W_CHANNEL:	y = 1; x = COLS - 21; break;
	case W_INPUT:	y = LINES - 2; x = 0; break;
	}

	mvwin( m_WindowData[type].Window, y, x );
	m_WindowData[type].IsWindowChanged = true;
}

void CCurses :: HideWindow( WindowType type )
{
	mvwin( m_WindowData[type].Window, 1000, 1000 );
	m_WindowData[type].IsWindowChanged = false;
}

void CCurses :: UpdateWindows( )
{
	for( uint32_t i = 0; i < 7; ++i )
	{
		ShowWindow( WindowType( i ) );
	}
}

void CCurses :: CompileFriends( )
{
	m_Buffers[B_FRIENDS]->clear( );

	for( uint32_t i = 0; i < 10; ++i )
	{
		if ( !m_RealmData[i].Friends.empty( ) )
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
		if ( !m_RealmData[i].Clan.empty( ) )
		{
			m_Buffers[B_CLAN]->push_back( pair<string, int>( "[" + m_RealmData[i].RealmAlias + "]", 1 ) );
			
			for( uint32_t j = 0; j < m_RealmData[i].Clan.size( ) && j < 25; ++j )
			{
				m_Buffers[B_CLAN]->push_back( pair<string, int>( " " + m_RealmData[i].Clan[j].first, m_RealmData[i].Clan[j].second ) );
			}

			m_Buffers[B_CLAN]->push_back( pair<string, int>( "\n", 1 ) );
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
	// accept only inner commands "!", "/commands", "/quit" and "/resize"
	// send inner commands to the first realm that user is logged in

	// view 2: main only, no channel
	// accept only inner commands "!", "/commands", "/quit" and "/resize"
	// send inner commands to the first realm that user is logged in

	// view 3: friends, no channel, no input
	// option: show only online friends (config: curses_online_friends_only)

	// view 4: clan, no channel, no input
	// option: show first x users of each clan (config: curses_max_clan)
	// option: show only online users of each clan (config: curses_online_clan_only)

	// view 5-x: realm, channel, input, (splitview: main)

	// draw tabs always
	DrawTabs( );

	if( !m_TabData.empty( ) )
		switch( m_TabData[m_SelectedTab].type )
		{
		case T_MAIN:
			DrawWindow( m_WindowData[W_FULL2], m_TabData[m_SelectedTab].bufferType );
			DrawWindow( m_WindowData[W_INPUT], B_INPUT );
			HideWindow( W_FULL );
			HideWindow( W_CHANNEL );
			HideWindow( W_UPPER );
			HideWindow( W_LOWER );
			break;
		case T_LIST:
			DrawListWindow( m_WindowData[W_FULL2], m_TabData[m_SelectedTab].bufferType );
			HideWindow( W_INPUT );
			HideWindow( W_FULL );
			HideWindow( W_CHANNEL );
			HideWindow( W_UPPER );
			HideWindow( W_LOWER );
			break;
		case T_REALM:
			m_WindowData[W_CHANNEL].title = m_RealmData[m_TabData[m_SelectedTab].id].ChannelName;
			DrawWindow( m_WindowData[W_FULL], m_TabData[m_SelectedTab].bufferType );
			DrawWindow( m_WindowData[W_CHANNEL], B_CHANNEL );
			DrawWindow( m_WindowData[W_INPUT], B_INPUT );
			HideWindow( W_FULL2 );
			HideWindow( W_UPPER );
			HideWindow( W_LOWER );
			break;
		case T_GAME:
			DrawWindow( m_WindowData[W_FULL], m_TabData[m_SelectedTab].bufferType );
			DrawWindow( m_WindowData[W_INPUT], B_INPUT );
			HideWindow( W_FULL2 );
			HideWindow( W_UPPER );
			HideWindow( W_LOWER );
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

void CCurses :: DrawWindow( SWindowData &data, BufferType type )
{
	if ( data.IsWindowChanged )
	{
		bool onlyLast = type == B_INPUT && m_Buffers[type]->size( ) > 1;

		wclear( data.Window );

		if( !data.title.empty( ) )
		{
			mvwhline( data.Window, 0, 0, 0, 20 );
			mvwaddnstr( data.Window, 0, data.title.size( ) < 20 ? ( 20 - data.title.size( ) ) / 2 : 0, data.title.c_str( ), 20 );
			wmove( data.Window, 1, 0 );
		}

		for( Buffer :: iterator i = onlyLast ? m_Buffers[type]->end( ) - 1 : m_Buffers[type]->begin( ); i != m_Buffers[type]->end( ); i++ )
		{
			string message = (*i).first;
			int &flag = (*i).second;
			
			message = UTIL_UTF8ToLatin1( message );

			SetAttribute( data, message, flag, type, true );

			for( string :: iterator j = message.begin( ); j != message.end( ); j++ )
				waddch( data.Window, UTIL_ToULong( *j ) );

			SetAttribute( data, message, flag, type, false );

			if( i != m_Buffers[type]->end( ) - 1 )
				waddch( data.Window, '\n' );
		}

		wrefresh( data.Window );
		data.IsWindowChanged = false;
	}
}

void CCurses :: DrawListWindow( SWindowData &data, BufferType type )
{
	if ( data.IsWindowChanged )
	{
		wclear( data.Window );

		for( Buffer :: iterator i = m_Buffers[type]->begin( ); i != m_Buffers[type]->end( ); i++ )
		{
			string message = (*i).first;
			int &flag = (*i).second;
			
			message = UTIL_UTF8ToLatin1( message );

			if( message[0] == '[' )
				waddch( data.Window, '\n' );

			SetAttribute( data, message, flag, type, true );

			for( string :: iterator j = message.begin( ); j != message.end( ); j++ )
				waddch( data.Window, UTIL_ToULong( *j ) );

			SetAttribute( data, message, flag, type, false );
		}

		wrefresh( data.Window );
		data.IsWindowChanged = false;
	}
}

void CCurses :: Resize( )
{
	resize_term( 0, 0 );
	clear();
    refresh();

	wresize( m_WindowData[W_TAB].Window, 1, COLS );
	wresize( m_WindowData[W_FULL].Window, LINES - 3, COLS - 21 );
	wresize( m_WindowData[W_FULL2].Window, LINES - 3, COLS );
	wresize( m_WindowData[W_UPPER].Window, LINES / 2 - 1, COLS - 21 );
	wresize( m_WindowData[W_LOWER].Window, LINES / 2 - 3, COLS - 21 );
	wresize( m_WindowData[W_CHANNEL].Window, LINES - 3, 20 );
	wresize( m_WindowData[W_INPUT].Window, 2, COLS );

	mvwin( m_WindowData[W_TAB].Window, 0, 0 );
	mvwin( m_WindowData[W_FULL].Window, 1, 0 );
	mvwin( m_WindowData[W_FULL2].Window, 1, 0 );
	mvwin( m_WindowData[W_UPPER].Window, 1, 0 );
	mvwin( m_WindowData[W_LOWER].Window, LINES / 2, 0 );
	mvwin( m_WindowData[W_CHANNEL].Window, 1, COLS - 20 );
	mvwin( m_WindowData[W_INPUT].Window, LINES - 2, 0 );

	m_WindowData[W_TAB].IsWindowChanged = true;
	m_WindowData[W_FULL].IsWindowChanged = true;
	m_WindowData[W_FULL2].IsWindowChanged = true;
	m_WindowData[W_UPPER].IsWindowChanged = true;
	m_WindowData[W_LOWER].IsWindowChanged = true;
	m_WindowData[W_CHANNEL].IsWindowChanged = true;
	m_WindowData[W_INPUT].IsWindowChanged = true;

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

bool CCurses :: Update( )
{
	bool Quit = false;

	bool Connected = !m_GHost->m_BNETs.empty();
	if( Connected ) Connected = IsConnected( m_RealmId, true );

	if( GetTime( ) > m_ListUpdateTimer && Connected )
	{
		m_ListUpdateTimer = GetTime( ) + 10;
		m_GHost->m_BNETs[m_RealmId]->RequestListUpdates( );
	}

	int c = wgetch( m_WindowData[W_INPUT].Window );

	if( c == KEY_BTAB || c == KEY_LEFT )	// SHIFT-TAB, LEFT
	{
		SelectTab( m_SelectedTab - 1 );
		return false;
	}
	else if( c == 9 || c == KEY_RIGHT )	// TAB, RIGHT
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

	while( c != ERR && Connected && m_TabData[m_SelectedTab].type != T_LIST )
	{
		string &InputBuffer = m_InputBuffer;

		if( c == 8 || c == 127 || c == KEY_BACKSPACE || c == KEY_DC )
		{
			// backspace, delete
			if( !InputBuffer.empty( ) )
				InputBuffer.erase( InputBuffer.size( ) - 1, 1 );
		}
#ifdef WIN32
		else if( c == 10 || c == 13 || c == PADENTER )
#else
		else if( c == 10 || c == 13 )
#endif
		{
			// cr, lf
			// process input buffer now

			string Command = InputBuffer;
			transform( Command.begin( ), Command.end( ), Command.begin( ), (int(*)(int))tolower );

			if( Command.size( ) >= 9 && Command.substr( 0, 8 ) == "/resize " )
			{
				string temp;
				int j = 0;
				int* dimensions = new int[2];

				for ( uint32_t i = 8; i < InputBuffer.size( ); ++i )
				{
					if ( InputBuffer[i] == ' ' )
					{
						dimensions[j++] = UTIL_ToInt32( temp );
						temp.clear( );
					}

					if ( InputBuffer[i] >= 48 && InputBuffer[i] <= 57 )
						temp += InputBuffer[i];
				}

				dimensions[j++] = UTIL_ToInt32( temp );
				temp.clear( );

				if ( j == 2 )
				{
					resize_term( dimensions[1], dimensions[0] );
					Resize( );
				}
			}
			else if( Command == "/commands" )
			{
				CONSOLE_Print( ">>> /commands", true );
				CONSOLE_Print( "", true );
				CONSOLE_Print( "  In the GHost++ console:", true );
				CONSOLE_Print( "   !<command>               : GHost command. Replace '!' with defined trigger character.", true );
				CONSOLE_Print( "   /<bnet-command> <...>    : Battle.net command.", true );
				CONSOLE_Print( "   /resize <width> <height> : Resizes console", true );
				CONSOLE_Print( "   /exit or /quit           : Close GHost++", true );
				CONSOLE_Print( "", true );
			}
			else if( Command == "/exit" || Command == "/quit" )
			{
				Quit = true;
				break;
			}
			else if( Command.size( ) >= 2 && Command[0] == m_GHost->m_BNETs[m_RealmId]->GetCommandTrigger( ) )
			{
				InputBuffer = UTIL_Latin1ToUTF8( InputBuffer );
				m_GHost->m_BNETs[m_RealmId2]->HiddenGhostCommand( InputBuffer );
			}
			else if( m_TabData[m_SelectedTab].type != T_MAIN )
			{
				InputBuffer = UTIL_Latin1ToUTF8( InputBuffer );
				m_GHost->m_BNETs[m_RealmId]->QueueChatCommand( InputBuffer, InputBuffer[0] == '/' );
			}
			else return false; // don't clear the inputbuffer

			InputBuffer.clear( );
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
				InputBuffer += string( clipboard, length );
				PDC_freeclipboard( clipboard );
			}
		}
		else if( c == 3 )
		{
			// copy
			string clipboard = UTIL_Latin1ToUTF8(InputBuffer);
			PDC_setclipboard( clipboard.c_str(), clipboard.length() );
		}
#endif
		else if( c == 27 )
		{
			// esc
			InputBuffer.clear( );
		}
		else if( c >= 32 && c <= 255 )
		{
			// printable characters
			InputBuffer.push_back( c );
		}
#ifdef WIN32
		else if( c == PADSLASH )
			InputBuffer.push_back( '/' );
		else if( c == PADSTAR )
			InputBuffer.push_back( '*' );
		else if( c == PADMINUS )
			InputBuffer.push_back( '-' );
		else if( c == PADPLUS )
			InputBuffer.push_back( '+' );
#endif
		else if( c == KEY_RESIZE )
			Resize( );

		// clamp input buffer size
		if( InputBuffer.size( ) > 200 )
			InputBuffer.erase( 200 );

		c = wgetch( m_WindowData[W_INPUT].Window );
		m_WindowData[W_INPUT].IsWindowChanged = true;

		// "/r " -> "/w <username> " just like in wc3 client and it works like that for a reason.
		if( m_TabData[m_SelectedTab].type != T_MAIN && !m_GHost->m_BNETs[m_RealmId]->GetReplyTarget( ).empty( ) &&
			InputBuffer.size( ) >= 3 &&	( InputBuffer.substr( 0, 3 ) == "/r " || InputBuffer.substr( 0, 3 ) == "/R " ) )
		{
			InputBuffer = "/w " + m_GHost->m_BNETs[m_RealmId]->GetReplyTarget( ) + " ";
		}

		m_Buffers[B_INPUT]->pop_back( );
		m_Buffers[B_INPUT]->push_back( pair<string, int>( InputBuffer, 0 ) );
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
			i = m_RealmData[realmId].ChannelUsers.erase( i );
	}

	m_WindowData[W_CHANNEL].IsWindowChanged = true;
}

void CCurses :: RemoveChannelUsers( uint32_t realmId )
{
	m_RealmData[realmId].ChannelUsers.clear( );
	m_WindowData[W_CHANNEL].IsWindowChanged = true;
}

void CCurses :: UpdateCustomLists( uint32_t realmId )
{
	m_RealmData[realmId].Friends = m_GHost->m_BNETs[realmId]->GetFriends( );
	m_RealmData[realmId].Clan = m_GHost->m_BNETs[realmId]->GetClan( );

	CompileFriends( );
	CompileClan( );

	m_WindowData[W_FULL2].IsWindowChanged = true;
}