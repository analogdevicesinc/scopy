/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "completion_circle.hpp"
#include "singletone_wrapper.hpp"

#include "ui_spinbox_a.h"

#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QRegExpValidator>
#include <QVBoxLayout>
#include <qmath.h>

#include <scopy/core/api_object_manager.hpp>
#include <scopy/gui/spinbox_a.hpp>

using namespace std;
using namespace scopy::gui;
using namespace scopy::core;

/*
 * SpinBoxA class implementation
 */

unsigned int SpinBoxA::current_id(0);

SpinBoxA::SpinBoxA(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::SpinBoxA)
	, m_value(0.0)
	, m_minValue(0.0)
	, m_maxValue(0.0)
	, m_decimalCount(3)
	, m_validator(new QRegExpValidator(this))
	, m_sbaApi(new SpinBoxA_API(this))
{
	ui->setupUi(this);
	ui->lineEditSBA->setValidator(m_validator);

	ui->lineEditSBA->installEventFilter(this);
	ui->SBA_CompletionCircle->installEventFilter(this);

	connect(ui->cmbBoxSBA, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxIndexChanged(int)));
	connect(ui->lineEditSBA, SIGNAL(editingFinished()), SLOT(onLineEditTextEdited()));
	connect(ui->btnUpSBA, SIGNAL(pressed()), SLOT(onUpButtonPressed()));
	connect(ui->btnDownSBA, SIGNAL(pressed()), SLOT(onDownButtonPressed()));

	connect(this, SIGNAL(valueChanged(double)), ui->SBA_CompletionCircle, SLOT(setValueDouble(double)));
	connect(ui->SBA_CompletionCircle, SIGNAL(toggled(bool)), SLOT(setFineMode(bool)));

	m_displayScale = 1;
}

SpinBoxA::SpinBoxA(vector<pair<QString, double>> units, const QString& name, double min_value, double max_value,
		   bool hasProgressWidget, bool invertCircle, QWidget* parent)
	: SpinBoxA(parent)
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

	ui->lblSBA->setText(name);

	m_displayScale = 1;

	if (m_value < min_value) {
		setValue(min_value);
	} else if (m_value > max_value) {
		setValue(max_value);
	}

	m_id = current_id++;

	QString spinBoxName = parent->objectName() + "SpinBox" + name + QString::number(m_id);
	spinBoxName.remove(" ");
	m_sbaApi->setObjectName(spinBoxName);

	QSettings oldSettings;
	QFile tempFile(oldSettings.fileName() + ".bak");
	m_settings = SingleToneWrapper<QSettings*>::getInstance().getWrapped();

	// m_sba_api->load(*m_settings);
	m_isStepDown = false;
}

SpinBoxA::~SpinBoxA()
{
	current_id--;

	// m_sba_api->save(*m_settings);

	delete m_sbaApi;
	delete ui;
}

void SpinBoxA::updateCompletionCircle(double value) { ui->SBA_CompletionCircle->setValueDouble(value); }

void SpinBoxA::silentSetValue(double value)
{
	// Set the value for the spinBox w/o
	// emitting any signals.
	QSignalBlocker signalBlocker(this);
	setValue(value);
	triggerCircleRedraw();
}

void SpinBoxA::silentSetMinValue(double value)
{
	// Set the minimum value w/o emitting any signals
	QSignalBlocker signalBlocker(this);
	setMinValue(value);
}

void SpinBoxA::silentSetMaxValue(double value)
{
	// Set the maximum value w/o emitting any signals
	QSignalBlocker signalBlocker(this);
	setMaxValue(value);
}

void SpinBoxA::onUpButtonPressed() { stepUp(); }

void SpinBoxA::onDownButtonPressed() { stepDown(); }

void SpinBoxA::onComboboxIndexChanged(int index)
{
	double value = ui->lineEditSBA->text().toDouble();

	setValue(value * m_units[index].second);
}

