#include "devicebrowsermenu.h"
#include "qbuttongroup.h"
#include "qvariant.h"

using namespace adiscope;
DeviceBrowserMenu::DeviceBrowserMenu(QWidget *parent)
	: QWidget{parent}
{
	layout = new QHBoxLayout(this);
	bg = new QButtonGroup(this);
}

DeviceBrowserMenu::~DeviceBrowserMenu()
{

}

QAbstractButton *DeviceBrowserMenu::getDeviceWidgetFor(QString id)
{
	for(auto &&w : list) {
		if(w->property(devBrowserId) == id)
			return w;
	}

	return nullptr;
}

void DeviceBrowserMenu::addDevice(QString id, QAbstractButton *w, int position)
{
	w->setProperty(devBrowserId,id);
	layout->insertWidget(position,w);
	bg->addButton(w);
	connect(w,&QAbstractButton::clicked,this, [=]{Q_EMIT requestDeviceSelect(id);});
}


void DeviceBrowserMenu::removeDevice(QString id)
{
	QAbstractButton *w = getDeviceWidgetFor(id);
	layout->removeWidget(w);
	bg->removeButton(w);
	disconnect(w, &QAbstractButton::clicked, this, nullptr); // disconnect all signals connected to this instance


}

QButtonGroup *DeviceBrowserMenu::getButtonGroup() const
{
	return bg;
}
