#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include "widget.h"

// List item
class CListWidgetItem
{
public:
	CListWidgetItem(CWidget *parent = 0);

	// Set text
	void setText(const string &text);

	// Get text
	string text();

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
	CWidget *_parent;

	string _text;
	Color _bgcolor;
	Color _fgcolor;
	bool _bold;
};

// List
class CListWidget : public CWidget
{
public:
	CListWidget(CWidget *parent = 0);

	void addItem(const string &text, Color fgcolor = White, Color bgcolor = Black, bool bold = false);

	// Update widget
	virtual void update();

	// Set scroll
	void setScroll(uint scroll);

	// Get scroll
	uint scroll();

	// Enable/disable automatic scroll
	void setAutoScroll(bool enabled);

	// Enable multiline
	void setMultiLine(bool enabled);

protected:
	vector<CListWidgetItem *> _items;

	uint _scroll;
	bool _autoScroll;
	bool _multiLine;

};

#endif
