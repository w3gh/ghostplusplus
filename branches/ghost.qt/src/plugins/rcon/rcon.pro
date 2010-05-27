! include( ../plugin.pri ) {
    error( Couldn't find plugin.pri! )
}

QT += network #sql

HEADERS       = rcon.h
SOURCES       = rcon.cpp
TARGET        = rcon


