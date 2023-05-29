#include "registercontroller.hpp"
#include "qtextspinbox.hpp"

#include <QVBoxLayout>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qdebug.h>
#include <QPushButton>
#include <qspinbox.h>

#include "utils.hpp"

RegisterController::RegisterController(QWidget *parent)
    : QWidget{parent}
{
    layout= new QHBoxLayout();
    setLayout(layout);

    regValue = new QLineEdit(this);
    regValue->setText("Not Read");
    QObject::connect(regValue, &QLineEdit::textChanged, this, &RegisterController::valueChanged);

    QVBoxLayout *auxLayout = new QVBoxLayout();
    // make address a spinbox with custom value for custom hexa values ?
    QHBoxLayout *addressLayout = new QHBoxLayout();
    addressLayout->addWidget(new QLabel("Address: "),1);
    addressPicker = new QSpinBox();
    addressPicker->setDisplayIntegerBase(16);
    addressPicker->setMinimum(0);
    addressPicker->setMaximum(INT_MAX);
    addressPicker->setPrefix("0x");

    QObject::connect(addressPicker, &QSpinBox::textChanged, this, [=](QString address){
        addressChanged = true;
        bool ok;
        Q_EMIT	registerAddressChanged(address.toInt(&ok,16));
    });

    addressLayout->addWidget(addressPicker,6);

    QHBoxLayout *valueLayout = new QHBoxLayout();
    valueLayout->addWidget(new QLabel("Value: "),1);
    valueLayout->addWidget(regValue,6);



    readButton = new QPushButton("Read");
    //request read
    QObject::connect( readButton, &QPushButton::clicked, this , [=](){
        bool ok;
        Q_EMIT requestRead(addressPicker->text().toInt(&ok,16));
    });


    writeButton = new QPushButton("Write");
    //request write on register
    QObject::connect( writeButton, &QPushButton::clicked, this, [=](){
        bool ok;
        Q_EMIT requestWrite(addressPicker->text().toInt(&ok,16), regValue->text().toInt(&ok,16));
    });

    scopy::regmap::Utils::applyScopyButtonStyle(readButton);
    scopy::regmap::Utils::applyScopyButtonStyle(writeButton);

    QVBoxLayout *buttonsLayout = new QVBoxLayout();
    buttonsLayout->addWidget(readButton);
    buttonsLayout->addWidget(writeButton);

    auxLayout->addLayout(addressLayout);
    auxLayout->addLayout(valueLayout);

    layout->addLayout(auxLayout,8);
    layout->addLayout(buttonsLayout,1);

    QSpacerItem *spacer = new QSpacerItem(10,10,QSizePolicy::Preferred, QSizePolicy::Expanding);
    layout->addItem(spacer);

}

RegisterController::~RegisterController()
{
    delete layout;
    delete regValue;
    delete readButton;
    delete writeButton;
}

void RegisterController::registerChanged(uint32_t address)
{
    if (!addressChanged) {
        addressPicker->setValue(address);
    } else {
        addressChanged = false;
    }
}

void RegisterController::registerValueChanged(QString value)
{
    regValue->setText(value);
}

