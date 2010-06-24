#ifndef WIDGET_H
#define WIDGET_H

#include "common.h"

class CLayout;
class CFwdData;
enum FwdType;

// Widget
class CWidget
{
public:
	CWidget(CWidget *parent = 0, bool dummy = false);

	CWidget(const string &name, int id);

	~CWidget();

	// Set name
	void setName(const string &name);

	// Get name
	string name();

	// Set custom id
	void setCustomID(int id);

	// Get custom id
	int customID();

	// Set parent
	void setParent(CWidget *parent);

	// Set position
	void setPosition(uint x, uint y);

	// Get position
	CPoint pos();

	// Set layout
	void setLayout(CLayout *layout);

	// Get layout
	CLayout *layout();

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
	bool focused();

	// Is widget visible?
	bool visible();

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

	// Forwards data when widget selected on tab.
	void forwardOnSelect(CFwdData *data);

	// Selects widget and forwards data.
	void select();

	// Force _changed
	void forceChange();

private:
	void initialize(CWidget *parent = 0, bool dummy = false);

protected:
	string _name;
	int _customID;

	CSize _size;
	CPoint _pos;

	uint _topMargin;
	uint _bottomMargin;
	uint _leftMargin;
	uint _rightMargin;

	bool _changed;
	bool _visible;

	CWidget *_parent;
	CLayout *_layout;

	Color _bgcolor;
	Color _fgcolor;
	bool _bold;

	// Mouse position
	CPoint _mousePos;

	CFwdData *_fwdDataSelect;

	// Curses
	WINDOW *_window;
	PANEL *_panel;

};

// Label
class CLabel : public CWidget
{
public:
	CLabel(CWidget *parent = 0);

	CLabel(const string &name, int id, Color fgcolor = Null, Color bgcolor = Null, bool bold = false);

	~CLabel();

	// Update widget
	virtual void update(int c);

	// Set text
	void setText(const string &text);

	// Get text
	string text();

	// Set text centered
	void setCentered(bool enabled);

	// Get centered
	bool centered();

protected:
	string _text;

	bool _centered;
};

// TextEdit
class CTextEdit : public CLabel
{
public:
	CTextEdit(CWidget *parent = 0);

	CTextEdit(const string &name, int id, Color fgcolor = Null, Color bgcolor = Null, bool bold = false);

	~CTextEdit();

	// Update widget
	virtual void update(int c);

	// Require focus to write text?
	void requireFocused(bool enabled);

	// Set forward type, when pressed Enter-key.
	void setForwardTypeOnEnter(FwdType type);

protected:
	bool _requireFocused;

	uint _selectedHistory;

	vector<string> _history;

	FwdType _fwdTypeEnter;

};

// TabWidget
class CTabWidget : public CWidget
{
public:
	CTabWidget(CWidget *parent = 0);

	CTabWidget(const string &name, int id, Color fgcolor = Null, Color bgcolor = Null, bool bold = false);

	~CTabWidget();

	// Add tab page. The label is widget's name. Returns index.
	int addTab(CWidget *page);

	// Remove tab page. Changes tab index automatically.
	void removeTab(CWidget *page);

	// Set current index
	void setCurrentIndex(int index);

	// Get current index
	int currentIndex();

	// Get index of widget. Returns -1, if not found.
	int indexOf(CWidget *w);

	// Find first widget with custom id. Returns -1, if not found.
	int indexOf(int id);

	// Get widget at i.
	CWidget *at(uint i);

	// Get widgets' count.
	uint count();

	// Set size
	virtual void setSize(uint width, uint height);

	// Update widget
	virtual void update(int c);

	// If bottom is true, tab is shown bottom instead of top.
	void setTabPosition(bool bottom);

	// Listen keys (left arrow, right arrow)
	void listenKeys(bool enabled);

protected:
	vector<CWidget *> _widgets;

	bool _bottom;

	int _currentIndex;

	bool _listenKeys;
};

// Button
class CButton : public CLabel
{
public:
	CButton(CWidget *parent = 0);

	~CButton();

};

// Menu item
class CMenuItem
{
public:
	CMenuItem(CWidget *parent = 0);

	~CMenuItem();

};

// Menu
class CMenu : public CWidget
{
public:
	CMenu(CWidget *parent = 0);

	~CMenu();
	
};

#endif
