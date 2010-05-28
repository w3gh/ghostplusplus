/*

   Copyright [2008] [Trevor Hogan]

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

#ifndef MAP_H
#define MAP_H

#define MAPSPEED_SLOW			1
#define MAPSPEED_NORMAL			2
#define MAPSPEED_FAST			3

#define MAPVIS_HIDETERRAIN		1
#define MAPVIS_EXPLORED			2
#define MAPVIS_ALWAYSVISIBLE	3
#define MAPVIS_DEFAULT			4

#define MAPOBS_NONE				1
#define MAPOBS_ONDEFEAT			2
#define MAPOBS_ALLOWED			3
#define MAPOBS_REFEREES			4

#define MAPFLAG_TEAMSTOGETHER	1
#define MAPFLAG_FIXEDTEAMS		2
#define MAPFLAG_UNITSHARE		4
#define MAPFLAG_RANDOMHERO		8
#define MAPFLAG_RANDOMRACES		16

#define MAPOPT_HIDEMINIMAP				1 << 0
#define MAPOPT_MODIFYALLYPRIORITIES		1 << 1
#define MAPOPT_MELEE					1 << 2		// the bot cares about this one...
#define MAPOPT_REVEALTERRAIN			1 << 4
#define MAPOPT_FIXEDPLAYERSETTINGS		1 << 5		// and this one...
#define MAPOPT_CUSTOMFORCES				1 << 6		// and this one, the rest don't affect the bot's logic
#define MAPOPT_CUSTOMTECHTREE			1 << 7
#define MAPOPT_CUSTOMABILITIES			1 << 8
#define MAPOPT_CUSTOMUPGRADES			1 << 9
#define MAPOPT_WATERWAVESONCLIFFSHORES	1 << 11
#define MAPOPT_WATERWAVESONSLOPESHORES	1 << 12

#define MAPFILTER_MAKER_USER			1
#define MAPFILTER_MAKER_BLIZZARD		2

#define MAPFILTER_TYPE_MELEE			1
#define MAPFILTER_TYPE_SCENARIO			2

#define MAPFILTER_SIZE_SMALL			1
#define MAPFILTER_SIZE_MEDIUM			2
#define MAPFILTER_SIZE_LARGE			4

#define MAPFILTER_OBS_FULL				1
#define MAPFILTER_OBS_ONDEATH			2
#define MAPFILTER_OBS_NONE				4

#define MAPGAMETYPE_UNKNOWN0			1			// always set except for saved games?
// AuthenticatedMakerBlizzard = 1 << 3
// OfficialMeleeGame = 1 << 5
#define MAPGAMETYPE_SAVEDGAME			1 << 9
#define MAPGAMETYPE_PRIVATEGAME			1 << 11
#define MAPGAMETYPE_MAKERUSER			1 << 13
#define MAPGAMETYPE_MAKERBLIZZARD		1 << 14
#define MAPGAMETYPE_TYPEMELEE			1 << 15
#define MAPGAMETYPE_TYPESCENARIO		1 << 16
#define MAPGAMETYPE_SIZESMALL			1 << 17
#define MAPGAMETYPE_SIZEMEDIUM			1 << 18
#define MAPGAMETYPE_SIZELARGE			1 << 19
#define MAPGAMETYPE_OBSFULL				1 << 20
#define MAPGAMETYPE_OBSONDEATH			1 << 21
#define MAPGAMETYPE_OBSNONE				1 << 22

#include "gameslot.h"

//
// CMap
//

class CMap
{
public:
	CGHost *m_GHost;

private:
	bool m_Valid;
	QString m_CFGFile;
	QString m_MapPath;							// config value: map path
	QByteArray m_MapSize;						// config value: map size (4 bytes)
	QByteArray m_MapInfo;						// config value: map info (4 bytes) -> this is the real CRC
	QByteArray m_MapCRC;							// config value: map crc (4 bytes) -> this is not the real CRC, it's the "xoro" value
	QByteArray m_MapSHA1;						// config value: map sha1 (20 bytes)
	unsigned char m_MapSpeed;
	unsigned char m_MapVisibility;
	unsigned char m_MapObservers;
	unsigned char m_MapFlags;
	unsigned char m_MapFilterMaker;
	unsigned char m_MapFilterType;
	unsigned char m_MapFilterSize;
	unsigned char m_MapFilterObs;
	quint32 m_MapOptions;
	QByteArray m_MapWidth;						// config value: map width (2 bytes)
	QByteArray m_MapHeight;						// config value: map height (2 bytes)
	QString m_MapType;							// config value: map type (for stats class)
	QString m_MapMatchMakingCategory;			// config value: map matchmaking category (for matchmaking)
	QString m_MapStatsW3MMDCategory;				// config value: map stats w3mmd category (for saving w3mmd stats)
	QString m_MapDefaultHCL;						// config value: map default HCL to use (this should really be specified elsewhere and not part of the map config)
	quint32 m_MapDefaultPlayerScore;			// config value: map default player score (for matchmaking)
	QString m_MapLocalPath;						// config value: map local path
	bool m_MapLoadInGame;
	QByteArray m_MapData;							// the map data itself, for sending the map to players
	quint32 m_MapNumPlayers;
	quint32 m_MapNumTeams;
	QList<CGameSlot> m_Slots;

public:
	CMap( CGHost *nGHost );
	CMap( CGHost *nGHost, const CConfig &CFG, const QString &nCFGFile );
	~CMap( );

	bool GetValid( ) const					{ return m_Valid; }
	const QString &GetCFGFile( ) const				{ return m_CFGFile; }
	const QString &GetMapPath( ) const				{ return m_MapPath; }
	const QByteArray &GetMapSize( ) const			{ return m_MapSize; }
	const QByteArray &GetMapInfo( ) const			{ return m_MapInfo; }
	const QByteArray &GetMapCRC( )	const			{ return m_MapCRC; }
	const QByteArray &GetMapSHA1( ) const			{ return m_MapSHA1; }
	unsigned char GetMapSpeed( ) const		{ return m_MapSpeed; }
	unsigned char GetMapVisibility( ) const	{ return m_MapVisibility; }
	unsigned char GetMapObservers( ) const	{ return m_MapObservers; }
	unsigned char GetMapFlags( ) const		{ return m_MapFlags; }
	QByteArray GetMapGameFlags( ) const;
	quint32 GetMapGameType( ) const;
	quint32 GetMapOptions( ) const			{ return m_MapOptions; }
	unsigned char GetMapLayoutStyle( ) const;
	const QByteArray &GetMapWidth( ) const			{ return m_MapWidth; }
	const QByteArray &GetMapHeight( ) const		{ return m_MapHeight; }
	const QString &GetMapType( ) const				{ return m_MapType; }
	const QString &GetMapMatchMakingCategory( ) const	{ return m_MapMatchMakingCategory; }
	const QString &GetMapStatsW3MMDCategory( ) const	{ return m_MapStatsW3MMDCategory; }
	const QString &GetMapDefaultHCL( )	const			{ return m_MapDefaultHCL; }
	quint32 GetMapDefaultPlayerScore( )	{ return m_MapDefaultPlayerScore; }
	const QString &GetMapLocalPath( ) const			{ return m_MapLocalPath; }
	bool GetMapLoadInGame( ) const			{ return m_MapLoadInGame; }
	const QByteArray &GetMapData( ) const					{ return m_MapData; }
	quint32 GetMapNumPlayers( ) const			{ return m_MapNumPlayers; }
	quint32 GetMapNumTeams( ) const				{ return m_MapNumTeams; }
	const QList<CGameSlot> &GetSlots( ) const			{ return m_Slots; }

	void Load( const CConfig &CFG, const QString &nCFGFile );
	void CheckValid( );
	quint32 XORRotateLeft( unsigned char *data, quint32 length );
};

#endif
