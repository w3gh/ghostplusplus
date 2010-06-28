#include "widget.h"
#include "layout.h"
#include "forward.h"

// These two functions are used by GHost++/GProxy++.
string UTIL_Latin1ToUTF8( string &s );
string UTIL_UTF8ToLatin1( string &s );

// Forwards data. Implement this method somewhere, if you use this UI in projects other than GHost++/GProxy++.
void forward(CFwdData *data);

static uint widgetID = 0;

CWidget::CWidget(CWidget *parent, bool dummy)
{
	initialize(parent, dummy);
}

CWidget::CWidget(const string &name, int id)
{
	initialize();

	setName(name);
	setCustomID(id);
}

CWidget::~CWidget()
{
	SafeDelete(_fwdDataSelect);

	del_panel(_panel);
	delwin(_window);

	_window = 0;
	_panel = 0;

	SafeDelete(_layout);
}

void CWidget::initialize(CWidget *parent, bool dummy)
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
	
	_bgcolor = Black;
	_fgcolor = White;

	_layout = 0;
	_fwdDataSelect = 0;
	_customID = 0;

	_listenKeys = false;
	_changed = false;
	_visible = true;
	_bold = false;

	// Set default name.
	_name = "Widget";

	hide();
}

void CWidget::setName(const string &name)
{
	_name = name;

	_changed = true;
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

	_changed = true;
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

	_changed = true;
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

	_changed = true;
}

void CWidget::setFixedSize(uint width, uint height)
{
	_size.set(width, height);
	_size.setFixed(true);

	wresize(_window, height, width);
	replace_panel(_panel, _window);

	if(_layout)
		_layout->setSize(width, height);

	_changed = true;
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

	_changed = true;
}

void CWidget::show()
{
	show_panel(_panel);
	_visible = true;

	// Show subwidgets too
	if(_layout)
		_layout->show();

	_changed = true;
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

	_changed = true;
}

bool CWidget::focused()
{
#ifdef __PDCURSES__
	// Update mouse position. Everytime we use mouse wheel, positions go to -1 for some reason... this fixes it.
	request_mouse_pos();
	if(MOUSE_X_POS != -1 && MOUSE_Y_POS != -1)
		_mousePos.set(MOUSE_X_POS, MOUSE_Y_POS);
#else
	MEVENT event;
	if(getmouse(&event) == OK)
	{
		if(event.x != -1 && event.y != -1)
			_mousePos.set(event.x, event.y);
	}
#endif

	// Check if mouse cursor is inside the widget
	if(uint(_mousePos.y()) >= _pos.y() && uint(_mousePos.y()) < _pos.y() + _size.height() &&
	   uint(_mousePos.x()) >= _pos.x() && uint(_mousePos.x()) < _pos.x() + _size.width())
	{
		return true;
	}

	return false;
}

bool CWidget::visible()
{
	return _visible;
}

void CWidget::setBackgroundColor(Color color)
{
	_bgcolor = color;
	
	wbkgdset(_window, attribute(_bgcolor, _fgcolor, _bold));
	wattr_set(_window, _bold ? A_BOLD : A_NORMAL, colorpair(_bgcolor, _fgcolor), 0);

	_changed = true;
}

void CWidget::setForegroundColor(Color color)
{
	_fgcolor = color;

	wbkgdset(_window, attribute(_bgcolor, _fgcolor, _bold));
	wattr_set(_window, _bold ? A_BOLD : A_NORMAL, colorpair(_bgcolor, _fgcolor), 0);

	_changed = true;
}

