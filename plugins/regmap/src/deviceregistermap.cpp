#include "deviceregistermap.hpp"

#include "dynamicWidget.h"
#include "logging_categories.h"

#include <QLineEdit>
#include <QPushButton>
#include <iio.h>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <stylehelper.h>
#include <toolbuttons.h>
#include <utils.h>
#include "readwrite/iioregisterreadstrategy.hpp"
#include "readwrite/iioregisterwritestrategy.hpp"
#include "register/registerdetailedwidget.hpp"
#include "register/registermodel.hpp"
#include "register/registersimplewidgetfactory.hpp"
#include "registercontroller.hpp"
#include "registermaptemplate.hpp"
#include "registermapvalues.hpp"
#include "regmapstylehelper.hpp"
#include "search.hpp"
#include "utils.hpp"

#include <iio.h>

#include <QLineEdit>
#include <QPushButton>
#include <qboxlayout.h>
#include <qcheckbox.h>

#include <src/readwrite/fileregisterwritestrategy.hpp>
#include <src/recyclerview/registermaptable.hpp>
#include <tool_view_builder.hpp>
#include <utils.h>

using namespace scopy;
using namespace regmap;

DeviceRegisterMap::DeviceRegisterMap(RegisterMapTemplate *registerMapTemplate, RegisterMapValues *registerMapValues,
				     QWidget *parent)
	: QWidget(parent)
	, registerMapValues(registerMapValues)
	, registerMapTemplate(registerMapTemplate)
{
	layout = new QVBoxLayout(this);
	Utils::removeLayoutMargins(layout);
	setLayout(layout);

	tool = new ToolTemplate(this);
	Utils::removeLayoutMargins(tool->layout());
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainerMenuControl()->hide();
	tool->bottomCentral()->setVisible(true);
	layout->addWidget(tool);

	RegmapStyleHelper::DeviceRegisterMap(this);

	initSettings();

	registerController = new RegisterController(this);

	QWidget *controllerWidget = new QWidget(this);
	QHBoxLayout *controllerLayout = new QHBoxLayout(controllerWidget);
	Utils::removeLayoutMargins(controllerLayout);
	controllerLayout->setSpacing(0);

	if(registerMapTemplate) {
		registerController->setHasMap(true);
		QObject::connect(registerController, &RegisterController::toggleDetailedMenu, this,
				 [=](bool toggled) { tool->openBottomContainerHelper(toggled); });

		QWidget *registerMapTable = new QWidget();
		QVBoxLayout *registerMapTableLayout = new QVBoxLayout(registerMapTable);
		Utils::removeLayoutMargins(registerMapTableLayout);
		registerMapTable->setLayout(registerMapTableLayout);
		tool->addWidgetToCentralContainerHelper(registerMapTable);

		QWidget *tableHeadWidget = new QWidget(this);
		QHBoxLayout *tableHeadWidgetLayout = new QHBoxLayout(tableHeadWidget);
		tableHeadWidgetLayout->setSpacing(4);
		tableHeadWidgetLayout->setMargin(2);
		tableHeadWidget->setLayout(tableHeadWidgetLayout);

		QWidget *registerTableHead = new QWidget(tableHeadWidget);
		RegmapStyleHelper::widgetidthRoundCornersStyle(registerTableHead);

		QHBoxLayout *registerTableHeadLayout = new QHBoxLayout(registerTableHead);
		registerTableHeadLayout->setSpacing(0);
		registerTableHead->setLayout(registerTableHeadLayout);

		QLabel *registerTableHeadName = new QLabel("Register", registerTableHead);
		registerTableHeadLayout->addWidget(registerTableHeadName);
		registerTableHead->setFixedWidth(180);

		QWidget *colBitCount = new QWidget(tableHeadWidget);
		RegmapStyleHelper::widgetidthRoundCornersStyle(colBitCount);
		QHBoxLayout *tableHead = new QHBoxLayout(colBitCount);
		colBitCount->setLayout(tableHead);

		for(int i = Utils::getBitsPerRow(); i >= 0; i--) {
			tableHead->addWidget(new QLabel("Bit" + QString::number(i)), 1);
		}

		tableHeadWidgetLayout->addWidget(registerTableHead, 1);
		tableHeadWidgetLayout->addWidget(colBitCount, 8);
		registerMapTableLayout->addWidget(tableHeadWidget);
		registerMapTableWidget = new RegisterMapTable(registerMapTemplate->getRegisterList(), this);

		QWidget *aux = registerMapTableWidget->getWidget();
		if(aux) {
			registerMapTableLayout->addWidget(aux);
		}

		QObject::connect(registerMapTableWidget, &RegisterMapTable::registerSelected, this,
				 [=](uint32_t address) {
					 registerController->blockSignals(true);
					 registerMapTableWidget->setRegisterSelected(address);
					 registerChanged(registerMapTemplate->getRegisterTemplate(address));
					 registerController->blockSignals(false);
				 });

		QObject::connect(registerController, &RegisterController::registerAddressChanged, this,
				 [=](uint32_t address) {
					 registerChanged(registerMapTemplate->getRegisterTemplate(address));
					 registerMapTableWidget->scrollTo(address);
					 if(autoread) {
						 Q_EMIT registerMapValues->requestRead(address);
					 }
				 });
	}

	QObject::connect(registerController, &RegisterController::requestRead, registerMapValues,
			 [=](uint32_t address) { Q_EMIT registerMapValues->requestRead(address); });
	QObject::connect(
		registerController, &RegisterController::requestWrite, registerMapValues,
		[=](uint32_t address, uint32_t value) { Q_EMIT registerMapValues->requestWrite(address, value); });
	QObject::connect(registerMapValues, &RegisterMapValues::registerValueChanged, this,
			 [=](uint32_t address, uint32_t value) {
				 int regSize = 8;
				 if(registerMapTemplate) {
					 regSize = registerMapTemplate->getRegisterTemplate(0)->getWidth();
				 }
				 registerController->registerValueChanged(Utils::convertToHexa(value, regSize));
				 if(registerMapTemplate) {
					 registerMapTableWidget->valueUpdated(address, value);
					 registerDetailedWidget->updateBitFieldsValue(value);
				 }
			 });

	tool->addWidgetToCentralContainerHelper(controllerWidget);
	controllerLayout->addWidget(registerController);

	if(registerMapTemplate) {
		registerChanged(registerMapTemplate->getRegisterList()->first());
	} else {
		tool->centralContainer()->layout()->addItem(
			new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));
	}
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
	registerDetailedWidget->setMaximumHeight(140);
	tool->bottomCentral()->layout()->addWidget(registerDetailedWidget);

	QObject::connect(registerDetailedWidget, &RegisterDetailedWidget::bitFieldValueChanged, registerController,
			 &RegisterController::registerValueChanged);
	QObject::connect(registerController, &RegisterController::valueChanged, this, [=](QString val) {
		registerDetailedWidget->updateBitFieldsValue(Utils::convertQStringToUint32(val));
	});

	if(registerMapValues) {
		uint32_t address = regModel->getAddress();
		if(registerMapValues->hasValue(address)) {
			uint32_t value = registerMapValues->getValueOfRegister(address);
			registerDetailedWidget->updateBitFieldsValue(value);
			registerController->registerValueChanged(Utils::convertToHexa(value, regModel->getWidth()));
		}
	}
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

void DeviceRegisterMap::initSettings()
{
	QObject::connect(this, &DeviceRegisterMap::requestRead, registerMapValues, &RegisterMapValues::requestRead);
	QObject::connect(this, &DeviceRegisterMap::requestRegisterDump, registerMapValues,
			 &RegisterMapValues::registerDump);
	QObject::connect(this, &DeviceRegisterMap::requestWrite, registerMapValues, &RegisterMapValues::requestWrite);
}
