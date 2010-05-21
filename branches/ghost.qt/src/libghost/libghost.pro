# -------------------------------------------------
# Project created by QtCreator 2010-04-19T13:31:03
# -------------------------------------------------
! include( ../lib.pri ) {
    error( Couldn't find lib.pri! )
}

QT += network sql
QT -= gui
TARGET = ghost
CONFIG += debug_and_release
QMAKE_CXXFLAGS += -fvisibility=hidden
LFLAGS += -fvisibility=hidden
#CONFIG -= app_bundle
INCLUDEPATH +=	.. \
				../../include \
				../../include/mysql
LIBS += -L../../src/lib
LIBS += -lbncsutil -lgmp
		
macx {
	LIBS += -framework CoreFoundation -framework CoreServices
	LIBS += -L. -L/usr/lib -lbz2 -lz
	CONFIG += x86_64 x86
}

#!isEmpty( $$MY_LIBRARY_DEST_PATH ) {
# QMAKE_POST_LINK = make install
# list_of_files_to_install.files = $TARGET.$TARGET_EXT
# list_of_files_to_install.path = $$MY_LIBRARY_DEST_PATH
# INSTALLS += list_of_files_to_install
#}



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
	mpqarchive.cpp \
	mpqfile.cpp
#OTHER_FILES += w3g_format.txt \
#	w3g_actions.txt \
#	ghost.vcproj

HEADERS += interfaces.h \
	util.h \
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
	ghost_p.h \
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
