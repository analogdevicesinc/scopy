#include "bitfieldsimplewidget.hpp"

#include <QVariant>
#include <qlabel.h>
#include <QStyle>
#include <QBoxLayout>
#include "dynamicWidget.h"
#include <src/utils.hpp>

using namespace scopy;
BitFieldSimpleWidget::BitFieldSimpleWidget(QString name, int defaultValue, QString description, int width, QString notes, int regOffset, int streach, QWidget *parent):
    width(width),
    description(description),
    streach(streach),
    QFrame{parent}
{
    scopy::setDynamicProperty(this, "has_frame", true);

    setMinimumWidth(10);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);

    layout->addWidget(new QLabel(name));
    layout->addWidget(new QLabel(QString::number(regOffset + width - 1) + ":" + QString::number(regOffset)));
    layout->addWidget(new QLabel(description));
    layout->addWidget(new QLabel("Default Value: " + Utils::convertToHexa(defaultValue, width)));

    QString toolTip = "Name : " + name + "\n"
                      + QString::number(regOffset + width - 1) + ":" + QString::number(regOffset) + "\n"
                      + "Description : " + description + "\n"
                      + "Notes : " + notes + "\n"
                      + "Default Value : " + Utils::convertToHexa(defaultValue, width);

    setToolTip(toolTip);

    value = new QLabel("Not Read");
    layout->addWidget(value);

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
