
#include "ghost.h"
#include "bnet.h"
#include "userinterface.h"
#include "util.h"

CCurses :: CCurses( int nTermWidth, int nTermHeight )
{
	// Initialize RealmBuffers, 16 realms
	vector<string> temp;
	for ( uint32_t i = 0; i < 16; ++i )
		m_RealmBuffers.push_back(temp);

	m_ListCycleTimer = 0;
	m_ShowFriends = true;

	m_RealmId = 0;

	// Initialize curses and windows
	initscr( );
	clear( );
	noecho( );
	cbreak( );

	m_MainWindow = newwin( LINES / 2 - 1, COLS - 17, 0, 0 );
	m_RealmWindow = newwin( LINES / 2 - 3, COLS - 17, LINES / 2, 0 );
	m_InputWindow = newwin( 2, COLS, LINES - 2, 0 );
	m_CustomListWindow = newwin( LINES / 2 - 1, 16, 0, COLS - 16 );
	m_ChannelWindow = newwin( LINES / 2 - 3, 16, LINES / 2 - 1, COLS - 16 );

	m_BottomBorder = newwin( 1, COLS, LINES - 3, 0 );
	m_MiddleBorder = newwin( 1, COLS - 16, LINES / 2 - 1, 0 );
	m_RightBorder = newwin( LINES - 3, 1, 0, COLS - 17 );
	mvwhline( m_BottomBorder, 0, 0, 0, COLS );
	mvwhline( m_MiddleBorder, 0, 0, 0, COLS );
	mvwvline( m_RightBorder, 0, 0, 0, LINES );
	wrefresh( m_BottomBorder );
	wrefresh( m_MiddleBorder );
	wrefresh( m_RightBorder );

	scrollok( m_MainWindow, TRUE );
	scrollok( m_RealmWindow, TRUE );
	keypad( m_InputWindow, TRUE );
	scrollok( m_InputWindow, TRUE );
	nodelay( m_InputWindow, TRUE );
	
	// Initialize colors
	start_color();
	init_pair( 0, COLOR_WHITE, COLOR_BLACK );
	init_pair( 1, COLOR_GREEN, COLOR_BLACK );
	init_pair( 2, COLOR_RED, COLOR_BLACK );
	init_pair( 3, COLOR_CYAN, COLOR_BLACK );
	init_pair( 4, COLOR_YELLOW, COLOR_BLACK );
	init_pair( 5, COLOR_BLUE, COLOR_BLACK );
	
	// Change terminal size
	resize_term( nTermHeight, nTermWidth );
	Resize( );
}

CCurses :: ~CCurses( )
{
	endwin( );
}

void CCurses :: SetGHost( CGHost* nGHost )
{
	m_GHost = nGHost;

	if ( !m_GHost->m_BNETs.empty() )
		m_RealmAlias = m_GHost->m_BNETs[m_RealmId]->GetServerAlias();

	m_ListCycleTimer = GetTime( ) + 10; 
}

void CCurses :: SetMessageAttribute( WINDOW* window, string message, bool on, bool errorEnabled )
{
	// Sets color on messages.

	transform( message.begin( ), message.end( ), message.begin( ), (int(*)(int))tolower );

	attr_t attribute = A_WHITE;

	if ( message.size( ) > 3 )
	{
		if ( message.compare(1, 3, "map") == 0 )			attribute = A_WHITE;
		else if ( message.compare(1, 4, "info") == 0 )		attribute = A_CYAN;
		else if ( message.compare(1, 4, "bnet") == 0 )		attribute = A_YELLOW;
		else if ( message.compare(1, 4, "util") == 0 )		attribute = A_CYAN;
		else if ( message.compare(1, 4, "game") == 0 )		attribute = A_WHITE;
		else if ( message.compare(1, 5, "ghost") == 0 )		attribute = A_CYAN;
		else if ( message.compare(1, 5, "bnlsc") == 0 )		attribute = A_YELLOW;
		else if ( message.compare(1, 5, "error") == 0 )		attribute = A_RED;
		else if ( message.compare(1, 5, "emote") == 0 )		attribute = A_CYAN;
		else if ( message.compare(1, 6, "config") == 0 )	attribute = A_WHITE;
		else if ( message.compare(1, 7, "whisper") == 0 )	attribute = A_GREEN;
		
		if ( message.find("error") != -1 && errorEnabled )	attribute = A_RED;
	}

	if ( on )
		wattr_on( window, attribute, 0 );
	else
		wattr_off( window, attribute, 0 );
}

