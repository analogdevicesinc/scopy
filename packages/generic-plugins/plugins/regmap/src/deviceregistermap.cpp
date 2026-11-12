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

#include "deviceregistermap.hpp"

#include "dynamicWidget.h"
#include "logging_categories.h"

#include <QLineEdit>
#include <QPushButton>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <stylehelper.h>
#include <toolbuttons.h>
#include <utils.h>
#include "register/registerdetailedwidget.hpp"
#include "register/registermodel.hpp"
#include "registercontroller.hpp"
#include "registermaptemplate.hpp"
#include "registermapaccess.hpp"
#include "registermapdatastore.hpp"
#include "regmapstylehelper.hpp"
#include "search.hpp"
#include "utils.hpp"

#include <QLineEdit>
#include <QPushButton>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <style.h>

#include <src/recyclerview/registermaptable.hpp>
#include <utils.h>

#include "style_properties.h"

using namespace scopy;
using namespace regmap;

DeviceRegisterMap::DeviceRegisterMap(RegisterMapTemplate *registerMapTemplate, RegisterMapAccess *registerMapAccess,
				     QWidget *parent)
	: QWidget(parent)
	, registerMapTemplate(registerMapTemplate)
	, registerMapAccess(registerMapAccess)
	, registerMapStore(registerMapAccess ? registerMapAccess->store() : nullptr)
{
	// a template without registers can't be shown, treat it as no template
	if(registerMapTemplate &&
	   (!registerMapTemplate->getRegisterList() || registerMapTemplate->getRegisterList()->isEmpty())) {
		qWarning(CAT_REGMAP) << "Register map template has no registers, it is ignored";
		registerMapTemplate = nullptr;
		this->registerMapTemplate = nullptr;
	}

	layout = new QVBoxLayout(this);
	Utils::removeLayoutMargins(layout);
	setLayout(layout);

	tool = new ToolTemplate(this);
	Utils::removeLayoutMargins(tool->layout());
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainerMenuControl()->hide();
	tool->bottomCentral()->setVisible(true);
	layout->addWidget(tool);

	Style::setBackgroundColor(this, Style::getAttribute(json::theme::interactive_secondary_disabled));

	initSettings();

	registerController = new RegisterController(this);
	Style::setStyle(registerController, style::properties::regmap::registercontroller, true, true);

	QWidget *controllerWidget = new QWidget(this);
	QHBoxLayout *controllerLayout = new QHBoxLayout(controllerWidget);
	Utils::removeLayoutMargins(controllerLayout);
	controllerLayout->setSpacing(0);

	if(registerMapTemplate) {
		registerController->setHasMap(true);
		QObject::connect(registerController, &RegisterController::toggleDetailedMenu, this,
				 [this](bool toggled) { tool->openBottomContainerHelper(toggled); });

		QWidget *registerMapTable = new QWidget();
		QVBoxLayout *registerMapTableLayout = new QVBoxLayout(registerMapTable);
		registerMapTableLayout->setContentsMargins(0, 0, 0, 0);
		registerMapTableLayout->setSpacing(0);
		Utils::removeLayoutMargins(registerMapTableLayout);
		registerMapTable->setLayout(registerMapTableLayout);
		tool->addWidgetToCentralContainerHelper(registerMapTable);

		QWidget *tableHeadWidget = new QWidget(this);
		QHBoxLayout *tableHeadWidgetLayout = new QHBoxLayout(tableHeadWidget);
		tableHeadWidgetLayout->setSpacing(4);
		tableHeadWidgetLayout->setContentsMargins(2, 2, 2, 2);
		tableHeadWidget->setLayout(tableHeadWidgetLayout);

		QWidget *registerTableHead = new QWidget(tableHeadWidget);
		Style::setBackgroundColor(registerTableHead,
					  Style::getAttribute(json::theme::interactive_secondary_disabled));

		QHBoxLayout *registerTableHeadLayout = new QHBoxLayout(registerTableHead);
		registerTableHeadLayout->setSpacing(0);
		registerTableHead->setLayout(registerTableHeadLayout);

		QLabel *registerTableHeadName = new QLabel("Register", registerTableHead);
		registerTableHeadLayout->addWidget(registerTableHeadName);
		registerTableHead->setFixedWidth(180);

		QWidget *colBitCount = new QWidget(tableHeadWidget);
		Style::setBackgroundColor(colBitCount,
					  Style::getAttribute(json::theme::interactive_secondary_disabled));

		QHBoxLayout *tableHead = new QHBoxLayout(colBitCount);
		colBitCount->setLayout(tableHead);

		for(int i = registerMapTemplate->bitsPerRow() - 1; i >= 0; i--) {
			tableHead->addWidget(new QLabel("Bit" + QString::number(i)), 1);
		}

		tableHeadWidgetLayout->addWidget(registerTableHead, 1);
		tableHeadWidgetLayout->addWidget(colBitCount, 8);
		registerMapTableLayout->addWidget(tableHeadWidget);

		registerMapTableWidget = new RegisterMapTable(registerMapTemplate->getRegisterList(), this);
		registerMapTable->setProperty("tutorial_name", "REGISTER_MAP");

		QWidget *aux = registerMapTableWidget->getWidget();
		if(aux) {
			registerMapTableLayout->addWidget(aux);
		}

		QObject::connect(registerMapTableWidget, &RegisterMapTable::registerSelected, this,
				 [this, registerMapTemplate](uint32_t address) {
					 registerController->blockSignals(true);
					 registerMapTableWidget->setRegisterSelected(address);
					 registerChanged(registerMapTemplate->getRegisterTemplate(address));
					 registerController->blockSignals(false);
					 if(autoread && this->registerMapAccess) {
						 this->registerMapAccess->read(address);
					 }
				 });

		QObject::connect(registerController, &RegisterController::registerAddressChanged, this,
				 [this, registerMapTemplate](uint32_t address) {
					 registerChanged(registerMapTemplate->getRegisterTemplate(address));
					 registerMapTableWidget->scrollTo(address);
					 if(autoread && this->registerMapAccess) {
						 this->registerMapAccess->read(address);
					 }
				 });
	}

	if(registerMapAccess) {
		// the returned tasks are dropped: the results reach the UI through the store's signals
		QObject::connect(registerController, &RegisterController::requestRead, registerMapAccess,
				 [registerMapAccess](uint32_t address) { registerMapAccess->read(address); });
		QObject::connect(registerController, &RegisterController::requestWrite, registerMapAccess,
				 [registerMapAccess](uint32_t address, uint32_t value) {
					 registerMapAccess->writeThenRead(address, value);
				 });
		// the Read / Write buttons follow the pending state of the selected register
		QObject::connect(registerMapStore, &RegisterMapDataStore::pendingChanged, this,
				 [this](QList<uint32_t> addresses, bool pending) {
					 if(addresses.contains(selectedAddress())) {
						 registerController->setBusy(pending);
					 }
				 });
		QObject::connect(registerController, &RegisterController::registerAddressChanged, this,
				 [this](uint32_t address) {
					 registerController->setBusy(registerMapStore->isPending(address));
				 });
		// one signal per strategy result: a range refreshes the table once
		QObject::connect(registerMapStore, &RegisterMapDataStore::dataChanged, this,
				 [this](QList<uint32_t> addresses) {
					 for(uint32_t address : addresses) {
						 showRegisterData(address);
					 }
				 });
	}

	tool->addWidgetToCentralContainerHelper(controllerWidget);
	if(!registerMapAccess) {
		// template only: the whole read/write section (address, read, value, write) is hidden.
		// Its bitfield details toggle goes with it; the details stay open, as they are by default.
		controllerWidget->hide();
	}
	controllerLayout->addWidget(registerController);

	initSimpleTutorial();
	if(registerMapTemplate) {
		registerChanged(registerMapTemplate->getRegisterList()->first());
		initTutorial();

	} else {
		tool->centralContainer()->layout()->addItem(
			new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));
	}

	connect(this, &DeviceRegisterMap::tutorialAborted, this, &DeviceRegisterMap::abortTutorial);
}

