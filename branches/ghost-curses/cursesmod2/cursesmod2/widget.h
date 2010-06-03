#ifndef WIDGET_H
#define WIDGET_H

#include "common.h"

class CLayout;

// Widget
class CWidget
{
public:
	CWidget(CWidget *parent = 0, bool dummy = false);

	// Set parent
	void setParent(CWidget *parent);

	// Set position
	void setPosition(uint x, uint y);

	// Get position
	CPoint pos();

	// Set layout
	void setLayout(CLayout *layout);

	// Set size
	virtual void setSize(uint width, uint height);

	// Set fixed size
	virtual void setFixedSize(uint width, uint height);

	// Get size
	CSize size();

	// Hide widget
	virtual void hide();

	// Show widget
	virtual void show();

	// Update widget
	virtual void update();

	// If mouse cursor is over this widget or
	// it's sub widgets, it returns true.
	bool isFocused();

protected:
	string _name;

	CSize _size;
	CPoint _pos;

	int _topMargin;
	int _bottomMargin;
	int _leftMargin;
	int _rightMargin;

	bool _selected;
	bool _visible;

	CWidget *_parent;
	CLayout *_layout;

	// Curses
	WINDOW *_window;
	PANEL *_panel;

};

// Menu item
class CMenuItem
{
public:
	CMenuItem(CWidget *parent = 0);

};

// Menu
class CMenu : public CWidget
{
public:
	CMenu(CWidget *parent = 0);
	
};

// Menubar
class CMenuBar : public CWidget
{
public:
	CMenuBar(CWidget *parent = 0);

};

// Table
class CTableWidget : public CWidget
{
public:
	CTableWidget(CWidget *parent = 0);

};

// Button
class CButton : public CWidget
{
public:
	CButton(CWidget *parent = 0);

};

// Label
class CLabel : public CWidget
{
public:
	CLabel(CWidget *parent = 0);

};

// TextEdit
class CTextEdit : public CWidget
{
public:
	CTextEdit(CWidget *parent = 0);

};

#endif
