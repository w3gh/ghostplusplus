#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include "widget.h"

// Table item
class CTableWidgetItem
{
public:
	CTableWidgetItem(CWidget *parent = 0);

	~CTableWidgetItem();

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

class CTableWidgetRow
{
public:
	CTableWidgetRow(CWidget *parent = 0);

	~CTableWidgetRow();

	// Add item
	void addItem(const string &text);

	// Replace
	void replaceWith(const vector<string> &row);

	// Item count
	uint count();

	// Get item at index
	CTableWidgetItem* at(uint index);

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

private:
	CWidget *_parent;

	vector<CTableWidgetItem *> _items;

	Color _bgcolor;
	Color _fgcolor;
	bool _bold;

};

// Table
class CTableWidget : public CWidget
{
public:
	CTableWidget(CWidget *parent = 0);

	~CTableWidget();

	// Set headers for columns
	void setColumnHeaders(const vector<string> &row);

	// Set column widths
	void setColumnWidths(const vector<uint> &widths);

	// Add row
	void addRow(const vector<string> &row);

	// Update row containing row[0] in first column
	void updateRow(const vector<string> &row);

	// Remove row containing text in first column
	void removeRow(const string &text);

	// Remove item
	void removeRow(uint index);

	// Remove items
	void removeRows();

	// Find first widget with text. Returns -1, if not found.
	int indexOf(const string &text);

	// Get row at index.
	CTableWidgetRow* at(uint index);

	// Update widget
	virtual void update(int c);

	// Enable/disable automatic scroll
	void setAutoScroll(bool enabled);

	// Sorts ascendingly rows by column. Items must contain integer-values. To disable sorting, set column as -1.
	void sortByColumn(int column);

private:
	void sort();

	bool _autoScroll;

	uint _scroll;

	CTableWidgetRow *_headers;
	vector<CTableWidgetRow *> _rows;
	vector<uint> _columnWidths;

	bool _doSort;
	int _sortColumn;
};

#endif
