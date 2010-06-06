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
	CWidget *sub3 = new CWidget();

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

	CListWidget *list2 = new CListWidget();
	list2->setBackgroundColor(White);
	list2->setForegroundColor(Black);
	list2->addItem("Testing 4", Magenta);
	list2->addItem("Testing 5", Red);
	list2->addItem("Testing 6", Blue);

	sub3->setFixedSize(20, 0);

	CLayout *layout0 = new CVBoxLayout(sub3);
	layout0->addWidget(label);
	layout0->addWidget(list2);

	CLayout *layout1 = new CHBoxLayout(sub2);
	layout1->addWidget(list);
	layout1->addWidget(sub3);

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

	while(true)
	{
		window->update();

		if(edit->text() == "/r ")
			edit->setText("/w " + replyTarget + " ");
	}

	SafeDelete(window);

	return 0;
}