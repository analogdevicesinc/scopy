/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "spinbox_a.hpp"
#include "completion_circle.h"

#include "ui_spinbox_a.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QComboBox>
#include <QFile>
#include <qmath.h>
#include <QRegExpValidator>
#include <QKeyEvent>

#include <QDebug>

using namespace std;
using namespace adiscope;

/*
 * SpinBoxA class implementation
 */

SpinBoxA::SpinBoxA(vector<pair<QString, double> >units, const QString &name,
		double min_value, double max_value,
		bool hasProgressWidget, bool invertCircle, QWidget *parent):
	QWidget(parent), ui(new Ui::SpinBoxA),
	m_SBA_CompCircle(nullptr), m_min_value(min_value),
	m_max_value(max_value), m_units(units), m_fine_mode(false)
{
	QString regex;
	QString sufixes;

	ui->setupUi(this);
	if (hasProgressWidget) {
		m_SBA_CompCircle = new CompletionCircle(this, invertCircle);
		m_SBA_CompCircle->setObjectName("SBA_CompletionCircle");
		ui->verticalLayout_circle->addWidget(m_SBA_CompCircle);
		m_SBA_CompCircle->setMinimumDouble(this->minValue());
		m_SBA_CompCircle->setMaximumDouble(this->maxValue());
		m_SBA_CompCircle->setValueDouble(this->value());
	} else {
		ui->verticalLayout_circle->addSpacerItem(new QSpacerItem(0, 0,
			QSizePolicy::Expanding, QSizePolicy::Fixed));
	}

	ui->SBA_Label->setText(name);

	// Configure line edit
	regex = "^(?!^.{18})(([+,-]?)([0-9]*)([.]?)([0-9]+))";
	for (auto it = m_units.begin(); it != m_units.end(); ++it) {
		QString s = (*it).first;
		if (!s.isEmpty()) {
			QString newS = s.replace(QRegExp("[μ]"), "u");
			sufixes += (newS.at(0) + '|');
		}
	}
	if (!sufixes.isEmpty()) {
		sufixes.chop(1);
		regex += "([" + sufixes + "]?)";
	}
	m_validator = new QRegExpValidator(this);
	QRegExp rx(regex);
	m_validator->setRegExp(rx);
	ui->SBA_LineEdit->setValidator(m_validator);

	// Configure combo box
	for (auto it = m_units.begin(); it != m_units.end(); it++)
		ui->SBA_Combobox->addItem((*it).first);

	QFile file(":stylesheets/stylesheets/spinbox_type_a.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QString::fromLatin1(file.readAll());
	this->setStyleSheet(styleSheet);

	setValue(0);

	ui->SBA_LineEdit->installEventFilter(this);

	connect(ui->SBA_Combobox, SIGNAL(currentIndexChanged(int)),
		SLOT(onComboboxIndexChanged(int)));
	connect(ui->SBA_LineEdit, SIGNAL(editingFinished()),
		SLOT(onLineEditTextEdited()));
	connect(ui->SBA_UpButton, SIGNAL(pressed()),
		SLOT(onUpButtonPressed()));
	connect(ui->SBA_DownButton, SIGNAL(pressed()),
		SLOT(onDownButtonPressed()));
	if (m_SBA_CompCircle) {
		connect(this, SIGNAL(valueChanged(double)),
			m_SBA_CompCircle, SLOT(setValueDouble(double)));
		connect(m_SBA_CompCircle, SIGNAL(toggled(bool)),
			SLOT(setFineMode(bool)));
	}
}

SpinBoxA::~SpinBoxA()
{
	delete ui;
}

void SpinBoxA::onUpButtonPressed()
{
	stepUp();
}

void SpinBoxA::onDownButtonPressed()
{
	stepDown();
}

void SpinBoxA::onComboboxIndexChanged(int index)
{
	double value = ui->SBA_LineEdit->text().toDouble();

	setValue(value * m_units[index].second);
}

void SpinBoxA::onLineEditTextEdited()
{
	QLineEdit *lineEdit = static_cast<QLineEdit*>(QObject::sender());
	QString text = lineEdit->text();
	QRegExp rx(m_validator->regExp());
	double value;
	QString unit;
	bool ok;
	int i;

	rx.indexIn(text);
	value = rx.cap(1).toDouble(&ok);
	if (!ok)
		return;

	unit = rx.cap(6);
	if (unit.isEmpty())
		unit = m_units[ui->SBA_Combobox->currentIndex()].first;
	else if (unit.at(0) == 'u')
		unit = unit.replace(0, 1, "μ");
	i = find_if(m_units.begin(), m_units.end(),
		[=](const pair<QString, double> pair)
		{return pair.first.at(0) == unit.at(0);} ) - m_units.begin();
	if (i < m_units.size()) {
		value *= m_units[i].second;
		setValue(value);
	}
}

QPushButton * SpinBoxA::upButton()
{
	return ui->SBA_UpButton;
}

QPushButton * SpinBoxA::downButton()
{
	return ui->SBA_DownButton;
}

QLabel * SpinBoxA::nameLabel()
{
	return ui->SBA_Label;
}

QLineEdit * SpinBoxA::lineEdit()
{
	return ui->SBA_LineEdit;
}

QFrame * SpinBoxA::line()
{
	return ui->SBA_Line;
}

QComboBox *SpinBoxA::comboBox()
{
	return ui->SBA_Combobox;
}

double SpinBoxA::value()
{
	return m_value;
}

static bool isZero(double value, double threshold)
{
	return (value >= -threshold && value <= threshold);
}

void SpinBoxA::setValue(double value)
{
	bool emitValueChanged = false;

	if (isZero(value, 1E-12))
		value = 0;

	if (value < m_min_value)
		value = m_min_value;
	else if (value > m_max_value)
		value = m_max_value;

	if (m_value != value) {
		m_value = value;
		emitValueChanged = true;
	}

	// Update line edit
	int index;
	double scale = findUnitOfValue(m_value, &index);
	ui->SBA_LineEdit->setText(QString::number(m_value / scale));

	if (m_value != 0) {
		ui->SBA_Combobox->blockSignals(true);
		ui->SBA_Combobox->setCurrentIndex(index);
		ui->SBA_Combobox->blockSignals(false);
	}

	if (emitValueChanged)
		emit valueChanged(m_value);
}

double SpinBoxA::minValue()
{
	return m_min_value;
}

void SpinBoxA::setMinValue(double value)
{
	m_min_value = value;
	if (m_value < m_min_value)
		setValue(m_min_value);
	if (m_SBA_CompCircle)
		m_SBA_CompCircle->setMinimumDouble(value);
}

double SpinBoxA::maxValue()
{
	return m_max_value;
}

void SpinBoxA::setMaxValue(double value)
{
	m_max_value = value;
	if (m_value > m_max_value)
		setValue(m_max_value);
	if (m_SBA_CompCircle)
		m_SBA_CompCircle->setMaximumDouble(value);
}

bool SpinBoxA::isInFineMode()
{
	return m_fine_mode;
}

void SpinBoxA::setFineMode(bool en)
{
	m_fine_mode = en;
}

bool SpinBoxA::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == ui->SBA_LineEdit) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyE = static_cast<QKeyEvent*>(event);
			if (keyE->key() == Qt::Key_Up) {
				stepUp();
				return true;
			} else if (keyE->key() == Qt::Key_Down) {
				stepDown();
				return true;
			} else if (keyE->key() == Qt::Key_PageUp) {
				stepUp();
				return true;

			} else if (keyE->key() == Qt::Key_PageDown) {
				stepDown();
				return true;
			}
		} else if (event->type() == QEvent::Wheel) {
			QWheelEvent* wheelE = static_cast<QWheelEvent*>(event);
			if (wheelE->angleDelta().y() > 0) {
				stepUp();
				return true;
			} else if (wheelE->angleDelta().y() < 0) {
				stepDown();
				return true;
			}
		}
	}

	return QWidget::eventFilter(obj, event);
}