void SpinBoxA::onLineEditTextEdited()
{
	QLineEdit* lineEdit = static_cast<QLineEdit*>(QObject::sender());
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
		unit = m_units[ui->cmbBoxSBA->currentIndex()].first;
	} else if (unit.startsWith(QString("u"), Qt::CaseInsensitive)) {
		qDebug() << "before unit " << unit;
		unit = unit.replace(0, 1, "μ");
		qDebug() << "after unit " << unit;
	} else if (unit.startsWith(QString("r"), Qt::CaseInsensitive)) {
		qDebug() << "before unit " << unit;
		unit = unit.replace(0, 1, "π");
		qDebug() << "after unit " << unit;
	}

	// check if the current user input fits to any unit measure
	if (isUnitMatched(unit, value)) {
		return;
	}

	bool isLowerCase = unit == unit.toLower();

	if (isLowerCase) {
		unit = unit.toUpper();
	} else {
		unit = unit.toLower();
	}

	// change the user input and try again to find a match
	isUnitMatched(unit, value);
}

bool SpinBoxA::isUnitMatched(const QString& unit, double value)
{
	int i = find_if(m_units.begin(), m_units.end(),
			[=](const pair<QString, double> pair) { return pair.first.at(0) == unit.at(0); }) -
		m_units.begin();

	if (i < m_units.size()) {
		value *= m_units[i].second;
		value /= m_displayScale;
		setValue(value);

		return true;
	}

	return false;
}

QPushButton* SpinBoxA::upButton() { return ui->btnUpSBA; }

QPushButton* SpinBoxA::downButton() { return ui->btnDownSBA; }

QLabel* SpinBoxA::nameLabel() { return ui->lblSBA; }

QLineEdit* SpinBoxA::lineEdit() { return ui->lineEditSBA; }

QFrame* SpinBoxA::line() { return ui->lineSBA; }

QComboBox* SpinBoxA::comboBox() { return ui->cmbBoxSBA; }

double SpinBoxA::value() { return m_value; }

static bool isZero(double value, double threshold) { return (value >= -threshold && value <= threshold); }

void SpinBoxA::setValue(double value)
{
	bool emitValueChanged = false;

	if (isZero(value, 1E-12)) {
		value = 0;
	}

	if (value < m_minValue) {
		value = m_minValue;
	} else if (value > m_maxValue) {
		value = m_maxValue;
	}

	if (m_value != value) {
		m_value = value;
		emitValueChanged = true;
	}

	// Update line edit
	int index;
	double scale = findUnitOfValue(m_value * m_displayScale, &index);
	double number = m_value / scale;
	double abs_number = qAbs(number);
	int significant_digits = m_decimalCount;

	if (abs_number >= 100) {
		significant_digits += 3;
	} else if (abs_number >= 10) {
		significant_digits += 2;
	} else if (abs_number >= 1) {
		significant_digits += 1;
	}

	number *= m_displayScale;

	ui->lineEditSBA->setText(QString::number(number, 'g', significant_digits));
	ui->lineEditSBA->setCursorPosition(0);

	if (m_value != 0) {
		ui->cmbBoxSBA->blockSignals(true);
		ui->cmbBoxSBA->setCurrentIndex(index);
		ui->cmbBoxSBA->blockSignals(false);
	}

	if (m_value == m_minValue) {
		ui->btnDownSBA->setEnabled(false);
	} else {
		ui->btnDownSBA->setEnabled(true);
	}

	if (m_value == m_maxValue) {
		ui->btnUpSBA->setEnabled(false);
	} else {
		ui->btnUpSBA->setEnabled(true);
	}

	if (emitValueChanged) {
		Q_EMIT valueChanged(m_value);
	}
}

double SpinBoxA::minValue() { return m_minValue; }

void SpinBoxA::setMinValue(double value)
{
	m_minValue = value;

	if (m_value < m_minValue) {
		setValue(m_minValue);
	} else if (m_value > m_minValue) {
		ui->btnDownSBA->setEnabled(true);
	}

	ui->SBA_CompletionCircle->setMinimumDouble(value);
}

