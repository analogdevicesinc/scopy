#include "bitfieldsimplewidget.hpp"

#include <QVariant>
#include <qlabel.h>
#include <QStyle>
#include <QBoxLayout>
#include <utils.h>
#include "dynamicWidget.h"
#include <src/utils.hpp>
#include <pluginbase/preferences.h>

using namespace scopy;
using namespace regmap;
using namespace regmap::gui;

BitFieldSimpleWidget::BitFieldSimpleWidget(QString name, int defaultValue, QString description, int width, QString notes, int regOffset, int streach, QWidget *parent):
    width(width),
    description(description),
    streach(streach),
    QFrame{parent}
{
    scopy::setDynamicProperty(this, "has_frame", true);


    setStyleSheet("::hover {background-color: #4a34ff; }");

    setMinimumWidth(60);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    layout = new QHBoxLayout();

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignRight);

    value = new QLabel("N/R");
    value->setAlignment(Qt::AlignRight);
//    value->setMinimumWidth(60);
    value->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
//    value->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    value->setMinimumWidth(25);
    QLabel *bitfieldWidth = new QLabel(QString::number(regOffset + width - 1) + ":" + QString::number(regOffset));
    bitfieldWidth->setAlignment(Qt::AlignRight);
    rightLayout->addWidget(bitfieldWidth);
    rightLayout->addWidget(value);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setAlignment(Qt::AlignTop);
    QLabel *descriptionLabel = new QLabel(name);
    descriptionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    leftLayout->addWidget(descriptionLabel);

    layout->addLayout(leftLayout);
    layout->addLayout(rightLayout);

    QString toolTip = "Name : " + name + "\n"
                      + QString::number(regOffset + width - 1) + ":" + QString::number(regOffset) + "\n"
                      + "Description : " + description + "\n"
                      + "Notes : " + notes + "\n"
                      + "Default Value : " + scopy::regmap::Utils::convertToHexa(defaultValue, width);

    setToolTip(toolTip);


    setLayout(layout);
}

BitFieldSimpleWidget::~BitFieldSimpleWidget()
{
    delete value;
    delete layout;
}

void BitFieldSimpleWidget::updateValue(QString newValue)
{
    value->setText(newValue);
    checkPreferences();
}

int BitFieldSimpleWidget::getWidth() const
{
    return width;
}

QString BitFieldSimpleWidget::getDescription() const
{
    return description;
}

int BitFieldSimpleWidget::getStreach() const
{
    return streach;
}

void BitFieldSimpleWidget::checkPreferences()
{
    scopy::Preferences *p = scopy::Preferences::GetInstance();
    QString background = p->get("regmap_color_by_value").toString();

    if (background.contains("Bitfield background")) {
        bool ok;
        this->setStyleSheet(QString("background-color: " +  Util::getColors().at(value->text().toInt(&ok,16) % 16)));
    }

    if (background.contains("Bitfield text")) {
        bool ok;
        value->setStyleSheet(QString("color: " +  Util::getColors().at(value->text().toInt(&ok,16) % 16)));
    }
}
