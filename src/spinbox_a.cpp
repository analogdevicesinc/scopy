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

SpinBoxA::SpinBoxA(QWidget *parent) : QWidget(parent),
	ui(new Ui::SpinBoxA), m_value(0.0), m_min_value(0.0), m_max_value(0.0),
	m_decimal_count(3),
	m_validator(new QRegExpValidator(this))
{
	ui->setupUi(this);
	ui->SBA_LineEdit->setValidator(m_validator);

	QFile file(":stylesheets/stylesheets/spinbox_type_a.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QString::fromLatin1(file.readAll());

	const QString& currentStylesheet = this->styleSheet();
	this->setStyleSheet(currentStylesheet + styleSheet);

	ui->SBA_LineEdit->installEventFilter(this);

	connect(ui->SBA_Combobox, SIGNAL(currentIndexChanged(int)),
	        SLOT(onComboboxIndexChanged(int)));
	connect(ui->SBA_LineEdit, SIGNAL(editingFinished()),
	        SLOT(onLineEditTextEdited()));
	connect(ui->SBA_UpButton, SIGNAL(pressed()),
	        SLOT(onUpButtonPressed()));
	connect(ui->SBA_DownButton, SIGNAL(pressed()),
	        SLOT(onDownButtonPressed()));

	connect(this, SIGNAL(valueChanged(double)),
	        ui->SBA_CompletionCircle, SLOT(setValueDouble(double)));
	connect(ui->SBA_CompletionCircle, SIGNAL(toggled(bool)),
	        SLOT(setFineMode(bool)));

	m_displayScale = 1;
}

SpinBoxA::SpinBoxA(vector<pair<QString, double> >units, const QString& name,
                   double min_value, double max_value, bool hasProgressWidget,
                   bool invertCircle, QWidget *parent) : SpinBoxA(parent)
{
	showProgress(hasProgressWidget);

	/* Compat */
	QStringList list;

	for (auto unit : units) {
		list.append(QString("%1=%2").arg(unit.first).arg(unit.second));
	}

	setUnits(list);

	setMinValue(min_value);
	setMaxValue(max_value);

	ui->SBA_Label->setText(name);

	m_displayScale = 1;

	if (m_value < min_value)
		setValue(min_value);
	else if (m_value > max_value)
		setValue(max_value);


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
	QLineEdit *lineEdit = static_cast<QLineEdit *>(QObject::sender());
	QString text = lineEdit->text();
	QRegExp rx(m_validator->regExp());
	double value;
	QString unit;
	bool ok;

	rx.indexIn(text);
	value = rx.cap(1).toDouble(&ok);

	if (!ok) {
		return;
	}

	unit = rx.cap(6);

	if (unit.isEmpty()) {
		unit = m_units[ui->SBA_Combobox->currentIndex()].first;
	} else if (unit.startsWith(QString("u"), Qt::CaseInsensitive)) {
		qDebug()<<"before unit "<<unit ;
		unit = unit.replace(0, 1, "μ");
		qDebug()<<"after unit "<<unit ;
	} else if (unit.startsWith(QString("r"), Qt::CaseInsensitive)) {
		qDebug()<<"before unit "<<unit ;
		unit = unit.replace(0, 1, "π");
		qDebug()<<"after unit "<<unit ;
	}
	//check if the current user input fits to any unit measure
	if (isUnitMatched(unit, value)) {
		return;
	}

	bool isLowerCase = unit == unit.toLower();

	if (isLowerCase) {
		unit = unit.toUpper();
	} else {
		unit = unit.toLower();
	}
	//change the user input and try again to find a match
	isUnitMatched(unit, value);
}

bool SpinBoxA::isUnitMatched(const QString &unit, double value)
{
	int i = find_if(m_units.begin(), m_units.end(),
		    [=](const pair<QString, double> pair) {
		return pair.first.at(0) == unit.at(0);
	}) - m_units.begin();

	if (i < m_units.size()) {
		value *= m_units[i].second;
		value /= m_displayScale;
		setValue(value);

		return true;
	}

	return false;
}

QPushButton *SpinBoxA::upButton()
{
	return ui->SBA_UpButton;
}

QPushButton *SpinBoxA::downButton()
{
	return ui->SBA_DownButton;
}

QLabel *SpinBoxA::nameLabel()
{
	return ui->SBA_Label;
}

QLineEdit *SpinBoxA::lineEdit()
{
	return ui->SBA_LineEdit;
}

QFrame *SpinBoxA::line()
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

	if (isZero(value, 1E-12)) {
		value = 0;
	}

	if (value < m_min_value) {
		value = m_min_value;
	} else if (value > m_max_value) {
		value = m_max_value;
	}

	if (m_value != value) {
		m_value = value;
		emitValueChanged = true;
	}

	// Update line edit
	int index;
	double scale = findUnitOfValue(m_value, &index);
	double number = m_value / scale;
	double abs_number = qAbs(number);
	int significant_digits = m_decimal_count;
	if (abs_number >= 100) {
		significant_digits += 3;
	} else if (abs_number >= 10) {
		significant_digits += 2;
	} else if (abs_number >= 1) {
		significant_digits += 1;
	}

	number *= m_displayScale;

	ui->SBA_LineEdit->setText(QString::number(number, 'g',
		significant_digits));
	ui->SBA_LineEdit->setCursorPosition(0);

	if (m_value != 0) {
		ui->SBA_Combobox->blockSignals(true);
		ui->SBA_Combobox->setCurrentIndex(index);
		ui->SBA_Combobox->blockSignals(false);
	}

	if (emitValueChanged) {
		Q_EMIT valueChanged(m_value);
	}
}

