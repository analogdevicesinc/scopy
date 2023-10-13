#include "titlespinbox.hpp"
#include "regmapstylehelper.hpp"
#include "regmapstylehelper.hpp"

#include <QBoxLayout>
#include <utils.h>

TitleSpinBox::TitleSpinBox(QString title, QWidget *parent)
	: QWidget(parent)
{

	QHBoxLayout *mainLayout = new QHBoxLayout(this);
	mainLayout->setSpacing(4);
	mainLayout->setMargin(0);
	setLayout(mainLayout);

	QWidget *spinboxWidget = new QWidget(this);
	QVBoxLayout *spinboxWidgetLayout = new QVBoxLayout(spinboxWidget);
	spinboxWidgetLayout->setSpacing(0);
	spinboxWidgetLayout->setMargin(0);

	QWidget *buttonWidget = new QWidget(this);
	QVBoxLayout *buttonWidgetLayout = new QVBoxLayout(buttonWidget);
	buttonWidgetLayout->setSpacing(0);
	buttonWidgetLayout->setMargin(0);

	spinBoxUpButton = new QPushButton("+", buttonWidget);
	scopy::regmap::RegmapStyleHelper::smallBlueButton(spinBoxUpButton);

	spinBoxDownButton = new QPushButton("-", buttonWidget);
	scopy::regmap::RegmapStyleHelper::smallBlueButton(spinBoxDownButton);

	buttonWidgetLayout->addWidget(spinBoxUpButton);
	buttonWidgetLayout->addWidget(spinBoxDownButton);

	titleLabel = new QLabel(title);
	titleLabel->setStyleSheet(scopy::regmap::RegmapStyleHelper::grayLabel(nullptr));

	spinBox = new QSpinBox(spinboxWidget);
	spinBox->setButtonSymbols(spinBox->ButtonSymbols::NoButtons);
	connect(spinBoxUpButton, &QPushButton::clicked, spinBox, [=]() { spinBox->setValue(spinBox->value() + 1); });
	connect(spinBoxDownButton, &QPushButton::clicked, spinBox, [=]() { spinBox->setValue(spinBox->value() - 1); });

	spinboxWidgetLayout->addWidget(titleLabel);
	spinboxWidgetLayout->addWidget(spinBox);

	mainLayout->addWidget(buttonWidget);
	mainLayout->addWidget(spinboxWidget);

	setStyleSheet(scopy::regmap::RegmapStyleHelper::titleSpinBoxStyle(nullptr));
}

TitleSpinBox::~TitleSpinBox()
{
	delete spinBoxUpButton;
	delete spinBoxDownButton;
	delete titleLabel;
	delete spinBox;
}

void TitleSpinBox::setTitle(QString title) { titleLabel->setText(title); }

QPushButton *TitleSpinBox::getSpinBoxUpButton() { return spinBoxUpButton; }

QPushButton *TitleSpinBox::getSpinBoxDownButton() { return spinBoxDownButton; }

QSpinBox *TitleSpinBox::getSpinBox() { return spinBox; }
