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
	void setFixedSize(uint width, uint height);

	// Get size
	CSize size();

	// Hide widget
	virtual void hide();

	// Show widget
	virtual void show();

	// Update widget
	virtual void update(int c);

	// Set margins
	void setMargins(uint top, uint bottom, uint left, uint right);

	// If mouse cursor is over this widget or
	// its subwidgets, it returns true.
	bool isFocused();

	// Set background color
	void setBackgroundColor(Color color);

	// Set foreground color
	void setForegroundColor(Color color);

	// Set bold
	void setBold(bool bold);

	// Get background color
	Color backgroundColor();

	// Get foreground color
	Color foregroundColor();

	// Get bold
	bool bold();

protected:
	string _name;

	CSize _size;
	CPoint _pos;

	uint _topMargin;
	uint _bottomMargin;
	uint _leftMargin;
	uint _rightMargin;

	bool _selected;
	bool _visible;

	CWidget *_parent;
	CLayout *_layout;

	Color _bgcolor;
	Color _fgcolor;
	bool _bold;

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

	// Update widget
	virtual void update(int c);

};

// Table
class CTableWidget : public CWidget
{
public:
	CTableWidget(CWidget *parent = 0);

};

// Label
class CLabel : public CWidget
{
public:
	CLabel(CWidget *parent = 0);

	// Update widget
	virtual void update(int c);

	// Set text
	void setText(const string &text);

	// Get text
	string text();

protected:
	string _text;
};

// TextEdit
class CTextEdit : public CLabel
{
public:
	CTextEdit(CWidget *parent = 0);

	// Update widget
	virtual void update(int c);

	// Require focus to write text?
	void requireFocused(bool enabled);

protected:
	bool _requireFocused;
};

// Button
class CButton : public CLabel
{
public:
	CButton(CWidget *parent = 0);

};

#endif
