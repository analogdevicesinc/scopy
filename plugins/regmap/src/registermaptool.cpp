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
#include <stylehelper.h>
#include <toolbuttons.h>
#include <xmlfilemanager.hpp>

#include <readwrite/fileregisterreadstrategy.hpp>
#include <readwrite/fileregisterwritestrategy.hpp>
#include <readwrite/iioregisterreadstrategy.hpp>
#include <readwrite/iioregisterwritestrategy.hpp>

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
	lay->addWidget(tool);

	InfoBtn *infoBtn = new InfoBtn(this);
	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	connect(infoBtn, &QAbstractButton::clicked, this, [=, this]() {
		QDesktopServices::openUrl(
			QUrl("https://analogdevicesinc.github.io/scopy/plugins/registermap/registermap.html"));
	});

	searchBarWidget = new SearchBarWidget();
	searchBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	tool->addWidgetToTopContainerHelper(searchBarWidget, TTA_LEFT);
	searchBarWidget->setEnabled(false);

	settings = new RegisterMapSettingsMenu(this);
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

	QObject::connect(searchBarWidget, &SearchBarWidget::requestSearch, this, [=](QString searchParam) {
		deviceList->value(registerDeviceList->currentText())->applyFilters(searchParam);
	});
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
