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
class CGameBase;
class CGame;
class CommandData;
class CGamePlayer;
class CBNET;

class CCommand {
 public:
   virtual int operator()(const QString &payload) = 0;
 };

class CGameCommand {
protected:
	CGHost *m_GHost;
public:
	CGameCommand(CGHost *ghost) : m_GHost(ghost) {}
	virtual int operator()(CGameBase *game, CGamePlayer *player, const QString &payload);
};

class CKickCommand : public CGameCommand
{
public:
	int operator()(const QString &payload)
	{

	}
};

template <typename FunctObj>
 void CallGameCommand(FunctObj f, const QString &payload)
 {
   f( payload );
 }

class CCommands : public QObject, public ICommandProvider
{
	Q_OBJECT
    Q_INTERFACES(ICommandProvider)
	Q_INTERFACES(IGHostPlugin)
private:
	CGHost *m_GHost;
	QStringList m_SupportedCommands;
public:
	CCommands();
	virtual QString GetName() const;
	QStringList GetCommands() const;
	virtual void PluginLoaded( CGHost *ghost, CConfig *cfg );
	virtual void OnGameCommand( CBaseGame *game, CGamePlayer *player, const CommandData &data );
	virtual void OnBNETCommand( CBNET *bnet, const QString &user, bool whisper, const CommandData &data );
};

#endif
