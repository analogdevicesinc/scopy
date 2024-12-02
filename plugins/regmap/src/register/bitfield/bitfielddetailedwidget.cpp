#include "bitfielddetailedwidget.hpp"

#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <QtMath>

BitFieldDetailedWidget::BitFieldDetailedWidget(QString name, int defaultValue, QString description,
					       int width, QString notes, int regOffset, QMap<QString, QString> *options, QWidget *parent)
	:options(options),
	  width(width),
	  description(description),
	  regOffset(regOffset)
{
//	setStyleSheet("border: 1px solid black");
	setMinimumWidth(10);
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

	layout = new QVBoxLayout();

	layout->addWidget(new QLabel(name));
	layout->addWidget(new QLabel(QString::number(regOffset + width - 1) + ":" + QString::number(regOffset)));
	layout->addWidget(new QLabel(description));
	layout->addWidget(new QLabel("Default Value: " + QString::number(defaultValue)));

	toolTip = "Name : " + name + "\n"
			+ QString::number(regOffset + width - 1) + ":" + QString::number(regOffset) + "\n"
			+ "Description : " + description + "\n"
			+ "Notes : " + notes + "\n"
			+ "Default Value : " + QString::number(defaultValue);

	setToolTip(toolTip);

	value = new QLabel("Not Read");
	layout->addWidget(value);

	setLayout(layout);

	if (description == "Reserved") {
		reserved = true;
	} else {
		reserved = false;
	}

}

BitFieldDetailedWidget::~BitFieldDetailedWidget()
{
	delete value;
	if (valueComboBox) delete valueComboBox;
	if (valueCheckBox) delete valueCheckBox;
	if (valueLineEdit) delete valueLineEdit;
}

QString BitFieldDetailedWidget::getToolTip() const
{
	return toolTip;
}

void BitFieldDetailedWidget::firstRead()
{
	if (description == "Reserved") {
		valueLineEdit = new QLineEdit();
		valueLineEdit->setText("0");
		valueLineEdit->setReadOnly(true);
		layout->replaceWidget(value,valueLineEdit);
	} else if (options &&  !options->isEmpty() && (options->count() == qPow(2,width))) {
		//check if there are enough options to cover all posible cases for the number of bits
		valueComboBox = new QComboBox();
		layout->replaceWidget(value,valueComboBox);
		QMap<QString, QString>::iterator i = options->begin();
		int index = 0;
		while (i != options->end()) {
			valueComboBox->insertItem(index,i.key());
			++i;
			++index;
		}

		QObject::connect(valueComboBox, &QComboBox::currentTextChanged, this, [=](QString val){
			Q_EMIT valueUpdated(options->value(val));
		});

		//if is only one bit we will use a toggle button
	} else if (width == 1) {
		valueCheckBox = new QCheckBox();
		layout->replaceWidget(value,valueCheckBox);

		QObject::connect(valueCheckBox, &QCheckBox::toggled, this, [=](){
			if (valueCheckBox->isChecked() ) {
				Q_EMIT valueUpdated("1");
			} else {
				Q_EMIT valueUpdated("0");
			}
		});

	} else {
		valueLineEdit = new QLineEdit();
		layout->replaceWidget(value,valueLineEdit);

		QObject::connect(valueLineEdit, &QLineEdit::textChanged, this, [=](QString val){
			Q_EMIT valueUpdated(val);
		});
	}
}

void BitFieldDetailedWidget::updateValue(QString newValue)
{
	if (value->text() == "Not Read") {
		value->setText("");
		firstRead();
	}

	if (description == "Reserved") {
		Q_EMIT valueUpdated("0");

	} else {
		if (valueLineEdit) {
			valueLineEdit->setText(newValue);

		} else if (valueCheckBox){
			valueCheckBox->setChecked(newValue == "1");
		} else if (valueComboBox) {
			valueComboBox->setCurrentText(options->key(newValue));
		} else {
			value->setText(newValue);
		}

		Q_EMIT valueUpdated(newValue);
	}

}

QString BitFieldDetailedWidget::getValue()
{
	if (valueLineEdit) {
		return  valueLineEdit->text();
	}
	if (valueCheckBox){
		if (valueCheckBox->isChecked() ) {
			return "1";
		} else {
			return "0";
		}
	}
	if (valueComboBox) {
		return options->value(valueComboBox->currentText());
	}

	return value->text();
}

int BitFieldDetailedWidget::getWidth() const
{
	return width;
}

int BitFieldDetailedWidget::getRegOffset() const
{
	return regOffset;
}
