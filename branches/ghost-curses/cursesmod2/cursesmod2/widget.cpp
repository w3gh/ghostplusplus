#include "widget.h"

#include "layout.h"

static uint widgetID = 0;

CWidget::CWidget(CWidget *parent, bool dummy)
{
	setParent(parent);

	// If we haven't initialized curses yet, don't create a new curses window and panel
	if(!dummy)
	{
		_window = newwin(1, 1, 0, 0);
		_panel = new_panel(_window);
	}
	else
	{
		_window = 0;
		_panel = 0;
	}

	setMargins(1, 1, 1, 1);
	setBackgroundColor(Black);
	setForegroundColor(White);
	
	_layout = 0;

	_visible = true;
	_bold = false;

	// Set default name. (for debugging)
	char *buf = new char[16];
	_itoa(widgetID++, buf, 16);
	_name = "Widget ";
	_name += buf;
	delete buf;

	hide();
}

void CWidget::setName(const string &name)
{
	_name = name;
}

string CWidget::name()
{
	return _name;
}

void CWidget::setCustomID(int id)
{
	_customID = id;
}

int CWidget::customID()
{
	return _customID;
}

void CWidget::setParent(CWidget *parent)
{
	_parent = parent;
}

void CWidget::setPosition(uint x, uint y)
{
	_pos.set(x, y);

	if(_layout)
		_layout->setPosition(x, y);
}

CPoint CWidget::pos()
{
	return _pos;
}

void CWidget::setLayout(CLayout *layout)
{
	_layout = layout;
	_layout->setSize(_size.width(), _size.height());
	_layout->setPosition(_pos.x(), _pos.y());
}

CLayout *CWidget::layout()
{
	return _layout;
}

void CWidget::setSize(uint width, uint height)
{
	_size.set(width, height);

	wresize(_window, height, width);
	replace_panel(_panel, _window);

	if(_layout)
		_layout->setSize(width, height);
}

void CWidget::setFixedSize(uint width, uint height)
{
	_size.set(width, height);
	_size.setFixed(true);

	wresize(_window, height, width);
	replace_panel(_panel, _window);

	if(_layout)
		_layout->setSize(width, height);
}

CSize CWidget::size()
{
	return _size;
}

void CWidget::hide()
{
	hide_panel(_panel);
	_visible = false;

	// Hide subwidgets too
	if(_layout)
		_layout->hide();
}

void CWidget::show()
{
	show_panel(_panel);
	_visible = true;

	// Show subwidgets too
	if(_layout)
		_layout->show();
}

void CWidget::update(int c)
{
	if(_visible)
	{
		move_panel(_panel, _pos.y(), _pos.x());
		//top_panel(_panel);

		if(_layout)
			_layout->update(c);
	}
}

void CWidget::setMargins(uint top, uint bottom, uint left, uint right)
{
	_topMargin = top;
	_bottomMargin = bottom;
	_leftMargin = left;
	_rightMargin = right;
}

bool CWidget::focused()
{
	// Update mouse position. Everytime we use mouse wheel, positions go to -1 for some reason... this fixes it.
	if(MOUSE_X_POS != -1 && MOUSE_Y_POS != -1)
		_mousePos.set(MOUSE_X_POS, MOUSE_Y_POS);

	// Check if mouse cursor is inside the widget
	if(uint(_mousePos.y()) >= _pos.y() && uint(_mousePos.y()) < _pos.y() + _size.height() &&
	   uint(_mousePos.x()) >= _pos.x() && uint(_mousePos.x()) < _pos.x() + _size.width())
	{
		return true;
	}

	return false;
}

void CWidget::setBackgroundColor(Color color)
{
	_bgcolor = color;
	
	wbkgdset(_window, attribute(_bgcolor, _fgcolor, _bold));
	wattr_set(_window, _bold ? A_BOLD : A_NORMAL, colorpair(_bgcolor, _fgcolor), 0);
}

void CWidget::setForegroundColor(Color color)
{
	_fgcolor = color;

	wbkgdset(_window, attribute(_bgcolor, _fgcolor, _bold));
	wattr_set(_window, _bold ? A_BOLD : A_NORMAL, colorpair(_bgcolor, _fgcolor), 0);
}

void CWidget::setBold(bool bold)
{
	_bold = bold;

	wbkgdset(_window, attribute(_bgcolor, _fgcolor, _bold));
	wattr_set(_window, _bold ? A_BOLD : A_NORMAL, colorpair(_bgcolor, _fgcolor), 0);
}

Color CWidget::backgroundColor()
{
	return _bgcolor;
}

Color CWidget::foregroundColor()
{
	return _fgcolor;
}

bool CWidget::bold()
{
	return _bold;
}

CMenuBar::CMenuBar(CWidget *parent)
	: CWidget(parent)
{
}

void CMenuBar::update(int c)
{
	if(_visible)
	{
		move_panel(_panel, _pos.y(), _pos.x());
		top_panel(_panel);
		wclear(_window);

		uint tw = _size.width() - _leftMargin - _rightMargin;

		mvwaddnstr(_window, 0, _leftMargin, "", tw);
	}
}

CLabel::CLabel(CWidget *parent)
	: CWidget(parent)
{
	_centered = true;
}

