#include "window.h"

CWindow::CWindow()
	: CWidget(0, true)
{
	// Initialize curses
	_window = initscr();
	_panel = new_panel(_window);
	refresh();

	_size.set(COLS, LINES);
	
	_widget = 0;

	start_color();
	init_pair( 0, COLOR_BLACK, COLOR_BLACK );
	init_pair( 1, COLOR_BLUE, COLOR_BLACK );
	init_pair( 2, COLOR_GREEN, COLOR_BLACK );
	init_pair( 3, COLOR_CYAN, COLOR_BLACK );
	init_pair( 4, COLOR_RED, COLOR_BLACK );
	init_pair( 5, COLOR_MAGENTA, COLOR_BLACK );
	init_pair( 6, COLOR_YELLOW, COLOR_BLACK );
	init_pair( 7, COLOR_WHITE, COLOR_BLACK );
}

CWindow::~CWindow()
{
	SafeDelete(_widget);

	// Exit curses
	endwin();
}

void CWindow::setWidget(CWidget *widget)
{
	_widget = widget;
	_widget->setParent(this);
	_widget->setSize(_size.width(), _size.height());
}

void CWindow::setSize(uint width, uint height)
{
	if(float(width)/float(height) <= 0.4f || height > 84)
		return;

	_size.set(width, height);

	resize_term(height, width);

	// Resize main widget too
	if(_widget)
		_widget->setSize(width, height);
}

void CWindow::setTitle(const string &title)
{
#ifdef __PDCURSES__
	PDC_set_title(title.c_str());
#endif
}

void CWindow::update()
{
	_widget->update();

	update_panels();
	doupdate();
}
