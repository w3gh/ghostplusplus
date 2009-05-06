========================================
DotaPod Automatic Game Host 0.6 for 1.23
========================================
Last updated: 5th May 2009

DotaPod Game Server (DPGS) is a port of GHost++, which is a port of the original GHost project to C++ (ported by Trevor Hogan).
The original GHost project can be found here: http://ghost.pwner.org/
You can compile and run GHost++ on Windows or Linux with this release although the code should be mostly portable to other operating systems such as OS X with only very minor changes.
Since it is written in native code you do not need to install the .NET framework on Windows or Mono on Linux.


==============
Required Files
==============

On windows, you need:
Visual C++ 2008 Redistributable Package (x86)

Download here:
http://www.microsoft.com/downloads/details.aspx?familyid=9B2DA534-3E03-4391-8A4D-074B9F2BC1BF&displaylang=en

=================
How to Start Fast
=================
Read: http://www.dotapod.com/thread-5212-1-1.html

1. After configuring, run ghost.exe.
2. Launch Warcraft, change your port to 6113 (or anything besides 6111 and 6112)
3. Join the 'DPGS Admin Game' (this is where you tell DPGS what to do, not for play!)
4. Enter '!password 12345' (default password)
5. Enter '!autohost 5 10 [SG] DotA 6.59d StarHub'
    - to host 5 games at one time max, with the name/message 'DotA 6.59d Hi!' that starts automatically when there are 10 people.

(UDP/TCP Port 6112 and 80 must be open and forwarded to your 
  computer.)

Note: You may need to wait up to 30 seconds before the game is created.
If game is not created, check that you have typed the right commands and make sure the name/message is not too long

That's it!
-sprion

===========
How to end?
===========

- To stop the current hosted game, type !unhost in the Admin Game.
- To stop all automatic games, type !autohost 

===========
Note!
===========

- Firewall/router - If your game is not listed on DotaPod.com , check that your firewall and/or router is
  configured correctly. UDP/TCP Port 6112 and 80 must be open and forwarded to your 
  computer.
- Always make sure there are no games in progress before shutting down DPGS.
  Use !getgames to see the games in progress and the users in them.
- Be a nice host!


For more information of DPGS, read the ghost's ghost_readme.txt file.

Please report any problems you have at http://www.dotapod.com/forum

-sprion