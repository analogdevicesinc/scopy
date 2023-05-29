#include "registercontroller.hpp"
#include "deviceregistermap.hpp"

#include "registermapvalues.hpp"
#include "searchbarwidget.hpp"
#include "registermaptemplate.hpp"
#include "search.hpp"
#include "registermapsettingsmenu.hpp"
#include "utils.hpp"

#include <QLineEdit>
#include <QPushButton>
#include <iio.h>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <utils.h>
#include "readwrite/iioregisterreadstrategy.hpp"
#include "readwrite/iioregisterwritestrategy.hpp"
#include "register/registerdetailedwidget.hpp"
#include "register/registermodel.hpp"
#include "register/registersimplewidgetfactory.hpp"
#include <src/readwrite/fileregisterwritestrategy.hpp>
#include <src/recycerview/registermaptable.hpp>
#include <tool_view_builder.hpp>
#include "dynamicWidget.h"

using namespace scopy;
using namespace regmap;
using namespace regmap::gui;

DeviceRegisterMap::DeviceRegisterMap(RegisterMapTemplate *registerMapTemplate, RegisterMapValues *registerMapValues,  QWidget *parent)
    : registerMapValues(registerMapValues),
    registerMapTemplate(registerMapTemplate),
    QWidget{parent}
{

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout( layout);

    deviceRegisterMapLayout = new QVBoxLayout();

    QWidget *toolViewWidget = new QWidget();
    toolViewWidget->setLayout(deviceRegisterMapLayout);

    mainWindow = new QMainWindow(this);

    deviceRegisterMapLayout->addWidget(mainWindow);

    initSettings();

    layout->addWidget(toolViewWidget);
    registerController = new RegisterController();

    if (registerMapTemplate) {
        QWidget *registerMapTable = new QWidget();
        QVBoxLayout *registerMapTableLayout = new QVBoxLayout();
        registerMapTable->setLayout(registerMapTableLayout);

        searchBarWidget = new SearchBarWidget();

        QWidget *tableHeadWidget = new QWidget();
        scopy::setDynamicProperty(tableHeadWidget, "has_frame", true);
        QHBoxLayout *tableHead = new QHBoxLayout();
        tableHeadWidget->setLayout(tableHead);
        tableHead->addWidget(new QLabel(""), 1);

        for (int i = Utils::getBitsPerRow(); i >= 0; i--) {
            tableHead->addWidget(new QLabel("Bit"+QString::number(i)),1);
        }

        registerMapTableWidget = new RegisterMapTable(registerMapTemplate->getRegisterList());

        QObject::connect(searchBarWidget, &SearchBarWidget::requestSearch, this, [=](QString searchParam){
            registerMapTableWidget->setFilters(Search::searchForRegisters(registerMapTemplate->getRegisterList(),searchParam));
        });
        QObject::connect(registerMapTableWidget, &RegisterMapTable::registerSelected, this, [=](uint32_t address){
            registerChanged(registerMapTemplate->getRegisterTemplate(address));
        });

        registerMapTableLayout->addWidget(searchBarWidget);
        registerMapTableLayout->addWidget(tableHeadWidget);
        registerMapTableLayout->addWidget(registerMapTableWidget->getWidget());
        docRegisterMapTable = DockerUtils::createDockWidget(mainWindow, registerMapTable, "Register map");
        mainWindow->addDockWidget(Qt::TopDockWidgetArea, docRegisterMapTable);

        QObject::connect(registerController, &RegisterController::registerAddressChanged, this , [=](uint32_t address){
            registerChanged(registerMapTemplate->getRegisterTemplate(address));
            registerMapTableWidget->scrollTo(address);
            if (autoread) {
                Q_EMIT registerMapValues->requestRead(address);
            }
        });
    }

    QObject::connect(registerController, &RegisterController::requestRead, registerMapValues, &RegisterMapValues::requestRead);
    QObject::connect(registerController, &RegisterController::requestWrite, registerMapValues, &RegisterMapValues::requestWrite);
    QObject::connect(registerMapValues, &RegisterMapValues::registerValueChanged, this, [=](uint32_t address, uint32_t value){
        registerController->registerValueChanged(Utils::convertToHexa(value, 8));
        if (registerMapTemplate) {
            registerMapTableWidget->valueUpdated(address, value);
            registerDetailedWidget->updateBitFieldsValue(value);
        }
    });

    mainWindow->setCentralWidget(registerController);

    if (registerMapTemplate) {
        registerChanged(registerMapTemplate->getRegisterTemplate(2000));
    }
}

DeviceRegisterMap::~DeviceRegisterMap()
{
    delete deviceRegisterMapLayout;
    if (registerController) delete registerController;
    if (registerMapTableWidget) delete registerMapTableWidget;
    if (docRegisterMapTable) delete docRegisterMapTable;
    if (registerDetailedWidget) delete registerDetailedWidget;
    if (mainWindow) delete mainWindow;
}

void DeviceRegisterMap::registerChanged(RegisterModel *regModel)
{
    registerController->registerChanged(regModel->getAddress());
    registerController->registerValueChanged("Not Read");

    if (dockRegisterDetailedWidget) {
        delete registerDetailedWidget;
        delete dockRegisterDetailedWidget;
    }

    registerDetailedWidget = new RegisterDetailedWidget(regModel);
    dockRegisterDetailedWidget =  DockerUtils::createDockWidget(mainWindow, registerDetailedWidget, "Detailed bitfield");
    mainWindow->addDockWidget(Qt::BottomDockWidgetArea, dockRegisterDetailedWidget);

    if (registerMapValues) {
        uint32_t address = regModel->getAddress();
        if (registerMapValues->hasValue(address)) {
            uint32_t value = registerMapValues->getValueOfRegister(address);
            registerDetailedWidget->updateBitFieldsValue(value);
            registerController->registerValueChanged(Utils::convertToHexa(value,regModel->getWidth()));
        }
    }

    QObject::connect(registerDetailedWidget, &RegisterDetailedWidget::bitFieldValueChanged, registerController, &RegisterController::registerValueChanged);
    QObject::connect(registerController, &RegisterController::valueChanged, this, [=](QString val){
        bool ok;
        registerDetailedWidget->updateBitFieldsValue(val.toUInt(&ok,16));
    });
}

void DeviceRegisterMap::toggleAutoread(bool toggled)
{
    autoread = toggled;
}

void DeviceRegisterMap::initSettings()
{
    QObject::connect(this, &DeviceRegisterMap::requestRead, registerMapValues, &RegisterMapValues::requestRead);
    QObject::connect(this, &DeviceRegisterMap::requestRegisterDump, registerMapValues, &RegisterMapValues::registerDump);
    QObject::connect(this, &DeviceRegisterMap::requestWrite, registerMapValues, &RegisterMapValues::requestWrite);
}

