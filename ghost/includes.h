/*

   Copyright [2008] [Trevor Hogan]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   CODE PORTED FROM THE ORIGINAL GHOST PROJECT: http://ghost.pwner.org/

*/

#ifndef INCLUDES_H
#define INCLUDES_H

// standard integer sizes for 64 bit compatibility

#ifdef WIN32
 #include "ms_stdint.h"
#else
 #include <stdint.h>
#endif

// STL

#include <iostream>

#include <QMap>
#include <QQueue>
#include <QString>
#include <QByteArray>
#include <QPair>
#include <QVector>

using namespace std;

typedef QPair<unsigned char,QString> PIDPlayer;

// time

uint32_t GetTime( );		// seconds
uint32_t GetTicks( );		// milliseconds

// output

void CONSOLE_Print( QString message );
void DEBUG_Print( QString message );
void DEBUG_Print( QByteArray b );

#endif
