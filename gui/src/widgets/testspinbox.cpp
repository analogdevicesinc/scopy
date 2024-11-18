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

#include "testspinbox.h"

#include <QBoxLayout>
#include <style.h>

using namespace scopy;
using namespace gui;

TestSpinbox::TestSpinbox(QString name, double val, QString unit, double min, double max, bool vertical, bool left,
			 QWidget *parent)
{
	m_min = min;
	m_max = max;
	m_value = val;

	m_label = new QLabel(name, parent);
	m_edit = new ProgressLineEdit(parent);
	m_plus = new QPushButton("", parent);
	m_minus = new QPushButton("", parent);
	m_mouseWheelGuard = new MouseWheelWidgetGuard(this);

	////SCALE
	m_scale = new Scale(unit, min, max);

	// ?? why not fixed as default since it seems to be the basic version of the method
	m_incrementStrategy = new IncrementStrategyPower2();

	m_plus->setAutoRepeat(true);
	m_plus->setAutoRepeatDelay(300);
	m_plus->setAutoRepeatInterval(20);

	m_minus->setAutoRepeat(true);
	m_minus->setAutoRepeatDelay(300);
	m_minus->setAutoRepeatInterval(20);

	if(vertical) {
		layoutVertically(left);
	} else {
		layoutHorizontally(left);
	}

	setValue(val);

	connect(m_plus, &QAbstractButton::clicked, this, [=]() { setValue(m_incrementStrategy->increment(m_value)); });
	connect(m_minus, &QAbstractButton::clicked, this, [=]() {
		double newValue = m_incrementStrategy->decrement(m_value);
		if(m_scale->scalingEnabled()) {
			// if value would be 0 check if lower scale available
			int idx = m_scale->scaleCb()->currentIndex();
			if(newValue == 0 && (idx - 1) >= 0) {
				// found lower scale update value based on that
				setValue(m_value - (1 * m_scale->scaleCb()->itemData(idx - 1).toDouble()));
			} else {
				setValue(newValue);
			}
		} else {
			setValue(newValue);
		}
	});

	connect(m_edit->getLineEdit(), &QLineEdit::editingFinished, this, [=]() {
		m_edit->getLineEdit()->blockSignals(true);
		updateValueStatus(ProgressBarState::BUSY);
		double value = userInput(m_edit->getLineEdit()->text());
		if(qIsNaN(value)) {
			updateWidgetsVal();
			updateValueStatus(ProgressBarState::ERROR);
		} else {
			setValue(value);
			updateValueStatus(ProgressBarState::SUCCESS);
		}
		m_edit->getLineEdit()->blockSignals(false);
	});

	connect(m_scale->scaleCb(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=]() {
		updateValueStatus(ProgressBarState::BUSY);
		double value = userInput(m_edit->getLineEdit()->text());
		if(qIsNaN(value)) {
			updateWidgetsVal();
			updateValueStatus(ProgressBarState::ERROR);
		} else {
			setValue(value);
			updateValueStatus(ProgressBarState::SUCCESS);
		}
	});
}

void TestSpinbox::layoutVertically(bool left)
{
	auto lay = new QHBoxLayout(this);
	setLayout(lay);

	lay->setSpacing(5);
	lay->setMargin(0);

	QLayout *btnLay;
	QLayout *editLay;

	btnLay = new QVBoxLayout();
	editLay = new QVBoxLayout();

	btnLay->setSpacing(2);
	btnLay->setMargin(0);
	editLay->setSpacing(2);
	editLay->setMargin(0);

	btnLay->addWidget(m_plus);
	btnLay->addWidget(m_minus);

	editLay->addWidget(m_label);
	editLay->addWidget(m_edit);

	editLay->addWidget(m_scale->scaleCb());

	if(left) {
		lay->addLayout(btnLay);
		lay->addLayout(editLay);
	} else {
		lay->addLayout(editLay);
		lay->addLayout(btnLay);
	}

	Style::setStyle(m_label, style::properties::label::subtle);

	int size = Style::getDimension(json::global::unit_2_5);
	m_plus->setIcon(Style::getPixmap(":/gui/icons/plus.svg", Style::getColor(json::theme::content_inverse)));
	Style::setStyle(m_plus, style::properties::button::spinboxButton);
	m_plus->setFixedSize(size, size);

	m_minus->setIcon(Style::getPixmap(":/gui/icons/minus.svg", Style::getColor(json::theme::content_inverse)));
	Style::setStyle(m_minus, style::properties::button::spinboxButton);
	m_minus->setFixedSize(size, size);
}

void TestSpinbox::layoutHorizontally(bool left)
{
	// Layout for the underline
	auto lineLay = new QVBoxLayout(this);
	lineLay->setSpacing(5);
	lineLay->setMargin(0);
	setLayout(lineLay);

	// Elements layout
	auto lay = new QHBoxLayout(this);
	lay->setSpacing(5);
	lay->setMargin(0);

	QLayout *btnLay;
	QLayout *editLay;

	btnLay = new QHBoxLayout();
	editLay = new QHBoxLayout();

	btnLay->setSpacing(2);
	btnLay->setMargin(0);
	editLay->setSpacing(8);
	editLay->setContentsMargins(0, 0, 5, 0);

	btnLay->addWidget(m_minus);
	btnLay->addWidget(m_plus);

	editLay->addWidget(m_label);
	editLay->addWidget(m_edit);

	editLay->addWidget(m_scale->scaleCb());
	lineLay->addLayout(lay);

	if(left) {
		lay->addLayout(btnLay);
		lay->addLayout(editLay);
	} else {
		lay->addLayout(editLay);
		lay->addLayout(btnLay);
	}

	Style::setStyle(m_label, style::properties::label::subtle);

	int size = Style::getDimension(json::global::unit_2_5);
	m_plus->setIcon(Style::getPixmap(":/gui/icons/plus.svg", Style::getColor(json::theme::content_inverse)));
	Style::setStyle(m_plus, style::properties::button::spinboxButton);
	m_plus->setFixedSize(size, size);

	m_minus->setIcon(Style::getPixmap(":/gui/icons/minus.svg", Style::getColor(json::theme::content_inverse)));
	Style::setStyle(m_minus, style::properties::button::spinboxButton);
	m_minus->setFixedSize(size, size);
}

