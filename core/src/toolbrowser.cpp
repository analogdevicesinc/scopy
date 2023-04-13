#include "toolbrowser.h"
#include "ui_toolbrowser.h"
#include <QDebug>

using namespace scopy;

ToolBrowser::ToolBrowser(QWidget *parent) :
    QWidget(parent),
	ui(new Ui::ToolBrowser),
	collapsed(false)
{
    ui->setupUi(this);

    ToolMenu *tm = ui->wToolMenu;   

    tm->getButtonGroup()->addButton(ui->btnPreferences);
    tm->getButtonGroup()->addButton(ui->btnAbout);

	ToolMenuItem* homeTmi = tm->createTool("home","Home",":/gui/icons/scopy-default/icons/tool_home.svg");
	homeTmi->setSeparator(true,true);
	homeTmi->getToolRunBtn()->setVisible(false);
	homeTmi->setEnabled(true);
	ui->homePlaceholder->layout()->addWidget(homeTmi);
	homeTmi->setDraggable(false);


//	connect(ui->btnCollapse, &QPushButton::clicked, this, &ToolBrowser::toggleCollapse);
    connect(ui->btnPreferences,&QPushButton::clicked,this,[=](){Q_EMIT requestTool("preferences");});
    connect(ui->btnAbout,&QPushButton::clicked,this,[=](){Q_EMIT requestTool("about");});

    connect(ui->btnSave,&QPushButton::clicked,this,[=](){Q_EMIT requestSave();});
    connect(ui->btnLoad,&QPushButton::clicked,this,[=](){Q_EMIT requestLoad();});

    connect(tm,SIGNAL(requestToolSelect(QString)),this,SIGNAL(requestTool(QString)));
}

ToolMenu* ToolBrowser::getToolMenu() {
	return ui->wToolMenu;
}

void ToolBrowser::toggleCollapse()
{
	 ToolMenu *tm = ui->wToolMenu;
	collapsed = !collapsed;
	tm->hideMenuText(collapsed);
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


#include "moc_toolbrowser.cpp"
