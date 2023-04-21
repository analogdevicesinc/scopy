#include "registercontroller.hpp"
#include "deviceregistermap.hpp"

#include "registermapvalues.hpp"
#include "searchbarwidget.hpp"
#include "registermaptemplate.hpp"
#include "registermaptable.hpp"
#include "search.hpp"

#include <iio.h>
#include <qboxlayout.h>
#include "readwrite/iioregisterreadstrategy.hpp"
#include "readwrite/iioregisterwritestrategy.hpp"
#include "register/registerdetailedwidget.hpp"
#include "register/registermodel.hpp"
#include "register/registersimplewidgetfactory.hpp"
#include <src/recycerview/recyclerviewadapter.hpp>

DeviceRegisterMap::DeviceRegisterMap(RegisterMapTemplate *registerMapTemplate, RegisterMapValues *registerMapValues,  QWidget *parent)
    : registerMapValues(registerMapValues),
    registerMapTemplate(registerMapTemplate),
    QWidget{parent}
{
    layout = new QVBoxLayout();
    setLayout(layout);

    registerController = new RegisterController();

    if (registerMapTemplate) {
        searchBarWidget = new SearchBarWidget();

        //        registerMapTableWidget = new RegisterMapTable(registerMapTemplate->getRegisterList());

        recycerViewAdapter = new RecyclerViewAdapter(registerMapTemplate->getRegisterList());

        QObject::connect(searchBarWidget, &SearchBarWidget::requestSearch, this, [=](QString searchParam){
            //            registerMapTableWidget->setFilters(Search::searchForRegisters(registerMapTemplate->getRegisterList(),searchParam));
            recycerViewAdapter->setFilters(Search::searchForRegisters(registerMapTemplate->getRegisterList(),searchParam));
        });

        //		QObject::connect(registerMapTableWidget, &RegisterMapTable::registerSelected, this, [=](uint32_t address){

        //			registerChanged(registerMapTemplate->getRegisterTemplate(address));
        //		});
        QObject::connect(recycerViewAdapter, &RecyclerViewAdapter::registerSelected, this, [=](uint32_t address){
            registerChanged(registerMapTemplate->getRegisterTemplate(address));
        });

        layout->addWidget(searchBarWidget);
        //		layout->addWidget(registerMapTableWidget);
        layout->addWidget(recycerViewAdapter->getWidget());


        QObject::connect(registerController, &RegisterController::registerAddressChanged, this , [=](uint32_t address){
            registerChanged(registerMapTemplate->getRegisterTemplate(address));
        });

    }

    QObject::connect(registerController, &RegisterController::requestRead, registerMapValues, &RegisterMapValues::requestRead);
    QObject::connect(registerController, &RegisterController::requestWrite, registerMapValues, &RegisterMapValues::requestWrite);
    QObject::connect(registerMapValues, &RegisterMapValues::registerValueChanged, this, [=](uint32_t address, uint32_t value){
        registerController->registerValueChanged(QString::number(value,16));
        if (registerMapTemplate) {
            //            registerMapTableWidget->valueUpdated(address, value);
            recycerViewAdapter->valueUpdated(address, value);
            registerDetailedWidget->updateBitFieldsValue(value);
        }
    });

    layout->addWidget(registerController);

    if (registerMapTemplate) {
        //	registerChanged(registerMapTemplate->getRegisterList()->begin().value(),registerMapValues->getValueOfRegister(registerMapTemplate->getRegisterList()->begin().key()));
        // temporary to init with default value for detailed register
        registerChanged(registerMapTemplate->getRegisterTemplate(2000));
    }
}

DeviceRegisterMap::~DeviceRegisterMap()
{
    delete layout;
    if (registerController) delete registerController;
    //    if (registerMapTableWidget) delete registerMapTableWidget;
    if (searchBarWidget) delete searchBarWidget;
    if (registerDetailedWidget) delete registerDetailedWidget;
}

void DeviceRegisterMap::registerChanged(RegisterModel *regModel)
{
    registerController->registerChanged(regModel->getAddress());
    registerController->registerValueChanged("Not Read");

    if (registerDetailedWidget) delete registerDetailedWidget;

    registerDetailedWidget = new RegisterDetailedWidget(regModel);
    layout->addWidget(registerDetailedWidget);
    if (registerMapValues) {
        uint32_t address = regModel->getAddress();
        if (registerMapValues->hasValue(address)) {
            uint32_t value = registerMapValues->getValueOfRegister(address);
            registerDetailedWidget->updateBitFieldsValue(value);
            registerController->registerValueChanged(QString::number(value,16));
        }


    }

    QObject::connect(registerDetailedWidget, &RegisterDetailedWidget::bitFieldValueChanged, registerController, &RegisterController::registerValueChanged);

    QObject::connect(registerController, &RegisterController::valueChanged, this, [=](QString val){
        bool ok;
        registerDetailedWidget->updateBitFieldsValue(val.toUInt(&ok,16));
    });
}