double TestSpinbox::clamp(double val, double min, double max)
{
	val = std::max(val, min);
	if(val == min) {
		m_minus->setEnabled(false);
	} else {
		m_minus->setEnabled(true);
	}

	val = std::min(val, max);

	if(val == max) {
		m_plus->setEnabled(false);
	} else {
		m_plus->setEnabled(true);
	}

	return val;
}

void TestSpinbox::updateValueStatus(ProgressBarState status)
{
	if(status == ProgressBarState::SUCCESS) {
		m_edit->getProgressBar()->setBarColor(Style::getAttribute(json::theme::content_success));
	}
	if(status == ProgressBarState::ERROR) {
		m_edit->getProgressBar()->setBarColor(Style::getAttribute(json::theme::content_error));
	}
	if(status == ProgressBarState::BUSY) {
		m_edit->getProgressBar()->startProgress(100, 10);
		m_edit->getProgressBar()->setBarColor(Style::getAttribute(json::theme::content_busy));
	}
}

Scale *TestSpinbox::scale() const { return m_scale; }

void TestSpinbox::setScale(Scale *newScale) { m_scale = newScale; }

void TestSpinbox::setScalingEnabled(bool en) { m_scale->setScalingEnabled(en); }

bool TestSpinbox::scallingEnabled() { return m_scale->scalingEnabled(); }

int TestSpinbox::findLastDigit(QString str)
{
	for(int i = str.length() - 1; i >= 0; --i) {
		if(str[i].isDigit()) {
			return i;
		}
	}
	return -1; // Return -1 if no digit is found
}

double TestSpinbox::userInput(QString s)
{
	// remove whitespace
	s = s.simplified();
	s.replace(" ", "");

	// find last digit position
	int i = findLastDigit(s);
	QString nr = s.left(
		i + 1); // interpret number up to that digit - this makes sure you can also set stuff like 2e6 or 2M
	bool ok;
	double val = nr.toDouble(&ok);
	if(!ok)
		return Q_QNAN; // reset

	if(m_scale->scalingEnabled()) {
		QString unit = s.mid(i + 1, s.length() - 1); // isolate unit from the whole string (min)
		if(unit.length() > 0) {			     // user wrote a unit
			double scaleValue = m_scale->getScaleForSymbol(unit); // find the unit in the map
			if(scaleValue == -1) {
				return Q_QNAN; // inputed prefix is invalid
			} else {
				val = val * scaleValue; // scale accordingly
			}

		} else {
			// Apply current scale to value
			double scaleValue = m_scale->scaleCb()->currentData().toDouble();
			val = val * scaleValue;
		}
	}

	return val;
}

double TestSpinbox::value() const { return m_value; }

IncrementStrategy *TestSpinbox::incrementStrategy() const { return m_incrementStrategy; }

void TestSpinbox::setIncrementMode(IncrementMode im)
{
	if(m_im == im)
		return;

	m_im = im;
	delete m_incrementStrategy;
	switch(m_im) {

	case IS_POW2:
		m_incrementStrategy = new IncrementStrategyPower2();
		break;
	case IS_125:
		m_incrementStrategy = new IncrementStrategy125();
		break;
	case IS_FIXED:
	default:
		m_incrementStrategy = new IncrementStrategyFixed();
		break;
	}
	/// TODO FIND WHY SET SCALE IS DEFAULT IF SCALE CAN BE DISABLED
	m_incrementStrategy->setScale(m_scale->scaleCb()->currentData().toDouble());
}

void TestSpinbox::setValue(double newValue) { setValueForce(newValue, 0); }

void TestSpinbox::setValueForce(double newValue, bool force)
{

	if(qFuzzyCompare(m_value, newValue) || force) {
		if(QString::number(m_value).compare(m_edit->getLineEdit()->text()) != 0)
			updateWidgetsVal();
		return;
	}

	m_value = clamp(newValue, m_min, m_max);
	updateWidgetsVal();
	Q_EMIT valueChanged(m_value);
}

void TestSpinbox::updateWidgetsVal()
{
	// block all signals that affect value changes before updating widgets
	// update values for edittext
	m_edit->getLineEdit()->blockSignals(true);

	if(m_scale->scalingEnabled()) {

		m_scale->scaleCb()->blockSignals(true);
		m_scale->computeScale(m_value);
		double scale = m_scale->scaleCb()->currentData().toDouble();

		// print value based on scale
		m_edit->getLineEdit()->setText(QString::number(m_value / scale));
		setToolTip(QString::number(m_value, 'f', 6)); // set tooltip

		// update scale for increment strategy
		m_incrementStrategy->setScale(scale);
		m_scale->scaleCb()->blockSignals(false);

	} else {
		// when no scaling is enabled we just update value
		m_edit->getLineEdit()->setText(QString::number(m_value));
	}
	m_edit->getLineEdit()->blockSignals(false);
}

#include "moc_testspinbox.cpp"