int SpinBoxA::decimalCount() const { return m_decimalCount; }

void SpinBoxA::setDecimalCount(int count)
{
	if (count >= 0) {
		m_decimalCount = count;
	}
}

double SpinBoxA::maxValue() { return m_maxValue; }

void SpinBoxA::setMaxValue(double value)
{
	m_maxValue = value;

	if (m_value > m_maxValue) {
		setValue(m_maxValue);
	} else if (m_value < m_maxValue) {
		ui->btnUpSBA->setEnabled(true);
	}

	ui->SBA_CompletionCircle->setMaximumDouble(value);
}

bool SpinBoxA::isInFineMode() { return ui->SBA_CompletionCircle->toggledState(); }

void SpinBoxA::setFineMode(bool en) { ui->SBA_CompletionCircle->setToggled(en); }

bool SpinBoxA::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == ui->lineEditSBA) {
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
			if (!isEnabled()) {
				return QWidget::eventFilter(obj, event);
			}

			QWheelEvent* wheelE = static_cast<QWheelEvent*>(event);

			if (wheelE->angleDelta().y() > 0) {
				stepUp();
				event->accept();
				return true;
			} else if (wheelE->angleDelta().y() < 0) {
				stepDown();
				event->accept();
				return true;
			}
		} else if (event->type() == QEvent::FocusOut) {
			setValue(m_value);
		}
	}

	if (obj == ui->SBA_CompletionCircle) {
		if (!isEnabled()) {
			return QWidget::eventFilter(obj, event);
		}

		if (event->type() == QEvent::Enter) {
			setCursor(Qt::SizeVerCursor);
		} else if (event->type() == QEvent::Leave) {
			setCursor(Qt::ArrowCursor);

		} else if (event->type() == QEvent::Wheel) {
			QWheelEvent* wheelE = static_cast<QWheelEvent*>(event);

			if (wheelE->angleDelta().y() > 0) {
				stepUp();
				event->accept();
				return true;
			} else if (wheelE->angleDelta().y() < 0) {
				stepDown();
				event->accept();
				return true;
			}
		}
	}

	return QWidget::eventFilter(obj, event);
}

double SpinBoxA::findUnitOfValue(double val, int* posInUnitsList)
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

bool SpinBoxA::isCircleInverted() const { return ui->SBA_CompletionCircle->property("inverted").toBool(); }

void SpinBoxA::invertCircle(bool invert) { ui->SBA_CompletionCircle->setProperty("inverted", QVariant(invert)); }

bool SpinBoxA::progressShown() const { return ui->SBA_CompletionCircle->isVisible(); }

void SpinBoxA::showProgress(bool show) { ui->SBA_CompletionCircle->setVisible(show); }

bool SpinBoxA::fineModeAvailable() { return ui->SBA_CompletionCircle->toggleable(); }
void SpinBoxA::setFineModeAvailable(bool tog) { ui->SBA_CompletionCircle->setToggleable(tog); }

bool SpinBoxA::isStepDown() const { return m_isStepDown; }

void SpinBoxA::triggerCircleRedraw() { ui->SBA_CompletionCircle->setValueDouble(value()); }

QString SpinBoxA::getName() const { return ui->lblSBA->text(); }

void SpinBoxA::setName(const QString& name) { ui->lblSBA->setText(name); }

void SpinBoxA::setDisplayScale(double value)
{
	m_displayScale = value;
	setValue(m_value);
}

