==================================
GHost++ Version 17.1 CursesMod-2.0
==================================

By SmokeMonster

========
Features
========

* Curses UI (user interface)
* Colored texts
* Latin-1 <=> UTF-8 conversion
* Tabs
* Mouse support (pdcurses)
* UI size configuration
* Game-tabs

Note: Supports only Latin-1 (Western European) or related character set. Changing keyboard layouts in runtime doesn't work.
      http://en.wikipedia.org/wiki/ISO/IEC_8859-1


=====
Usage
=====

1.) Configure GHost++

 a.) If you have already configured GHost++, just copy the new ghost.exe over the old one.

 b.) Otherwise configure your GHost++ normally. Check readme.txt or forums for more information. You may also use ghost dynamic configurator.


2.) You must add next five lines to your config file (ghost.cfg). Change values as you like. More information in default.cfg.

ui_enabled = 1
ui_width = 125
ui_height = 45
ui_splitsid = 1
ui_spliton = 0

(Note: These values cannot be changed using ghost dynamic configurator)

Important! Make sure to configure ui_width and ui_height properly or it might crash. More in default.cfg.


3.) After configuring GHost++:

 a.) Run ghost.exe

 b.) GHost++ will now log on to battle.net realms and joins into the chat channels, which you have configured.

 c.) Use left/right arrow keys or mouse to navigate tabs.

 d.) Type /commands to see the console commands.

     You can directly type GHost++ commands in game-tabs and server-tabs:
     !load
     !priv game_name

 e.) When you have hosted a game, join the game using Warcraft 3's Local Area Network menu.


User interface:

 - UI can be navigated with arrow keys, tab key and mouse + wheel.

 - Logs can be scrolled with page down/page up (useful on linux and ssh).

 - Up and down arrows browse sent message history.


=========
Compiling
=========

Check readme.txt.

ghost/         <-- new cursesmod v2.0
ghost-legacy/  <-- old cursesmod v1.12

Notes for Linux-users:
 
 You may compile this with ncurses. Ncurses version supports mouse (but not wheel).

 If you want to compile with pdcurses/X11, write "make XCURSES=1".

 If you are using this with a SSH client or you don't have/use a mouse then compile with "make NOMOUSE=1".
 This makes textedit-widgets always focused. Note that you can't use split view with this option.