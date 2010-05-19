# -------------------------------------------------
# Project created by QtCreator 2010-04-19T13:31:03
# -------------------------------------------------
! include( ../lib.pri ) {
    error( Couldn't find lib.pri! )
}

QT -= gui
TARGET = bncsutil
INCLUDEPATH += ../




SOURCES +=	bsha1.cpp cdkeydecoder.cpp checkrevision.cpp decodekey.cpp file.cpp libinfo.cpp oldauth.cpp \
			nls.c pe.c sha1.c stack.c

LIBS +=		-L../../lib \
			-lgmp
#macx {
#	LIBS += -framework CoreFoundation -framework CoreServices
#	LIBS += -L. -L/usr/lib -lbz2 -lz
#}

HEADERS += 