void SpinBoxA::setUnits(const QStringList& list)
{
	QString regex = "^(?!^.{18})(([+,-]?)([0-9]*)([.]?)([0-9]+))";
	QString sufixes;

	ui->cmbBoxSBA->clear();

	if (list.at(0).section("=", 0, 0).trimmed().isEmpty() || list.count() == 1) {
		ui->cmbBoxSBA->setEnabled(false);
	}

	for (auto it = list.begin(); it != list.end(); ++it) {
		QStringList curr = it->split('=');
		QString s = curr.at(0);
		double val = curr.at(1).toDouble();
		sufixes += (s.at(0) + '|');

		m_units.push_back(std::pair<QString, double>(s, val));
		ui->cmbBoxSBA->addItem(s);
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

	m_unitsList = list;
}

/*
 * ScaleSpinButton class implementation
 */
ScaleSpinButton::ScaleSpinButton(QWidget* parent)
	: SpinBoxA(parent)
	, m_steps(1E-3, 1E+3, 10, {1, 2, 5})
	, m_fineIncrement(1)
{
	ui->SBA_CompletionCircle->setIsLogScale(true);
}

ScaleSpinButton::ScaleSpinButton(vector<pair<QString, double>> units, const QString& name, double min_value,
				 double max_value, bool hasProgressWidget, bool invertCircle, QWidget* parent,
				 std::vector<double> steps)
	: SpinBoxA(units, name, min_value, max_value, hasProgressWidget, invertCircle, parent)
	, m_steps(1E-3, 1E+3, 10, steps)
	, m_fineIncrement(1)
	, m_numberSeriesRebuild(true)
	, m_integerDivider(0)
{
	ui->SBA_CompletionCircle->setIsLogScale(true);

	setMinValue(min_value);
	setMaxValue(max_value);
}

void ScaleSpinButton::setMinValue(double value)
{
	if (m_numberSeriesRebuild) {
		m_steps.setLower(value);
	}
	SpinBoxA::setMinValue(value);

	if (m_value == m_minValue) {
		ui->btnDownSBA->setEnabled(false);
	} else {
		ui->btnDownSBA->setEnabled(true);
	}
}

void ScaleSpinButton::setMaxValue(double value)
{
	if (m_numberSeriesRebuild) {
		m_steps.setUpper(value);
	}
	SpinBoxA::setMaxValue(value);

	if (m_value == m_maxValue) {
		ui->btnUpSBA->setEnabled(false);
	} else {
		ui->btnUpSBA->setEnabled(true);
	}
}

void ScaleSpinButton::enableNumberSeriesRebuild(bool enable) { m_numberSeriesRebuild = enable; }

void ScaleSpinButton::setIntegerDivider(int val) { m_integerDivider = val; }

int ScaleSpinButton::getIntegerDivider() { return m_integerDivider; }

void ScaleSpinButton::setValue(double newVal)
{
	if (m_integerDivider) {
		m_integerStep = round(m_integerDivider / newVal);
		newVal = m_integerDivider / m_integerStep;
	}
	SpinBoxA::setValue(newVal);
}

void ScaleSpinButton::stepUp()
{
	double current_val = ui->lineEditSBA->text().toDouble();
	current_val /= m_displayScale;
	double current_scale = m_units[ui->cmbBoxSBA->currentIndex()].second;
	double newVal;

	double epsilon = 1E-12;

	if (isInFineMode()) {
		newVal = (current_val + m_fineIncrement) * current_scale;
	} else {
		auto oldVal = current_val * current_scale + epsilon;
		newVal = m_steps.getNumberAfter(oldVal);

		if (oldVal >= newVal) { // reached end of scale
			newVal = maxValue();
		}
	}
	if (m_integerDivider) {
		auto new_integer_step = ceil(m_integerDivider / newVal);
		if (new_integer_step == m_integerStep && new_integer_step > 1)
			m_integerStep--;
		else
			m_integerStep = new_integer_step;
		newVal = m_integerDivider / m_integerStep;
	}

	setValue(newVal);
}

void ScaleSpinButton::stepDown()
{
	double current_val = ui->lineEditSBA->text().toDouble();
	current_val /= m_displayScale;
	double current_scale = m_units[ui->cmbBoxSBA->currentIndex()].second;
	double newVal;

	double epsilon = 1E-12;

	if (isInFineMode()) {
		newVal = (current_val - m_fineIncrement) * current_scale;

		if (current_val - m_fineIncrement < 1 && current_val - m_fineIncrement > 0) {
			newVal = 0;
		}

		if ((m_minValue > 0 && newVal <= m_minValue) && (ui->cmbBoxSBA->currentIndex() > 0)) {
			int i = ui->cmbBoxSBA->currentIndex() - 1;
			double nextLowerScale = m_units[i].second;

			newVal = current_scale / nextLowerScale - m_fineIncrement;
			newVal *= nextLowerScale;
		}

	} else {
		newVal = m_steps.getNumberBefore(current_val * current_scale - epsilon);
	}

	if (m_integerDivider) {
		auto new_integer_step = floor(m_integerDivider / newVal);
		if (new_integer_step == m_integerStep)
			m_integerStep++;
		else
			m_integerStep = new_integer_step;
		newVal = m_integerDivider / m_integerStep;
	}
	setValue(newVal);
}

/*
 * PositionSpinButton class implementation
 */
PositionSpinButton::PositionSpinButton(QWidget* parent)
	: SpinBoxA(parent)
	, m_step(1)
{}

PositionSpinButton::PositionSpinButton(vector<pair<QString, double>> units, const QString& name, double min_value,
				       double max_value, bool hasProgressWidget, bool invertCircle, QWidget* parent)
	: SpinBoxA(units, name, min_value, max_value, hasProgressWidget, invertCircle, parent)
	, m_step(1)
{}

double PositionSpinButton::step() { return m_step; }

void PositionSpinButton::setStep(double step) { m_step = step; }

void PositionSpinButton::stepUp()
{
	double current_val = ui->lineEditSBA->text().toDouble();
	current_val /= m_displayScale;
	double current_scale = m_units[ui->cmbBoxSBA->currentIndex()].second;
	double newVal;
	double step = m_step;

	double ratio = current_scale / m_step;

	if (ratio > 1e3) {
		step = step * 1e3;
	}

	if (isInFineMode()) {
		step /= 10;
	}

	newVal = current_val * current_scale + step;

	setValue(newVal);
}

void PositionSpinButton::stepDown()
{
	double current_val = ui->lineEditSBA->text().toDouble();
	current_val /= m_displayScale;
	double current_scale = m_units[ui->cmbBoxSBA->currentIndex()].second;
	double newVal;
	double step = m_step;

	double ratio = current_scale / m_step;

	if (ratio > 1e3) {
		step = step * 1e3;
	}

	if (isInFineMode()) {
		step /= 10;
	}

	newVal = current_val * current_scale - step;

	setValue(newVal);
}

PhaseSpinButton::PhaseSpinButton(QWidget* parent)
	: SpinBoxA(parent)
	, m_fineIncrement(1)
	, m_inSeconds(false)
	, m_indexValue(0)
	, m_step(15)
	, m_secondsValue(0)
	, m_scale(1)
	, m_frequency(1)
{
	ui->SBA_CompletionCircle->setIsLogScale(false);
	ui->SBA_CompletionCircle->setOrigin(0);
}

PhaseSpinButton::PhaseSpinButton(std::vector<std::pair<QString, double>> units, const QString& name, double min_value,
				 double max_value, bool hasProgressWidget, bool invertCircle, QWidget* parent)
	: SpinBoxA(units, name, min_value, max_value, hasProgressWidget, invertCircle, parent)
	, m_fineIncrement(1)
	, m_inSeconds(false)
	, m_indexValue(0)
	, m_step(15)
	, m_secondsValue(0)
	, m_scale(1)
{
	ui->SBA_CompletionCircle->setIsLogScale(false);
	setMinValue(min_value);
	setMaxValue(max_value);

	setFrequency(0.01);
	ui->SBA_CompletionCircle->setOrigin(0);

	ui->btnDownSBA->setEnabled(true);
	ui->btnUpSBA->setEnabled(true);
}

void PhaseSpinButton::setValue(double value)
{
	/* This method might be called when the PhaseSpinButton looses focus with
	 * value = m_value. In this case value would be the real value (m_value)
	 * not the one that is displayed. In this case we ignore the call and
	 * return immediately */
	if (m_value == value && !QObject::sender()) {
		return;
	}

	bool emitValueChanged = false;

	if (isZero(value, 1E-12)) {
		value = 0;
	}

	if (inSeconds()) {
		double periodInSeconds = changeValueFromDegreesToSeconds(360);
		if (value < 0) {
			value = value + periodInSeconds;
		} else if (value > periodInSeconds) {
			int full_periods = value / periodInSeconds;
			value -= full_periods * periodInSeconds;
		}
	}

	double period = 360;
	double scale;

	if (inSeconds()) {
		int index;
		scale = findUnitOfValue(value, &index);
		value = computeSecondsTransformation(scale, index, value);
	} else {
		scale = m_units.at(ui->cmbBoxSBA->currentIndex()).second;
	}

	// Update line edit

	int full_periods = value / period;
	value -= full_periods * period;

	if (value < 0) {
		value = value + period;
	}

	if (m_value != value) {
		m_value = value;
		emitValueChanged = true;
	}

	if (m_value == 0) {
		setSecondsValue(0);
	}

	if (m_scale != scale) {
		m_scale = scale;
		emitValueChanged = true;
	}

	if (!inSeconds() || m_value == 0) {
		ui->lineEditSBA->setText(QString::number(round((m_value * 10) / 10) / scale));
	}

	// adjust scale for the ecurrent measure unit and handle unit measure change

	if (emitValueChanged) {
		Q_EMIT valueChanged(m_value);
	}
}

void PhaseSpinButton::setComboboxIndex(int index) { ui->cmbBoxSBA->setCurrentIndex(index); }

void PhaseSpinButton::onComboboxIndexChanged(int index)
{
	m_indexValue = index;

	if (index < 2) {
		setInSeconds(false);
	} else {
		bool isGoingFromDegreesToSeconds = false;

		if (!inSeconds()) {
			double period = 360;
			m_secondsValue = m_value / frequency();
			m_secondsValue /= period;
			isGoingFromDegreesToSeconds = true;
		}

		setInSeconds(true);

		if (isGoingFromDegreesToSeconds) {
			setValue(m_secondsValue);
		} else {
			double value = ui->lineEditSBA->text().toDouble();
			setValue(value * m_units[index].second);
		}

		return;
	}

	setValue(m_value);
}

void PhaseSpinButton::setInSeconds(bool val) { m_inSeconds = val; }

bool PhaseSpinButton::inSeconds() { return m_inSeconds; }

void PhaseSpinButton::setSecondsValue(double val) { m_secondsValue = val; }

double PhaseSpinButton::secondsValue() { return m_secondsValue; }

void PhaseSpinButton::setFrequency(double val)
{
	if (inSeconds()) {
		qDebug() << "frequency changed:" << val << "update phase";

		updatePhaseAfterFrequenceChanged(val);
	}

	m_frequency = val;
}

double PhaseSpinButton::frequency() { return m_frequency; }

double PhaseSpinButton::value() { return m_value; }

double PhaseSpinButton::changeValueFromDegreesToSeconds(double val)
{
	double period = 360;
	double valueInSeconds = val / frequency();
	valueInSeconds /= period;

	return valueInSeconds;
}

int PhaseSpinButton::indexValue() { return m_indexValue; }

void PhaseSpinButton::updatePhaseAfterFrequenceChanged(double val)
{
	if (!inSeconds()) {
		return;
	}

	double period = 360;
	m_value = secondsValue() * period * val;
}

double PhaseSpinButton::computeSecondsTransformation(double scale, int index, double value)
{
	setSecondsValue(value);

	double period = 360;
	double number = value / scale;
	double abs_number = qAbs(number);
	int significant_digits = m_decimalCount;

	if (abs_number >= 100) {
		significant_digits += 3;
	} else if (abs_number >= 10) {
		significant_digits += 2;
	} else if (abs_number >= 1) {
		significant_digits += 1;
	}

	number *= m_displayScale;
	double degreesValue = secondsValue() * period * frequency();

	int full_periods = degreesValue / period;
	degreesValue -= full_periods * period;

	if (value != 0 && degreesValue != 0) {
		ui->cmbBoxSBA->blockSignals(true);
		ui->cmbBoxSBA->setCurrentIndex(index);

		if (index < 2) {
			setInSeconds(false);
		} else {
			setInSeconds(true);
		}

		ui->cmbBoxSBA->blockSignals(false);
		ui->lineEditSBA->setText(QString::number(number, 'g', significant_digits));
		ui->lineEditSBA->setCursorPosition(0);
	}
	value = degreesValue;

	return value;
}

void PhaseSpinButton::stepUp()
{
	m_isStepDown = false;
	double current_scale = m_units[ui->cmbBoxSBA->currentIndex()].second;
	double newVal;
	double step = inSeconds() ? changeValueFromDegreesToSeconds(45) : 45;

	if (isInFineMode()) {
		step = inSeconds() ? changeValueFromDegreesToSeconds(1) : 1;
	}

	if (inSeconds()) {
		m_value = secondsValue();
	}

	newVal = m_value + step;

	ui->lineEditSBA->setText(QString::number(round((newVal * 10) / 10) / current_scale));
	setValue(newVal);
}

void PhaseSpinButton::stepDown()
{
	m_isStepDown = true;
	double current_scale = m_units[ui->cmbBoxSBA->currentIndex()].second;
	double newVal;
	double step = inSeconds() ? changeValueFromDegreesToSeconds(45) : 45;

	if (inSeconds()) {
		m_value = secondsValue();
	}

	if (isInFineMode()) {
		step = inSeconds() ? changeValueFromDegreesToSeconds(1) : 1;
	}

	newVal = m_value - step;

	ui->lineEditSBA->setText(QString::number(round((newVal * 10) / 10) / current_scale));
	setValue(newVal);
}

void PhaseSpinButton::setStep(double step) { m_step = step; }

SpinBoxA_API::SpinBoxA_API(SpinBoxA* sba)
	: ApiObject()
	, sba(sba)
{
	ApiObjectManager::getInstance().registerApiObject(this);
}

SpinBoxA_API::~SpinBoxA_API() { ApiObjectManager::getInstance().unregisterApiObject(this); }

double SpinBoxA_API::value() const { return sba->value(); }

void SpinBoxA_API::setValue(double value) { sba->setValue(value); }

double SpinBoxA_API::minValue() const { return sba->minValue(); }

void SpinBoxA_API::setMinValue(double value) { sba->setMinValue(value); }

double SpinBoxA_API::maxValue() const { return sba->maxValue(); }

void SpinBoxA_API::setMaxValue(double value) { sba->setMaxValue(value); }

int SpinBoxA_API::decimalCount() const { return sba->decimalCount(); }

void SpinBoxA_API::setDecimalCount(int count) { sba->setDecimalCount(count); }

bool SpinBoxA_API::isInFineMode() { return sba->isInFineMode(); }

void SpinBoxA_API::setFineMode(bool enabled) { sba->setFineMode(enabled); }

bool SpinBoxA_API::isCircleInverted() const { return sba->isCircleInverted(); }

void SpinBoxA_API::invertCircle(bool invert) { sba->invertCircle(invert); }

bool SpinBoxA_API::progressShown() const { return sba->progressShown(); }

void SpinBoxA_API::showProgress(bool show) { sba->showProgress(show); }

bool SpinBoxA_API::fineModeAvailable() { return sba->fineModeAvailable(); }

void SpinBoxA_API::setFineModeAvailable(bool available) { sba->setFineModeAvailable(available); }

QString SpinBoxA_API::getName() const { return sba->getName(); }

void SpinBoxA_API::setName(const QString& name) { sba->setName(name); }
