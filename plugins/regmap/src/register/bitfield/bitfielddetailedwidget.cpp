#include "bitfielddetailedwidget.hpp"

#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <QtMath>
#include <utils.hpp>

#include "dynamicWidget.h"
#include "utils.hpp"

using namespace scopy;
using namespace regmap;
using namespace regmap::gui;

BitFieldDetailedWidget::BitFieldDetailedWidget(QString name, QString access, int defaultValue, QString description,
                                               int width, QString notes, int regOffset, QVector<BitFieldOption*> *options, QWidget *parent)
    :options(options),
    width(width),
    description(description),
    regOffset(regOffset),
    access(access)
{
    scopy::setDynamicProperty(this, "has_frame", true);
    scopy::setDynamicProperty(this, "has_bottom_border", true);
    setMinimumWidth(10);
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    layout = new QVBoxLayout();
    Utils::removeLayoutMargins(layout);
    layout->setSpacing(0);

    QLabel *nameLabel = new QLabel(name);
    nameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);


    QHBoxLayout *firstLayout = new QHBoxLayout();
    firstLayout->addWidget(nameLabel);
    firstLayout->addWidget(new QLabel(QString::number(regOffset + width - 1) + ":" + QString::number(regOffset)));
    firstLayout->itemAt(1)->setAlignment(Qt::AlignRight);

    layout->addLayout(firstLayout);
    layout->addWidget(new QLabel("Default Value: " + scopy::regmap::Utils::convertToHexa(defaultValue, width)));

    toolTip = "Name : " + name + "\n"
              + QString::number(regOffset + width - 1) + ":" + QString::number(regOffset) + "\n"
              + "Description : " + description + "\n"
              + "Notes : " + notes + "\n"
              + "Default Value : " + scopy::regmap::Utils::convertToHexa(defaultValue, width);

    setToolTip(toolTip);

    value = new QLabel("N/R");
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
        valueLineEdit->setText("0x0");
        valueLineEdit->setReadOnly(true);
        layout->replaceWidget(value,valueLineEdit);
    }else if (access == "R") {

    }  else if (options &&  !options->isEmpty()) {
        valueComboBox = new QComboBox();

        //check if there are enough options to cover all posible cases for the number of bits
        if (!(options->count() == qPow(2,width))) {
            valueComboBox->setEditable(true);
        }

        layout->replaceWidget(value,valueComboBox);

        for (int i = 0; i < options->length(); i++) {
            valueComboBox->insertItem(i, options->at(i)->getDescription());
        }

        QObject::connect(valueComboBox, &QComboBox::currentTextChanged, this, [=](QString val){

            for (int i = 0; i < options->length(); i++) {
                if (options->at(i)->getDescription() == val) {
                    Q_EMIT valueUpdated(options->at(i)->getValue());
                    break;
                }
            }
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
    if (value->text() == "N/R") {
        value->setText("");
        firstRead();
    }

    // if bit is reserved or read only
    if (description == "Reserved") {
        Q_EMIT valueUpdated("0");

    } else if (access == "R") {
        value->setText(newValue);
        Q_EMIT valueUpdated(newValue);
    } else {
        if (valueLineEdit) {
            valueLineEdit->setText(newValue);

        } else if (valueCheckBox){
            valueCheckBox->setChecked(newValue == "1");
        } else if (valueComboBox) {

            for (int i = 0; i < options->length(); i++) {
                if (options->at(i)->getValue() == newValue) {
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
        for (int i = 0; i < options->length(); i++) {
            if (options->at(i)->getDescription() == valueComboBox->currentText()) {
                return options->at(i)->getValue();
            }
        }
        return "0";
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
