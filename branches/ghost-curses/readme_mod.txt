==============================
GHost++ Version 17.0 CursesMod
==============================

This mod brings pdcurses user interface to GHost++.

=====
Usage
=====

1.) If you have already a configured GHost++, just copy your ghost.cfg to this directory.
    You must add next four lines to your config file (ghost.cfg). Change values as you like. More information in default.cfg.

term_width = 135
term_height = 52
curses_enabled = 1
curses_splitview = 0

2.) Otherwise configure your GHost++ normally. Copy default.cfg and rename it ghost.cfg. More information in readme.txt.

3.) After configuring GHost++:
 a.) Run ghost.exe
 b.) GHost++ will now log on to battle.net realms and joins into the chat channels which you have configured.
 c.) Type /commands to see all the console commands.
     You can directly type GHost++ commands
     !load
     !priv game_name
 d.) When you have hosted a game, join the game using Warcraft 3's Local Area Network menu.

Note: Starting from v1.7 you cannot resize console buffer using Properties->Layout. Use "/resize" command or config-file instead.

Note 2: Supports only Latin-1 (Western European) or related character set. Changing keyboard layouts in runtime doesn't work.
        http://en.wikipedia.org/wiki/ISO/IEC_8859-1

=======
Changes
=======

The first version was basicly a copy-paste from GProxy++ code. You can see additional features and changes below.

 - Ascii<->UTF8 conversions
 - Colored texts
 - Own window for general messages and own window for realm-specific chat-channel messages
 - Console resizing
 - Copying text (Ctrl+C)
 - Friend List / Clan Lis
 - New UI: tabs
 - Mouse support

Version history:
----------------
v1.0
 - initial release
v1.1
 - fixed console resizing in win32 (resize through Properties->Layout)
 - added ctrl+c (copy)
v1.2
 - fixed bug: sending and receiving åäö characters
v1.3
 - UTF8 conversions
 - small compilation fix for linux (thanks to KentControl)
 - attempt to fix linux term resizing
v1.4
 - now UTF8 conversions should work :D (sorry!)
 - added support for multiple realms
 - added new window for realm-specific chat-messages
 - added colors
 - other fixes
v1.5
 - extended UTF8 conversion map from 160-255 to 128-255
 - friendlist/clanlist
 - bugfixes and crashfixes
 - /resize command
 - config: term size, disable curses
 - support /f msg
 - simplified code
v1.6
 - updated to svn r408
 - moved all curses-specific code to userinterface.h and userinterface.cpp
 - removed useless changes
 - swapped positions of channel users window and friend list
 - now most realm-specific messages go to the realm window (and not to the main window)
v1.7
 - updated to svn r418
 - fixed the Makefile
 - fixed a small bug with coloring in main window
 - moved channel name below the middle border
 - removed all references to "ascii"
 - removed Windows-specific code for buffer resizing (from now on use "/resize" command or config file)
v1.8 (svn r421-422)
 - new UI
 - new tabs
 - mouse support
 - bugfixes
v1.9 (svn r423-424)
 - applied changes from svn r419 trunk
 - scrolling support (pg up, pg down, mouse wheel)
 - moved some tabs (friends, clan, bans, admins) to the right corner

=========
Compiling
=========

You need Boost libraries and PDCurses in addition to included libraries.

1.) On Windows:

Get Boost libraries from the web or from GHost++ Google Code -page and compile with Visual Studio 2008 (or something else).

2.) On Linux:

PDCurses requires libxt-dev and libxaw7-dev (or similar). http://pdcurses.sourceforge.net/
Compile PDCurses succesfully (check the README-file in X11 directory, make, make install) or get it from your package manager (if it's there).
Remember to copy the .so and .a file to /usr/lib if they aren't there yet.

Get all the other dependencies needed in building bncsutil, stormlib and ghost. See each one's Makefile.
Compile bncsutil and stormlib first and copy the .so-files to /usr/lib, then compile ghost.
You may need to change -lcurses into -lXCurses or -lpdcurses (or whatever the PDCurses' .so/.a filename is called) in Makefile LFLAGS.
You might also want to look at userinterface.h for #include <curses.h> and change it to #include <xcurses.h> if needed.

Note: Compiling with ncurses instead of pdcurses might make the user interface look messy (on Linux).
