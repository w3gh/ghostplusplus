#include "window.h"
#include "listwidget.h"
#include "layout.h"

//fix: layout resizing (s-f-s, f-s-f, ...)

int main()
{
	CWindow *window = new CWindow();
	window->setTitle("CursesMod-2.0");
	
	CWidget *mainWidget = new CWidget();
	CWidget *sub1 = new CWidget();
	CWidget *sub2 = new CWidget();

	CListWidget *list = new CListWidget();
	list->addItem("Testing 1", Yellow);
	list->addItem("Testing 2", Green);
	list->addItem("Testing 3", Red);

	CListWidget *edit = new CListWidget();
	edit->setBackgroundColor(Cyan);
	edit->setFixedSize(0, 2);

	CListWidget *menu = new CListWidget();
	menu->setBackgroundColor(Cyan);
	menu->setFixedSize(0, 1);

	CListWidget *list2 = new CListWidget();
	list2->setFixedSize(20, 0);
	list2->setBackgroundColor(White);
	list2->setForegroundColor(Black);
	list2->addItem("Testing 4", Magenta, White);
	list2->addItem("Testing 5", Red, White);
	list2->addItem("Testing 6", Blue, White);

	CLayout *layout1 = new CVBoxLayout(sub2);
	layout1->addWidget(list);
	layout1->addWidget(edit);

	CLayout *layout2 = new CHBoxLayout(sub1);
	layout2->addWidget(sub2);
	layout2->addWidget(list2);

	CLayout *layoutA = new CVBoxLayout(mainWidget);
	layoutA->addWidget(menu);
	layoutA->addWidget(sub1);

	window->setWidget(mainWidget);
	window->show();

	window->setSize(135, 43);

	while(true)
		window->update();

	SafeDelete(window);

	return 0;
}