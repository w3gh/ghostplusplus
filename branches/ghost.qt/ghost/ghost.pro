# -------------------------------------------------
# Project created by QtCreator 2010-04-19T13:31:03
# -------------------------------------------------
QT += network \
	sql
QT -= gui
TARGET = ghost
CONFIG += console \
	debug
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += util.cpp \
	statsw3mmd.cpp \
	statsdota.cpp \
	stats.cpp \
	sqlite3.c \
	sha1.cpp \
	savegame.cpp \
	replay.cpp \
	packed.cpp \
	map.cpp \
	language.cpp \
	gpsprotocol.cpp \
	ghostdbsqlite.cpp \
	ghostdbmysql.cpp \
	ghostdb.cpp \
	ghost.cpp \
	gameslot.cpp \
	gameprotocol.cpp \
	gameplayer.cpp \
	game_base.cpp \
	game_admin.cpp \
	game.cpp \
	csvparser.cpp \
	crc32.cpp \
	config.cpp \
	commandpacket.cpp \
	bnlsprotocol.cpp \
	bnlsclient.cpp \
	bnetprotocol.cpp \
	bnet.cpp \
	bncsutilinterface.cpp \
	main.cpp \
	mpqarchive.cpp \
	mpqfile.cpp
OTHER_FILES += w3g_format.txt \
	w3g_actions.txt \
	ghost.vcproj
INCLUDEPATH += ../bncsutil/src \
	../zlib/include
LIBS += -L../bncsutil/src/bncsutil \
	-L"../bncsutil/vc8_build/Release MySQL" \
	-lbncsutil
macx {
	LIBS += -framework CoreFoundation -framework CoreServices
	LIBS += -L. -L/usr/lib -lbz2 -lz
}

HEADERS += util.h \
	statsw3mmd.h \
	statsdota.h \
	stats.h \
	sqlite3ext.h \
	sqlite3.h \
	sha1.h \
	savegame.h \
	replay.h \
	packed.h \
	next_combination.h \
	map.h \
	language.h \
	includes.h \
	gpsprotocol.h \
	ghostdbsqlite.h \
	ghostdbmysql.h \
	ghostdb.h \
	ghost.h \
	gameslot.h \
	gameprotocol.h \
	gameplayer.h \
	game_base.h \
	game_admin.h \
	game.h \
	csvparser.h \
	crc32.h \
	config.h \
	commandpacket.h \
	bnlsprotocol.h \
	bnlsclient.h \
	bnetprotocol.h \
	bnet.h \
	bncsutilinterface.h \
	mpqarchive.h \
	mpqfile.h
