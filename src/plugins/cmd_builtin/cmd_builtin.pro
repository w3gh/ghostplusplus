! include( ../plugin.pri ) {
    error( Couldn't find plugin.pri! )
}

QT += network #sql

HEADERS       = commands.h
SOURCES       = commands.cpp
#TARGET        = $$qtLibraryTarget(cmd_builtin)
TARGET        = cmd_builtin


