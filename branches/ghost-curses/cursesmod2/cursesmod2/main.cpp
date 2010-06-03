#include "window.h"
#include "listwidget.h"
#include "layout.h"

int main()
{
	CWindow *window = new CWindow();
	window->setTitle("CursesMod-2.0");
 	window->setSize(135, 43);
	
	CWidget *mainWidget = new CWidget();
	CVBoxLayout *layout = new CVBoxLayout(mainWidget);

	CWidget *sub1 = new CWidget();
	CWidget *sub2 = new CWidget();
	CVBoxLayout *layout2 = new CVBoxLayout(sub1);
	CHBoxLayout *layout3 = new CHBoxLayout(sub2);

	CListWidget *list = new CListWidget();
	//list->setAutoScroll(false);
	//list->setMultiLine(false);
	list->setFixedSize(120, 20);
	list->addItem("Testing 1", 4);
	list->addItem("Testing 2", 3);
	list->addItem("Testing 3", 6);

	CListWidget *list2 = new CListWidget();
	list2->addItem("Testing 4", 3);
	list2->addItem("Testing 5", 5);
	list2->addItem("Testing 6", 2);

	CListWidget *list3 = new CListWidget();
	list3->setFixedSize(40, 5);
	list3->addItem("Testing 7", 5);
	list3->addItem("Testing 8", 6);
	list3->addItem("Testing 9", 3);

	CListWidget *list4 = new CListWidget();
	list4->setFixedSize(60, 5);
	list4->addItem("Testing 10", 6);
	list4->addItem("Testing 11", 3);
	list4->addItem("Testing 12", 2);

	layout->addWidget(sub1);

	layout3->addWidget(list3);
	layout3->addWidget(list4);
	
	layout2->addWidget(list);
	layout2->addWidget(list2);
	layout2->addWidget(sub2);

	window->setWidget(mainWidget);

	sub1->setLayout(layout2);
	sub2->setLayout(layout3);
	mainWidget->setLayout(layout);

	layout->setSize(120, 40);

	mainWidget->show();
	//sub2->hide();

	char *buf = new char[4];

	for(int i = 0; i < 100; ++i)
	{
		_itoa(i, buf, 10);
		sub1->setPosition(i, i / 5);
		list->addItem(buf);
		list2->addItem(buf);
		list3->addItem(buf);
		list4->addItem(buf);
		window->update();
	}

 	getch();

	SafeDelete(window);

	return 0;
}