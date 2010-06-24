#include "tablewidget.h"
#include "layout.h"

#include <cstdlib>

unsigned long toULong(int i);

int toInt(const string &str)
{
	return atoi(str.c_str());
}

CTableWidgetItem::CTableWidgetItem(CWidget *parent)
{
	_parent = parent;
	_bgcolor = Null;
	_fgcolor = Null;
	_bold = false;
}

CTableWidgetItem::~CTableWidgetItem()
{
}

void CTableWidgetItem::setText(const string &text)
{
	_text = text;
}

string CTableWidgetItem::text()
{
	return _text;
}

void CTableWidgetItem::setBackgroundColor(Color color)
{
	_bgcolor = color;
}

void CTableWidgetItem::setForegroundColor(Color color)
{
	_fgcolor = color;
}

void CTableWidgetItem::setBold(bool bold)
{
	_bold = bold;
}

Color CTableWidgetItem::backgroundColor()
{
	return _bgcolor;
}

Color CTableWidgetItem::foregroundColor()
{
	return _fgcolor;
}

bool CTableWidgetItem::bold()
{
	return _bold;
}

CTableWidgetRow::CTableWidgetRow(CWidget *parent)
{
	_parent = parent;
	_bgcolor = Null;
	_fgcolor = Null;
	_bold = false;
}

CTableWidgetRow::~CTableWidgetRow()
{
	for(vector<CTableWidgetItem *>::iterator i = _items.begin(); i != _items.end(); i++)
		SafeDelete(*i);
}

void CTableWidgetRow::addItem(const string &text)
{
	CTableWidgetItem *item = new CTableWidgetItem();
	item->setBackgroundColor(_bgcolor);
	item->setForegroundColor(_fgcolor);
	item->setBold(_bold);
	item->setText(text);

	_items.push_back(item);
}

void CTableWidgetRow::replaceWith(const vector<string> &row)
{
	for(uint i = 0; i < _items.size(); i++)
		_items[i]->setText(row[i]);
}

uint CTableWidgetRow::count()
{
	return _items.size();
}

CTableWidgetItem* CTableWidgetRow::at(uint index)
{
	if(index < _items.size())
		return _items[index];

	return 0;
}

void CTableWidgetRow::setBackgroundColor(Color color)
{
	_bgcolor = color;
}

void CTableWidgetRow::setForegroundColor(Color color)
{
	_fgcolor = color;
}

void CTableWidgetRow::setBold(bool bold)
{
	_bold = bold;
}

Color CTableWidgetRow::backgroundColor()
{
	return _bgcolor;
}

Color CTableWidgetRow::foregroundColor()
{
	return _fgcolor;
}

bool CTableWidgetRow::bold()
{
	return _bold;
}

CTableWidget::CTableWidget(CWidget *parent)
	: CWidget(parent)
{
	scrollok(_window, true);
	_scroll = 0;
	_autoScroll = true;

	_sortColumn = -1;
}

CTableWidget::CTableWidget(const string &name, int id, Color fgcolor, Color bgcolor, bool bold)
	: CWidget(name, id)
{
	scrollok(_window, true);
	_scroll = 0;
	_autoScroll = true;

	_sortColumn = -1;

	setForegroundColor(fgcolor);
	setBackgroundColor(bgcolor);
	setBold(bold);
}

CTableWidget::~CTableWidget()
{
	for(vector<CTableWidgetRow *>::iterator i = _rows.begin(); i != _rows.end(); i++)
		SafeDelete(*i);
}

void CTableWidget::setColumnHeaders(const vector<PairedColumnHeader> &headers)
{
	_headers = headers;
}

void CTableWidget::addRow(const vector<string> &row)
{
	if(!row.empty())
	{
		CTableWidgetRow *nrow = new CTableWidgetRow(this);
		nrow->setBackgroundColor(_bgcolor);
		nrow->setForegroundColor(_fgcolor);
		
		for(vector<string>::const_iterator i = row.begin(); i != row.end(); i++)
			nrow->addItem(*i);

		_rows.push_back(nrow);

		if(_autoScroll || _rows.size() < _size.height())
			_scroll++;

		_doSort = true;
		_changed = true;
	}
}

void CTableWidget::updateRow(const vector<string> &row)
{
	if(!row.empty())
	{
		for(vector<CTableWidgetRow *>::const_iterator i = _rows.begin(); i != _rows.end(); i++)
		{
			if((*i)->at(0)->text() == row[0])
			{
				(*i)->replaceWith(row);

				_doSort = true;
				_changed = true;
				break;
			}
		}
	}
}

void CTableWidget::removeRow(const string &text)
{
	for(vector<CTableWidgetRow *>::iterator i = _rows.begin(); i != _rows.end(); i++)
	{
		if((*i)->at(0)->text() == text)
		{
			delete *i;
			_rows.erase(i);

			if(_autoScroll)
				_scroll--;

			_changed = true;
			break;
		}
	}
}

void CTableWidget::removeRow(uint index)
{
	uint k = 0;
	for(vector<CTableWidgetRow *>::iterator j = _rows.begin(); j != _rows.end(); j++, k++)
	{
		if(k == index)
		{
			delete *j;
			_rows.erase(j);

			if(_autoScroll)
				_scroll--;

			_changed = true;
			break;
		}
	}
}

