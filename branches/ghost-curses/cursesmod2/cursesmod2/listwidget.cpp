#include "listwidget.h"

#include "layout.h"

CListWidgetItem::CListWidgetItem(CWidget *parent)
{
	_parent = parent;
}

void CListWidgetItem::setText(const string &text)
{
	_text = text;
}

string CListWidgetItem::text()
{
	return _text;
}

void CListWidgetItem::setColor(uint color)
{
	_color = color;
}

uint CListWidgetItem::color()
{
	return _color;
}

CListWidget::CListWidget(CWidget *parent)
	: CWidget(parent)
{
	scrollok(_window, true);
	_scroll = 0;
	_autoScroll = true;
	_multiLine = true;
}

void CListWidget::addItem(const string &text, uint color)
{
	CListWidgetItem *item = new CListWidgetItem(this);
	item->setText(text);
	item->setColor(color);
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
		waddch(_window, '\n');
		for(uint i = 0; i < _items.size() && (_scroll >= _size.height() - 2 ? i < _scroll : i < _size.height() - 2); i++)
		{
			wattr_on(_window, attribute(_items[i]->color()), 0);
			waddch(_window, ' ');
			
			for(uint j = 0; j < _items[i]->text().size() && (_multiLine ? true : j < _size.width() - 2); j++)
				waddch(_window, _items[i]->text()[j]);

			waddch(_window, '\n');
			wattr_off(_window, attribute(_items[i]->color()), 0);
		}
		box(_window, 0, 0);
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