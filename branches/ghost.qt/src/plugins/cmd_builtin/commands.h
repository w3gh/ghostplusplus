/*
 *  commands.h
 *  ghost
 *
 *  Created by Lucas on 08.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <QObject>
#include "interfaces.h"

class CGHost;
class CommandData;
class CGamePlayer;
class CBNET;

class CCommands : public QObject, public ICommandProvider
{
	Q_OBJECT
    Q_INTERFACES(ICommandProvider)
	Q_INTERFACES(IGHostPlugin)
private:
	CGHost *m_GHost;
public:
	virtual QString GetName() const;
	QStringList GetCommands() const;
	virtual void PluginLoaded( CGHost *ghost, CConfig *cfg );
	virtual void OnGameCommand( CBaseGame *game, CGamePlayer *player, const CommandData &data );
	virtual void OnBNETCommand( CBNET *bnet, const QString &user, bool whisper, const CommandData &data );
};

#endif