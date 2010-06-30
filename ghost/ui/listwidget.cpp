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
	_nocrlf = false;
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

void CListWidgetItem::setNocrlf(bool enabled)
{
	_nocrlf = enabled;
}

bool CListWidgetItem::nocrlf()
{
	return _nocrlf;
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
	_count = 0;
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

	uint th = _size.height();

	if(!text.empty() && text[text.size() - 1] == '\3')
	{
		item->setNocrlf(true);
		item->setText(text.substr(0, text.size() - 1));
	}
	else
		item->setText(text);
	
	item->setBackgroundColor(bgcolor == Null ? _bgcolor : bgcolor);
	item->setForegroundColor(fgcolor == Null ? _fgcolor : fgcolor);
	item->setBold(bold);
	_items.push_back(item);

	if(!item->nocrlf())
		_count++;

	if(!item->nocrlf() && (_autoScroll || _scroll < th))
		_scroll++;

	_changed = true;

	if(_items.size() > 512)
	{
		if((*_items.begin())->nocrlf())
			_items.erase(_items.begin());

		_items.erase(_items.begin());
		
		if(!item->nocrlf() && (_autoScroll && _scroll > 512))
			_scroll--;

		if(!item->nocrlf())
			_count--;
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
			if(!(*i)->nocrlf())
				_count--;

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
			if(!(*j)->nocrlf())
				_count--;

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

		_count = 0;
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
			if(c == KEY_MOUSE && _count > th)
			{
				if(Mouse_status.changes == MOUSE_WHEEL_DOWN)
				{
					_scroll += 4;
					_changed = true;
				}
				else if(Mouse_status.changes == MOUSE_WHEEL_UP && _scroll >= 4)
				{
					_scroll -= 4;
					_changed = true;
				}
			}
#endif
		}

		if((_listenKeys || focused()) && _count > th)
		{
			if(c == KEY_NPAGE)	// PAGE DOWN
			{
				_scroll += 4;
				_changed = true;
			}
			else if(c == KEY_PPAGE && _scroll >= 4)	// PAGE UP
			{
				_scroll -= 4;
				_changed = true;
			}
		}

		if(_changed)
		{
			move_panel(_panel, _pos.y(), _pos.x());
			top_panel(_panel);
			wclear(_window);

			if((_count < th && _scroll < _count) || (_count > th && _scroll > _count)) _scroll = _count;
			if(_count > th && _scroll < th) _scroll = th;

			uint b = 0, prevn = 0, n = 0;

			for(uint i = 0; i < _items.size() && i < _scroll + b; i++)
			{
				if(_items[i]->nocrlf()) b++;

				n = 0;
				
				if(i > 0 && _items[i-1]->nocrlf())
					n = prevn;

				const string &message = _items[i]->text();

				for(uint j = 0; j < _leftMargin; j++)
					waddch(_window, ' ');

				attr_t a = attribute(_items[i]->backgroundColor(), _items[i]->foregroundColor(), _items[i]->bold());
				wattr_on(_window, a, 0);

				n++;

				for(uint j = 0; j < message.size(); j++, n++)
				{
					if(n >= tw - _leftMargin - _rightMargin)
					{
						n = 0;
						wattr_off(_window, a, 0);
						waddch(_window, '\n');
						for(uint j = 0; j < _leftMargin; j++)
							waddch(_window, ' ');
						wattr_on(_window, a, 0);
					}

					waddch(_window, toULong(message[j]));
				}

				wattr_off(_window, a, 0);

				if(i < _scroll + b - 1 && i != _items.size() - 1 && !_items[i]->nocrlf())
					waddch(_window, '\n');

				prevn = n;
			}

			_changed = false;
		}
	}
}

void CListWidget::setAutoScroll(bool enabled)
{
	_autoScroll = enabled;
}
