QT -= gui
TEMPLATE      = lib
CONFIG       += plugin debug_and_release #link_prl

LIBS         += -lghost -lbncsutil -lgmp -L/usr/lib -L../../../lib
macx {
  LIBS += -lz
CONFIG += x86_64 x86
QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
}

INCLUDEPATH  += ../../libghost
DESTDIR       = ../../../bin/plugins
