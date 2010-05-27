CONFIG += staticlib create_prl
macx {
	CONFIG += x86_64 x86
	QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
}
DESTDIR = ../../lib
TEMPLATE = lib
