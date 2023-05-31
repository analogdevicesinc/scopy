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

    setStyleSheet("::hover {background-color: #4a34ff; }");

    setMinimumWidth(10);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignRight);

    value = new QLabel("Not Read");
    QLabel *bitfieldWidth = new QLabel(QString::number(regOffset + width - 1) + ":" + QString::number(regOffset));
    bitfieldWidth->setAlignment(Qt::AlignRight);
    rightLayout->addWidget(bitfieldWidth);
    rightLayout->addWidget(value);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setAlignment(Qt::AlignTop);
    QLabel *descriptionLabel = new QLabel(name);
    descriptionLabel->setWordWrap(true);

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
