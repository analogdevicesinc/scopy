#include "bitfielddetailedwidget.hpp"

#include "dynamicWidget.h"
#include "utils.hpp"

#include <QtMath>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <QtMath>
#include <utils.hpp>
#include <regmapstylehelper.hpp>
#include <stylehelper.h>

#include <regmapstylehelper.hpp>
#include <utils.hpp>

using namespace scopy;
using namespace regmap;

BitFieldDetailedWidget::BitFieldDetailedWidget(QString name, QString access, int defaultValue, QString description,
					       int width, QString notes, int regOffset,
					       QVector<BitFieldOption *> *options, QWidget *parent)
	: options(options)
	, width(width)
	, description(description)
	, regOffset(regOffset)
	, access(access)
{

	mainFrame = new QFrame();
	layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setMargin(4);

	QHBoxLayout *firstLayout = new QHBoxLayout();
	nameLabel = new QLabel(name);
	nameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	firstLayout->addWidget(nameLabel);
	firstLayout->addWidget(new QLabel(QString::number(regOffset + width - 1) + ":" + QString::number(regOffset)));
	firstLayout->itemAt(1)->setAlignment(Qt::AlignRight);

	layout->addLayout(firstLayout);

	descriptionLabel = new QLabel(description);
	layout->addWidget(descriptionLabel);

	QHBoxLayout *secondLayout = new QHBoxLayout();

	lastReadValue = new QLabel("Current : N/R");
	secondLayout->addWidget(lastReadValue);

	defaultValueLabel = new QLabel("Default : " + Utils::convertToHexa(defaultValue, width));
	secondLayout->addWidget(defaultValueLabel, Qt::AlignRight, Qt::AlignRight);

	layout->addLayout(secondLayout);

	QString defaultValueString = Utils::convertToHexa(defaultValue, width);

	toolTip = "Name : " + name + "\n" + QString::number(regOffset + width - 1) + ":" + QString::number(regOffset) +
		"\n" + "Description : " + description + "\n" + "Notes : " + notes + "\n" +
		"Default Value : " + defaultValueString;

	setToolTip(toolTip);

	mainFrame->setLayout(layout);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->addWidget(mainFrame);
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	setLayout(mainLayout);

	if(description == "Reserved") {
		reserved = true;
	} else {
		reserved = false;
	}

	updateValue(defaultValueString);
}

BitFieldDetailedWidget::~BitFieldDetailedWidget()
{
	delete defaultValueLabel;
	if(value)
		delete value;
	if(valueComboBox)
		delete valueComboBox;
	if(valueSwitch)
		delete valueSwitch;
	if(valueLineEdit)
		delete valueLineEdit;
	delete mainFrame;
}

QString BitFieldDetailedWidget::getToolTip() const { return toolTip; }

void BitFieldDetailedWidget::firstRead()
{
	if(description == "Reserved") {
		valueLineEdit = new QLineEdit(this);
		valueLineEdit->setText("0x0");
		valueLineEdit->setEnabled(false);
		layout->addWidget(valueLineEdit);
	} else if(access == "R") {
		value = new QLabel(this);
	} else if(options && !options->isEmpty()) {
		valueComboBox = new QComboBox(this);

		// check if there are enough options to cover all posible cases for the number of bits
		if(!(options->count() == qPow(2, width))) {
			valueComboBox->setEditable(true);
		}

		layout->addWidget(valueComboBox);

		for(int i = 0; i < options->length(); i++) {
			valueComboBox->insertItem(i, options->at(i)->getDescription());
		}

		QObject::connect(valueComboBox, &QComboBox::currentTextChanged, this, [=](QString val) {
			for(int i = 0; i < options->length(); i++) {
				if(options->at(i)->getDescription() == val) {
					Q_EMIT valueUpdated(options->at(i)->getValue());
					break;
				}
			}
		});

		// if is only one bit we will use a toggle button
	} else if(width == 1) {
		valueSwitch = new SmallOnOffSwitch(this);
		valueSwitch->setChecked(false);
		layout->addWidget(valueSwitch);
		layout->setAlignment(valueSwitch, Qt::AlignRight);

		QObject::connect(valueSwitch, &SmallOnOffSwitch::toggled, this, [=](bool toggled) {
			if(toggled) {
				Q_EMIT valueUpdated("0");
			} else {
				Q_EMIT valueUpdated("1");
			}
		});

	} else {
		valueLineEdit = new QLineEdit(this);
		layout->addWidget(valueLineEdit);
		QObject::connect(valueLineEdit, &QLineEdit::textChanged, this,
				 [=](QString val) { Q_EMIT valueUpdated(val); });
	}
}

void BitFieldDetailedWidget::updateValue(QString newValue)
{
	if(isFirstRead) {
		firstRead();
		isFirstRead = false;
	}

	// if bit is reserved or read only
	if(description == "Reserved") {
		Q_EMIT valueUpdated("0");

	} else if(access == "R") {
		value->setText(newValue);
		Q_EMIT valueUpdated(newValue);
	} else {
		if(valueLineEdit) {
			valueLineEdit->setText(newValue);

		} else if(valueSwitch) {
			if(newValue == "0x0" && valueSwitch->isChecked()) {
				valueSwitch->toggle();
			}
			if(newValue == "0x1" && !valueSwitch->isChecked()) {
				valueSwitch->toggle();
			}

		} else if(valueComboBox) {

			for(int i = 0; i < options->length(); i++) {
				if(options->at(i)->getValue() == newValue) {
					valueComboBox->setCurrentText(options->at(i)->getDescription());
					break;
				}
			}

		} else {
			value->setText(newValue);
		}

		Q_EMIT valueUpdated(newValue);
	}
}

void BitFieldDetailedWidget::registerValueUpdated(QString newValue)
{
	lastReadValue->setText(newValue);
	updateValue(newValue);
}

QString BitFieldDetailedWidget::getValue()
{
	if(valueLineEdit) {
		return valueLineEdit->text();
	}
	if(valueSwitch) {
		if(valueSwitch->isChecked()) {
			return "1";
		} else {
			return "0";
		}
	}
	if(valueComboBox) {
		for(int i = 0; i < options->length(); i++) {
			if(options->at(i)->getDescription() == valueComboBox->currentText()) {
				return options->at(i)->getValue();
			}
		}
		return "0";
	}

	return value->text();
}

int BitFieldDetailedWidget::getWidth() const { return width; }

int BitFieldDetailedWidget::getRegOffset() const { return regOffset; }
