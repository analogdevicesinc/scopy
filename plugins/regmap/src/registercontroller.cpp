#include "registercontroller.hpp"
#include "qtextspinbox.hpp"

#include <QVBoxLayout>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qdebug.h>
#include <QPushButton>
#include <qspinbox.h>

RegisterController::RegisterController(QWidget *parent)
    : QWidget{parent}
{
    layout= new QVBoxLayout();
    setLayout(layout);

    regValue = new QLineEdit(this);
    regValue->setText("Not Read");
    QObject::connect(regValue, &QLineEdit::textChanged, this, &RegisterController::valueChanged);

    // make address a spinbox with custom value for custom hexa values ?
    QHBoxLayout *addressLayout = new QHBoxLayout();
    addressLayout->addWidget(new QLabel("Address: "));
    addressPicker = new QSpinBox();
    addressPicker->setDisplayIntegerBase(16);
    addressPicker->setMinimum(0);

    QObject::connect(addressPicker, &QSpinBox::textChanged, this, [=](QString address){
        addressChanged = true;
        bool ok;
        Q_EMIT	registerAddressChanged(address.toInt(&ok,16));
    });

    addressLayout->addWidget(addressPicker);

    QHBoxLayout *valueLayout = new QHBoxLayout();
    valueLayout->addWidget(new QLabel("Value: "));
    valueLayout->addWidget(regValue);


    QHBoxLayout *buttonsLayout = new QHBoxLayout();
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

    buttonsLayout->addWidget(readButton);
    buttonsLayout->addWidget(writeButton);

    layout->addLayout(addressLayout);
    layout->addLayout(valueLayout);
    layout->addLayout(buttonsLayout);

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

