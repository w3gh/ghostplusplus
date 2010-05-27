/*
 
 Copyright [2010] [Lucas Romero]
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 
 CODE PORTED FROM THE ORIGINAL GHOST PROJECT: http://ghost.pwner.org/
 
 */

#ifndef INTERFACES_H
#define INTERFACES_H

#include <QtPlugin>
#include <QStringList>
#include <QString>
#include <QHash>
#include <QList>

class CBNET;
class CGHost;
class CConfig;
class CBaseGame;
class CGamePlayer;
class CCommandPacket;
//class CommandData;



class IGHostPlugin
{
protected:
	CGHost *m_GHost;
public:
	virtual ~IGHostPlugin( ) { }
	virtual QString GetName( ) const = 0;
	virtual void PluginLoaded( CGHost *ghost, CConfig */*cfg*/ ) { m_GHost = ghost; }
	virtual void AllPluginsLoaded( CGHost */*ghost*/, const QList<IGHostPlugin *> &/*plugins*/ ) { }
};

class IPacketHandler : public IGHostPlugin
{
public:
	virtual bool HandlePacket( CCommandPacket* packet, bool wasHandled ) = 0;
};

class IOutputListener : public IGHostPlugin
{
public:
	virtual void ConsoleOutput( const QString &message ) = 0;
};

class ILogPlugin : public IGHostPlugin
{
public:
	virtual void LogInfo( const QString &message ) = 0;
	virtual void LogWarning( const QString &message ) = 0;
	virtual void LogError( const QString &message ) = 0;
};

class ICommandProvider : public IGHostPlugin
{
public:
	class CommandData
	{
	public:
		CommandData( const QString &cmd, const QString &payload ) :
		m_Command( cmd ),
		m_Payload( payload ),
		m_Admin( false ),
		m_RootAdmin( false )
		{ /* empty constructor */ }
	protected:
		QString m_Command;
		QString m_Payload;
		bool m_Admin;
		bool m_RootAdmin;
	public:
		const QString &GetCommand( ) const { return m_Command; }
		const QString &GetPayload( ) const { return m_Payload; }
		
		bool IsAdmin( ) const { return m_Admin; }
		bool IsRootAdmin( ) const { return m_RootAdmin; }
		void SetAdmin( bool value ) { m_Admin = value; }
		void SetRootAdmin( bool value ) { m_RootAdmin = value; }
	};
	virtual ~ICommandProvider() {}
    virtual QStringList GetCommands() const = 0;
	virtual void OnGameCommand( CBaseGame *game, CGamePlayer *player, const CommandData &data ) = 0;
	virtual void OnBNETCommand( CBNET *bnet, const QString &user, bool whisper, const CommandData &data ) = 0;
	
};



Q_DECLARE_INTERFACE(IGHostPlugin, "org.GHost.IGHost/1.0")
//Q_DECLARE_INTERFACE(IOutputListener, "org.GHost.IOutputListener/1.0")
Q_DECLARE_INTERFACE(ILogPlugin, "org.GHost.ILogPlugin/1.0")
Q_DECLARE_INTERFACE(ICommandProvider, "org.GHost.ICommandProvider/1.0")
Q_DECLARE_INTERFACE(IPacketHandler, "org.GHost.IPacketHandler/1.0")

#endif
