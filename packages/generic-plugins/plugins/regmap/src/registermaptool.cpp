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

#include "registermaptool.hpp"
#include "utils.hpp"

#include <QBoxLayout>
#include <QComboBox>
#include <QDesktopServices>
#include <deviceregistermap.hpp>
#include <deviceregistermap.hpp>
#include <registermapsettingsmenu.hpp>
#include <registermaptemplate.hpp>
#include <registermapvalues.hpp>
#include <registermapvalues.hpp>
#include <registermapvalues.hpp>
#include <regmapstylehelper.hpp>
#include <searchbarwidget.hpp>
#include <style.h>
#include <stylehelper.h>
#include <toolbuttons.h>
#include <tutorialbuilder.h>
#include <xmlfilemanager.hpp>

#include <readwrite/fileregisterreadstrategy.hpp>
#include <readwrite/fileregisterwritestrategy.hpp>
#include <readwrite/iioregisterreadstrategy.hpp>
#include <readwrite/iioregisterwritestrategy.hpp>
#include <style_properties.h>

#include <pluginbase/preferences.h>

using namespace scopy;
using namespace regmap;

RegisterMapTool::RegisterMapTool(QWidget *parent)
	: QWidget{parent}
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *lay = new QHBoxLayout(this);
	Utils::removeLayoutMargins(lay);
	setLayout(lay);

	tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->layout()->setMargin(4);
	tool->layout()->setSpacing(2);
	tool->centralContainer()->layout()->setSpacing(0);
	lay->addWidget(tool);

	InfoBtn *infoBtn = new InfoBtn(this, true);
	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);

	connect(infoBtn, &InfoBtn::clicked, this, [=]() {
		infoBtn->generateInfoPopup(this);
		connect(infoBtn->getTutorialButton(), &QPushButton::clicked, this, [=]() {
			if(searchBarWidget->isVisible()) {
				startTutorial();
			} else {
				startSimpleTutorial();
			}
		});

		connect(infoBtn->getDocumentationButton(), &QAbstractButton::clicked, this, [=]() {
			QDesktopServices::openUrl(
				QUrl("https://analogdevicesinc.github.io/scopy/plugins/registermap/registermap.html"));
		});
	});

	searchBarWidget = new SearchBarWidget();
	searchBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	tool->addWidgetToTopContainerHelper(searchBarWidget, TTA_LEFT);
	searchBarWidget->setEnabled(false);

	settings = new RegisterMapSettingsMenu(this);
	tool->rightStack()->add("settings", settings);
	tool->setRightContainerWidth(settings->sizeHint().width());

	settingsMenu = new GearBtn(this);
	Style::setStyle(settingsMenu, style::properties::button::squareIconButton, true, true);

	connect(settingsMenu, &QAbstractButton::toggled, this, [=](bool toggled) {
		tool->openRightContainerHelper(toggled);
		tool->requestMenu("settings");
	});
	Q_EMIT settingsMenu->toggled(false);
	tool->topContainerMenuControl()->setStyleSheet("background : transparent; ");
	tool->addWidgetToTopContainerMenuControlHelper(settingsMenu, TTA_RIGHT);

	registerDeviceList = new QComboBox(tool->topContainer());
	Style::setStyle(registerDeviceList, style::properties::regmap::deviceComboBox, true, true);
	Style::setStyle(registerDeviceList, style::properties::regmap::simpleWidget);
	registerDeviceList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	QObject::connect(registerDeviceList, &QComboBox::currentTextChanged, this,
			 &RegisterMapTool::updateActiveRegisterMap);
	tool->addWidgetToTopContainerMenuControlHelper(registerDeviceList, TTA_LEFT);

	deviceList = new QMap<QString, DeviceRegisterMap *>();
	tool->getContainerSpacer(tool->topContainer())->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
	tool->getContainerSpacer(tool->topContainerMenuControl())
		->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);

	QObject::connect(searchBarWidget, &SearchBarWidget::requestSearch, this, [=](QString searchParam) {
		deviceList->value(registerDeviceList->currentText())->applyFilters(searchParam);
	});

	initTutorialProperties();
}

RegisterMapTool::~RegisterMapTool() { delete deviceList; }

void RegisterMapTool::addDevice(QString devName, RegisterMapTemplate *registerMapTemplate,
				RegisterMapValues *registerMapValues)
{

	DeviceRegisterMap *regMap = new DeviceRegisterMap(registerMapTemplate, registerMapValues, this);
	deviceList->insert(devName, regMap);
	tool->addWidgetToCentralContainerHelper(regMap);
	deviceList->value(devName)->hide();

	registerDeviceList->addItem(devName);

	if(first) {
		// the first regmap is set active
		activeRegisterMap = devName;
		first = false;
		deviceList->value(devName)->show();
		toggleSettingsMenu(devName, true);
		toggleSearchBarEnabled(deviceList->value(devName)->hasTemplate());
	}
}