DeviceRegisterMap::~DeviceRegisterMap()
{
	delete layout;
	if(registerController)
		delete registerController;
	if(registerMapTableWidget)
		delete registerMapTableWidget;
	if(docRegisterMapTable)
		delete docRegisterMapTable;
	if(registerDetailedWidget)
		delete registerDetailedWidget;
	delete tool;
}

void DeviceRegisterMap::registerChanged(RegisterModel *regModel)
{
	registerController->registerChanged(regModel->getAddress());
	registerController->registerValueChanged("N/R");

	if(registerDetailedWidget) {
		delete registerDetailedWidget;
	}

	registerDetailedWidget = new RegisterDetailedWidget(regModel, tool->bottomContainer());
	registerDetailedWidget->setProperty("tutorial_name", "DETAILED_BITFIELDS");
	registerDetailedWidget->setMaximumHeight(140);
	tool->bottomCentral()->layout()->addWidget(registerDetailedWidget);

	QObject::connect(registerDetailedWidget, &RegisterDetailedWidget::bitFieldValueChanged, registerController,
			 &RegisterController::registerValueChanged);
	QObject::connect(registerController, &RegisterController::valueChanged, this, [this](QString val) {
		registerDetailedWidget->updateBitFieldsValue(Utils::convertQStringToUint32(val));
	});

	if(registerMapStore) {
		uint32_t address = regModel->getAddress();
		// a table selection blocks the controller's signals, so the pending state is set here
		registerController->setBusy(registerMapStore->isPending(address));
		const component::RegisterData *entry = registerMapStore->get(address);
		if(entry && entry->value) {
			registerDetailedWidget->updateBitFieldsValue(*entry->value);
			registerController->registerValueChanged(Utils::convertToHexa(*entry->value, regModel->getWidth()));
		}
	}
}