void CCurses :: SetNameAttribute( WINDOW* window, int flags, bool on, bool isChannelUser )
{
	// Sets color on names. Different coloring for channel names and friend names.

	attr_t attribute = A_WHITE;

	if ( isChannelUser )
	{
		switch ( flags )
		{
		case 1: attribute = A_BBLUE; break;
		case 2: attribute = A_YELLOW; break;
		case 4: attribute = A_CYAN; break;
		case 8: attribute = A_BBLUE; break;
		case 32: attribute = A_RED; break;
		}
	}
	else // isFriend
	{
		switch ( flags )
		{
		case 1: attribute = A_RED; break;
		case 2: attribute = A_WHITE; break;
		case 3: attribute = A_GREEN; break;
		case 4: attribute = A_YELLOW; break;
		case 5: attribute = A_CYAN; break;
		}
	}

	if ( on )
		wattr_on( window, attribute, 0 );
	else
		wattr_off( window, attribute, 0 );
}

void CCurses :: Draw ( )
{
	DrawMainWindow( );
	DrawRealmWindow( );
	DrawChannelWindow( );
	DrawCustomListWindow( );
	DrawInputWindow( );
}

void CCurses :: DrawMainWindow ( )
{
	if( m_MainWindowChanged )
	{
		wclear( m_MainWindow );
		wmove( m_MainWindow, 0, 0 );

		for( vector<string> :: iterator i = m_MainBuffer.begin( ); i != m_MainBuffer.end( ); i++ )
		{
			*i = UTIL_UTF8ToLatin1(*i);

			SetMessageAttribute( m_MainWindow, *i, true, true );	// colors on

			for( string :: iterator j = (*i).begin( ); j != (*i).end( ); j++ )
				waddch( m_MainWindow, UTIL_ToULong(*j) );

			if( i != m_MainBuffer.end( ) - 1 )
				waddch( m_MainWindow, '\n' );

			SetMessageAttribute( m_MainWindow, *i, false, true );	// colors off
		}

		wrefresh( m_MainWindow );
		m_MainWindowChanged = false;
	}
}

void CCurses :: DrawRealmWindow ( )
{
	if( m_RealmWindowChanged && !m_RealmBuffers.empty() )
	{
		wclear( m_RealmWindow );
		wmove( m_RealmWindow, 0, 0 );

		for( vector<string> :: iterator i = m_RealmBuffers[m_RealmId].begin( ); i != m_RealmBuffers[m_RealmId].end( ); i++ )
		{
			*i = UTIL_UTF8ToLatin1(*i);

			SetMessageAttribute( m_RealmWindow, *i, true, false );	// colors on

			for( string :: iterator j = (*i).begin( ); j != (*i).end( ); j++ )
				waddch( m_RealmWindow, UTIL_ToULong(*j) );

			if( i != m_RealmBuffers[m_RealmId].end( ) - 1 )
				waddch( m_RealmWindow, '\n' );

			SetMessageAttribute( m_RealmWindow, *i, false, false );	// colors off
		}

		wrefresh( m_RealmWindow );
		m_RealmWindowChanged = false;
	}
}

