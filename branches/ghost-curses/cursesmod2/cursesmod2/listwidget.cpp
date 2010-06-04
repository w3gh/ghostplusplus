#include "listwidget.h"

#include "layout.h"

CListWidgetItem::CListWidgetItem(CWidget *parent)
{
	_parent = parent;
	_bold = false;
}

void CListWidgetItem::setText(const string &text)
{
	_text = text;
}

string CListWidgetItem::text()
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
	_multiLine = true;
}

void CListWidget::addItem(const string &text, Color fgcolor, Color bgcolor, bool bold)
{
	CListWidgetItem *item = new CListWidgetItem(this);
	item->setText(text);
	item->setBackgroundColor(bgcolor);
	item->setForegroundColor(fgcolor);
	item->setBold(bold);
	_items.push_back(item);

	if(_autoScroll || _items.size() < _size.height())
		_scroll++;
}

void CListWidget::update()
{
	if(_visible)
	{
		move_panel(_panel, _pos.y(), _pos.x());
		top_panel(_panel);
		wclear(_window);

		if(isFocused())
			box(_window, 0, 0);

		uint tw = _size.width() - _leftMargin - _rightMargin;
		uint th = _size.height() - _topMargin - _bottomMargin;

		for(uint i = 0; i < _items.size() && (_scroll >= th ? i < _scroll : i < th); i++)
		{
			attr_t a = attribute(_items[i]->backgroundColor(), _items[i]->foregroundColor(), _items[i]->bold());
			wattr_on(_window, a, 0);
			
			for(uint j = 0; j < _items[i]->text().size() && (_multiLine ? true : j < tw); j++)
				mvwaddch(_window, i + _topMargin, j + _leftMargin, _items[i]->text()[j]);

			wattr_off(_window, a, 0);
		}	
	}
}

void CListWidget::setScroll(uint scroll)
{
	_scroll = scroll;
}

uint CListWidget::scroll()
{
	return _scroll;
}

void CListWidget::setAutoScroll(bool enabled)
{
	_autoScroll = enabled;
}

void CListWidget::setMultiLine(bool enabled)
{
	_multiLine = enabled;
}