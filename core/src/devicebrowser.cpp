#include "devicebrowser.h"
#include "deviceiconimpl.h"
#include "ui_devicebrowser.h"
#include <QLoggingCategory>
#include <QDebug>
#include "deviceicon.h"

Q_LOGGING_CATEGORY(CAT_DEVBROWSER, "DeviceBrowser")

using namespace adiscope;
DeviceBrowser::DeviceBrowser(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DeviceBrowser)
{
	qDebug(CAT_DEVBROWSER) << "ctor";
	ui->setupUi(this);

	auto dbm = ui->wDeviceBrowserMenu;
	layout = new QHBoxLayout(dbm);

	initBtns();

	connect(ui->btnHome, SIGNAL(clicked()),this, SLOT(forwardRequestDeviceWithDirection()));
	connect(ui->btnAdd, SIGNAL(clicked()),this, SLOT(forwardRequestDeviceWithDirection()));
	connect(this,SIGNAL(requestDevice(QString,int)),this,SLOT(updateSelectedDeviceIdx(QString)));
}

DeviceBrowser::~DeviceBrowser()
{
	qDebug(CAT_DEVBROWSER) << "dtor";
	delete ui;
}

void DeviceBrowser::initBtns()
{
	
	bg = new QButtonGroup(this);

	bg->addButton(ui->btnAdd);
	bg->addButton(ui->btnHome);
	ui->btnHome->setProperty(devBrowserId,"home");
	ui->btnAdd->setProperty(devBrowserId,"add");
	list.append(ui->btnHome);
	list.append(ui->btnAdd);
	ui->btnHome->setChecked(true);
	currentIdx = 0;
}

QAbstractButton *DeviceBrowser::getDeviceWidgetFor(QString id)
{
	for(auto &&w : list) {
		if(w->property(devBrowserId) == id)
			return w;
	}

	return nullptr;
}

void DeviceBrowser::addDevice(QString id, Device *d,int position)
{
	QString name = d->name();
	QString description = d->description();
	QWidget* icon = d->icon();


	qInfo(CAT_DEVBROWSER)<<"adding device " << id;
	auto w = dynamic_cast<QAbstractButton*>(buildDeviceIcon(name, description, icon, this));
	w->setProperty(devBrowserId,id);
	layout->insertWidget(position,w);
	bg->addButton(w);
	if(position == -1)
		list.append(w);
	else
		list.insert(position,w);

	connect(w, SIGNAL(clicked()),this, SLOT(forwardRequestDeviceWithDirection()));
	connect(w, SIGNAL(forget()), this, SLOT(forwardRequestRemoveDevice()));
}

void DeviceBrowser::removeDevice(QString id)
{
	qInfo(CAT_DEVBROWSER)<<"removing device " << id;
	QAbstractButton *w = getDeviceWidgetFor(id);
	layout->removeWidget(w);
	bg->removeButton(w);
	int idx = getIndexOfId(id);
	list.removeAt(idx);
	disconnect(w, &QAbstractButton::clicked, this, nullptr); // disconnect all signals connected to this instance
	delete(w);

	if(currentIdx == idx) { // removed currently selected device
		currentIdx = 0;
		Q_EMIT requestDevice("home",-1);
	} else if(currentIdx>idx) {
		currentIdx--;
	}
}


int DeviceBrowser::getIndexOfId(QString k)
{
    for(int i = 0;i<list.size();i++) {
		if(list[i]->property(devBrowserId) == k)
			return i;
	}
	return -1;
}

QString DeviceBrowser::getIdOfIndex(int idx){
	return (list[idx]->property(devBrowserId).toString());
}

