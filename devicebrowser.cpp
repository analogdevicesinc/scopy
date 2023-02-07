#include "devicebrowser.h"
#include "ui_devicebrowser.h"


using namespace adiscope;
DeviceBrowser::DeviceBrowser(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DeviceBrowser)
{
	ui->setupUi(this);

	auto dbm = ui->wDeviceBrowserMenu;
	layout = new QHBoxLayout(dbm);
	bg = new QButtonGroup(this);

	bg->addButton(ui->btnAdd);
	bg->addButton(ui->btnHome);

	connect(ui->btnHome,&QPushButton::clicked, this, [=]() {Q_EMIT requestDevice("home");});
	connect(ui->btnAdd,&QPushButton::clicked, this, [=]() {Q_EMIT requestDevice("add");});
	connect(this,SIGNAL(requestDevice(QString)),this,SLOT(updateSelectedDeviceIdx(QString)));
}

DeviceBrowser::~DeviceBrowser()
{
	delete ui;
}


QAbstractButton *DeviceBrowser::getDeviceWidgetFor(QString id)
{
	for(auto &&w : list) {
		if(w->property(devBrowserId) == id)
			return w;
	}

	return nullptr;
}

void DeviceBrowser::addDevice(QString id, QAbstractButton *w, int position)
{
	w->setProperty(devBrowserId,id);
	layout->insertWidget(position,w);
	bg->addButton(w);
	if(position == -1)
		list.append(w);
	else
		list.insert(position,w);

	connect(w,&QAbstractButton::clicked,this, [=]{Q_EMIT requestDevice(id);});
}


void DeviceBrowser::removeDevice(QString id)
{
	QAbstractButton *w = getDeviceWidgetFor(id);
	layout->removeWidget(w);
	bg->removeButton(w);
	list.removeAt(getIndexOfId(id));
	disconnect(w, &QAbstractButton::clicked, this, nullptr); // disconnect all signals connected to this instance
}

int DeviceBrowser::getIndexOfId(QString k){
	for(int i = 0;i<list.size();i++) {
		if(list[i]->property(devBrowserId) == k)
			return i;
	}
	return -1;

}

QString DeviceBrowser::getIdOfIndex(int idx){
	return (list[idx]->property(devBrowserId).toString());
}

void DeviceBrowser::updateSelectedDeviceIdx(QString k)
{
	if(k=="home") {
		currentIdx=0;
	} else if(k=="add") {
		currentIdx=1;
	} else {
		currentIdx = getIndexOfId(k);
	}
}



