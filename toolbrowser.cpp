#include "toolbrowser.h"
#include "ui_toolbrowser.h"
#include <QDebug>

using namespace adiscope;

ToolBrowser::ToolBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolBrowser)
{
    ui->setupUi(this);

    ToolMenu *tm = ui->wToolMenu;   

    tm->getButtonGroup()->addButton(ui->btnHome);
    tm->getButtonGroup()->addButton(ui->btnPreferences);
    tm->getButtonGroup()->addButton(ui->btnAbout);

    connect(ui->btnHome,&QPushButton::clicked,this,[=](){Q_EMIT requestTool("home");});
    connect(ui->btnPreferences,&QPushButton::clicked,this,[=](){Q_EMIT requestTool("preferences");});
    connect(ui->btnAbout,&QPushButton::clicked,this,[=](){Q_EMIT requestTool("about");});
    connect(tm,SIGNAL(requestToolSelect(QString)),this,SIGNAL(requestTool(QString)));
}

ToolMenu* ToolBrowser::getToolMenu() {
	return ui->wToolMenu;
}

ToolBrowser::~ToolBrowser()
{
    delete ui;
}

//TEST
/*         
 ts->detachTool("home");
 tm->addTool("home1","Home12","");
 tm->addTool("home2","Home2","");
 tm->addTool("home3","Home3","");
 tm->getToolMenuItemFor("home1")->setToolEnabled(true);
 tm->getToolMenuItemFor("home2")->setToolEnabled(true);
 tm->getToolMenuItemFor("home3")->setToolEnabled(true);

 ts->addTool("home1", new QLabel("home1"));
 ts->addTool("home2", new QLabel("home2"));
 ts->addTool("home3", new QLabel("home3"));
*/