void DeviceBrowser::nextDevice()
{
	int maxIdx = list.size();
	int nextIdx = currentIdx;

	do { // find next visible and enabled button
		nextIdx = (nextIdx + 1) % maxIdx;
		nextIdx = (nextIdx < 0) ? nextIdx + maxIdx : nextIdx; // wrap around negative
	} while(!(list.at(nextIdx)->isVisible() && list.at(nextIdx)->isEnabled()));

	QString nextId = getIdOfIndex(nextIdx);
	Q_EMIT requestDevice(nextId, 1); // start animation
	list[nextIdx]->setChecked(true); // set checked afterwards	
}

void DeviceBrowser::prevDevice()
{
	int maxIdx = list.size();
	int nextIdx = currentIdx;

	do { // find next visible and enabled button
		nextIdx = (nextIdx - 1) % maxIdx;
		nextIdx = (nextIdx < 0) ? nextIdx + maxIdx : nextIdx; // wrap around negative
	} while(!(list.at(nextIdx)->isVisible() && list.at(nextIdx)->isEnabled()));

	QString nextId = getIdOfIndex(nextIdx);
	Q_EMIT requestDevice(nextId, -1); // start animation
	list[nextIdx]->setChecked(true);  // set checked afterwards
}

void DeviceBrowser::forwardRequestRemoveDevice() {
	QString id = QObject::sender()->property(devBrowserId).toString();
	Q_EMIT requestRemoveDevice(id);
}

void DeviceBrowser::forwardRequestDeviceWithDirection()
{
    QString id = QObject::sender()->property(devBrowserId).toString();
    int idx = getIndexOfId(id);
    int direction = currentIdx - idx;
    Q_EMIT requestDevice(id, direction);
}

void DeviceBrowser::updateSelectedDeviceIdx(QString k)
{	
	int prevIdx = currentIdx; // local, just for debug	
	currentIdx = getIndexOfId(k);
	qDebug(CAT_DEVBROWSER)<<"prev: "
			     << "["<<prevIdx<<"] -"
			     << getIdOfIndex(prevIdx)<<"-> current: "
			     << "["<<currentIdx<<"] -"
			     << getIdOfIndex(currentIdx);
}

void DeviceBrowser::connectDevice(QString id) {
	auto w = dynamic_cast<DeviceIcon*>(getDeviceWidgetFor(id));
	w->setConnected(true);
}

void DeviceBrowser::disconnectDevice(QString id) {
	auto w = dynamic_cast<DeviceIcon*>(getDeviceWidgetFor(id));
	w->setConnected(false);
}

DeviceIcon* DeviceBrowser::buildDeviceIcon(QString name, QString description, QWidget *icon, QWidget *parent) {
	return new DeviceIconImpl(name, description, icon, parent);
}

/*
   auto &&is = ui->wInfoPageStack;
    auto &&hc = is->getHomepageControls();

    is->add("home",new ScopyHomeInfoPage());
    is->add("add", new ScopyHomeAddPage());
    is->add("dev1", new QLabel("dev1"));
    is->add("dev2", new QLabel("dev2"));

    auto &&db = ui->wDeviceBrowser;
    QPushButton *w1 = new QPushButton("dev1");
    QPushButton *w2 = new QPushButton("dev2");
    w1->setCheckable(true);
    w2->setCheckable(true);
    db->addDevice("dev1",w1);
    db->addDevice("dev2",w2);
    w1->setVisible(false);
    */


/*
	ui->setupUi(this);
	auto &&is = ui->wInfoPageStack;
	auto &&hc = is->getHomepageControls();

	is->add("home",new ScopyHomeInfoPage());
	is->add("add", new ScopyHomeAddPage());
	is->add("dev1", new QLabel("dev1"));
	is->add("dev2", new QLabel("dev2"));

	auto &&db = ui->wDeviceBrowser;
	DeviceIcon *w1 = new DeviceIcon("dev1","uri",this);
	DeviceIcon *w2 = new DeviceIcon("dev2","uri2",this);
	w1->setCheckable(true);
	w2->setCheckable(true);
	db->addDevice("dev1",w1);
	db->addDevice("dev2",w2);
	w1->setVisible(false);
	*/