void CLabel::update(int c)
{
	if(_visible)
	{
		move_panel(_panel, _pos.y(), _pos.x());
		top_panel(_panel);
		wclear(_window);

		if(focused())
		{
			wattr_on(_window, A_BOLD, 0);
			box(_window, 0, 0);
			wattr_off(_window, A_BOLD, 0);
		}

		uint tw = _size.width() - _leftMargin - _rightMargin;

		if(_centered)
			mvwaddnstr(_window, _topMargin, _leftMargin + (_text.size() < tw ? ( tw - _text.size() ) / 2 : 0), _text.c_str(), tw);
		else
			mvwaddnstr(_window, _topMargin, _leftMargin, _text.c_str(), tw);
	}
}


void CLabel::setText(const string &text)
{
	_text = text;
}

string CLabel::text()
{
	return _text;
}

void CLabel::setCentered(bool enabled)
{
	_centered = enabled;
}

bool CLabel::centered()
{
	return _centered;
}

CTextEdit::CTextEdit(CWidget *parent)
	: CLabel(parent)
{
	_requireFocused = false;
	_selectedHistory = 0;
}

void CTextEdit::update(int c)
{
	if(_visible)
	{
		move_panel(_panel, _pos.y(), _pos.x());
		top_panel(_panel);
		wclear(_window);

		if(_requireFocused ? focused() : true)
		{
			switch(c)
			{
			case ERR:
				break;
			case KEY_UP:
				if(!_history.empty())
				{
					if(_selectedHistory > 0)
						_text = _history[--_selectedHistory];
					else if(_selectedHistory == 0)
						_text = _history[0];
				}
				break;
			case KEY_DOWN:
				if(!_history.empty())
				{
					if(_selectedHistory < _history.size() - 1)
						_text = _history[++_selectedHistory];
					else if(_selectedHistory == _history.size() - 1)
						_text = _history[_history.size() - 1];
				}
				break;
			case 8:
			case 127:
			case KEY_BACKSPACE:
			case KEY_DC:
				if(!_text.empty())
					_text.erase(_text.end() - 1);
				break;
			case 27:
				_text.clear();
				break;
			case 10:
			case 13:
			case PADENTER:
				_history.push_back(_text);
				_selectedHistory = _history.size();
				_text.clear();
				// todo: forward text somewhere
				break;
			case PADSLASH:
				_text += '/';
				break;
			case PADSTAR:
				_text += '*';
				break;
			case PADMINUS:
				_text += '-';
				break;
			case PADPLUS:
				_text += '+';
				break;
			default:
				if(c >= 32 && c <= 255)
					_text += c;
				break;
			}
		}

		// print text
		mvwaddstr(_window, 0, 0, _text.c_str());
	}
}

CTabWidget::CTabWidget(CWidget *parent)
	: CWidget(parent)
{
	_currentIndex = -1;

	CLayout *layout = new CVBoxLayout();
	setLayout(layout);
}

int CTabWidget::addTab(CWidget *page)
{
	_widgets.push_back(page);

	if(currentIndex() == -1)
		setCurrentIndex(_widgets.size() - 1);

	return _widgets.size() - 1;
}

void CTabWidget::setCurrentIndex(int index)
{
	if(_layout->count() > 0)
		_layout->removeWidget(_widgets[_currentIndex]);

	_currentIndex = index;

	_layout->addWidget(_widgets[_currentIndex]);

	if(_visible)
		_layout->show();
}

int CTabWidget::currentIndex()
{
	return _currentIndex;
}

int CTabWidget::indexOf(CWidget *w)
{
	for(uint i = 0; i < _widgets.size(); i++)
	{
		if(_widgets[i] == w)
			return i; // found
	}

	return -1; // not found
}

int CTabWidget::indexOf(int id)
{
	for(uint i = 0; i < _widgets.size(); i++)
	{
		if(_widgets[i]->customID() == id)
			return i; // found
	}

	return -1; // not found
}

CWidget *CTabWidget::at(uint i)
{
	if(i < _widgets.size())
		return _widgets[i];
	
	return 0;
}

void CTabWidget::update(int c)
{
	if(_visible)
	{
		move_panel(_panel, _pos.y(), _pos.x());
		top_panel(_panel);
		wclear(_window);

		uint tw = _size.width() - _leftMargin - _rightMargin;

		// Is left mouse button pressed?
		bool leftClick = focused() && Mouse_status.button[0] == BUTTON_PRESSED;
		uint x = MOUSE_X_POS;
		uint y = MOUSE_Y_POS;

		int k = 0;
		for(uint i = 0; i < _widgets.size(); i++)
		{
			if(leftClick &&
				x > _pos.x() + _leftMargin + k &&
				x < _pos.x() + _leftMargin + k + _widgets[i]->name().size() &&
				y == _pos.y())
			{
				// select tab
				setCurrentIndex(i);
			}

			if(currentIndex() == i)
				wattr_on(_window, A_BOLD, 0);

			mvwaddnstr(_window, 0, _leftMargin + k, _widgets[i]->name().c_str(), tw);

			if(_currentIndex == i)
				wattr_off(_window, A_BOLD, 0);

			k += _widgets[i]->name().size() + 1;
		}
		
		if(_layout)
		{
			_layout->setSize(_size.width(), _size.height() - 1);
			_layout->setPosition(_pos.x(), _pos.y() + 1);
			_layout->update(c);
		}
	}
}
