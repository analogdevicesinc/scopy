#include "registermapinstrument.hpp"
#include "readwrite/iioregisterreadstrategy.hpp"
#include "readwrite/iioregisterwritestrategy.hpp"
#include "readwrite/fileregisterwritestrategy.hpp"
#include "readwrite/fileregisterreadstrategy.hpp"
#include "deviceregistermap.hpp"
#include "registermapsettingsmenu.hpp"
#include "registermaptemplate.hpp"
#include "registermapvalues.hpp"
#include "xmlfilemanager.hpp"
#include <QTabWidget>
#include <qboxlayout.h>
#include <iio.h>
#include <tool_view_builder.hpp>

RegisterMapInstrument::RegisterMapInstrument(QWidget *parent)
    : QWidget{parent}
{
    layout = new QVBoxLayout();
    this->setLayout(layout);
    mainWidget = new QWidget();
    mainWidget->setLayout(new QVBoxLayout());

    scopy::gui::ToolViewRecipe recepie;
    recepie.hasChannels = true;
    recepie.hasHeader = false;
    channelManager = new scopy::gui::ChannelManager();

    toolView = scopy::gui::ToolViewBuilder(recepie, channelManager, parent).build();
    toolView->addFixedCentralWidget(mainWidget,0,0,0,0);
    layout->addWidget(toolView);

    tabs = new QMap<QString, scopy::regmap::DeviceRegisterMap*>();
}

RegisterMapInstrument::~RegisterMapInstrument()
{
    delete mainWidget;
    delete tabs;
}

RegisterMapValues *RegisterMapInstrument::getRegisterMapValues(iio_device *dev)
{
    IIORegisterReadStrategy *iioReadStrategy = new IIORegisterReadStrategy(dev);
    IIORegisterWriteStrategy *iioWriteStrategy = new IIORegisterWriteStrategy(dev);
    RegisterMapValues *registerMapValues = new RegisterMapValues();
    registerMapValues->setReadStrategy(iioReadStrategy);
    registerMapValues->setWriteStrategy(iioWriteStrategy);

    return registerMapValues;
}

void RegisterMapInstrument::addTab(QWidget *widget, QString title)
{
    scopy::regmap::gui::RegisterMapSettingsMenu *settings = new scopy::regmap::gui::RegisterMapSettingsMenu(this);
    toolView->buildNewChannel(channelManager,settings, false, -1, false, false, QColor("green"),
                              title, title);
}

void RegisterMapInstrument::addTab(iio_device *dev, QString title)
{
    addTab(dev,title, "");
}

void RegisterMapInstrument::addTab(iio_device *dev, QString title, QString xmlPath)
{
    RegisterMapTemplate *registerMapTemplate = nullptr;
    if (xmlPath != "") {
        registerMapTemplate = new RegisterMapTemplate();
        XmlFileManager xmlFileManager(dev, xmlPath);
        registerMapTemplate->setRegisterList(xmlFileManager.getAllRegisters());
    }

    RegisterMapValues *registerMapValues = getRegisterMapValues(dev);
    scopy::regmap::DeviceRegisterMap *regMap = new scopy::regmap::DeviceRegisterMap(registerMapTemplate,registerMapValues);
    tabs->insert(title, regMap);

    scopy::regmap::gui::RegisterMapSettingsMenu *settings = new scopy::regmap::gui::RegisterMapSettingsMenu(this);
    ChannelWidget *ch_widget = toolView->buildNewChannel(channelManager,settings, false, -1, false, false, QColor("orange"),
                                                         title, title);

    ch_widget->enableButton()->setVisible(false);
    mainWidget->layout()->addWidget(regMap);
     regMap->hide();
    QObject::connect(ch_widget, &ChannelWidget::selected, this, [=](bool selected){
        if (selected) {
            tabs->value(title)->show();
        } else {
            tabs->value(title)->hide();
        }
    });

     if (!first) {
        ch_widget->selected(true);
        first = true;
     }

    QObject::connect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::autoreadToggled, regMap, &regmap::DeviceRegisterMap::toggleAutoread);
    QObject::connect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::requestRead, regMap, &regmap::DeviceRegisterMap::requestRead);
    QObject::connect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::requestRegisterDump, regMap, &regmap::DeviceRegisterMap::requestRegisterDump);
    QObject::connect(settings, &scopy::regmap::gui::RegisterMapSettingsMenu::requestWrite, regMap, &regmap::DeviceRegisterMap::requestWrite);

}


void RegisterMapInstrument::addTab(QString filePath, QString title)
{
    FileRegisterReadStrategy *fileRegisterReadStrategy = new FileRegisterReadStrategy(filePath);
    FileRegisterWriteStrategy *fileRegisterWriteStrategy = new FileRegisterWriteStrategy(filePath);
    RegisterMapValues *registerMapValues = new RegisterMapValues();
    registerMapValues->setReadStrategy(fileRegisterReadStrategy);
    registerMapValues->setWriteStrategy(fileRegisterWriteStrategy);
}
