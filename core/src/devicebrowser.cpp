/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "devicebrowser.h"

#include "deviceicon.h"
#include "deviceiconimpl.h"
#include "dynamicWidget.h"
#include "stylehelper.h"

#include "ui_devicebrowser.h"

#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_DEVBROWSER, "DeviceBrowser")

using namespace scopy;
DeviceBrowser::DeviceBrowser(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::DeviceBrowser)
{
	qDebug(CAT_DEVBROWSER) << "ctor";
	ui->setupUi(this);
	this->setFixedHeight(185);

	auto dbm = ui->wDeviceBrowserMenu;
	layout = new QHBoxLayout(dbm);

	initBtns();

	connect(ui->btnHome, SIGNAL(clicked()), this, SLOT(forwardRequestDeviceWithDirection()));
	connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(forwardRequestDeviceWithDirection()));
	connect(this, SIGNAL(requestDevice(QString, int)), this, SLOT(updateSelectedDeviceIdx(QString)));
}

DeviceBrowser::~DeviceBrowser()
{
	qDebug(CAT_DEVBROWSER) << "dtor";
	delete ui;
}

void DeviceBrowser::initBtns()
{
	StyleHelper::FrameBackgroundShadow(ui->containerHome);
	StyleHelper::FrameBackgroundShadow(ui->containerAdd);

	bg = new QButtonGroup(this);

	bg->addButton(ui->btnAdd);
	bg->addButton(ui->btnHome);
	ui->btnHome->setProperty(devBrowserId, "home");
	ui->btnAdd->setProperty(devBrowserId, "add");
	list.append(ui->btnHome);
	list.append(ui->btnAdd);
	ui->btnHome->setChecked(true);
	setDynamicProperty(ui->containerHome, "selected", true); // select home shadow on init
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

void DeviceBrowser::addDevice(QString id, Device *d, int position)
{
	qInfo(CAT_DEVBROWSER) << "adding device " << id;
	auto w = dynamic_cast<QAbstractButton *>(buildDeviceIcon(d, this));
	w->setProperty(devBrowserId, id);
	layout->insertWidget(position, w);
	bg->addButton(w);
	if(position == -1)
		list.append(w);
	else
		list.insert(position, w);

	connect(w, &QAbstractButton::clicked, this, &DeviceBrowser::forwardRequestDeviceWithDirection);
}

void DeviceBrowser::removeDevice(QString id)
{
	qInfo(CAT_DEVBROWSER) << "removing device " << id;
	QAbstractButton *w = getDeviceWidgetFor(id);
	layout->removeWidget(w);
	bg->removeButton(w);
	int idx = getIndexOfId(id);
	list.removeAt(idx);
	disconnect(w, &QAbstractButton::clicked, this, nullptr); // disconnect all signals connected to this instance
	delete(w);

	if(currentIdx == idx) { // removed currently selected device
		currentIdx = 0;
		Q_EMIT requestDevice("home", -1);
	} else if(currentIdx > idx) {
		currentIdx--;
	}
}

int DeviceBrowser::getIndexOfId(QString k)
{
	for(int i = 0; i < list.size(); i++) {
		if(list[i]->property(devBrowserId) == k)
			return i;
	}
	return -1;
}

QString DeviceBrowser::getIdOfIndex(int idx) { return (list[idx]->property(devBrowserId).toString()); }

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

	QWidget *prevDevice = getDeviceWidgetFor(getIdOfIndex(prevIdx));
	QWidget *currentDevice = getDeviceWidgetFor(getIdOfIndex(currentIdx));

	// hackish -- the btnHome and btnAdd already have a background color so their container must display the shadow
	if(currentDevice == ui->btnHome)
		currentDevice = ui->containerHome;

	if(currentDevice == ui->btnAdd)
		currentDevice = ui->containerAdd;

	if(prevDevice == ui->btnHome)
		prevDevice = ui->containerHome;

	if(prevDevice == ui->btnAdd)
		prevDevice = ui->containerAdd;

	setDynamicProperty(prevDevice, "selected", false);
	setDynamicProperty(currentDevice, "selected", true);

	qDebug(CAT_DEVBROWSER) << "prev: "
			       << "[" << prevIdx << "] -" << getIdOfIndex(prevIdx) << "-> current: "
			       << "[" << currentIdx << "] -" << getIdOfIndex(currentIdx);
}

void DeviceBrowser::connectDevice(QString id)
{
	auto w = dynamic_cast<DeviceIcon *>(getDeviceWidgetFor(id));
	w->setConnected(true);
}

void DeviceBrowser::disconnectDevice(QString id)
{
	auto w = dynamic_cast<DeviceIcon *>(getDeviceWidgetFor(id));
	w->setConnected(false);
}

DeviceIcon *DeviceBrowser::buildDeviceIcon(Device *d, QWidget *parent)
{
	DeviceIconImpl *devIcon = new DeviceIconImpl(d, parent);
	connect(devIcon, &DeviceIconImpl::displayNameChanged, this,
		[this, d](QString newName) { Q_EMIT displayNameChanged(d->id(), newName); });
	return devIcon;
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

#include "moc_devicebrowser.cpp"
