#include "registermaptool.hpp"
#include "utils.hpp"

#include <QBoxLayout>
#include <QComboBox>
#include <deviceregistermap.hpp>
#include <deviceregistermap.hpp>
#include <registermapsettingsmenu.hpp>
#include <registermaptemplate.hpp>
#include <registermapvalues.hpp>
#include <registermapvalues.hpp>
#include <registermapvalues.hpp>
#include <regmapstylehelper.hpp>
#include <searchbarwidget.hpp>
#include <stylehelper.h>
#include <toolbuttons.h>
#include <xmlfilemanager.hpp>

#include <readwrite/fileregisterreadstrategy.hpp>
#include <readwrite/fileregisterwritestrategy.hpp>
#include <readwrite/iioregisterreadstrategy.hpp>
#include <readwrite/iioregisterwritestrategy.hpp>

using namespace scopy::regmap;

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
	lay->addWidget(tool);

	InfoBtn *infoBtn = new InfoBtn(this);
	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	// TODO on info btn click open wiki page

	settings = new scopy::regmap::RegisterMapSettingsMenu(this);
	tool->rightStack()->add("settings", settings);
	tool->setRightContainerWidth(settings->sizeHint().width());

	settingsMenu = new GearBtn(this);

	connect(settingsMenu, &QAbstractButton::toggled, this, [=](bool toggled) {
		tool->openRightContainerHelper(toggled);
		tool->requestMenu("settings");
	});
	Q_EMIT settingsMenu->toggled(false);
	tool->topContainerMenuControl()->setStyleSheet("background : transparent; ");
	tool->addWidgetToTopContainerMenuControlHelper(settingsMenu, TTA_RIGHT);

	searchBarWidget = new scopy::regmap::SearchBarWidget();
	searchBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	QObject::connect(searchBarWidget, &scopy::regmap::SearchBarWidget::requestSearch, this,
			 [=](QString searchParam) {
				 deviceList->value(registerDeviceList->currentText())->applyFilters(searchParam);
			 });

	tool->addWidgetToTopContainerHelper(searchBarWidget, TTA_LEFT);

	registerDeviceList = new QComboBox(tool->topContainer());
	RegmapStyleHelper::comboboxStyle(registerDeviceList);
	registerDeviceList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	QObject::connect(registerDeviceList, &QComboBox::currentTextChanged, this,
			 &RegisterMapTool::updateActiveRegisterMap);
	tool->addWidgetToTopContainerMenuControlHelper(registerDeviceList, TTA_LEFT);

	deviceList = new QMap<QString, DeviceRegisterMap *>();
	tool->getContainerSpacer(tool->topContainer())->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
	tool->getContainerSpacer(tool->topContainerMenuControl())
		->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
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
	}
}

void RegisterMapTool::toggleSettingsMenu(QString registerName, bool toggle)
{
	if(toggle) {
		QObject::connect(settings, &scopy::regmap::RegisterMapSettingsMenu::autoreadToggled,
				 deviceList->value(registerName), &regmap::DeviceRegisterMap::toggleAutoread);
		QObject::connect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestRead,
				 deviceList->value(registerName), &regmap::DeviceRegisterMap::requestRead);
		QObject::connect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestRegisterDump,
				 deviceList->value(registerName), &regmap::DeviceRegisterMap::requestRegisterDump);
		QObject::connect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestWrite,
				 deviceList->value(registerName), &regmap::DeviceRegisterMap::requestWrite);
	} else {
		QObject::disconnect(settings, &scopy::regmap::RegisterMapSettingsMenu::autoreadToggled,
				    deviceList->value(registerName), &regmap::DeviceRegisterMap::toggleAutoread);
		QObject::disconnect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestRead,
				    deviceList->value(registerName), &regmap::DeviceRegisterMap::requestRead);
		QObject::disconnect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestRegisterDump,
				    deviceList->value(registerName), &regmap::DeviceRegisterMap::requestRegisterDump);
		QObject::disconnect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestWrite,
				    deviceList->value(registerName), &regmap::DeviceRegisterMap::requestWrite);
	}
}

void RegisterMapTool::updateActiveRegisterMap(QString registerName)
{
	if(activeRegisterMap != "" && registerName != activeRegisterMap) {
		deviceList->value(activeRegisterMap)->hide();
		toggleSettingsMenu(activeRegisterMap, false);
		deviceList->value(registerName)->show();
		toggleSettingsMenu(registerName, true);
		toggleSearchBarVisible(deviceList->value(registerName)->hasTemplate());

		activeRegisterMap = registerName;
	}
}

void RegisterMapTool::toggleSearchBarVisible(bool visible) { searchBarWidget->setVisible(visible); }
