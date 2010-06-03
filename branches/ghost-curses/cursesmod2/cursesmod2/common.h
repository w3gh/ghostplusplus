#ifndef COMMON_H
#define COMMON_H

#include <panel.h>
#include <string>
#include <vector>
using namespace std;

// Safe Delete
#define SafeDelete(x) if((x)) { delete x; x = 0; }
#define SafeDeleteArray(x) if((x)) { delete[] x; x = 0; }

typedef unsigned int uint;

// Get attribute from number
attr_t attribute(uint i);

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
	void setFixed(bool enabled);

	// Get fixed.
	bool fixed();

private:
	uint _width;
	uint _height;

	bool _fixed;
};

#endif
