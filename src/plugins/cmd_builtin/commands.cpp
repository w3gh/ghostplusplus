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
#include "bnet.h"
#include "game.h"
#include "language.h"
#include <QTextStream>
#include <iostream>

CCommands :: CCommands()
{
	m_SupportedCommands << "close";
}

QStringList CCommands :: GetCommands() const
{
	return m_SupportedCommands;
}

void CCommands :: PluginLoaded( CGHost *ghost, CConfig *cfg )
{
	m_GHost = ghost;
}

void CCommands :: OnGameCommand( CBaseGame *game, CGamePlayer *player, const CommandData &data )
{
	
}

QString CCommands :: GetName( ) const
{
	return "Built-in commands";
}

void CCommands :: OnBNETCommand( CBNET *bnet, const QString &user, bool whisper, const CommandData &data )
{
	if( data.GetCommand() == "close" && !data.GetCommand().isEmpty( ) && m_GHost->GetCurrentGame() )
	{
		if( !m_GHost->GetCurrentGame()->GetLocked( ) )
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
					m_GHost->GetCurrentGame()->CloseSlot( (unsigned char)( SID - 1 ), true );
			}
		}
		else
		{
			//m_GHost->m_Language->TheGameIsLockedBNET();
			bnet->QueueChatCommand( QString(), user, whisper );
		}
	}
}

Q_EXPORT_PLUGIN2(cmd_builtin, CCommands)