void DeviceRegisterMap::showRegisterData(uint32_t address)
{
	const component::RegisterData *entry = registerMapStore->get(address);
	if(!entry || !entry->value) {
		return;
	}
	const uint32_t value = *entry->value;
	if(registerMapTemplate) {
		registerMapTableWidget->valueUpdated(address, value);
	}
	// the value field and the bitfields belong to the selected register only
	if(address != selectedAddress()) {
		return;
	}
	registerController->registerValueChanged(Utils::convertToHexa(value, registerWidth()));
	if(registerDetailedWidget) {
		registerDetailedWidget->updateBitFieldsValue(value);
	}
}

uint32_t DeviceRegisterMap::selectedAddress() { return Utils::convertQStringToUint32(registerController->getAddress()); }

int DeviceRegisterMap::registerWidth()
{
	// one width per device, the width of the first register
	RegisterModel *reg0 = registerMapTemplate ? registerMapTemplate->getRegisterTemplate(0) : nullptr;
	return reg0 ? reg0->getWidth() : 8;
}

void DeviceRegisterMap::toggleAutoread(bool toggled) { autoread = toggled; }

void DeviceRegisterMap::applyFilters(QString filter)
{
	if(registerMapTemplate) {
		registerMapTableWidget->setFilters(
			Search::searchForRegisters(registerMapTemplate->getRegisterList(), filter));
	}
}

bool DeviceRegisterMap::hasTemplate()
{
	if(registerMapTemplate) {
		return true;
	}

	return false;
}

bool DeviceRegisterMap::hasReadWrite() const { return registerMapAccess != nullptr; }

bool DeviceRegisterMap::getAutoread() { return autoread; }
void DeviceRegisterMap::startTutorial() { registerController->startTutorial(); }
void DeviceRegisterMap::startSimpleTutorial() { registerController->startSimpleTutorial(); }

void DeviceRegisterMap::initSettings()
{
	if(!registerMapAccess) {
		return;
	}
	// the returned tasks are dropped: the results reach the UI through the store's signals
	QObject::connect(this, &DeviceRegisterMap::requestReadInterval, registerMapAccess,
			 [this](uint32_t start, uint32_t end) { registerMapAccess->readRange(start, end); });
	QObject::connect(this, &DeviceRegisterMap::requestWriteFromFile, registerMapAccess,
			 [this](QString path) { registerMapAccess->writeFromFile(path); });
	QObject::connect(this, &DeviceRegisterMap::requestRegisterDump, registerMapAccess,
			 [this](QString path, bool readFirst, uint32_t start, uint32_t end) {
				 std::optional<QPair<uint32_t, uint32_t>> range;
				 if(readFirst) {
					 range = qMakePair(start, end);
				 }
				 registerMapAccess->dump(path, range);
			 });
}

void DeviceRegisterMap::initTutorial()
{

	controllerTutorialFinish = connect(registerController, &RegisterController::tutorialFinished, this, [this]() {
		QWidget *parent = Util::findContainingWindow(this);
		tutorial = new gui::TutorialBuilder(this, ":/registermap/tutorial_chapters.json", "device_register_map",
						    parent);

		connect(tutorial, &gui::TutorialBuilder::finished, this, [this]() { Q_EMIT tutorialFinished(); });
		connect(tutorial, &gui::TutorialBuilder::aborted, this, &DeviceRegisterMap::tutorialAborted);
		tutorial->setTitle("Tutorial");
		tutorial->start();
	});
}

void DeviceRegisterMap::initSimpleTutorial()
{
	connect(registerController, &RegisterController::tutorialAborted, this, &DeviceRegisterMap::tutorialAborted);
	connect(registerController, &RegisterController::simpleTutorialFinished, this,
		&DeviceRegisterMap::simpleTutorialFinished);
}

void DeviceRegisterMap::abortTutorial() { disconnect(controllerTutorialFinish); }
