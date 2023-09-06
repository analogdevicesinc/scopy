#include "registercontroller.hpp"
#include "deviceregistermap.hpp"

#include "registermapvalues.hpp"
#include "registermaptemplate.hpp"
#include "search.hpp"
#include "utils.hpp"
#include "logging_categories.h"

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
#include "regmapstylehelper.hpp"

using namespace scopy;
using namespace regmap;

DeviceRegisterMap::DeviceRegisterMap(RegisterMapTemplate *registerMapTemplate, RegisterMapValues *registerMapValues,  QWidget *parent)
    : QWidget(parent),
    registerMapValues(registerMapValues),
    registerMapTemplate(registerMapTemplate)
{
    layout = new QVBoxLayout(this);
    Utils::removeLayoutMargins(layout);
    setLayout( layout);

    RegmapStyleHelper::RegisterMapStyle(this);

    initSettings();

    registerController = new RegisterController(this);

    if (registerMapTemplate) {
        QWidget *registerMapTable = new QWidget();
        QVBoxLayout *registerMapTableLayout = new QVBoxLayout(registerMapTable);
        Utils::removeLayoutMargins(registerMapTableLayout);
        registerMapTable->setLayout(registerMapTableLayout);
        layout->addWidget(registerMapTable);

        QWidget *tableHeadWidget = new QWidget();
        scopy::regmap::RegmapStyleHelper::FrameWidget(tableHeadWidget);
        QHBoxLayout *tableHead = new QHBoxLayout();
        tableHeadWidget->setLayout(tableHead);

        QLabel *empty = new QLabel("");
        empty->setFixedWidth(130);
        tableHead->addWidget(empty, 1);

        for (int i = Utils::getBitsPerRow(); i >= 0; i--) {
            tableHead->addWidget(new QLabel("Bit"+QString::number(i)),1);
        }
        registerMapTableLayout->addWidget(tableHeadWidget);
        registerMapTableWidget = new RegisterMapTable(registerMapTemplate->getRegisterList(), this);

        QWidget *aux = registerMapTableWidget->getWidget();
        if (aux) {
            registerMapTableLayout->addWidget(aux);
        }

        QObject::connect(registerMapTableWidget, &RegisterMapTable::registerSelected, this, [=](uint32_t address){
            registerController->blockSignals(true);
            registerMapTableWidget->setRegisterSelected(address);
            registerChanged(registerMapTemplate->getRegisterTemplate(address));
            registerController->blockSignals(false);
        });

        QObject::connect(registerController, &RegisterController::registerAddressChanged, this , [=](uint32_t address){
            registerChanged(registerMapTemplate->getRegisterTemplate(address));
            registerMapTableWidget->scrollTo(address);
            if (autoread) {
                Q_EMIT registerMapValues->requestRead(address);
            }
        });
        layout->addWidget(registerController);
    }

    QObject::connect(registerController, &RegisterController::requestRead, registerMapValues, &RegisterMapValues::requestRead);
    QObject::connect(registerController, &RegisterController::requestWrite, registerMapValues, &RegisterMapValues::requestWrite);
    QObject::connect(registerMapValues, &RegisterMapValues::registerValueChanged, this, [=](uint32_t address, uint32_t value){
        registerController->registerValueChanged(Utils::convertToHexa(value, 8));
        if (registerMapTemplate) {
            registerMapTableWidget->valueUpdated(address, value);
            registerDetailedWidget->updateBitFieldsValue(value);
            registerDetailedWidget->registerValueUpdated(value);
        }
    });

    layout->addWidget(registerController);

    if (registerMapTemplate) {
        registerChanged(registerMapTemplate->getRegisterList()->first());
    } else {
        layout->addItem(new QSpacerItem(10,10,QSizePolicy::Preferred, QSizePolicy::Expanding));
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
    layout->addWidget(registerDetailedWidget);

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