void CCurses :: DrawInputWindow ( )
{
	if( m_InputWindowChanged )
	{
		wclear( m_InputWindow );
		wmove( m_InputWindow, 0, 0 );

		m_InputBuffer = UTIL_UTF8ToLatin1(m_InputBuffer);

		for( string :: iterator i = m_InputBuffer.begin( ); i != m_InputBuffer.end( ); i++ )
			waddch( m_InputWindow, UTIL_ToULong(*i) );

		wrefresh( m_InputWindow );
		m_InputWindowChanged = false;
	}
}

void CCurses :: DrawChannelWindow ( )
{
	if( m_ChannelWindowChanged )
	{
		wclear( m_ChannelWindow );
		mvwhline( m_ChannelWindow, 0, 0, 0, 16 );
		mvwaddnstr( m_ChannelWindow, 1, m_ChannelName.size( ) < 16 ? ( 16 - m_ChannelName.size( ) ) / 2 : 0, m_ChannelName.c_str( ), 16 );
		mvwhline( m_ChannelWindow, 2, 0, 0, 16 );
		int y = 2;

		for( vector<pair<string, int> > :: iterator i = m_ChannelUsers.begin( ); i != m_ChannelUsers.end( ) && y++ < LINES / 2; i++ )
		{
			SetNameAttribute( m_ChannelWindow, (*i).second, true, true );	// colors on
			mvwaddnstr( m_ChannelWindow, y, 0, (*i).first.c_str( ), 16 );
			SetNameAttribute( m_ChannelWindow, (*i).second, false, true );	// colors off
		}

		wrefresh( m_ChannelWindow );
		m_ChannelWindowChanged = false;
	}
}

void CCurses :: DrawCustomListWindow ( )
{
	if( m_CustomListWindowChanged )
	{
		wclear( m_CustomListWindow );
		mvwhline( m_CustomListWindow, 1, 0, 0, 16 );
		int y = 1;

		if ( ( m_ShowFriends == true && m_Friends.size() > 0 ) || ( m_ShowFriends == false && m_Friends.size() > 0 && m_Clan.size() == 0 ) )
		{
			mvwaddnstr( m_CustomListWindow, 0, 9/2, "Friends", 16 );

			for( vector<pair<string, char> > :: iterator i = m_Friends.begin( ); i != m_Friends.end( ) && y++ < LINES / 2; i++ )
			{
				SetNameAttribute( m_CustomListWindow, (*i).second, true, false );	// colors on
				mvwaddnstr( m_CustomListWindow, y, 0, (*i).first.c_str( ), 16 );
				SetNameAttribute( m_CustomListWindow, (*i).second, false, false );	// colors off
			}
		}
		else if ( m_Clan.size() > 0 )
		{
			mvwaddnstr( m_CustomListWindow, 0, 6, "Clan", 16 );

			for( vector<pair<string, char> > :: iterator i = m_Clan.begin( ); i != m_Clan.end( ) && y++ < LINES / 2; i++ )
				mvwaddnstr( m_CustomListWindow, y, 0, (*i).first.c_str( ), 16 );
		}

		wrefresh( m_CustomListWindow );
		m_CustomListWindowChanged = false;
	}
}

