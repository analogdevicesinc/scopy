#include "titlespinbox.hpp"
#include "regmapstylehelper.hpp"
#include "regmapstylehelper.hpp"

#include <QBoxLayout>
#include <utils.h>

using namespace scopy;
using namespace regmap;

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
	RegmapStyleHelper::smallBlueButton(spinBoxUpButton);

	spinBoxDownButton = new QPushButton("-", buttonWidget);
	RegmapStyleHelper::smallBlueButton(spinBoxDownButton);

	buttonWidgetLayout->addWidget(spinBoxUpButton);
	buttonWidgetLayout->addWidget(spinBoxDownButton);

	titleLabel = new QLabel(title);

	spinBox = new QSpinBox(spinboxWidget);
	spinBox->setButtonSymbols(spinBox->ButtonSymbols::NoButtons);
	connect(spinBoxUpButton, &QPushButton::clicked, spinBox, [=]() { spinBox->setValue(spinBox->value() + 1); });
	connect(spinBoxDownButton, &QPushButton::clicked, spinBox, [=]() { spinBox->setValue(spinBox->value() - 1); });

	spinboxWidgetLayout->addWidget(titleLabel);
	spinboxWidgetLayout->addWidget(spinBox);

	mainLayout->addWidget(buttonWidget);
	mainLayout->addWidget(spinboxWidget);

	RegmapStyleHelper::titleSpinBoxStyle(this);
}

TitleSpinBox::~TitleSpinBox() {}

void TitleSpinBox::setTitle(QString title) { titleLabel->setText(title); }

QPushButton *TitleSpinBox::getSpinBoxUpButton() { return spinBoxUpButton; }

QPushButton *TitleSpinBox::getSpinBoxDownButton() { return spinBoxDownButton; }

QSpinBox *TitleSpinBox::getSpinBox() { return spinBox; }
