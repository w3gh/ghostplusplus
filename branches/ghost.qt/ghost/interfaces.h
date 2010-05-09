/*
 
 Copyright [2010] [Lucas ROmero]
 
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

class CBNET;
class CGHost;
class CConfig;
class CGameBase;
class CGamePlayer;
class CCommandPacket;

/*class IGHostPlugin : public QObject
{
	Q_OBJECT
public:
	
	//virtual QString GetName() const = 0;
	virtual ~IGHostPlugin() {}
};*/

class IPacketHandler //: public IGHostPlugin
{
	virtual bool HandlePacket( CCommandPacket* packet, bool wasHandled ) = 0;
};

class ICommandProvider //: public IGHostPlugin
{
protected:
	//QHash<QString, 
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
		const QString *GetCommand( ) const { return &m_Command; }
		const QString *GetPayload( ) const { return &m_Payload; }
		
		bool IsAdmin( ) const { return m_Admin; }
		bool IsRootAdmin( ) const { return m_RootAdmin; }
		void SetAdmin( bool value ) { m_Admin = value; }
		void SetRootAdmin( bool value ) { m_RootAdmin = value; }
	};
	virtual void PluginLoaded( CGHost *ghost, CConfig *cfg ) = 0;
    virtual QStringList GetCommands() const = 0;
	virtual void OnGameCommand( CGamePlayer *player, const CommandData &data ) = 0;
	virtual void OnBNETCommand( CBNET *bnet, const QString &user, bool whisper, const CommandData &data ) = 0;
};

Q_DECLARE_INTERFACE(ICommandProvider, "org.GHost.ICommandProvider/1.0")
Q_DECLARE_INTERFACE(IPacketHandler, "org.GHost.IPacketHandler/1.0")

#endif