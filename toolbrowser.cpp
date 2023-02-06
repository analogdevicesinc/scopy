#include "toolbrowser.h"
#include "ui_toolbrowser.h"
#include <QDebug>
#include "toolmenuitem.h"

using namespace adiscope;

ToolBrowser::ToolBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolBrowser)
{
    ui->setupUi(this);

    ToolMenu *tm = ui->wToolMenu;
    tm->addTool("home1","Home12","");
    tm->addTool("home2","Home2","");
    tm->addTool("home3","Home3","");
    tm->getToolMenuItemFor("home1")->setToolEnabled(true);
    tm->getToolMenuItemFor("home2")->setToolEnabled(true);
    tm->getToolMenuItemFor("home3")->setToolEnabled(true);

    tm->getButtonGroup()->addButton(ui->btnHome);
    tm->getButtonGroup()->addButton(ui->btnPreferences);
    tm->getButtonGroup()->addButton(ui->btnAbout);

    connect(ui->btnHome,&QPushButton::clicked,this,[=](){Q_EMIT toolSelected("home");});
    connect(ui->btnPreferences,&QPushButton::clicked,this,[=](){Q_EMIT toolSelected("preferences");});
    connect(ui->btnAbout,&QPushButton::clicked,this,[=](){Q_EMIT toolSelected("about");});
    connect(tm,SIGNAL(requestToolSelect(QString)),this,SIGNAL(toolSelected(QString)));

}

ToolMenu* ToolBrowser::getToolMenu() {
	return ui->wToolMenu;
}

ToolBrowser::~ToolBrowser()
{
    delete ui;
}

//TEST
/*  ui->setupUi(this);
  auto tb = ui->wToolBrowser;
  auto ts = ui->wsToolStack;
  auto tm = tb->getToolMenu();

  connect(tm,&ToolMenu::requestAttach,ts,&ToolStack::attachTool);
  connect(tm,&ToolMenu::requestDetach,ts,&ToolStack::detachTool);
  connect(ts,&ToolStack::attachSuccesful,tm,&ToolMenu::attachSuccesful);
  connect(ts,&ToolStack::detachSuccesful,tm,&ToolMenu::detachSuccesful);

  connect(tb,&ToolBrowser::toolSelected,ts, &ToolStack::showTool);
//    connect(tb,&ToolBrowser::detach,ts, &ToolStack::showTool);
  ts->addTool("home", new ScopyHomePage());
//    ts->detachTool("home");

  ts->addTool("home1", new QLabel("home1"));
  ts->addTool("home2", new QLabel("home2"));
  ts->addTool("home3", new QLabel("home3"));
 */