void CWidget::setBold(bool bold)
{
	_bold = bold;

	wbkgdset(_window, attribute(_bgcolor, _fgcolor, _bold));
	wattr_set(_window, _bold ? A_BOLD : A_NORMAL, colorpair(_bgcolor, _fgcolor), 0);

	_changed = true;
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

void CWidget::forwardOnSelect(CFwdData *data)
{
	_fwdDataSelect = data;
}

void CWidget::select()
{
	if(_fwdDataSelect)
		forward(new CFwdData(_fwdDataSelect->_type, _fwdDataSelect->_text, _fwdDataSelect->_flag, _fwdDataSelect->_id));
}

void CWidget::forceChange()
{
	_changed = true;
}

void CWidget::listenKeys(bool enabled)
{
	_listenKeys = enabled;
}

CLabel::CLabel(CWidget *parent)
	: CWidget(parent)
{
	_centered = true;
}

CLabel::CLabel(const string &name, int id, Color fgcolor, Color bgcolor, bool bold)
	: CWidget(name, id)
{
	_centered = true;

	setForegroundColor(fgcolor);
	setBackgroundColor(bgcolor);
	setBold(bold);
}

CLabel::~CLabel()
{
}

void CLabel::update(int c)
{
	if(_visible && _changed)
	{
		move_panel(_panel, _pos.y(), _pos.x());
		top_panel(_panel);
		wclear(_window);

		uint tw = _size.width() - _leftMargin - _rightMargin;

		if(_centered)
			mvwaddnstr(_window, _topMargin, _leftMargin + (_text.size() < tw ? ( tw - _text.size() ) / 2 : 0), _text.c_str(), tw);
		else
			mvwaddnstr(_window, _topMargin, _leftMargin, _text.c_str(), tw);

		_changed = false;
	}
}


void CLabel::setText(const string &text)
{
	_text = text;

	_changed = true;
}

string CLabel::text()
{
	return _text;
}

void CLabel::setCentered(bool enabled)
{
	_centered = enabled;

	_changed = true;
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
	_fwdTypeEnter = (FwdType)0;
}

CTextEdit::CTextEdit(const string &name, int id, Color fgcolor, Color bgcolor, bool bold)
	: CLabel(name, id, fgcolor, bgcolor, bold)
{
	_requireFocused = false;
	_selectedHistory = 0;
	_fwdTypeEnter = (FwdType)0;
}

CTextEdit::~CTextEdit()
{
}

void CTextEdit::update(int c)
{
	if(_visible)
	{
		bool alwaysFocused = false;
#ifdef NO_MOUSE
		alwaysfocused = true; // If we have no mouse, we can't focus widgets so let's have it always focused.
#endif

		if((_requireFocused ? focused() : true) && (_parent ? _parent->focused() || alwaysFocused : true))
		{
			switch(c)
			{
			case ERR:
				break;
#ifdef __PDCURSES__
			case 3: // copy
				{
					string clipboard = UTIL_Latin1ToUTF8(_text);
					PDC_setclipboard(clipboard.c_str(), clipboard.length());
					break;
				}
			case 22: // paste
				{
					char *clipboard = NULL;
					long length = 0;

					if(PDC_getclipboard(&clipboard, &length) == PDC_CLIP_SUCCESS)
					{
						_text += string(clipboard, length);
						PDC_freeclipboard(clipboard);
					}
					break;
				}
#endif
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
#ifdef WIN32
			case PADENTER:
#endif
				if(!_text.empty())
				{
					_history.push_back(_text);
					_selectedHistory = _history.size();
					forward( new CFwdData(_fwdTypeEnter, _text, 0 ));
					_text.clear();
				}
				break;
#ifdef WIN32
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
#endif
			default:
				if(c >= 32 && c <= 255)
					_text += c;
				break;
			}
		}

		if(c != ERR)
		{
			move_panel(_panel, _pos.y(), _pos.x());
			top_panel(_panel);
			wclear(_window);

			string temp = UTIL_Latin1ToUTF8(_text);

			// print text
			mvwaddstr(_window, 0, 0, temp.c_str());
		}
	}
}

void CTextEdit::requireFocused(bool enabled)
{
	_requireFocused = enabled;
}

void CTextEdit::setForwardTypeOnEnter(FwdType type)
{
	_fwdTypeEnter = type;
}

CTabWidget::CTabWidget(CWidget *parent)
	: CWidget(parent)
{
	_currentIndex = -1;
	_bottom = false;

	CLayout *layout = new CVBoxLayout();
	setLayout(layout);
}

CTabWidget::CTabWidget(const string &name, int id, Color fgcolor, Color bgcolor, bool bold)
	: CWidget(name, id)
{
	_currentIndex = -1;
	_bottom = false;
	_listenKeys = false;

	CLayout *layout = new CVBoxLayout();
	setLayout(layout);

	setForegroundColor(fgcolor);
	setBackgroundColor(bgcolor);
	setBold(bold);
}

CTabWidget::~CTabWidget()
{
	for(vector<CWidget *>::iterator i = _widgets.begin(); i != _widgets.end(); i++)
		delete *i;
}

int CTabWidget::addTab(CWidget *page)
{
	_widgets.push_back(page);

	if(currentIndex() == -1)
		setCurrentIndex(_widgets.size() - 1);

	return _widgets.size() - 1;
}

void CTabWidget::removeTab(CWidget *page)
{
	for(vector<CWidget *>::iterator i = _widgets.begin(); i != _widgets.end(); i++)
	{
		if(*i == page)
		{
			setCurrentIndex(_widgets.size() - 2);
			delete *i;
			_widgets.erase(i);
			break;
		}
	}
}

void CTabWidget::setCurrentIndex(int index)
{
	_layout->hide();

	if(_layout->count() > 0)
		_layout->removeWidget(_widgets[_currentIndex]);

	_currentIndex = index;

	if(_currentIndex >= 0)
	{
		_layout->addWidget(_widgets[_currentIndex]);

		_widgets[_currentIndex]->select();
		
		if(_visible)
			_layout->show();
	}

	_changed = true;
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

uint CTabWidget::count()
{
	return _widgets.size();
}

void CTabWidget::setSize(uint width, uint height)
{
	_size.set(width, height);

	wresize(_window, height, width);
	replace_panel(_panel, _window);

	for(vector<CWidget *>::const_iterator i = _widgets.begin(); i != _widgets.end(); i++)
		(*i)->setSize(width, height - 1);

	_changed = true;
}

void CTabWidget::update(int c)
{
	if(_visible)
	{
		move_panel(_panel, _pos.y(), _pos.x());
		//top_panel(_panel);
		wclear(_window);

		if(_listenKeys)
		{
			if(c == KEY_LEFT)
			{
				if(currentIndex() - 1 >= 0)
					setCurrentIndex(currentIndex() - 1);
				else
					setCurrentIndex(count() - 1);
			}
			else if(c == KEY_RIGHT)
			{
				if(currentIndex() + 1 < int(count()))
					setCurrentIndex(currentIndex() + 1);
				else
					setCurrentIndex(0);
			}
		}

		uint tw = _size.width() - _leftMargin - _rightMargin;

		// Is left mouse button pressed?
#ifdef __PDCURSES__
		bool leftClick = focused() && Mouse_status.button[0] == BUTTON_PRESSED;
#else
		bool leftClick = false;
		MEVENT event;
		if(getmouse(&event) == OK)
			leftClick = focused() && (event.bstate & BUTTON1_PRESSED);
#endif

		int k = 0;
		for(uint i = 0; i < _widgets.size(); i++)
		{
			if(leftClick)
			{
				if(_mousePos.x() >= _pos.x() + _leftMargin + k &&
				   _mousePos.x() < _pos.x() + _leftMargin + k + _widgets[i]->name().size() &&
				   _mousePos.y() == (_bottom ? _pos.y() + _size.height() - 1 : _pos.y()))
				{
					// select tab
					setCurrentIndex(i);
				}
			}

			if(currentIndex() == i)
				wattr_on(_window, A_BOLD, 0);

			mvwaddstr(_window, _bottom ? _size.height() - 1 : 0, _leftMargin + k, _widgets[i]->name().c_str());

			if(_currentIndex == i)
				wattr_off(_window, A_BOLD, 0);

			k += _widgets[i]->name().size() + 1;
		}

		if(_layout)
		{
			if(_changed)
			{
				_layout->setSize(_size.width(), _size.height() - 1);
				_layout->setPosition(_pos.x(), _bottom ? _pos.y() : _pos.y() + 1);
				_changed = false;
			}

			_layout->update(c);
		}
	}
}

void CTabWidget::setTabPosition(bool bottom)
{
	_bottom = bottom;
}

CSeparatorWidget::CSeparatorWidget(CWidget *parent)
	: CLabel(parent)
{
	setMargins(0, 0, 0, 0);
}

CSeparatorWidget::CSeparatorWidget(const string &name, int id, Color fgcolor, Color bgcolor, bool bold)
	: CLabel(name, id, fgcolor, bgcolor, bold)
{
	setMargins(0, 0, 0, 0);
}

CSeparatorWidget::~CSeparatorWidget()
{

}

void CSeparatorWidget::update(int c)
{
	if(_visible && _changed)
	{
		move_panel(_panel, _pos.y(), _pos.x());
		top_panel(_panel);
		wclear(_window);

		uint tw = _size.width() - _leftMargin - _rightMargin;

		mvwhline(_window, _topMargin, _leftMargin, 0, tw);

		_changed = false;
	}
}