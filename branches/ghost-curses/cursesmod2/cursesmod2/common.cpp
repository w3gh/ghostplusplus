#include "common.h"

/*
0 = Black	8 = Gray
1 = Blue	9 = Light Blue
2 = Green	A = Light Green
3 = Aqua	B = Light Aqua
4 = Red		C = Light Red
5 = Purple	D = Light Purple
6 = Yellow	E = Light Yellow
7 = White	F = Bright White
*/
attr_t attribute(uint i)
{
	switch(i)
	{
	case 0: return COLOR_PAIR(0);
	case 1: return COLOR_PAIR(1);
	case 2: return COLOR_PAIR(2);
	case 3: return COLOR_PAIR(3);
	case 4: return COLOR_PAIR(4);
	case 5: return COLOR_PAIR(5);
	case 6: return COLOR_PAIR(6);
	case 7: return COLOR_PAIR(7);
	case 8: return COLOR_PAIR(7);
	case 9: return COLOR_PAIR(1) | A_BOLD;
	case 10: return COLOR_PAIR(2) | A_BOLD;
	case 11: return COLOR_PAIR(3) | A_BOLD;
	case 12: return COLOR_PAIR(4) | A_BOLD;
	case 13: return COLOR_PAIR(5) | A_BOLD;
	case 14: return COLOR_PAIR(6) | A_BOLD;
	case 15: return COLOR_PAIR(7) | A_BOLD;
	}

	return COLOR_PAIR(0);
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