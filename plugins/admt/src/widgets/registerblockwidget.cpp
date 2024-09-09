#include <stylehelper.h>
#include "widgets/registerblockwidget.h"

using namespace scopy;
using namespace scopy::admt;

RegisterBlockWidget::RegisterBlockWidget(QString header, QString description, uint32_t address, uint32_t defaultValue, RegisterBlockWidget::ACCESS_PERMISSION accessPermission, QWidget *parent)
    : QWidget(parent)
    , m_address(address)
    , m_accessPermission(accessPermission)
{
    QVBoxLayout *container = new QVBoxLayout(this);
    setLayout(container);
    container->setMargin(0);
    container->setSpacing(0);
    MenuSectionWidget *menuSectionWidget = new MenuSectionWidget(this);
    MenuCollapseSection *menuCollapseSection = new MenuCollapseSection(header, MenuCollapseSection::MHCW_NONE, menuSectionWidget);
    menuCollapseSection->contentLayout()->setSpacing(10);
    menuSectionWidget->setFixedHeight(180);
    menuSectionWidget->contentLayout()->setSpacing(10);
    menuSectionWidget->contentLayout()->addWidget(menuCollapseSection);

    QLabel *descriptionLabel = new QLabel(description, menuSectionWidget);
    QString labelStyle = QString(R"css(
				QLabel {
					color: white;
					background-color: rgba(255,255,255,0);
					font-weight: 500;
					font-family: Open Sans;
					font-size: 12px;
					font-style: normal;
					}
				QLabel:disabled {
					color: grey;
				}
				)css");
    descriptionLabel->setStyleSheet(labelStyle);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setMinimumHeight(24);
    descriptionLabel->setAlignment(Qt::AlignTop);
    descriptionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    // QLineEdit *lineEdit = new QLineEdit(menuSectionWidget);
    // applyLineEditStyle(lineEdit);

    m_spinBox = new QSpinBox(menuSectionWidget);
    applySpinBoxStyle(m_spinBox);
    m_spinBox->setDisplayIntegerBase(16);
    m_spinBox->setMinimum(0);
    m_spinBox->setMaximum(INT_MAX);
    m_spinBox->setPrefix("0x");
    m_value = defaultValue;
    m_spinBox->setValue(m_value);

    QWidget *buttonsWidget = new QWidget(menuSectionWidget);
    QHBoxLayout *buttonsContainer = new QHBoxLayout(buttonsWidget);
    buttonsWidget->setLayout(buttonsContainer);

    buttonsContainer->setMargin(0);
    buttonsContainer->setSpacing(10);
    switch(m_accessPermission)
    {
        case ACCESS_PERMISSION::READWRITE:
            addReadButton(buttonsWidget);
            addWriteButton(buttonsWidget);
            break;
        case ACCESS_PERMISSION::WRITE:
            addWriteButton(buttonsWidget);
            break;
        case ACCESS_PERMISSION::READ:
            addReadButton(buttonsWidget);
            m_spinBox->setReadOnly(true);
            break;
    }

    menuCollapseSection->contentLayout()->setSpacing(10);
    menuCollapseSection->contentLayout()->addWidget(descriptionLabel);
    menuCollapseSection->contentLayout()->addWidget(m_spinBox);
    menuCollapseSection->contentLayout()->addWidget(buttonsWidget);
    
    container->addWidget(menuSectionWidget);
    container->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));

    connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &RegisterBlockWidget::onValueChanged);
}

void RegisterBlockWidget::onValueChanged(int newValue){ m_value = static_cast<uint32_t>(newValue); }

uint32_t RegisterBlockWidget::getValue() { return m_value; }

void RegisterBlockWidget::setValue(uint32_t value)
{
    m_value = value;
    m_spinBox->setValue(m_value);
}

uint32_t RegisterBlockWidget::getAddress() { return m_address; }

RegisterBlockWidget::ACCESS_PERMISSION RegisterBlockWidget::getAccessPermission() { return m_accessPermission; }

void RegisterBlockWidget::addReadButton(QWidget *parent)
{
    m_readButton = new QPushButton("Read", parent);
    StyleHelper::BlueButton(m_readButton, "readButton");
    parent->layout()->addWidget(m_readButton);
}

QPushButton *RegisterBlockWidget::readButton() { return m_readButton; }

void RegisterBlockWidget::addWriteButton(QWidget *parent)
{
    m_writeButton = new QPushButton("Write", parent);
    StyleHelper::BlueButton(m_writeButton, "writeButton");
    parent->layout()->addWidget(m_writeButton);
}

QPushButton *RegisterBlockWidget::writeButton() { return m_writeButton; }

void RegisterBlockWidget::applyLineEditStyle(QLineEdit *widget)
{
    QString style = QString(R"css(
                                background-color: black;
                                font-family: Open Sans;
								font-size: 16px;
                                color: &&colorname&&;
                                border: none;
                                border-radius: 4px;
                                qproperty-frame: false;
                                )css");
    style = style.replace(QString("&&colorname&&"), StyleHelper::getColor("CH0"));
    widget->setStyleSheet(style);
    widget->setFixedHeight(30);
    widget->setAlignment(Qt::AlignRight);
    widget->setContentsMargins(0, 0, 0, 0);
    widget->setTextMargins(6, 4, 6, 4);
}

void RegisterBlockWidget::applySpinBoxStyle(QSpinBox *widget)
{
    QString style = QString(R"css(
                                background-color: black;
                                font-family: Open Sans;
								font-size: 16px;
                                color: &&colorname&&;
                                border: none;
                                border-radius: 4px;
                                font-weight: normal;
                                )css");
    style = style.replace(QString("&&colorname&&"), StyleHelper::getColor("CH0"));
    widget->setStyleSheet(style);
    widget->setFixedHeight(30);
    widget->setAlignment(Qt::AlignRight);
    widget->setContentsMargins(12, 4, 12, 4);
    widget->setButtonSymbols(widget->ButtonSymbols::NoButtons);
}