double SpinBoxA::minValue()
{
	return m_min_value;
}

void SpinBoxA::setMinValue(double value)
{
	m_min_value = value;

	if (m_value < m_min_value) {
		setValue(m_min_value);
	}

	ui->SBA_CompletionCircle->setMinimumDouble(value);
}

int SpinBoxA::decimalCount() const
{
	return m_decimal_count;
}

void SpinBoxA::setDecimalCount(int count)
{
	if (count >= 0) {
		m_decimal_count = count;
	}
}

double SpinBoxA::maxValue()
{
	return m_max_value;
}

void SpinBoxA::setMaxValue(double value)
{
	m_max_value = value;

	if (m_value > m_max_value) {
		setValue(m_max_value);
	}

	ui->SBA_CompletionCircle->setMaximumDouble(value);
}

bool SpinBoxA::isInFineMode()
{
	return ui->SBA_CompletionCircle->toggledState();
}

void SpinBoxA::setFineMode(bool en)
{
	ui->SBA_CompletionCircle->setToggled(en);
}

bool SpinBoxA::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == ui->SBA_LineEdit) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent *keyE = static_cast<QKeyEvent *>(event);

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
			QWheelEvent *wheelE = static_cast<QWheelEvent *>(event);

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

	if (posInUnitsList) {
		*posInUnitsList = index;
	}

	return m_units[index].second;
}

bool SpinBoxA::isCircleInverted() const
{
	return ui->SBA_CompletionCircle->property("inverted").toBool();
}

void SpinBoxA::invertCircle(bool invert)
{
	ui->SBA_CompletionCircle->setProperty("inverted", QVariant(invert));
}

bool SpinBoxA::progressShown() const
{
	return ui->SBA_CompletionCircle->isVisible();
}

void SpinBoxA::showProgress(bool show)
{
	ui->SBA_CompletionCircle->setVisible(show);
}

bool SpinBoxA::fineModeAvailable()
{
    return ui->SBA_CompletionCircle->toggleable();
}
void SpinBoxA::setFineModeAvailable(bool tog)
{
    ui->SBA_CompletionCircle->setToggleable(tog);
}

QString SpinBoxA::getName() const
{
	return ui->SBA_Label->text();
}

void SpinBoxA::setName(const QString& name)
{
	ui->SBA_Label->setText(name);
}

void SpinBoxA::setDisplayScale(double value)
{
	m_displayScale = value;
	setValue(m_value);
}

void SpinBoxA::setUnits(const QStringList& list)
{
	QString regex = "^(?!^.{18})(([+,-]?)([0-9]*)([.]?)([0-9]+))";
	QString sufixes;

	ui->SBA_Combobox->clear();

	for (auto it = list.begin(); it != list.end(); ++it) {
		QStringList curr = it->split('=');
		QString s = curr.at(0);
		double val = curr.at(1).toDouble();
		sufixes += (s.at(0) + '|');

		m_units.push_back(std::pair<QString, double>(s, val));
		ui->SBA_Combobox->addItem(s);
	}

	if (!sufixes.isEmpty()) {
		if (sufixes.contains("μ", Qt::CaseInsensitive)) {
			sufixes += 'u', Qt::CaseInsensitive;
			sufixes += '|';
		}
		if (sufixes.contains("π", Qt::CaseInsensitive)) {
			sufixes += 'r', Qt::CaseInsensitive;
			sufixes += '|';
		}

		QString lowercaseSufixes = sufixes.toLower();
		sufixes.remove("μ|", Qt::CaseSensitive);
		sufixes.remove("π|", Qt::CaseSensitive);
		QString upercaseSufixes = sufixes.toUpper();
		sufixes = upercaseSufixes + lowercaseSufixes;
		sufixes.chop(1);
		regex += "([" + sufixes + "]?)";
	}

	m_validator->setRegExp(QRegExp(regex));

	m_units_list = list;
}