void CCurses :: Resize( )
{
	resize_term( 0, 0 );
	clear();
    refresh();

	wresize( m_MainWindow, LINES / 2 - 1, COLS - 17 );
	wresize( m_BottomBorder, 1, COLS );
	wresize( m_RealmWindow, LINES / 2 - 3, COLS - 17 );
	wresize( m_MiddleBorder, 1, COLS );
	wresize( m_RightBorder, LINES - 3, 1 );
	wresize( m_InputWindow, 2, COLS );
	wresize( m_CustomListWindow, LINES / 2 - 2, 16 );
	wresize( m_ChannelWindow, LINES / 2 - 3, 16 );

	mvwin( m_MainWindow, 0, 0 );
	mvwin( m_RealmWindow, LINES / 2, 0 );
	mvwin( m_InputWindow, LINES - 2, 0 );
	mvwin( m_CustomListWindow, 0, COLS - 16 );
	mvwin( m_ChannelWindow, LINES / 2 - 1, COLS - 16 );

	mvwin( m_BottomBorder, LINES - 3, 0 );
	mvwin( m_MiddleBorder, LINES / 2 - 1, 0 );
	mvwin( m_RightBorder, 0, COLS - 17 );

	mvwhline( m_BottomBorder, 0, 0, 0, COLS );
	mvwhline( m_MiddleBorder, 0, 0, 0, COLS );
	mvwvline( m_RightBorder, 0, 0, 0, LINES );
	wrefresh( m_BottomBorder );
	wrefresh( m_MiddleBorder );
	wrefresh( m_RightBorder );

	m_MainWindowChanged = true;
	m_RealmWindowChanged = true;
	m_InputWindowChanged = true;
	m_CustomListWindowChanged = true;
	m_ChannelWindowChanged = true;

	Draw( );
}

void CCurses :: Print( string message, string realmAlias, bool toMainBuffer )
{
	if ( toMainBuffer )
	{
		m_MainBuffer.push_back( message );

		if( m_MainBuffer.size( ) > 100 )
			m_MainBuffer.erase( m_MainBuffer.begin( ) );

		m_MainWindowChanged = true;
	}
	else
	{
		uint32_t realmId = GetRealmId(realmAlias);
		m_RealmBuffers[realmId].push_back( message );

		if( m_RealmBuffers[realmId].size( ) > 100 )
			m_RealmBuffers[realmId].erase( m_RealmBuffers[realmId].begin( ) );

		m_RealmWindowChanged = true;
	}

	Draw( );
}

bool CCurses :: Update( )
{
	bool Quit = false;

	bool Connected = !m_GHost->m_BNETs.empty();
	if ( Connected ) Connected = IsConnected( m_RealmId, true );

	if ( GetTime( ) > m_ListCycleTimer && Connected )
	{
		m_ListCycleTimer = GetTime( ) + 10;
		m_ShowFriends = !m_ShowFriends;
		m_GHost->m_BNETs[m_RealmId]->RequestListUpdates( );
	}

	int c = wgetch( m_InputWindow );

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
			// tab
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
					resize_term( dimensions[1], dimensions[0] );
					Resize( );
				}
			}
			else if( Command == "/commands" )
			{
				CONSOLE_Print( ">>> /commands", false );
				CONSOLE_Print( "", false );
				CONSOLE_Print( "  In the GHost++ console:", false );
				CONSOLE_Print( "   !<command>               : GHost command. Replace '!' with defined trigger character.", false );
				CONSOLE_Print( "   /commands                : show command list", false );
				CONSOLE_Print( "   /realm <number>          : changes current channel view to another realm (if possible)", false );
				CONSOLE_Print( "   /resize <width> <height> : resizes console", false );
				CONSOLE_Print( "   /exit or /quit           : close GHost++", false );
				CONSOLE_Print( "   /r <message>             : reply to the last received whisper", false );
				CONSOLE_Print( "   /w <user> <message>      : whispers <message> to <user>", false );
				CONSOLE_Print( "", false );
			}
			else if( Command == "/exit" || Command == "/quit" )
			{
				Quit = true;
				break;
			}
			else if( Command.size( ) >= 4 && Command.substr( 0, 3 ) == "/r " )
			{
				if( !m_GHost->m_BNETs[m_RealmId]->GetReplyTarget( ).empty( ) )
				{
					m_InputBuffer = UTIL_Latin1ToUTF8( m_InputBuffer );
					m_GHost->m_BNETs[m_RealmId]->QueueChatCommand( m_InputBuffer.substr( 3 ), m_GHost->m_BNETs[m_RealmId]->GetReplyTarget( ), true );
				}
				else
					CONSOLE_Print( "[BNET] nobody has whispered you yet", false );
			}
			else if( Command.size( ) >= 2 && Command[0] == m_GHost->m_BNETs[m_RealmId]->GetCommandTrigger() )
			{
				m_InputBuffer = UTIL_Latin1ToUTF8( m_InputBuffer );
				m_GHost->m_BNETs[m_RealmId]->HiddenGhostCommand( m_InputBuffer );
			}
			else if( Command.size( ) >= 4 && Command.substr( 0, 7 ) == "/realm " )
			{
				uint32_t newRealm = atoi( m_InputBuffer.substr( 7 ).c_str( ) );

				if (newRealm >= 0 && newRealm < m_GHost->m_BNETs.size() )
				{
					m_RealmId = newRealm;
					m_RealmAlias = m_GHost->m_BNETs[m_RealmId]->GetServerAlias( );

					string currentChannel = m_GHost->m_BNETs[m_RealmId]->GetCurrentChannel( );
					m_GHost->m_BNETs[m_RealmId]->QueueEnterChat( );
					m_GHost->m_BNETs[m_RealmId]->QueueChatCommand( "/j " + currentChannel, true);
				}
			}
			else
			{
				m_InputBuffer = UTIL_Latin1ToUTF8( m_InputBuffer );
				m_GHost->m_BNETs[m_RealmId]->QueueChatCommand( m_InputBuffer, m_InputBuffer[0] == '/' );
			}

			m_InputBuffer.clear( );
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

		c = wgetch( m_InputWindow );
		m_InputWindowChanged = true;
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
		if ( realmId == m_RealmId && entry == false)
			return false;
		else
			return IsConnected( GetNextRealm( realmId ), false );
	}

	// If we are connected on some realm, this will be it.
	m_RealmId = realmId;

	return true;
}

