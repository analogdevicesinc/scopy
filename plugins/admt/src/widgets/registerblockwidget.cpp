#include <stylehelper.h>
#include "widgets/registerblockwidget.h"

using namespace scopy;
using namespace scopy::admt;

RegisterBlockWidget::RegisterBlockWidget(QString header, QString description, uint32_t address, uint32_t defaultValue, RegisterBlockWidget::ACCESS_PERMISSION accessPermission, QWidget *parent)
    : QWidget(parent)
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

    QSpinBox *spinBox = new QSpinBox(menuSectionWidget);
    applySpinBoxStyle(spinBox);
    spinBox->setDisplayIntegerBase(16);
    spinBox->setMinimum(0);
    spinBox->setMaximum(INT_MAX);
    spinBox->setPrefix("0x");
    spinBox->setValue(defaultValue);

    QWidget *buttonsWidget = new QWidget(menuSectionWidget);
    QHBoxLayout *buttonsContainer = new QHBoxLayout(buttonsWidget);
    buttonsWidget->setLayout(buttonsContainer);

    buttonsContainer->setMargin(0);
    buttonsContainer->setSpacing(10);
    switch(accessPermission)
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
            // lineEdit->setReadOnly(true);
            spinBox->setReadOnly(true);
            break;
    }

    menuCollapseSection->contentLayout()->setSpacing(10);
    menuCollapseSection->contentLayout()->addWidget(descriptionLabel);
    // menuCollapseSection->contentLayout()->addWidget(lineEdit);
    menuCollapseSection->contentLayout()->addWidget(spinBox);
    menuCollapseSection->contentLayout()->addWidget(buttonsWidget);
    
    container->addWidget(menuSectionWidget);
    container->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));
}

void RegisterBlockWidget::addReadButton(QWidget *parent)
{
    QPushButton *readButton = new QPushButton("Read", parent);
    StyleHelper::BlueButton(readButton, "readButton");
    parent->layout()->addWidget(readButton);
}

void RegisterBlockWidget::addWriteButton(QWidget *parent)
{
    QPushButton *writeButton = new QPushButton("Write", parent);
    StyleHelper::BlueButton(writeButton, "writeButton");
    parent->layout()->addWidget(writeButton);
}

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