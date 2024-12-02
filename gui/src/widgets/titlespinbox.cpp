/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "titlespinbox.h"
#include <style.h>
#include <QBoxLayout>
#include <QLoggingCategory>
#include <utils.h>

using namespace scopy;
Q_LOGGING_CATEGORY(CAT_TITLESPINBOX, "TitleSpinBox")

TitleSpinBox::TitleSpinBox(QString title, bool isCompact, QWidget *parent)
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
	Style::setStyle(m_lineedit, style::properties::widget::solidBorder);

	Style::setStyle(m_titleLabel, style::properties::label::subtle);
	m_lineedit->setMaximumHeight(25);

	QWidget *spinboxWidget = new QWidget(this);
	QWidget *buttonWidget = new QWidget(this);
	buttonWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	int size = Style::getDimension(json::global::unit_2_5);
	m_spinBoxUpButton->setAutoRepeat(true); // so the user can hold down the button and it will react
	m_spinBoxUpButton->setIcon(
		Style::getPixmap(":/gui/icons/plus.svg", Style::getColor(json::theme::content_inverse)));
	Style::setStyle(m_spinBoxUpButton, style::properties::button::spinboxButton);
	m_spinBoxUpButton->setFixedSize(size, size);

	m_spinBoxDownButton->setAutoRepeat(true);
	m_spinBoxDownButton->setIcon(
		Style::getPixmap(":/gui/icons/minus.svg", Style::getColor(json::theme::content_inverse)));
	Style::setStyle(m_spinBoxDownButton, style::properties::button::spinboxButton);
	m_spinBoxDownButton->setFixedSize(size, size);

	QLayout *buttonWidgetLayout;
	QLayout *spinboxWidgetLayout;

	if(isCompact) {
		m_titleLabel->setText(m_titleLabel->text().toUpper());
		m_titleLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		m_lineedit->setAlignment(Qt::AlignRight);

		buttonWidgetLayout = new QHBoxLayout(buttonWidget);
		spinboxWidgetLayout = new QHBoxLayout(spinboxWidget);

		buttonWidgetLayout->addWidget(m_spinBoxDownButton);
		buttonWidgetLayout->addWidget(m_spinBoxUpButton);
		spinboxWidgetLayout->addWidget(m_titleLabel);
		spinboxWidgetLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
		spinboxWidgetLayout->addWidget(m_lineedit);

		mainLayout->addWidget(spinboxWidget);
		mainLayout->addWidget(buttonWidget);
	} else {
		buttonWidgetLayout = new QVBoxLayout(buttonWidget);
		spinboxWidgetLayout = new QVBoxLayout(spinboxWidget);

		buttonWidgetLayout->addWidget(m_spinBoxUpButton);
		buttonWidgetLayout->addWidget(m_spinBoxDownButton);
		spinboxWidgetLayout->addWidget(m_titleLabel);
		spinboxWidgetLayout->addWidget(m_lineedit);

		mainLayout->addWidget(spinboxWidget);
		mainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
		mainLayout->addWidget(buttonWidget);
	}

	buttonWidgetLayout->setSpacing(1);
	buttonWidgetLayout->setContentsMargins(0, 0, 0, 1);

	spinboxWidgetLayout->setSpacing(0);
	spinboxWidgetLayout->setMargin(0);

	connectSignalsAndSlots();
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

void TitleSpinBox::connectSignalsAndSlots()
{
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
}

#include "moc_titlespinbox.cpp"
