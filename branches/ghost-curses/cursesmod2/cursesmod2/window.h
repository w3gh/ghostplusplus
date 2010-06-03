#ifndef WINDOW_H
#define WINDOW_H

#include "widget.h"

/*
	CWindow initializes curses. It contains only one widget,
	which can be set using SetWidget(). CWindow is not the same
	as WINDOW in curses. It is the actual terminal window.

	Use SetSize() to resize window. Size should be reasonable.
	Normally resizing crashes with aspect ratios smaller than 2:5
	and with too large y-values (85+), but SetSize() avoids them.

	Use Update() to update everything.

	Deleting CWindow deletes all the widgets and exits curses.

	Note: You can only have one instance of CWindow.
*/

// Terminal window
class CWindow : public CWidget
{
public:
	// Creates new window
	CWindow();

	// Deletes window and all of its widgets
	~CWindow();

	// Sets main widget. Widget's parent is this window.
	void setWidget(CWidget *widget);

	// Resizes window
	void setSize(uint width, uint height);

	void setTitle(const string &title);

	// Updates window
	void update();

private:
	// Main Widget
	CWidget *_widget;

};

#endif