int CTableWidget::indexOf(const string &text)
{
	for(uint i = 0; i < _rows.size(); i++)
	{
		if(_rows[i]->at(0)->text() == text)
		{
			return i;
		}
	}

	return -1;
}

CTableWidgetRow* CTableWidget::at(uint index)
{
	if(index < _rows.size())
		return _rows[index];

	return 0;
}

void CTableWidget::removeRows()
{
	if(!_rows.empty())
	{
		for(vector<CTableWidgetRow *>::iterator j = _rows.begin(); j != _rows.end(); j++)
			delete *j;

		_rows.clear();

		_scroll = 0;
		_changed = true;
	}
}

void CTableWidget::update(int c)
{
	if(_visible)
	{
		uint tw = _size.width();
		uint th = _size.height();

		if(focused())
		{		
#ifdef __PDCURSES__
			// Mouse wheel scrolling
			if(c == KEY_MOUSE && _scroll >= th - 1)
			{
				if(Mouse_status.changes == MOUSE_WHEEL_DOWN)
				{
					_scroll = _scroll < _rows.size() ? _scroll + 4 : _scroll;
					_changed = true;
				}
				else if(Mouse_status.changes == MOUSE_WHEEL_UP)
				{
					_scroll = _scroll - 4 >= th ? _scroll - 4 : th - 1;
					_changed = true;
				}
			}
#endif
		}

		if(_listenKeys)
		{
			if( c == KEY_NPAGE )	// PAGE DOWN
			{
				_scroll = _scroll < _rows.size() ? _scroll + 4 : _scroll;
				_changed = true;
			}
			else if( c == KEY_PPAGE )	// PAGE UP
			{
				_scroll = _scroll - 4 >= th ? _scroll - 4 : th - 1;
				_changed = true;
			}
		}

		if(_changed)
		{
			move_panel(_panel, _pos.y(), _pos.x());
			//top_panel(_panel);
			wclear(_window);
			
			sort();

			uint k = (_scroll > tw ? _scroll - tw : 0), n = 0, m = 0, cw = 0;
			attr_t a = 0;
			bool stopHere = false;

			waddch(_window, '\n');

			for(vector<CTableWidgetRow *>::iterator i = _rows.begin() + k; i != _rows.end(); i++)
			{
				m = 0;
				stopHere = false;

				for(uint l = 0; l < _headers.size() && !stopHere; l++)
				{
					string message = (*i)->at(l)->text();
					cw = _headers[l].second;

					a = attribute((*i)->at(l)->backgroundColor(), (*i)->at(l)->foregroundColor(), (*i)->at(l)->bold());
					wattr_on(_window, a, 0);

					waddch(_window, ' ');

					m++;

					n = 0;

					for(uint j = 0; j < message.size(); j++)
					{
						waddch(_window, toULong(message[j]));

						if(m++ >= tw - 2)
						{
							stopHere = true;
							break;
						}

						if(n++ >= cw - 1 && l < _headers.size() - 1)
							break;
					}

					if(stopHere)
						break;

					for(; n < cw; n++)
					{
						if(m++ >= tw - 2)
						{
							stopHere = true;
							break;
						}
						waddch(_window, ' ');
					}

					wattr_off(_window, a, 0);
				}

				if(k++ >= _scroll - 1)
				{
					break;
				}

				if(i != _rows.end() - 1)
					waddch(_window, '\n');
			}
			
			m = 0;
			stopHere = false;
			a = attribute(White, Black);
			wattr_on(_window, a, 0);
			mvwaddch(_window, 0, 0, ' ');

			for(uint i = 0; i < _headers.size() && !stopHere; i++)
			{
				string &message = _headers[i].first;
				cw = _headers[i].second;

				if(i != 0)
					waddch(_window, ' ');

				m++;

				n = 0;

				for(uint j = 0; j < message.size(); j++)
				{
					waddch(_window, toULong(message[j]));

					if(m++ >= tw - 1)
					{
						stopHere = true;
						break;
					}

					if(n++ >= cw - 1 && i < _headers.size() - 1)
						break;
				}

				if(stopHere)
					break;

				for(; i == _headers.size() - 1 ? m < tw : n < cw; n++)
				{
					if(m++ >= tw - 1)
					{
						stopHere = true;
						break;
					}
					waddch(_window, ' ');
				}
			}

			wattr_off(_window, a, 0);

			_changed = false;
		}
	}
}

void CTableWidget::setAutoScroll(bool enabled)
{
	_autoScroll = enabled;
}

void CTableWidget::sortByColumn(int column)
{
	_sortColumn = column;
}

void CTableWidget::sort()
{
	if(_doSort && _sortColumn >= 0 && _sortColumn < int(_headers.size()))
	{
		// Insertion sort. Good at sorting small number of rows.

		uint j;
		for (uint i = 1; i < _rows.size(); i++)
		{
			j = i;
			while(j > 0 && toInt(_rows[j - 1]->at(_sortColumn)->text()) > toInt(_rows[j]->at(_sortColumn)->text()))
			{
				swap(_rows[j - 1], _rows[j]);
				j--;
			}
		}
	}
}
