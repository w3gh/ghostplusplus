# -------------------------------------------------
# Project created by QtCreator 2010-04-19T13:31:03
# -------------------------------------------------
QT += network sql
QT -= gui
TARGET = ghost++
CONFIG += console debug_and_release link_prl
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp
OTHER_FILES += w3g_format.txt \
	w3g_actions.txt \
	ghost.vcproj
INCLUDEPATH +=	../ \
				../libghost
#	../zlib/include
LIBS +=	-L../../lib \
		-lghost -lbncsutil -lgmp

macx {
	LIBS += -framework CoreFoundation -framework CoreServices
	QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
	LIBS += -L/usr/lib -lbz2 -lz
}

#HEADERS += ghost.h

DESTDIR       = ../../bin
