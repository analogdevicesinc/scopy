#include "registercontroller.hpp"

#include "qtextspinbox.hpp"
#include "regmapstylehelper.hpp"
#include "utils.hpp"

#include <QPushButton>
#include <QVBoxLayout>
#include <qboxlayout.h>
#include <qdebug.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>

using namespace scopy;
using namespace regmap;

RegisterController::RegisterController(QWidget *parent)
	: QWidget{parent}
{
	mainLayout = new QHBoxLayout(this);
	setLayout(mainLayout);

	QHBoxLayout *addressValueLayout = new QHBoxLayout();

	QVBoxLayout *leftLayout = new QVBoxLayout();
	QHBoxLayout *labelLayout = new QHBoxLayout();
	QHBoxLayout *widgetsLayout = new QHBoxLayout();

	QVBoxLayout *buttonLayout = new QVBoxLayout();

	leftLayout->addLayout(labelLayout, 1);
	leftLayout->addLayout(widgetsLayout, 1);
	addressValueLayout->addLayout(leftLayout);
	addressValueLayout->addLayout(buttonLayout);
	mainLayout->addLayout(addressValueLayout, 1);

	addressLabel = new QLabel("Address: ", this);
	valueLabel = new QLabel("Value: ", this);

	labelLayout->addWidget(addressLabel, 1);
	labelLayout->addWidget(valueLabel, 1);

	addressPicker = new QSpinBox(this);
	addressPicker->setDisplayIntegerBase(16);
	addressPicker->setMinimum(0);
	addressPicker->setMaximum(INT_MAX);
	addressPicker->setPrefix("0x");
	QObject::connect(addressPicker, &QSpinBox::textChanged, this, [=](QString address) {
		bool ok;
		Q_EMIT registerAddressChanged(address.toInt(&ok, 16));
	});

	regValue = new QLineEdit(this);
	regValue->setText("N/R");
	regValue->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	QObject::connect(regValue, &QLineEdit::textChanged, this, &RegisterController::valueChanged);

	widgetsLayout->addWidget(addressPicker, 1);
	widgetsLayout->addWidget(regValue, 1);

	readButton = new QPushButton("Read", this);
	// request read
	QObject::connect(readButton, &QPushButton::clicked, this, [=]() {
		bool ok;
		Q_EMIT requestRead(addressPicker->text().toInt(&ok, 16));
	});

	writeButton = new QPushButton("Write", this);
	// request write on register
	QObject::connect(writeButton, &QPushButton::clicked, this, [=]() {
		bool ok;
		Q_EMIT requestWrite(addressPicker->value(), regValue->text().toInt(&ok, 16));
	});

	buttonLayout->addWidget(readButton);
	buttonLayout->addWidget(writeButton);

	applyStyle();
}

RegisterController::~RegisterController()
{

	delete regValue;
	delete addressPicker;
	if(nameLabel)
		delete nameLabel;
	if(descriptionLabel)
		delete descriptionLabel;
	delete mainLayout;
}

void RegisterController::registerChanged(uint32_t address)
{
	if(!addressChanged) {
		addressPicker->setValue(address);
	} else {
		addressChanged = false;
	}
}

void RegisterController::registerValueChanged(QString value) { regValue->setText(value); }

void RegisterController::addNameAndDescription(QString name, QString description)
{
	if(nameLabel) {
		nameLabel->setText(QString("Name: " + name));
		descriptionLabel->setText(QString("Description: " + description));
	} else {
		QVBoxLayout *nameDescriptionLayout = new QVBoxLayout();
		nameLabel = new QLabel("Name: " + name);
		descriptionLabel = new QLabel("Description: " + description);
		nameDescriptionLayout->addWidget(nameLabel);
		nameDescriptionLayout->addWidget(descriptionLabel);
		mainLayout->addLayout(nameDescriptionLayout, 1);
	}
}

void RegisterController::applyStyle()
{
	setMaximumHeight(90);
	RegmapStyleHelper::BlueButton(readButton, "");
	RegmapStyleHelper::BlueButton(writeButton, "");
	RegmapStyleHelper::labelStyle(addressLabel, "");
	RegmapStyleHelper::labelStyle(valueLabel, "");
	if(nameLabel)
		RegmapStyleHelper::labelStyle(nameLabel, "");
	if(descriptionLabel)
		RegmapStyleHelper::labelStyle(descriptionLabel, "");
}