void RegisterMapTool::toggleSettingsMenu(QString registerName, bool toggle)
{
	if(toggle) {
		QObject::connect(settings, &RegisterMapSettingsMenu::autoreadToggled, deviceList->value(registerName),
				 &DeviceRegisterMap::toggleAutoread);
		QObject::connect(settings, &RegisterMapSettingsMenu::requestRead, deviceList->value(registerName),
				 &DeviceRegisterMap::requestRead);
		QObject::connect(settings, &RegisterMapSettingsMenu::requestRegisterDump,
				 deviceList->value(registerName), &DeviceRegisterMap::requestRegisterDump);
		QObject::connect(settings, &RegisterMapSettingsMenu::requestWrite, deviceList->value(registerName),
				 &DeviceRegisterMap::requestWrite);
	} else {
		QObject::disconnect(settings, &RegisterMapSettingsMenu::autoreadToggled,
				    deviceList->value(registerName), &DeviceRegisterMap::toggleAutoread);
		QObject::disconnect(settings, &RegisterMapSettingsMenu::requestRead, deviceList->value(registerName),
				    &DeviceRegisterMap::requestRead);
		QObject::disconnect(settings, &RegisterMapSettingsMenu::requestRegisterDump,
				    deviceList->value(registerName), &DeviceRegisterMap::requestRegisterDump);
		QObject::disconnect(settings, &RegisterMapSettingsMenu::requestWrite, deviceList->value(registerName),
				    &DeviceRegisterMap::requestWrite);
	}
}

void RegisterMapTool::initTutorialProperties()
{
	registerDeviceList->setProperty("tutorial_name", "DEVICE_LIST");
	settingsMenu->setProperty("tutorial_name", "SETTINGS_BUTTON");
	searchBarWidget->setProperty("tutorial_name", "SEARCH_BAR");
}

void RegisterMapTool::startTutorial()
{
	settingsMenu->setChecked(true);

	QWidget *parent = Util::findContainingWindow(this);
	gui::TutorialBuilder *registerMapTutorial =
		new gui::TutorialBuilder(this, ":/registermap/tutorial_chapters.json", "registermap", parent);

	settingsTutorialFinish = connect(registerMapTutorial, &gui::TutorialBuilder::finished, settings,
					 &RegisterMapSettingsMenu::startTutorial, Qt::UniqueConnection);

	controllerTutorial = connect(
		settings, &RegisterMapSettingsMenu::tutorialDone, this,
		[this]() { deviceList->value(activeRegisterMap)->startTutorial(); }, Qt::UniqueConnection);

	connect(deviceList->value(activeRegisterMap), &DeviceRegisterMap::tutorialFinished, this,
		&RegisterMapTool::tutorialAborted);
	connect(deviceList->value(activeRegisterMap), &DeviceRegisterMap::tutorialAborted, this,
		&RegisterMapTool::tutorialAborted);

	connect(registerMapTutorial, &gui::TutorialBuilder::aborted, this, &RegisterMapTool::tutorialAborted);
	connect(settings, &RegisterMapSettingsMenu::tutorialAborted, this, &RegisterMapTool::tutorialAborted);

	registerMapTutorial->setTitle("Tutorial");
	registerMapTutorial->start();
}

void RegisterMapTool::startSimpleTutorial()
{
	settingsMenu->setChecked(true);

	QWidget *parent = Util::findContainingWindow(this);
	gui::TutorialBuilder *registerMapTutorial =
		new gui::TutorialBuilder(this, ":/registermap/tutorial_chapters.json", "simple_registermap", parent);

	settingsTutorialFinish = connect(registerMapTutorial, &gui::TutorialBuilder::finished, settings,
					 &RegisterMapSettingsMenu::startTutorial, Qt::UniqueConnection);

	controllerTutorial = connect(
		settings, &RegisterMapSettingsMenu::tutorialDone, this,
		[this]() { deviceList->value(activeRegisterMap)->startSimpleTutorial(); }, Qt::UniqueConnection);

	connect(deviceList->value(activeRegisterMap), &DeviceRegisterMap::simpleTutorialFinished, this,
		&RegisterMapTool::tutorialAborted);
	connect(deviceList->value(activeRegisterMap), &DeviceRegisterMap::tutorialAborted, this,
		&RegisterMapTool::tutorialAborted);
	connect(registerMapTutorial, &gui::TutorialBuilder::aborted, this, &RegisterMapTool::tutorialAborted);
	connect(settings, &RegisterMapSettingsMenu::tutorialAborted, this, &RegisterMapTool::tutorialAborted);

	registerMapTutorial->setTitle("Tutorial");
	registerMapTutorial->start();
}

void RegisterMapTool::tutorialAborted()
{
	disconnect(settingsTutorialFinish);
	disconnect(controllerTutorial);
}

void RegisterMapTool::updateActiveRegisterMap(QString registerName)
{
	if(activeRegisterMap != "" && registerName != activeRegisterMap) {
		deviceList->value(activeRegisterMap)->hide();
		toggleSettingsMenu(activeRegisterMap, false);
		deviceList->value(registerName)->show();
		toggleSettingsMenu(registerName, true);
		toggleSearchBarEnabled(deviceList->value(registerName)->hasTemplate());

		activeRegisterMap = registerName;
	}
}

void RegisterMapTool::toggleSearchBarEnabled(bool enabled)
{
	searchBarWidget->setEnabled(enabled);
	if(enabled) {
		tool->getContainerSpacer(tool->topContainer())
			->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
		searchBarWidget->show();
	} else {
		tool->getContainerSpacer(tool->topContainer())
			->changeSize(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		searchBarWidget->hide();
	}
}