void CCurses :: ChangeChannel( string channel, string realmAlias )
{
	if ( m_RealmAlias == realmAlias )
	{
		m_ChannelName = channel;
		m_ChannelWindowChanged = true;
	}
}

void CCurses :: AddChannelUser( string name, string realmAlias, int userFlags )
{
	if ( m_RealmAlias == realmAlias )
	{
		for( vector<pair<string, int> > :: iterator i = m_ChannelUsers.begin( ); i != m_ChannelUsers.end( ); i++ )
		{
			if( (*i).first == name )
				return;
		}

		m_ChannelUsers.push_back( pair<string, int>( name, userFlags ) );
		m_ChannelWindowChanged = true;
	}
}

void CCurses :: UpdateChannelUser( string name, string realmAlias, int userFlags )
{
	if ( m_RealmAlias == realmAlias )
	{
		for( vector<pair<string, int> > :: iterator i = m_ChannelUsers.begin( ); i != m_ChannelUsers.end( ); i++ )
		{
			if( (*i).first == name )
			{
				(*i).second = userFlags;
				m_ChannelWindowChanged = true;
				return;
			}
		}
	}
}

void CCurses :: RemoveChannelUser( string name, string realmAlias )
{
	if ( m_RealmAlias == realmAlias )
	{
		for( vector<pair<string, int> > :: iterator i = m_ChannelUsers.begin( ); i != m_ChannelUsers.end( ); )
		{
			if( (*i).first == name )
				i = m_ChannelUsers.erase( i );
			else
				i++;
		}

		m_ChannelWindowChanged = true;
	}
}

void CCurses :: RemoveChannelUsers( string realmAlias )
{
	if ( m_RealmAlias == realmAlias )
	{
		m_ChannelUsers.clear( );
		m_ChannelWindowChanged = true;
	}
}

void CCurses :: UpdateCustomList( string realmAlias )
{
	if ( m_RealmAlias == realmAlias )
	{
		m_Friends = m_GHost->m_BNETs[m_RealmId]->GetOnlineFriends( );
		m_Clan = m_GHost->m_BNETs[m_RealmId]->GetOnlineClans( );

		m_CustomListWindowChanged = true;
	}
}