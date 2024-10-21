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

#include "scopyhomepage.h"

#include "scopyhomeinfopage.h"

#include "ui_scopyhomepage.h"

#include <QPushButton>

#include <deviceicon.h>
#include <style.h>

using namespace scopy;
ScopyHomePage::ScopyHomePage(QWidget *parent, PluginManager *pm)
	: QWidget(parent)
	, ui(new Ui::ScopyHomePage)
{
	ui->setupUi(this);
	auto &&is = ui->wInfoPageStack;
	auto &&hc = is->getHomepageControls();
	auto &&db = ui->wDeviceBrowser;
	add = new ScopyHomeAddPage(this, pm);

	ui->wInfoPageStack->setStyleSheet(".QWidget {border-radius: " + Style::getAttribute(json::global::radius_1) +
					  ";}");
	// ui->horizontalLayout_2->setStyleSheet(
	// ".QWidget {border-radius: " + Style::getAttribute(json::global::radius_1) + ";}");
	// ui->container->setStyleSheet(".QWidget#container { background-color: " +
	// Style::getAttribute(json::theme::background_subtle) + "; }"); Style::setStyle(ui->horizontalLayout_2,
	// style::properties::widget::border); ui->container->setStyleSheet(".QWidget#container { background-color: " +
	// Style::getAttribute(json::theme::background_subtle) + "; }");
	// Style::setStyle(ui->horizontalLayout_2, style::properties::widget::border);
	Style::setStyle(ui->wInfoPageStack, style::properties::widget::border);

	is->add("home", new ScopyHomeInfoPage());
	is->add("add", add);

	//	addDevice("dev1","dev1","descr1",new QPushButton("abc"),new QLabel("page1"));

	Style::setStyle(scanBtn(), style::properties::button::basicButton);
	scanBtn()->setFixedWidth(80);
	connect(hc, SIGNAL(goLeft()), db, SLOT(prevDevice()));
	connect(hc, SIGNAL(goRight()), db, SLOT(nextDevice()));
	connect(db, SIGNAL(requestDevice(QString, int)), is, SLOT(slideInKey(QString, int)));
	connect(db, SIGNAL(requestDevice(QString, int)), this, SIGNAL(requestDevice(QString)));
	connect(this, SIGNAL(deviceAddedToUi(QString)), add, SLOT(deviceAddedToUi(QString)));

	connect(add, &ScopyHomeAddPage::requestDevice, this, [=](QString id) { Q_EMIT db->requestDevice(id, -1); });
	connect(add, &ScopyHomeAddPage::newDeviceAvailable, this, [=](DeviceImpl *d) { Q_EMIT newDeviceAvailable(d); });

	connect(db, &DeviceBrowser::displayNameChanged, this, &ScopyHomePage::displayNameChanged);
}

ScopyHomePage::~ScopyHomePage() { delete ui; }

void ScopyHomePage::addDevice(QString id, Device *d)
{
	auto &&is = ui->wInfoPageStack;
	auto &&db = ui->wDeviceBrowser;
	db->addDevice(id, d);
	is->add(id, d);
	Q_EMIT deviceAddedToUi(id);
}

void ScopyHomePage::removeDevice(QString id)
{
	auto &&is = ui->wInfoPageStack;
	auto &&db = ui->wDeviceBrowser;
	db->removeDevice(id);
	is->remove(id);
}

void ScopyHomePage::viewDevice(QString id)
{
	auto &&db = ui->wDeviceBrowser;
	Q_EMIT db->requestDevice(id, -1);
}

void ScopyHomePage::connectingDevice(QString id)
{
	auto &&db = ui->wDeviceBrowser;
	db->connectingDevice(id);
}

void ScopyHomePage::connectDevice(QString id)
{
	auto &&db = ui->wDeviceBrowser;
	db->connectDevice(id);
}
void ScopyHomePage::disconnectDevice(QString id)
{
	auto &&db = ui->wDeviceBrowser;
	db->disconnectDevice(id);
}

void ScopyHomePage::setScannerEnable(bool b)
{
	ui->btnScan->setVisible(b);
	ui->label_2->setVisible(b);
	ui->btnScanNow->setVisible(!b);
}
QCheckBox *ScopyHomePage::scanControlBtn() { return ui->btnScan; }

QPushButton *ScopyHomePage::scanBtn() { return ui->btnScanNow; }

#include "moc_scopyhomepage.cpp"
