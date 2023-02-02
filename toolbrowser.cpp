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
    connect(tm,SIGNAL(toolSelected(QString)),this,SIGNAL(toolSelected(QString)));

}

ToolBrowser::~ToolBrowser()
{
    delete ui;
}
