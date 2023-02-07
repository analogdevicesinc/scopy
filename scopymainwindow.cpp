#include "scopymainwindow.h"
#include "ui_scopymainwindow.h"
#include "scopyhomepage.h"
#include <QLabel>

using namespace adiscope;
ScopyMainWindow::ScopyMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScopyMainWindow)
{	
	ui->setupUi(this);
	 auto tb = ui->wToolBrowser;
	 auto ts = ui->wsToolStack;
	 auto tm = tb->getToolMenu();	 

	 connect(tm,&ToolMenu::requestAttach,ts,&ToolStack::attachTool);
	 connect(tm,&ToolMenu::requestDetach,ts,&ToolStack::detachTool);
	 connect(ts,&ToolStack::attachSuccesful,tm,&ToolMenu::attachSuccesful);
	 connect(ts,&ToolStack::detachSuccesful,tm,&ToolMenu::detachSuccesful);

	 connect(tb,&ToolBrowser::requestTool,ts, &ToolStack::show);
//	 connect(tb,&ToolBrowser::detach,ts, &ToolStack::showTool);
	 ts->add("home", new ScopyHomePage());
	 ts->add("about", new QLabel("about scopy ... "));

}

ScopyMainWindow::~ScopyMainWindow()
{
    delete ui;
}

