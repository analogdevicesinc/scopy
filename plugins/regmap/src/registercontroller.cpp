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
#include "regmapstylehelper.hpp"

using namespace scopy;
using namespace regmap;
using namespace regmap::gui;

RegisterController::RegisterController(QWidget *parent)
    : QWidget{parent}
{
    RegmapStyleHelper::BlueButton(this);
    this->setMaximumHeight(90);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    layout= new QHBoxLayout();

    QVBoxLayout *addressLayout = new QVBoxLayout();
    addressPicker = new QSpinBox();
    addressPicker->setDisplayIntegerBase(16);
    addressPicker->setMinimum(0);
    addressPicker->setMaximum(INT_MAX);
    addressPicker->setPrefix("0x");

    QObject::connect(addressPicker, &QSpinBox::textChanged, this, [=](QString address){
        bool ok;
        Q_EMIT	registerAddressChanged(address.toInt(&ok,16));
    });

    readButton = new QPushButton("Read");
    //request read
    QObject::connect( readButton, &QPushButton::clicked, this , [=](){
        bool ok;
        Q_EMIT requestRead(addressPicker->text().toInt(&ok,16));
    });

    addressLayout->addWidget(addressPicker);

    regValue = new QLineEdit(this);
    regValue->setText("N/R");
    regValue->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    QObject::connect(regValue, &QLineEdit::textChanged, this, &RegisterController::valueChanged);

    QVBoxLayout *valueLayout = new QVBoxLayout();
    //    valueLayout->addWidget(new QLabel("Value: "));
    valueLayout->addWidget(regValue);

    writeButton = new QPushButton("Write");
    //request write on register
    QObject::connect( writeButton, &QPushButton::clicked, this, [=](){
        bool ok;
        Q_EMIT requestWrite(addressPicker->value(), regValue->text().toInt(&ok,16));
    });

    QVBoxLayout *auxLayout = new QVBoxLayout();

    QHBoxLayout *labelLayout = new QHBoxLayout();

    labelLayout->addWidget(new QLabel("Address: "),1);
    labelLayout->addWidget(new QLabel("Value: "),1);

    QHBoxLayout *widgetsLayout = new QHBoxLayout();
    widgetsLayout->addWidget(addressPicker,1);
    widgetsLayout->addWidget(regValue,1);

    auxLayout->addLayout(labelLayout,1);
    auxLayout->addLayout(widgetsLayout,1);
    layout->addLayout(auxLayout);

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->addWidget(readButton);
    buttonLayout->addWidget(writeButton);

    layout->addLayout(buttonLayout);
    mainLayout->addLayout(layout);
}

RegisterController::~RegisterController()
{
    delete layout;
    delete regValue;
    delete addressPicker;
    if (nameLabel) delete nameLabel;
    if (descriptionLabel) delete descriptionLabel;
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

void RegisterController::addNameAndDescription(QString name, QString description)
{
    if (nameLabel) {
        nameLabel->setText(QString("Name: " + name));
        descriptionLabel->setText(QString("Description: " + description));
    } else {
        QVBoxLayout *nameDescription = new QVBoxLayout();
        nameLabel = new QLabel("Name: " +name);
        descriptionLabel = new QLabel("Description: " + description);
        nameDescription->addWidget(nameLabel);
        nameDescription->addWidget(descriptionLabel);
        layout->addLayout(nameDescription,1);
        layout->addItem( new QSpacerItem(10,10,QSizePolicy::Expanding, QSizePolicy::Preferred));

    }
}

