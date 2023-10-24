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
#include <tabinfo.hpp>
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
	settingsMenu->toggled(false);
	tool->topContainerMenuControl()->setStyleSheet("background : transparent; ");
	tool->addWidgetToTopContainerMenuControlHelper(settingsMenu, TTA_RIGHT);

	searchBarWidget = new scopy::regmap::SearchBarWidget();
	searchBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	QObject::connect(searchBarWidget, &scopy::regmap::SearchBarWidget::requestSearch, this,
			 [=](QString searchParam) {
				 tabs->value(registerDeviceList->currentText())->applyFilters(searchParam);
			 });

	tool->addWidgetToTopContainerHelper(searchBarWidget, TTA_LEFT);

	registerDeviceList = new QComboBox(tool->topContainer());
	registerDeviceList->setStyleSheet(RegmapStyleHelper::comboboxStyle(nullptr));
	registerDeviceList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	QObject::connect(registerDeviceList, &QComboBox::currentTextChanged, this,
			 &RegisterMapTool::updateActiveRegisterMap);
	tool->addWidgetToTopContainerMenuControlHelper(registerDeviceList, TTA_LEFT);

	tabs = new QMap<QString, DeviceRegisterMap *>();
	tabsInfo = new QMap<QString, TabInfo *>();
	tool->getContainerSpacer(tool->topContainer())->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
	tool->getContainerSpacer(tool->topContainerMenuControl())
		->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
}

RegisterMapTool::~RegisterMapTool() { delete tabs; }

void RegisterMapTool::addTab(iio_device *dev, QString title, QString xmlPath, bool isAxi)
{
	tabsInfo->insert(title, new TabInfo(dev, title, xmlPath, isAxi));

	registerDeviceList->addItem(title);

	if(first) {
		// the first regmap is set active
		activeRegisterMap = title;
		first = false;
		generateDeviceRegisterMap(tabsInfo->value(title));
		tabs->value(title)->show();
		toggleSettingsMenu(title, true);
	}
}

void RegisterMapTool::addTab(iio_device *dev, QString title, bool isAxi) { addTab(dev, title, "", isAxi); }

void RegisterMapTool::addTab(QString filePath, QString title, bool isAxi) { addTab(nullptr, title, filePath, isAxi); }

RegisterMapValues *RegisterMapTool::getRegisterMapValues(iio_device *dev)
{
	IIORegisterReadStrategy *iioReadStrategy = new IIORegisterReadStrategy(dev);
	IIORegisterWriteStrategy *iioWriteStrategy = new IIORegisterWriteStrategy(dev);
	RegisterMapValues *registerMapValues = new RegisterMapValues();
	registerMapValues->setReadStrategy(iioReadStrategy);
	registerMapValues->setWriteStrategy(iioWriteStrategy);

	return registerMapValues;
}

RegisterMapValues *RegisterMapTool::getRegisterMapValues(QString filePath)
{
	FileRegisterReadStrategy *fileRegisterReadStrategy = new FileRegisterReadStrategy(filePath);
	FileRegisterWriteStrategy *fileRegisterWriteStrategy = new FileRegisterWriteStrategy(filePath);
	RegisterMapValues *registerMapValues = new RegisterMapValues();
	registerMapValues->setReadStrategy(fileRegisterReadStrategy);
	registerMapValues->setWriteStrategy(fileRegisterWriteStrategy);

	return registerMapValues;
}

void RegisterMapTool::generateDeviceRegisterMap(TabInfo *tabInfo)
{
	RegisterMapTemplate *registerMapTemplate = nullptr;
	if(!tabInfo->getXmlPath().isEmpty()) {
		registerMapTemplate = new RegisterMapTemplate(this);
		XmlFileManager xmlFileManager(tabInfo->getDev(), tabInfo->getXmlPath());
		auto aux = xmlFileManager.getAllRegisters();
		if(!aux->isEmpty()) {
			registerMapTemplate->setRegisterList(aux);
		}
	}
	RegisterMapValues *registerMapValues = nullptr;
	if(tabInfo->getDev()) {
		registerMapValues = getRegisterMapValues(tabInfo->getDev());
	} else {
		registerMapValues = getRegisterMapValues(tabInfo->getXmlPath());
	}

	DeviceRegisterMap *regMap = new DeviceRegisterMap(registerMapTemplate, registerMapValues, this);
	tabs->insert(tabInfo->getDeviceName(), regMap);
	tool->addWidgetToCentralContainerHelper(regMap);
	tabs->value(tabInfo->getDeviceName())->hide();
}

void RegisterMapTool::toggleSettingsMenu(QString registerName, bool toggle)
{
	if(toggle) {
		QObject::connect(settings, &scopy::regmap::RegisterMapSettingsMenu::autoreadToggled,
				 tabs->value(registerName), &regmap::DeviceRegisterMap::toggleAutoread);
		QObject::connect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestRead,
				 tabs->value(registerName), &regmap::DeviceRegisterMap::requestRead);
		QObject::connect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestRegisterDump,
				 tabs->value(registerName), &regmap::DeviceRegisterMap::requestRegisterDump);
		QObject::connect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestWrite,
				 tabs->value(registerName), &regmap::DeviceRegisterMap::requestWrite);
	} else {
		QObject::disconnect(settings, &scopy::regmap::RegisterMapSettingsMenu::autoreadToggled,
				    tabs->value(registerName), &regmap::DeviceRegisterMap::toggleAutoread);
		QObject::disconnect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestRead,
				    tabs->value(registerName), &regmap::DeviceRegisterMap::requestRead);
		QObject::disconnect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestRegisterDump,
				    tabs->value(registerName), &regmap::DeviceRegisterMap::requestRegisterDump);
		QObject::disconnect(settings, &scopy::regmap::RegisterMapSettingsMenu::requestWrite,
				    tabs->value(registerName), &regmap::DeviceRegisterMap::requestWrite);
	}
}

void RegisterMapTool::updateActiveRegisterMap(QString registerName)
{
	if(activeRegisterMap != "" && registerName != activeRegisterMap) {
		tabs->value(activeRegisterMap)->hide();
		toggleSettingsMenu(activeRegisterMap, false);

		if(!tabs->value(registerName)) {
			generateDeviceRegisterMap(tabsInfo->value(registerName));
		}
		tabs->value(registerName)->show();
		toggleSettingsMenu(registerName, true);
		toggleSearchBarVisible(tabs->value(registerName)->hasTemplate());

		activeRegisterMap = registerName;
	}
}

void RegisterMapTool::toggleSearchBarVisible(bool visible) { searchBarWidget->setVisible(visible); }
