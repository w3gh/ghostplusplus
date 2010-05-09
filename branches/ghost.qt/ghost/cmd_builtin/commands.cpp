/*
 *  commands.cpp
 *  ghost
 *
 *  Created by Lucas on 08.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "commands.h"
#include "ghost.h"
#include "config.h"
#include "game_base.h"
#include "bnet.h"
#include "language.h"
#include <QTextStream>

QStringList CCommands :: GetCommands() const
{
	QStringList bla;
	bla << "bla";
	return bla;
}

void CCommands :: PluginLoaded( CGHost *ghost, CConfig *cfg )
{
	m_GHost = ghost;
}

void CCommands :: OnGameCommand( CGamePlayer *player, const CommandData &data )
{
	
}

void CCommands :: OnBNETCommand( CBNET *bnet, const QString &user, bool whisper, const CommandData &data )
{
	if( *data.GetCommand() == "close" && !data.GetCommand()->isEmpty( ) && m_GHost->m_CurrentGame )
	{
		if( !m_GHost->m_CurrentGame->GetLocked( ) )
		{
			// close as many slots as specified, e.g. "5 10" closes slots 5 and 10
			
			QTextStream SS;
			SS << data.GetPayload();
			
			while( !SS.atEnd() )
			{
				quint32 SID;
				SS >> SID;
				
				if( SS.status() != QTextStream::Ok )
				{
					//CONSOLE_Print( "[BNET: " + bnet->GetServerAlias() + "] bad input to close command" );
					break;
				}
				else
					m_GHost->m_CurrentGame->CloseSlot( (unsigned char)( SID - 1 ), true );
			}
		}
		else
			bnet->QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), user, whisper );
	}
}

Q_EXPORT_PLUGIN2(cmd_builtin, CCommands)