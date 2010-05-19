QT -= gui
TEMPLATE      = lib
CONFIG       += plugin debug_and_release #link_prl

LIBS         += -lghost -lbncsutil -lgmp -L/usr/lib -L../../../lib
macx {
  LIBS += -lz
}

INCLUDEPATH  += ../../libghost
DESTDIR       = ../../../bin/plugins