/*
 * ScaleSpinButton class implementation
 */
ScaleSpinButton::ScaleSpinButton(QWidget *parent) : SpinBoxA(parent),
	m_steps(1E-3, 1E+3, 10,
{
	1, 2, 5
}),
m_fine_increment(1)
{
	ui->SBA_CompletionCircle->setIsLogScale(true);
}

ScaleSpinButton::ScaleSpinButton(vector<pair<QString, double> >units,
                                 const QString& name,
                                 double min_value, double max_value,
				 bool hasProgressWidget, bool invertCircle, QWidget *parent, std::vector<double> steps):
	SpinBoxA(units, name, min_value, max_value,
	         hasProgressWidget, invertCircle, parent),
	m_steps(1E-3, 1E+3, 10,
steps),
m_fine_increment(1)
{
	ui->SBA_CompletionCircle->setIsLogScale(true);

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
	current_val /= m_displayScale;
	double current_scale = m_units[ui->SBA_Combobox->currentIndex()].second;
	double newVal;

	double epsilon = 1E-15;

	if (isInFineMode()) {
		newVal = (current_val + m_fine_increment) * current_scale;
	} else {
		auto oldVal = current_val * current_scale + epsilon;
		newVal =  m_steps.getNumberAfter(oldVal);
		if(oldVal>=newVal) // reached end of scale
			newVal = maxValue();

	}

	setValue(newVal);
}

void ScaleSpinButton::stepDown()
{
	double current_val = ui->SBA_LineEdit->text().toDouble();
	current_val /= m_displayScale;
	double current_scale = m_units[ui->SBA_Combobox->currentIndex()].second;
	double newVal;

	double epsilon = 1E-15;

	if (isInFineMode()) {
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
PositionSpinButton::PositionSpinButton(QWidget *parent) : SpinBoxA(parent),
	m_step(1)
{
}

PositionSpinButton::PositionSpinButton(vector<pair<QString, double> >units,
                                       const QString& name,
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
	current_val /= m_displayScale;
	double current_scale = m_units[ui->SBA_Combobox->currentIndex()].second;
	double newVal;
	double step = m_step;

	if(m_step < current_scale) {
		step = current_scale / 10;
	}

	if (isInFineMode()) {
		step /= 10;
	}

	newVal =  current_val * current_scale + step;

	setValue(newVal);
}

void PositionSpinButton::stepDown()
{
	double current_val = ui->SBA_LineEdit->text().toDouble();
	current_val /= m_displayScale;
	double current_scale = m_units[ui->SBA_Combobox->currentIndex()].second;
	double newVal;
	double step = m_step;

	if(m_step < current_scale) {
		step = current_scale / 10;
	}

	if (isInFineMode()) {
		step /= 10;
	}

	newVal = current_val * current_scale - step;

	setValue(newVal);
}

PhaseSpinButton::PhaseSpinButton(QWidget *parent) : SpinBoxA(
{ {"deg",1}, {"π rad",180}
},"Phase",0,360,true,true,parent),
m_step(15),
m_fine_increment(1)
{
	ui->SBA_CompletionCircle->setIsLogScale(false);
	ui->SBA_CompletionCircle->setOrigin(0);
}

void PhaseSpinButton::setValue(double value)
{
	bool emitValueChanged = false;

	if (isZero(value, 1E-12)) {
		value = 0;
	}

	double period = 360;
	int full_periods=value/period;
	value = value - full_periods * period;

	// Update line edit
	int index;
	auto scale = m_units.at(ui->SBA_Combobox->currentIndex()).second;

	if (value<0) {
		value = value + period;
	}

	if (m_value != value) {
		m_value = value;
		emitValueChanged = true;
	}

	ui->SBA_LineEdit->setText(QString::number(round((m_value*10)/10) / scale));

	// Update line edit
	if (emitValueChanged) {
		Q_EMIT valueChanged(m_value);
	}
}

void PhaseSpinButton::onComboboxIndexChanged(int index)
{
	setValue(m_value);
}

void PhaseSpinButton::stepUp()
{
	double current_scale = m_units[ui->SBA_Combobox->currentIndex()].second;
	double newVal;
	double step = m_step;

	if (isInFineMode()) {
		step = 1;
	}

	newVal =  m_value + step;

	setValue(newVal);
}

void PhaseSpinButton::stepDown()
{
	double current_val = ui->SBA_LineEdit->text().toDouble();
	double current_scale = m_units[ui->SBA_Combobox->currentIndex()].second;
	double newVal;
	double step = m_step;

	if (isInFineMode()) {
		step = 1;
	}

	newVal = m_value - step;

	setValue(newVal);
}

double PhaseSpinButton:: step()
{
	return m_step;
}

void PhaseSpinButton::setStep(double step)
{
	m_step = step;
}
