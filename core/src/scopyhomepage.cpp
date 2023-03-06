#include "scopyhomepage.h"
#include "scopyhomeinfopage.h"
#include "ui_scopyhomepage.h"
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
	add = new ScopyHomeAddPage();

	is->add("home",new ScopyHomeInfoPage());
	is->add("add", add);

//	addDevice("dev1","dev1","descr1",new QPushButton("abc"),new QLabel("page1"));
	connect(hc,SIGNAL(goLeft()),db,SLOT(prevDevice()));
	connect(hc,SIGNAL(goRight()),db,SLOT(nextDevice()));
	connect(db,SIGNAL(requestDevice(QString,int)),is,SLOT(slideInKey(QString,int)));
	connect(db,SIGNAL(requestDevice(QString,int)),this,SIGNAL(requestDevice(QString)));
	connect(this,SIGNAL(deviceAddedToUi(QString)),add,SLOT(deviceAddedToUi(QString)));

	connect(db,SIGNAL(requestRemoveDevice(QString)),this,SIGNAL(requestRemoveDevice(QString)));
	connect(add,SIGNAL(requestAddDevice(QString, QString)),this,SIGNAL(requestAddDevice(QString, QString)));
	connect(add,&ScopyHomeAddPage::requestDevice,this,[=](QString id){Q_EMIT db->requestDevice(id,-1);});
}

ScopyHomePage::~ScopyHomePage()
{
	delete ui;
}

void ScopyHomePage::addDevice(QString id, Device *d)
{
	auto &&is = ui->wInfoPageStack;
	auto &&db = ui->wDeviceBrowser;
	db->addDevice(id, d);
	is->add(id, d);
	Q_EMIT deviceAddedToUi(id);
}

void ScopyHomePage::removeDevice(QString id) {
	auto &&is = ui->wInfoPageStack;
	auto &&db = ui->wDeviceBrowser;
	db->removeDevice(id);
	is->remove(id);
}

void ScopyHomePage::viewDevice(QString id) {
	auto &&db = ui->wDeviceBrowser;
	Q_EMIT db->requestDevice(id,-1);
}

void ScopyHomePage::connectDevice(QString id) {
	auto &&db = ui->wDeviceBrowser;
	db->connectDevice(id);

}
void ScopyHomePage::disconnectDevice(QString id) {
	auto &&db = ui->wDeviceBrowser;
	db->disconnectDevice(id);
}

QPushButton* ScopyHomePage::scanControlBtn() {
	return ui->btnScan;
}
