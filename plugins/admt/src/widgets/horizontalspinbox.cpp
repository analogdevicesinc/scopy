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

    m_lineEdit = new QLineEdit(controlWidget);
    applyLineEditStyle(m_lineEdit);

    if(QString::compare(m_unit, "") != 0) {
        QWidget *lineEditContainer = new QWidget(controlWidget);
        QHBoxLayout *lineEditLayout = new QHBoxLayout(lineEditContainer);
        lineEditContainer->setLayout(lineEditLayout);
        lineEditLayout->setMargin(0);
        lineEditLayout->setSpacing(0);

        m_unitLabel = new QLabel(m_unit, controlWidget);
        applyUnitLabelStyle(m_unitLabel);

        m_lineEdit->setTextMargins(12, 4, 0, 4);

        lineEditLayout->addWidget(m_lineEdit);
        lineEditLayout->addWidget(m_unitLabel);
        controlLayout->addWidget(lineEditContainer);
    }
    else{
        controlLayout->addWidget(m_lineEdit);
    }
    
    m_minusButton = new QPushButton(controlWidget);
    m_minusButton->setIcon(QIcon(":/admt/minus.svg"));
    applyPushButtonStyle(m_minusButton);

    m_plusButton = new QPushButton(controlWidget);
    m_plusButton->setIcon(QIcon(":/admt/plus.svg"));
    applyPushButtonStyle(m_plusButton, 0, 4, 0, 4);

    controlLayout->addWidget(m_minusButton);
    controlLayout->addWidget(m_plusButton);

    container->addWidget(controlWidget);

    setValue(m_value);
    connect(m_lineEdit, SIGNAL(editingFinished()), SLOT(onLineEditTextEdited()));
    connect(m_minusButton, SIGNAL(clicked()), SLOT(onMinusButtonPressed()));
    connect(m_plusButton, SIGNAL(clicked()), SLOT(onPlusButtonPressed()));
}

void HorizontalSpinBox::onMinusButtonPressed()
{
    m_value--;
    setValue(m_value);
    Q_EMIT m_lineEdit->editingFinished();
}

void HorizontalSpinBox::onPlusButtonPressed()
{
    m_value++;
    setValue(m_value);
    Q_EMIT m_lineEdit->editingFinished();
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
    m_lineEdit->setText(QString::number(value));
}

void HorizontalSpinBox::setEnabled(double value)
{
    m_lineEdit->setEnabled(value);
    m_minusButton->setEnabled(value);
    m_plusButton->setEnabled(value);
    if(QString::compare(m_unit, "") != 0){
        applyUnitLabelStyle(m_unitLabel, value);
    }
}

void HorizontalSpinBox::applyLineEditStyle(QLineEdit *widget)
{
    QString style = QString(R"css(
                                QLineEdit {
									font-family: Open Sans;
									font-size: 16px;
									font-weight: normal;
									text-align: right;
									color: &&colorname&&;
                                    border-top-left-radius: 4px;
                                    border-bottom-left-radius: 4px;

									background-color: black;
									border: none;
                                    qproperty-frame: false;
								}

								QLineEdit:disabled {
                                    background-color: #18181d;
									color: #9c4600;
                                }
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
                                QPushButton{
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
                                }
                                QPushButton:disabled{
                                    background-color: #18181d;
									color: #2d3d9c;
                                }
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

void HorizontalSpinBox::applyUnitLabelStyle(QLabel *widget, bool isEnabled)
{
    QString style = QString(R"css(
                                background-color: &&backgroundcolor&&;
                                font-family: Open Sans;
								font-size: 16px;
                                text-align: right;
                                color: &&colorname&&;
                                border: none;
                                )css");
    if(isEnabled){
        style = style.replace(QString("&&backgroundcolor&&"), "black");
        style = style.replace(QString("&&colorname&&"), StyleHelper::getColor("CH0"));
    }
    else{
        style = style.replace(QString("&&backgroundcolor&&"), "#18181d");
        style = style.replace(QString("&&colorname&&"), "#9c4600");
    }
    widget->setStyleSheet(style);
    widget->setFixedHeight(30);
    widget->setAlignment(Qt::AlignRight);
    widget->setContentsMargins(0, 4, 12, 4);
}

QLineEdit *HorizontalSpinBox::lineEdit() { return m_lineEdit; }