double SpinBoxA::findUnitOfValue(double val, int *posInUnitsList)
{
	int index = 0;

	val = qFabs(val);
	for (auto it = m_units.rbegin(); it != m_units.rend(); ++it) {
		if (val >= (*it).second) {
			index = m_units.rend() - it - 1;
			break;
		}
	}

	if (posInUnitsList)
		*posInUnitsList = index;

	return m_units[index].second;
}

/*
 * ScaleSpinButton class implementation
 */
ScaleSpinButton::ScaleSpinButton(vector<pair<QString, double> >units,
		const QString &name,
		double min_value, double max_value,
		bool hasProgressWidget, bool invertCircle, QWidget *parent):
	SpinBoxA(units, name, min_value, max_value,
			hasProgressWidget, invertCircle, parent),
	m_steps(1E-3, 1E+3, 10, {1, 2, 5}),
	m_fine_increment(1)
{
	if (m_SBA_CompCircle)
		m_SBA_CompCircle->setIsLogScale(true);

	setMinValue(min_value);
	setMaxValue(max_value);
}

void ScaleSpinButton::setMinValue(double value)
{
	m_steps.setLower(value);
	SpinBoxA::setMinValue(value);
}

void ScaleSpinButton::setMaxValue(double value)
{
	m_steps.setUpper(value);
	SpinBoxA::setMaxValue(value);
}

