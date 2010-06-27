#ifndef COMMON_H
#define COMMON_H

#ifdef WIN32
#	include <panel.h>
#else
#	ifdef USE_XCURSES
#		include <xpanel.h>
#	else
#		include <panel.h>
#	endif
#endif

#include <string>
#include <vector>
using namespace std;

// Safe Delete
#define SafeDelete(x) if((x)) { delete x; x = 0; }
#define SafeDeleteArray(x) if((x)) { delete[] x; x = 0; }

typedef unsigned int uint;

#ifdef __PDCURSES__
enum Color
{
	Null = 99,
	Black = 0,
	Blue,
	Green,
	Cyan,
	Red,
	Magenta,
	Yellow,
	White
};
#else
enum Color
{
	Null = 99,
	Black = 0,
	Red,
	Green,
	Yellow,
	Blue,
	Magenta,
	Cyan,
	White
};
#endif

// Get attribute from bgcolor and fgcolor
attr_t attribute(Color bgcolor, Color fgcolor, bool bold = false);

// Get color pair from bgcolor and fgcolor
uint colorpair(Color bgcolor, Color fgcolor);

// Point (x, y)
class CPoint
{
public:
	CPoint(uint x = 0, uint y = 0);

	// Set point
	void set(uint x, uint y);

	// Get x
	uint x();

	// Get y
	uint y();

private:
	uint _x;
	uint _y;
};

// Size (width, height)
class CSize
{
public:
	CSize(uint width = 0, uint height = 0);

	// Set size
	void set(uint width, uint height);

	// Get width
	uint width();

	// Get height
	uint height();

	// Set fixed size. If enabled, vertical layout will not
	// resize height and horizontal layout will not resize width.
	// Note: layout will correct the size, if it's invalid.
	void setFixed(bool enabled);

	// Get fixed.
	bool fixed();

private:
	uint _width;
	uint _height;

	bool _fixed;
};

#endif
