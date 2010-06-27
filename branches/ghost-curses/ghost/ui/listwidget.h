#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include "widget.h"

// List item
class CListWidgetItem
{
public:
	CListWidgetItem(CWidget *parent = 0);

	~CListWidgetItem();

	// Set text
	void setText(const string &text);

	// Get text
	const string &text();

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

	// Set NOCRLF
	void setNocrlf(bool enabled);

	// Is NOCRLF enabled?
	bool nocrlf();

protected:
	CWidget *_parent;

	string _text;
	Color _bgcolor;
	Color _fgcolor;
	bool _bold;
	bool _nocrlf;
};

// List
class CListWidget : public CWidget
{
public:
	CListWidget(CWidget *parent = 0);

	CListWidget(const string &name, int id, Color fgcolor = Null, Color bgcolor = Null, bool bold = false);

	~CListWidget();

	// Add item
	void addItem(const string &text, Color fgcolor = Null, Color bgcolor = Null, bool bold = false);

	// Update item
	void updateItem(const string &text, Color fgcolor = Null, Color bgcolor = Null, bool bold = false);

	// Update item
	void updateItem(uint i, const string &text, Color fgcolor = Null, Color bgcolor = Null, bool bold = false);

	// Remove item
	void removeItem(const string &text, Color fgcolor = Null);

	// Remove item
	void removeItem(uint i);

	// Remove items
	void removeItems();

	// Find first widget with text. Returns -1, if not found.
	int indexOf(const string &text);

	// Get item at index.
	CListWidgetItem* at(uint index);

	// Update widget
	virtual void update(int c);

	// Enable/disable automatic scroll
	void setAutoScroll(bool enabled);

protected:
	vector<CListWidgetItem *> _items;

	uint _count;
	uint _scroll;
	bool _autoScroll;

};

#endif
