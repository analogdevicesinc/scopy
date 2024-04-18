#include "titlespinbox.h"
#include "stylehelper.h"

#include <string>

#include <QBoxLayout>
#include <QLoggingCategory>
#include <utils.h>

using namespace scopy;
Q_LOGGING_CATEGORY(CAT_TITLESPINBOX, "TitleSpinBox")

TitleSpinBox::TitleSpinBox(QString title, QWidget *parent)
	: QWidget(parent)
	, m_titleLabel(new QLabel(title, this))
	, m_lineedit(new QLineEdit(this))
	, m_spinBoxUpButton(new QPushButton(this))
	, m_spinBoxDownButton(new QPushButton(this))
	, m_min(0)
	, m_step(1)
	, m_max(99)
{
	QHBoxLayout *mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);
	m_lineedit->setStyleSheet("border: 0px solid gray; font-weight: normal;");

	StyleHelper::MenuSmallLabel(m_titleLabel);
	m_lineedit->setMaximumHeight(25);

	QWidget *spinboxWidget = new QWidget(this);
	QVBoxLayout *spinboxWidgetLayout = new QVBoxLayout(spinboxWidget);
	spinboxWidgetLayout->setSpacing(0);
	spinboxWidgetLayout->setMargin(0);

	spinboxWidgetLayout->addWidget(m_titleLabel);
	spinboxWidgetLayout->addWidget(m_lineedit);

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
	connect(m_spinBoxUpButton, &QPushButton::pressed, m_lineedit, [this] {
		bool ok = true;
		QString text = m_lineedit->text();
		double value = text.toDouble(&ok);
		if(!ok) {
			// If the cast fails that means that there is an issue with the text and the
			// min/max/step values are useless here. The signal will just be skipped and
			// a debug message will de displayed.
			qDebug(CAT_TITLESPINBOX) << "Cannot increase the value:" << text;
			return;
		}

		double newValue = value + m_step;
		if(newValue > m_max) {
			newValue = value;
		}

		m_lineedit->setText(truncValue(newValue));
	});

	connect(m_spinBoxDownButton, &QPushButton::pressed, m_lineedit, [this] {
		bool ok = true;
		QString text = m_lineedit->text();
		double value = text.toDouble(&ok);
		if(!ok) {
			// If the cast fails that means that there is an issue with the text and the
			// min/max/step values are useless here. The signal will just be skipped and
			// a debug message will de displayed.
			qDebug(CAT_TITLESPINBOX) << "Cannot decrease the value:" << text;
			return;
		}

		double newValue = value - m_step;
		if(newValue < m_min) {
			newValue = value;
		}

		m_lineedit->setText(truncValue(newValue));
	});

	spinboxWidgetLayout->addWidget(m_titleLabel);
	spinboxWidgetLayout->addWidget(m_lineedit);

	mainLayout->addWidget(spinboxWidget);
	mainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
	mainLayout->addWidget(buttonWidget);
}

TitleSpinBox::~TitleSpinBox() {}

void TitleSpinBox::setTitle(QString title) { m_titleLabel->setText(title); }

QPushButton *TitleSpinBox::getSpinBoxUpButton() { return m_spinBoxUpButton; }

QPushButton *TitleSpinBox::getSpinBoxDownButton() { return m_spinBoxDownButton; }

QLineEdit *TitleSpinBox::getLineEdit() { return m_lineedit; }

double TitleSpinBox::step() const { return m_step; }

void TitleSpinBox::setStep(double newStep) { m_step = newStep; }

double TitleSpinBox::max() const { return m_max; }

void TitleSpinBox::setMax(double newMax) { m_max = newMax; }

double TitleSpinBox::min() const { return m_min; }

void TitleSpinBox::setMin(double newMin) { m_min = newMin; }

void TitleSpinBox::setValue(double newValue) { m_lineedit->setText(truncValue(newValue)); }

void TitleSpinBox::setSpinButtonsDisabled(bool isDisabled)
{
	m_spinBoxUpButton->setVisible(!isDisabled);
	m_spinBoxDownButton->setVisible(!isDisabled);
}

QString TitleSpinBox::truncValue(double value)
{
	QString sReturn = QString::number(value, 'f', 7); // magic number
	int i = sReturn.size() - 1;
	int toChop = 0;
	while(sReturn[i] == '0') {
		++toChop;
		--i;
	}

	if(sReturn[i] == '.') {
		++toChop;
	}

	sReturn.chop(toChop);
	return sReturn;
}

#include "moc_titlespinbox.cpp"
