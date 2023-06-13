#include "registermapinstrument.hpp"
#include "readwrite/iioregisterreadstrategy.hpp"
#include "readwrite/iioregisterwritestrategy.hpp"
#include "readwrite/fileregisterwritestrategy.hpp"
#include "readwrite/fileregisterreadstrategy.hpp"
#include "deviceregistermap.hpp"
#include "registermapsettingsmenu.hpp"
#include "registermaptemplate.hpp"
#include "registermapvalues.hpp"
#include "utils.hpp"
#include "xmlfilemanager.hpp"
#include <QTabWidget>
#include <qboxlayout.h>
#include <iio.h>
#include <tool_view_builder.hpp>
#include <searchbarwidget.hpp>
#include <QComboBox>

using namespace scopy;
using namespace regmap;
using namespace regmap::gui;

RegisterMapInstrument::RegisterMapInstrument(QWidget *parent)
    : QWidget{parent}
{
    layout = new QVBoxLayout();
    Utils::removeLayoutMargins(layout);
    this->setLayout(layout);
    mainWidget = new QWidget();
    mainWidget->setLayout(new QVBoxLayout());

    scopy::gui::ToolViewRecipe recepie;
    recepie.hasChannels = false;
    recepie.hasHeader = true;
    recepie.hasPairSettingsBtn = true;
    recepie.hasHamburgerMenuBtn = false;
    channelManager = new scopy::gui::ChannelManager();

    toolView = scopy::gui::ToolViewBuilder(recepie, channelManager, parent).build();
    toolView->addFixedCentralWidget(mainWidget,0,0,0,0);

    registerDeviceList = new QComboBox();
    activeRegisterMap = "";
    settings = new scopy::regmap::gui::RegisterMapSettingsMenu(this);
    QObject::connect(registerDeviceList, &QComboBox::currentTextChanged, this, &RegisterMapInstrument::updateActiveRegisterMap);
    toolView->addTopExtraWidget(registerDeviceList);

    searchBarWidget =  new SearchBarWidget();
    searchBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QObject::connect(searchBarWidget, &SearchBarWidget::requestSearch, this, [=](QString searchParam){
        tabs->value(registerDeviceList->currentText())->applyFilters(searchParam);
    });

    toolView->addTopExtraWidget(searchBarWidget);
    layout->addWidget(toolView);

    tabs = new QMap<QString, scopy::regmap::DeviceRegisterMap*>();
}

RegisterMapInstrument::~RegisterMapInstrument()
{
    delete mainWidget;
    delete tabs;
}


void RegisterMapInstrument::toggleSettingsMenu(QString registerName, bool toggle)
{
    if (toggle) {
        QObject::connect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::autoreadToggled, tabs->value(registerName), &regmap::DeviceRegisterMap::toggleAutoread);
        QObject::connect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::requestRead, tabs->value(registerName), &regmap::DeviceRegisterMap::requestRead);
        QObject::connect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::requestRegisterDump, tabs->value(registerName), &regmap::DeviceRegisterMap::requestRegisterDump);
        QObject::connect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::requestWrite, tabs->value(registerName), &regmap::DeviceRegisterMap::requestWrite);
    } else {
        QObject::disconnect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::autoreadToggled, tabs->value(registerName), &regmap::DeviceRegisterMap::toggleAutoread);
        QObject::disconnect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::requestRead, tabs->value(registerName), &regmap::DeviceRegisterMap::requestRead);
        QObject::disconnect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::requestRegisterDump, tabs->value(registerName), &regmap::DeviceRegisterMap::requestRegisterDump);
        QObject::disconnect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::requestWrite, tabs->value(registerName), &regmap::DeviceRegisterMap::requestWrite);
    }
}

void RegisterMapInstrument::updateActiveRegisterMap(QString registerName)
{
    if (activeRegisterMap != "" && registerName != activeRegisterMap) {
        tabs->value(activeRegisterMap)->hide();
        toggleSettingsMenu(activeRegisterMap, false);

        tabs->value(registerName)->show();
        toggleSettingsMenu(registerName, true);
        toggleSearchBarVisible(tabs->value(registerName)->hasTemplate());
        activeRegisterMap = registerName;
    }
}

void RegisterMapInstrument::toggleSearchBarVisible(bool visible)
{
    searchBarWidget->setVisible(visible);
}


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

void RegisterMapInstrument::addTab(iio_device *dev, QString title)
{
    addTab(dev,title, "");
}

void RegisterMapInstrument::addTab(QString filePath, QString title)
{
    addTab(nullptr,title, filePath);

}

void RegisterMapInstrument::addTab(iio_device *dev, QString title, QString xmlPath)
{
    RegisterMapTemplate *registerMapTemplate = nullptr;
    if (xmlPath != "") {
        registerMapTemplate = new RegisterMapTemplate();
        XmlFileManager xmlFileManager(dev, xmlPath);
        if (!xmlFileManager.getAllRegisters()->isEmpty()){
            registerMapTemplate->setRegisterList(xmlFileManager.getAllRegisters());
        }
    }
    RegisterMapValues *registerMapValues = nullptr;
    if (dev){
        registerMapValues = getRegisterMapValues(dev);
    } else {
        registerMapValues = getRegisterMapValues(xmlPath);
    }
    scopy::regmap::DeviceRegisterMap *regMap = new scopy::regmap::DeviceRegisterMap(registerMapTemplate,registerMapValues);

    tabs->insert(title, regMap);
    mainWidget->layout()->addWidget(regMap);
    registerDeviceList->addItem(title);

    if (first) {
        tabs->value(title)->hide();
    } else {
        // the first regmap is set active
        activeRegisterMap = title;
        toggleSettingsMenu(title, true);
        first = true;
        toolView->setGeneralSettingsMenu(settings, true);
    }
}
