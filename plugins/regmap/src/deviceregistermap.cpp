#include "registercontroller.hpp"
#include "deviceregistermap.hpp"

#include "registermapvalues.hpp"
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
    layout = new QVBoxLayout();
    Utils::removeLayoutMargins(layout);
    setLayout( layout);

    registerController = new gui::RegisterController();

    if (registerMapTemplate) {
        QWidget *registerMapTable = new QWidget();
        QVBoxLayout *registerMapTableLayout = new QVBoxLayout();
        Utils::removeLayoutMargins(registerMapTableLayout);
        registerMapTable->setLayout(registerMapTableLayout);

        QWidget *tableHeadWidget = new QWidget();
        scopy::setDynamicProperty(tableHeadWidget, "has_frame", true);
        scopy::setDynamicProperty(tableHeadWidget, "has_bottom_border", true);
        QHBoxLayout *tableHead = new QHBoxLayout();
        tableHeadWidget->setLayout(tableHead);

        QLabel *empty = new QLabel("");
        empty->setFixedWidth(130);
        tableHead->addWidget(empty, 1);

        for (int i = Utils::getBitsPerRow(); i >= 0; i--) {
            tableHead->addWidget(new QLabel("Bit"+QString::number(i)),1);
        }

        registerMapTableWidget = new gui::RegisterMapTable(registerMapTemplate->getRegisterList());

        QObject::connect(registerMapTableWidget, &gui::RegisterMapTable::registerSelected, this, [=](uint32_t address){
            registerChanged(registerMapTemplate->getRegisterTemplate(address));
        });

        registerMapTableLayout->addWidget(tableHeadWidget);
        registerMapTableLayout->addWidget(registerMapTableWidget->getWidget());
        layout->addWidget(registerMapTable,5);

        QObject::connect(registerController, &gui::RegisterController::registerAddressChanged, this , [=](uint32_t address){
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

    layout->addWidget(registerController,1);

    if (registerMapTemplate) {
        registerChanged(registerMapTemplate->getRegisterList()->first());
    }
}

DeviceRegisterMap::~DeviceRegisterMap()
{
    delete layout;
    if (registerController) delete registerController;
    if (registerMapTableWidget) delete registerMapTableWidget;
    if (docRegisterMapTable) delete docRegisterMapTable;
    if (registerDetailedWidget) delete registerDetailedWidget;
}

void DeviceRegisterMap::registerChanged(RegisterModel *regModel)
{
    registerController->registerChanged(regModel->getAddress());
    registerController->registerValueChanged("N/R");
    registerController->addNameAndDescription(regModel->getName(), regModel->getDescription());

    if (registerDetailedWidget) {
        delete registerDetailedWidget;
    }

    registerDetailedWidget = new RegisterDetailedWidget(regModel);
    layout->addWidget(registerDetailedWidget,2);

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

void DeviceRegisterMap::applyFilters(QString filter)
{
    if (registerMapTemplate) {
        registerMapTableWidget->setFilters(Search::searchForRegisters(registerMapTemplate->getRegisterList(),filter));
    }
}

bool DeviceRegisterMap::hasTemplate()
{
    if (registerMapTemplate) {
        return true;
    }

    return false;
}

void DeviceRegisterMap::initSettings()
{
    QObject::connect(this, &DeviceRegisterMap::requestRead, registerMapValues, &RegisterMapValues::requestRead);
    QObject::connect(this, &DeviceRegisterMap::requestRegisterDump, registerMapValues, &RegisterMapValues::registerDump);
    QObject::connect(this, &DeviceRegisterMap::requestWrite, registerMapValues, &RegisterMapValues::requestWrite);
}