void ScaleSpinButton::stepUp()
{
	double current_val = ui->SBA_LineEdit->text().toDouble();
	double current_scale = m_units[ui->SBA_Combobox->currentIndex()].second;
	double newVal;

	double epsilon = 1E-15;

	if (m_fine_mode)
		newVal =  (current_val + m_fine_increment) * current_scale;
	else
		newVal =  m_steps.getNumberAfter(current_val * current_scale + epsilon);

	setValue(newVal);
}

void ScaleSpinButton::stepDown()
{
	double current_val = ui->SBA_LineEdit->text().toDouble();
	double current_scale = m_units[ui->SBA_Combobox->currentIndex()].second;
	double newVal;

	double epsilon = 1E-15;

	if (m_fine_mode) {
		newVal = (current_val - m_fine_increment) * current_scale;
		if ((m_min_value > 0 && newVal <= m_min_value) &&
					(ui->SBA_Combobox->currentIndex() > 0)) {
			int i = ui->SBA_Combobox->currentIndex() - 1;
			double nextLowerScale = m_units[i].second;

			newVal = current_scale / nextLowerScale -
					m_fine_increment;
			newVal *= nextLowerScale;
		}

	} else {
		newVal = m_steps.getNumberBefore(current_val * current_scale - epsilon);
	}

	setValue(newVal);
}

/*
 * PositionSpinButton class implementation
 */
PositionSpinButton::PositionSpinButton(vector<pair<QString, double> >units,
	const QString &name,
	double min_value, double max_value,
	bool hasProgressWidget, bool invertCircle, QWidget *parent):
	SpinBoxA(units, name, min_value, max_value,
			hasProgressWidget, invertCircle, parent),
	m_step(1)
{
}

double PositionSpinButton:: step()
{
	return m_step;
}

void PositionSpinButton::setStep(double step)
{
	m_step = step;
}

void PositionSpinButton::stepUp()
{
	double current_val = ui->SBA_LineEdit->text().toDouble();
	double current_scale = m_units[ui->SBA_Combobox->currentIndex()].second;
	double newVal;
	double step = m_step;

	if (m_fine_mode)
		step /= 10;

	newVal =  current_val * current_scale + step;

	setValue(newVal);
}

void PositionSpinButton::stepDown()
{
	double current_val = ui->SBA_LineEdit->text().toDouble();
	double current_scale = m_units[ui->SBA_Combobox->currentIndex()].second;
	double newVal;
	double step = m_step;

	if (m_fine_mode)
		step /= 10;

	newVal = current_val * current_scale - step;

	setValue(newVal);
}
