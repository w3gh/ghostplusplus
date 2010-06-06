#include "window.h"
#include "listwidget.h"
#include "layout.h"

//fix: layout resizing (s-f-s, f-s-f, ...)

//todo: tabwidget, tabText=widgetName, show widget[i], hide others

int main()
{
	CWindow *window = new CWindow();
	window->setTitle("CursesMod-2.0");
	
	CWidget *mainWidget = new CWidget();
	CWidget *sub1 = new CWidget();
	CWidget *sub2 = new CWidget();

	CWidget *sub3 = new CWidget();
	sub3->setName("Channel");

	CWidget *sub4 = new CWidget();
	sub4->setName("Friends");

	CWidget *sub5 = new CWidget();
	sub5->setName("Clan");

	CListWidget *list = new CListWidget();
	list->setBackgroundColor(White);
	list->setForegroundColor(Black);
	list->addItem("Testing 1", Yellow);
	list->addItem("Testing 2", Green);
	list->addItem("Testing 3", Red);

	CLabel *label = new CLabel();
	label->setBackgroundColor(White);
	label->setForegroundColor(Black);
	label->setFixedSize(0, 3);	

	CTextEdit *edit = new CTextEdit();
	edit->setBackgroundColor(Cyan);
	edit->setFixedSize(0, 2);

	CMenuBar *menu = new CMenuBar();
	menu->setBackgroundColor(Cyan);
	menu->setFixedSize(0, 1);

	CTabWidget *tab = new CTabWidget();
	tab->setBackgroundColor(White);
	tab->setForegroundColor(Blue);
	tab->setFixedSize(22, 0);

	CListWidget *list2 = new CListWidget();
	list2->setBackgroundColor(White);
	list2->setForegroundColor(Black);
	list2->addItem("Testing 4", Magenta);
	list2->addItem("Testing 5", Red);
	list2->addItem("Testing 6", Blue);

	CListWidget *list3 = new CListWidget();
	list3->setBackgroundColor(White);
	list3->setForegroundColor(Black);
	list3->addItem("Testing 7", Red);
	list3->addItem("Testing 8", Red);
	list3->addItem("Testing 9", Green);

	CListWidget *list4 = new CListWidget();
	list4->setBackgroundColor(White);
	list4->setForegroundColor(Black);
	list4->addItem("Testing 10", Yellow);
	list4->addItem("Testing 11", Yellow);
	list4->addItem("Testing 12", Cyan);

	CLayout *layout0a = new CVBoxLayout(sub3);
	layout0a->addWidget(label);
	layout0a->addWidget(list2);

	CLayout *layout0b = new CVBoxLayout(sub4);
	layout0b->addWidget(list3);

	CLayout *layout0c = new CVBoxLayout(sub5);
	layout0c->addWidget(list4);

	tab->addTab(sub3);
	tab->addTab(sub4);
	tab->addTab(sub5);

	CLayout *layout1 = new CHBoxLayout(sub2);
	layout1->addWidget(list);
	layout1->addWidget(tab);

	CLayout *layout2 = new CVBoxLayout(sub1);
	layout2->addWidget(sub2);
	layout2->addWidget(edit);

	CLayout *layoutA = new CVBoxLayout(mainWidget);
	layoutA->addWidget(menu);
	layoutA->addWidget(sub1);

	window->setWidget(mainWidget);
	window->show();

	window->setSize(135, 43);

	string replyTarget = "testuser";
	string channelName = "The Void";

	label->setText(channelName);

	// some numbers so we can test scrolling
	for (int i = 10000; i < 10050; i++)
	{
		char *buf = new char[6];
		_itoa(i, buf, 16);
		list->addItem(buf);
		delete buf;
	}

	while(true)
	{
		window->update();

		if(edit->text() == "/r ")
			edit->setText("/w " + replyTarget + " ");
	}

	SafeDelete(window);

	return 0;
}