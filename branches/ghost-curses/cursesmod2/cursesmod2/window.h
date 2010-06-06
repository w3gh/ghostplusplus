#ifndef WINDOW_H
#define WINDOW_H

#include "widget.h"

/*
	CWindow initializes curses. It contains only one widget,
	which can be set using SetWidget(). CWindow is not the same
	as WINDOW in curses. It is the actual terminal window.

	Use setSize() to resize window. Size should be reasonable.
	Normally resizing crashes with aspect ratios smaller than 2:5
	and with too large y-values (85+), but setSize() avoids them.

	Call show() to show widgets. (The window is "hidden".)

	Use update() to update everything.

	Deleting CWindow deletes all the widgets and exits curses.

	Note: You can only have one instance of CWindow.
*/

// Terminal window
class CWindow : public CWidget
{
public:
	// Create new window
	CWindow();

	// Delete window and all of its widgets
	~CWindow();

	// Set main widget. Widget's parent is this window.
	void setWidget(CWidget *widget);

	// Resize window
	void setSize(uint width, uint height);

	// Set title
	void setTitle(const string &title);

	// Show widgets
	void show();

	// Hide widgets
	void hide();

	// Update window
	void update();

private:
	// Update input
	void updateInput();

	// Update mouse
	void updateMouse(int c);

	// Main Widget
	CWidget *_widget;

	// Mouse position
	CPoint _mousePos;

	// Pressed key from getch()
	int _key;

};

#endif