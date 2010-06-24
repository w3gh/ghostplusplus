#include "listwidget.h"
#include "layout.h"

unsigned long toULong(int i)
{
	unsigned long result = i;
	if (i < 0) result += 256;
	return result;
}

CListWidgetItem::CListWidgetItem(CWidget *parent)
{
	_parent = parent;
	_bgcolor = Null;
	_fgcolor = Null;
	_bold = false;
}

CListWidgetItem::~CListWidgetItem()
{
}

void CListWidgetItem::setText(const string &text)
{
	_text = text;
}

const string &CListWidgetItem::text()
{
	return _text;
}

void CListWidgetItem::setBackgroundColor(Color color)
{
	_bgcolor = color;
}

void CListWidgetItem::setForegroundColor(Color color)
{
	_fgcolor = color;
}

void CListWidgetItem::setBold(bool bold)
{
	_bold = bold;
}

Color CListWidgetItem::backgroundColor()
{
	return _bgcolor;
}

Color CListWidgetItem::foregroundColor()
{
	return _fgcolor;
}

bool CListWidgetItem::bold()
{
	return _bold;
}

CListWidget::CListWidget(CWidget *parent)
	: CWidget(parent)
{
	scrollok(_window, true);
	_scroll = 0;
	_autoScroll = true;
}

CListWidget::CListWidget(const string &name, int id, Color fgcolor, Color bgcolor, bool bold)
	: CWidget(name, id)
{
	scrollok(_window, true);
	_scroll = 0;
	_autoScroll = true;

	setForegroundColor(fgcolor);
	setBackgroundColor(bgcolor);
	setBold(bold);
}

CListWidget::~CListWidget()
{
	for(vector<CListWidgetItem *>::iterator i = _items.begin(); i != _items.end(); i++)
		SafeDelete(*i);
}

void CListWidget::addItem(const string &text, Color fgcolor, Color bgcolor, bool bold)
{
	CListWidgetItem *item = new CListWidgetItem(this);
	item->setText(text);
	item->setBackgroundColor(bgcolor == Null ? _bgcolor : bgcolor);
	item->setForegroundColor(fgcolor == Null ? _fgcolor : fgcolor);
	item->setBold(bold);
	_items.push_back(item);

	if(_autoScroll || _items.size() < _size.height())
		_scroll++;

	_changed = true;

	if(_items.size() > 512)
	{
		_items.erase(_items.begin());
		
		if(_autoScroll && _scroll > 512)
			_scroll--;
	}
}

void CListWidget::updateItem(const string &text, Color fgcolor, Color bgcolor, bool bold)
{
	for(uint i = 0; i < _items.size(); i++)
	{
		if(_items[i]->text() == text)
		{
			_items[i]->setForegroundColor(fgcolor);
			_items[i]->setBackgroundColor(bgcolor);
			_items[i]->setBold(bold);
			break;
		}
	}

	_changed = true;
}

void CListWidget::updateItem(uint i, const string &text, Color fgcolor, Color bgcolor, bool bold)
{
	if(i < _items.size())
	{
		_items[i]->setText(text);
		_items[i]->setForegroundColor(fgcolor);
		_items[i]->setBackgroundColor(bgcolor);
		_items[i]->setBold(bold);

		_changed = true;
	}
}

void CListWidget::removeItem(const string &text, Color fgcolor)
{
	for(vector<CListWidgetItem *>::iterator i = _items.begin(); i != _items.end(); i++)
	{
		if((*i)->text() == text && (fgcolor == Null ? true : (*i)->foregroundColor() == fgcolor))
		{
			delete *i;
			_items.erase(i);

			if(_autoScroll)
				_scroll--;

			_changed = true;
			break;
		}
	}
}

void CListWidget::removeItem(uint i)
{
	uint k = 0;
	for(vector<CListWidgetItem *>::iterator j = _items.begin(); j != _items.end(); j++, k++)
	{
		if(k == i)
		{
			delete *j;
			_items.erase(j);

			if(_autoScroll)
				_scroll--;

			_changed = true;
			break;
		}
	}
}

int CListWidget::indexOf(const string &text)
{
	for(uint i = 0; i < _items.size(); i++)
	{
		if(_items[i]->text() == text)
		{
			return i;
		}
	}

	return -1;
}

CListWidgetItem* CListWidget::at(uint index)
{
	if(index < _items.size())
		return _items[index];

	return 0;
}

void CListWidget::removeItems()
{
	if(!_items.empty())
	{
		for(vector<CListWidgetItem *>::const_iterator j = _items.begin(); j != _items.end(); j++)
			delete *j;

		_items.clear();

		_scroll = 0;
		_changed = true;
	}
}

void CListWidget::update(int c)
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
					_scroll = _scroll < _items.size() ? _scroll + 4 : _scroll;
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

		if(_changed)
		{
			move_panel(_panel, _pos.y(), _pos.x());
			top_panel(_panel);
			wclear(_window);

			uint k = (_scroll > tw ? _scroll - tw : 0), n = 0;

			for(uint i = k; i < _items.size(); i++)
			{
				const string &message = _items[i]->text();

				waddch(_window, ' ');
				
				attr_t a = attribute(_items[i]->backgroundColor(), _items[i]->foregroundColor(), _items[i]->bold());
				wattr_on(_window, a, 0);

				n = 0;

				for(uint j = 0; j < message.size(); j++)
				{
					waddch(_window, toULong(message[j]));

					if(n++ >= tw - 4)
					{
						n = 0;
						wattr_off(_window, a, 0);
						waddch(_window, '\n');
						waddch(_window, ' ');
						wattr_on(_window, a, 0);
					}
				}

				wattr_off(_window, a, 0);

				if(k++ >= _scroll)
					break;

				if(i != _items.size() - 1)
					waddch(_window, '\n');
			}

			_changed = false;
		}
	}
}

void CListWidget::setAutoScroll(bool enabled)
{
	_autoScroll = enabled;
}
