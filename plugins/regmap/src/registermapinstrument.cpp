#include "registermapinstrument.hpp"

#include "deviceregistermap.hpp"
#include "readwrite/fileregisterreadstrategy.hpp"
#include "readwrite/fileregisterwritestrategy.hpp"
#include "readwrite/iioregisterreadstrategy.hpp"
#include "readwrite/iioregisterwritestrategy.hpp"
#include "registermapsettingsmenu.hpp"
#include "registermaptemplate.hpp"
#include "registermapvalues.hpp"
#include "utils.hpp"
#include "xmlfilemanager.hpp"

#include <QComboBox>
#include <QDebug>
#include <QElapsedTimer>
#include <QTabWidget>
#include <qboxlayout.h>

#include <searchbarwidget.hpp>
#include <tool_view_builder.hpp>

using namespace scopy;
using namespace regmap;

RegisterMapInstrument::RegisterMapInstrument(QWidget *parent)
	: QWidget{parent}
{
	layout = new QVBoxLayout(this);
	Utils::removeLayoutMargins(layout);
	this->setLayout(layout);
	mainWidget = new QWidget();
	mainWidget->setLayout(new QVBoxLayout(mainWidget));

	scopy::gui::ToolViewRecipe recepie;
	recepie.hasChannels = false;
	recepie.hasHeader = true;
	recepie.hasPairSettingsBtn = true;
	recepie.hasHamburgerMenuBtn = false;

	toolView = scopy::gui::ToolViewBuilder(recepie, nullptr, parent).build();
	toolView->addFixedCentralWidget(mainWidget, 0, 0, 0, 0);
	toolView->getTopExtraWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	Utils::removeLayoutMargins(toolView->getCentralWidget()->layout());

	registerDeviceList = new QComboBox();
	activeRegisterMap = "";
	settings = new scopy::regmap::RegisterMapSettingsMenu(this);
	QObject::connect(registerDeviceList, &QComboBox::currentTextChanged, this,
			 &RegisterMapInstrument::updateActiveRegisterMap);
	toolView->addTopExtraWidget(registerDeviceList);

	searchBarWidget = new scopy::regmap::SearchBarWidget();
	searchBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	QObject::connect(searchBarWidget, &scopy::regmap::SearchBarWidget::requestSearch, this,
			 [=](QString searchParam) {
				 tabs->value(registerDeviceList->currentText())->applyFilters(searchParam);
			 });

	toolView->addTopExtraWidget(searchBarWidget);
	toolView->getTopExtraWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	layout->addWidget(toolView);

	tabs = new QMap<QString, DeviceRegisterMap *>();
	tabsInfo = new QMap<QString, TabInfo *>();
}

RegisterMapInstrument::~RegisterMapInstrument()
{
	delete mainWidget;
	delete tabs;
}

void RegisterMapInstrument::toggleSettingsMenu(QString registerName, bool toggle)
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

void RegisterMapInstrument::generateDeviceRegisterMap(TabInfo *tabInfo)
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
	mainWidget->layout()->addWidget(regMap);
	tabs->value(tabInfo->getDeviceName())->hide();
}

void RegisterMapInstrument::updateActiveRegisterMap(QString registerName)
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

void RegisterMapInstrument::toggleSearchBarVisible(bool visible) { searchBarWidget->setVisible(visible); }

RegisterMapValues *RegisterMapInstrument::getRegisterMapValues(struct iio_device *dev)
{
	IIORegisterReadStrategy *iioReadStrategy = new IIORegisterReadStrategy(dev);
	IIORegisterWriteStrategy *iioWriteStrategy = new IIORegisterWriteStrategy(dev);
	RegisterMapValues *registerMapValues = new RegisterMapValues();
	registerMapValues->setReadStrategy(iioReadStrategy);
	registerMapValues->setWriteStrategy(iioWriteStrategy);

	return registerMapValues;
}

RegisterMapValues *RegisterMapInstrument::getRegisterMapValues(QString filePath)
{
	FileRegisterReadStrategy *fileRegisterReadStrategy = new FileRegisterReadStrategy(filePath);
	FileRegisterWriteStrategy *fileRegisterWriteStrategy = new FileRegisterWriteStrategy(filePath);
	RegisterMapValues *registerMapValues = new RegisterMapValues();
	registerMapValues->setReadStrategy(fileRegisterReadStrategy);
	registerMapValues->setWriteStrategy(fileRegisterWriteStrategy);

	return registerMapValues;
}

void RegisterMapInstrument::addTab(iio_device *dev, QString title) { addTab(dev, title, ""); }

void RegisterMapInstrument::addTab(QString filePath, QString title) { addTab(nullptr, title, filePath); }

void RegisterMapInstrument::addTab(iio_device *dev, QString title, QString xmlPath)
{

	tabsInfo->insert(title, new TabInfo(dev, title, xmlPath));

	registerDeviceList->addItem(title);

	if(first) {
		// the first regmap is set active
		activeRegisterMap = title;
		first = false;
		generateDeviceRegisterMap(tabsInfo->value(title));
		toolView->setGeneralSettingsMenu(settings, true);
		tabs->value(title)->show();
		toggleSettingsMenu(title, true);
	}
}
