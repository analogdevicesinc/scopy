#include "scopyhomepage.h"
#include "ui_scopyhomepage.h"
#include "scopyhomeinfopage.h"
#include "addcontextordemoinfopage.h"
#include <QPushButton>
#include <deviceicon.h>


using namespace adiscope;
ScopyHomePage::ScopyHomePage(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ScopyHomePage)
{
	ui->setupUi(this);
	auto &&is = ui->wInfoPageStack;
	auto &&hc = is->getHomepageControls();
	auto &&db = ui->wDeviceBrowser;

	is->add("home",new ScopyHomeInfoPage());
	is->add("add", new AddContextOrDemoInfoPage());


//	addDevice("dev1","dev1","descr1",new QPushButton("abc"),new QLabel("page1"));
	connect(hc,SIGNAL(goLeft()),db,SLOT(prevDevice()));
	connect(hc,SIGNAL(goRight()),db,SLOT(nextDevice()));
	connect(db,SIGNAL(requestDevice(QString,int)),is,SLOT(slideInKey(QString,int)));
	connect(db,SIGNAL(requestDevice(QString,int)),this,SIGNAL(requestDevice(QString)));
}

ScopyHomePage::~ScopyHomePage()
{
	delete ui;
}

void ScopyHomePage::addDevice(QString id, QString name, QString description, QWidget *icon, QWidget *page)
{
	auto &&is = ui->wInfoPageStack;
	auto &&db = ui->wDeviceBrowser;
	db->addDevice(id, name, description, icon);
	is->add(id,page);
}

void ScopyHomePage::removeDevice(QString id) {
	auto &&is = ui->wInfoPageStack;
	auto &&db = ui->wDeviceBrowser;
	db->removeDevice(id);
	is->remove(id);
}
