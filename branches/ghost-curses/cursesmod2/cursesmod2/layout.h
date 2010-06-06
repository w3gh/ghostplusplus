#ifndef LAYOUT_H
#define LAYOUT_H

#include "common.h"

class CWidget;

/*
	Layouts make it possible to place widgets in different
	styles (vertical, horizontal).

	You need to create a layout and add widgets into it.

	Methods setSize and setPosition have limitations so
	given values should be reasonable.
*/

// Layout
class CLayout
{
public:
	CLayout(CWidget *parent = 0);

	// Add widget to layout
	void addWidget(CWidget *widget);

	// Show layout
	void show();
	
	// Hide layout
	void hide();

	// Update layout
	void update(int c);

	// Get widgets count
	int count();

	// Set size
	void setSize(uint width, uint height);

	// Set position
	void setPosition(uint x, uint y);

protected:
	virtual void recursiveResize(uint from, uint to, uint width, uint height, uint x, uint y);

	CWidget *_parent;

	CSize _size;
	CPoint _pos;

	vector<CWidget *> _widgets;

};

// Vertical layout
class CVBoxLayout : public CLayout
{
public:
	CVBoxLayout(CWidget *parent = 0);

protected:
	void recursiveResize(uint from, uint to, uint width, uint height, uint x, uint y); //fixme?
};

// Horizontal layout
class CHBoxLayout : public CLayout
{
public:
	CHBoxLayout(CWidget *parent = 0);

protected:
	void recursiveResize(uint from, uint to, uint width, uint height, uint x, uint y); //fixme
};

#endif