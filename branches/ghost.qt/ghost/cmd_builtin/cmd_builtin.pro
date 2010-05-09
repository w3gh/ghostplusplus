TEMPLATE      = lib
CONFIG       += plugin dynamic #release
INCLUDEPATH  += ..
HEADERS       = commands.h
SOURCES       = commands.cpp
TARGET        = $$qtLibraryTarget(cmd_builtin)
DESTDIR       = ../build/Debug/plugins

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaint/plugins
#sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS basictools.pro
#sources.path = $$[QT_INSTALL_EXAMPLES]/tools/plugandpaintplugins/basictools
#INSTALLS += target sources

#symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
