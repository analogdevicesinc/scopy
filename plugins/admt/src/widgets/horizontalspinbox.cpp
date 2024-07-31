#include <stylehelper.h>
#include "widgets/horizontalspinbox.h"

using namespace scopy::admt;

HorizontalSpinBox::HorizontalSpinBox(QString header, double initialValue, QString unit, QWidget *parent)
    : QWidget(parent)
    , m_value(initialValue)
    , m_unit(unit)
{
    QVBoxLayout *container = new QVBoxLayout(this);
    setLayout(container);
    container->setMargin(0);
    container->setSpacing(4);

    if(header != ""){
        QLabel *headerLabel = new QLabel(header, this);
        StyleHelper::MenuSmallLabel(headerLabel, "headerLabel");
        container->addWidget(headerLabel);
    }

    QWidget *controlWidget = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlWidget);
    controlWidget->setLayout(controlLayout);
    controlLayout->setMargin(0);
    controlLayout->setSpacing(2);

    lineEdit = new QLineEdit(controlWidget);
    applyLineEditStyle(lineEdit);

    if(QString::compare(m_unit, "") != 0) {
        QWidget *lineEditContainer = new QWidget(controlWidget);
        QHBoxLayout *lineEditLayout = new QHBoxLayout(lineEditContainer);
        lineEditContainer->setLayout(lineEditLayout);
        lineEditLayout->setMargin(0);
        lineEditLayout->setSpacing(0);

        QLabel *unitLabel = new QLabel(m_unit, controlWidget);
        applyUnitLabelStyle(unitLabel);

        lineEdit->setTextMargins(12, 4, 0, 4);

        lineEditLayout->addWidget(lineEdit);
        lineEditLayout->addWidget(unitLabel);
        controlLayout->addWidget(lineEditContainer);
    }
    else{
        controlLayout->addWidget(lineEdit);
    }
    
    QPushButton *minusButton = new QPushButton(controlWidget);
    minusButton->setIcon(QIcon(":/admt/minus.svg"));
    applyPushButtonStyle(minusButton);

    QPushButton *plusButton = new QPushButton(controlWidget);
    plusButton->setIcon(QIcon(":/admt/plus.svg"));
    applyPushButtonStyle(plusButton, 0, 4, 0, 4);

    controlLayout->addWidget(minusButton);
    controlLayout->addWidget(plusButton);

    container->addWidget(controlWidget);

    setValue(m_value);
    connect(lineEdit, SIGNAL(editingFinished()), SLOT(onLineEditTextEdited()));
    connect(minusButton, SIGNAL(clicked()), SLOT(onMinusButtonPressed()));
    connect(plusButton, SIGNAL(clicked()), SLOT(onPlusButtonPressed()));
}

void HorizontalSpinBox::onMinusButtonPressed()
{
    m_value--;
    setValue(m_value);
}

void HorizontalSpinBox::onPlusButtonPressed()
{
    m_value++;
    setValue(m_value);
}

void HorizontalSpinBox::onLineEditTextEdited()
{
    QLineEdit *lineEdit = static_cast<QLineEdit *>(QObject::sender());
    bool ok;
    double value = lineEdit->text().toDouble(&ok);
    if (ok) {
        m_value = value;
    }
    setValue(m_value);
}

void HorizontalSpinBox::setValue(double value)
{
    lineEdit->setText(QString::number(value));
}

void HorizontalSpinBox::applyLineEditStyle(QLineEdit *widget)
{
    QString style = QString(R"css(
                                background-color: black;
                                font-family: Open Sans;
								font-size: 16px;
                                color: &&colorname&&;
                                border: none;
                                border-top-left-radius: 4px;
                                border-bottom-left-radius: 4px;
                                qproperty-frame: false;
                                )css");
    style = style.replace(QString("&&colorname&&"), StyleHelper::getColor("CH0"));
    widget->setStyleSheet(style);
    widget->setFixedHeight(30);
    widget->setAlignment(Qt::AlignRight);
    widget->setContentsMargins(0, 0, 0, 0);
    widget->setTextMargins(6, 4, 6, 4);
}

void HorizontalSpinBox::applyPushButtonStyle(QPushButton *widget, int topLeftBorderRadius, int topRightBorderRadius, int bottomLeftBorderRadius, int bottomRightBorderRadius)
{
    QString style = QString(R"css(
                                background-color: black;
                                font-family: Open Sans;
								font-size: 32px;
								font-weight: bold;
                                text-align: center center;
                                color: &&colorname&&;
                                border-top-left-radius: &&topLeftBorderRadius&&px;
                                border-top-right-radius: &&topRightBorderRadius&&px;
                                border-bottom-left-radius: &&bottomLeftBorderRadius&&px;
                                border-bottom-right-radius: &&bottomRightBorderRadius&&px;
                                )css");
    style = style.replace(QString("&&colorname&&"), StyleHelper::getColor("ScopyBlue"));
    style = style.replace(QString("&&topLeftBorderRadius&&"), QString::number(topLeftBorderRadius));
    style = style.replace(QString("&&topRightBorderRadius&&"), QString::number(topRightBorderRadius));
    style = style.replace(QString("&&bottomLeftBorderRadius&&"), QString::number(bottomLeftBorderRadius));
    style = style.replace(QString("&&bottomRightBorderRadius&&"), QString::number(bottomRightBorderRadius));
    widget->setStyleSheet(style);
    widget->setFixedHeight(30);
    widget->setFixedWidth(38);
}

void HorizontalSpinBox::applyUnitLabelStyle(QLabel *widget)
{
    QString style = QString(R"css(
                                background-color: black;
                                font-family: Open Sans;
								font-size: 16px;
                                text-align: right;
                                color: &&colorname&&;
                                border: none;
                                )css");
    style = style.replace(QString("&&colorname&&"), StyleHelper::getColor("CH0"));
    widget->setStyleSheet(style);
    widget->setFixedHeight(30);
    widget->setAlignment(Qt::AlignRight);
    widget->setContentsMargins(0, 4, 12, 4);
}