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

	// Set color
	void setColor(uint color);

	// Get color
	uint color();

protected:
	CWidget *_parent;

	string _text;
	uint _color;
};

// List
class CListWidget : public CWidget
{
public:
	CListWidget(CWidget *parent = 0);

	void addItem(const string &text, uint color = 7);

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
