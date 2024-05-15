#include "titlespinbox.h"
#include "stylehelper.h"

#include <QBoxLayout>
#include <utils.h>

using namespace scopy;

TitleSpinBox::TitleSpinBox(QString title, QWidget *parent)
	: QWidget(parent)
	, m_titleLabel(new QLabel(title, this))
	, m_spinBox(new QSpinBox(this))
	, m_spinBoxUpButton(new QPushButton(this))
	, m_spinBoxDownButton(new QPushButton(this))
{
	QHBoxLayout *mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);
	m_spinBox->setStyleSheet("border: 0px solid gray; font-weight: normal;");

	StyleHelper::MenuSmallLabel(m_titleLabel);
	m_spinBox->setButtonSymbols(QSpinBox::NoButtons);
	m_spinBox->setMaximumHeight(25);

	QWidget *spinboxWidget = new QWidget(this);
	QVBoxLayout *spinboxWidgetLayout = new QVBoxLayout(spinboxWidget);
	spinboxWidgetLayout->setSpacing(0);
	spinboxWidgetLayout->setMargin(0);

	spinboxWidgetLayout->addWidget(m_titleLabel);
	spinboxWidgetLayout->addWidget(m_spinBox);

	QWidget *buttonWidget = new QWidget(this);
	buttonWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	QVBoxLayout *buttonWidgetLayout = new QVBoxLayout(buttonWidget);
	buttonWidgetLayout->setSpacing(1);
	buttonWidgetLayout->setContentsMargins(0, 0, 0, 1);

	m_spinBoxUpButton->setAutoRepeat(true); // so the user can hold down the button and it will react
	StyleHelper::SpinBoxUpButton(m_spinBoxUpButton, "SpinBoxUpButton");
	m_spinBoxUpButton->setIconSize(QSize(20, 20));
	m_spinBoxUpButton->setFixedSize(20, 20);

	m_spinBoxDownButton->setAutoRepeat(true);
	StyleHelper::SpinBoxDownButton(m_spinBoxDownButton, "SpinBoxDownButton");
	m_spinBoxDownButton->setIconSize(QSize(20, 20));
	m_spinBoxDownButton->setFixedSize(20, 20);

	buttonWidgetLayout->addWidget(m_spinBoxUpButton);
	buttonWidgetLayout->addWidget(m_spinBoxDownButton);

	// here we preffer the pressed signal rather than the clicked one to speed up the change of values
	connect(m_spinBoxUpButton,   &QPushButton::pressed, m_spinBox, [this] { m_spinBox->setValue(m_spinBox->value() + m_spinBox->singleStep()); });
	connect(m_spinBoxDownButton, &QPushButton::pressed, m_spinBox, [this] { m_spinBox->setValue(m_spinBox->value() - m_spinBox->singleStep()); });

	spinboxWidgetLayout->addWidget(m_titleLabel);
	spinboxWidgetLayout->addWidget(m_spinBox);

	mainLayout->addWidget(spinboxWidget);
	mainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
	mainLayout->addWidget(buttonWidget);
}

TitleSpinBox::~TitleSpinBox() {}

void TitleSpinBox::setTitle(QString title) { m_titleLabel->setText(title); }

QPushButton *TitleSpinBox::getSpinBoxUpButton() { return m_spinBoxUpButton; }

QPushButton *TitleSpinBox::getSpinBoxDownButton() { return m_spinBoxDownButton; }

QSpinBox *TitleSpinBox::getSpinBox() { return m_spinBox; }
