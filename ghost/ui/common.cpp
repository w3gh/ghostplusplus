#include "common.h"

attr_t attribute(Color bgcolor, Color fgcolor, bool bold)
{
	attr_t result = 0;

	uint k = 0;
	for(uint i = 0; i <= 7; i++)
	{
		for(uint j = 0; j <= 7; j++)
		{
			if(i == fgcolor && j == bgcolor)
			{
				result = COLOR_PAIR(k);
				
				if(bold) result |= A_BOLD;

				return result;
			}
			k++;
		}
	}

	return result;
}

uint colorpair(Color bgcolor, Color fgcolor)
{
	uint k = 0;
	for(uint i = 0; i <= 7; i++)
	{
		for(uint j = 0; j <= 7; j++)
		{
			if(i == fgcolor && j == bgcolor)
			{
				return k;
			}
			k++;
		}
	}

	return 0;
}

CPoint::CPoint(uint x, uint y)
{
	set(x, y);
}

void CPoint::set(uint x, uint y)
{
	_x = x;
	_y = y;
}

uint CPoint::x()
{
	return _x;
}

uint CPoint::y()
{
	return _y;
}

CSize::CSize(uint width, uint height)
{
	set(width, height);
	_fixed = false;
}

void CSize::set(uint width, uint height)
{
	_width = width;
	_height = height;
}

uint CSize::width()
{
	return _width;
}

uint CSize::height()
{
	return _height;
}

void CSize::setFixed(bool enabled)
{
	_fixed = enabled;
}

bool CSize::fixed()
{
	return _fixed;
}