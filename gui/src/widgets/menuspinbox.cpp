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

#include "menuspinbox.h"
#include <style.h>
#include <stylehelper.h>

namespace scopy {
namespace gui {

MenuSpinbox::MenuSpinbox(QString name, double val, QString unit, double min, double max, bool vertical, bool left,
			 bool large_widget, QWidget *parent)
	: m_name(name)
	, m_min(min)
	, m_max(max)
	, QWidget(parent)
{
	m_large_widget = large_widget;
	m_label = new QLabel(name, parent);
	Style::setStyle(m_label, style::properties::label::subtle);

	m_edit = new QLineEdit("0", parent);
	m_plus = new QPushButton("", parent);
	m_minus = new QPushButton("", parent);
	m_mouseWheelGuard = new MouseWheelWidgetGuard(this);

	////SCALE
	m_scale = new Scale(unit, min, max);

	m_incrementStrategy = new IncrementStrategyFixed();

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
	connect(m_edit, &QLineEdit::editingFinished, this, [=]() {
		m_edit->blockSignals(true);
		userInput(m_edit->text());
		m_edit->blockSignals(false);
	});

	connect(m_scale->scaleCb(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=]() {
		userInput(m_edit->text());
		// if the value is 0 we still need to consider scale options
		if(m_value == 0) {
			m_incrementStrategy->setScale(m_scale->scaleCb()->currentData().toDouble());
		}
	});

	connect(m_scale, &Scale::unitChanged, this, &MenuSpinbox::unitChanged);
	connect(m_scale, &Scale::scaleUpdated, this, &MenuSpinbox::populateWidgets);

	m_mouseWheelGuard->installEventRecursively(this);
}

MenuSpinbox::~MenuSpinbox() { delete m_incrementStrategy; }

void MenuSpinbox::layoutVertically(bool left)
{
	auto lay = new QHBoxLayout(this);
	setLayout(lay);

	lay->setSpacing(0);
	lay->setMargin(0);

	QLayout *btnLay;
	QLayout *editLay;

	btnLay = new QVBoxLayout();
	editLay = new QVBoxLayout();

	btnLay->setSpacing(2);
	btnLay->setContentsMargins(5, 0, 0, 0);
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
	Style::setStyle(m_scale->scaleCb(), style::properties::widget::noBorder);
	int size = m_large_widget ? Style::getDimension(json::global::unit_2_5)
				  : Style::getDimension(json::global::unit_1);

	m_plus->setIcon(Style::getPixmap(":/gui/icons/plus.svg", Style::getColor(json::theme::content_inverse)));
	Style::setStyle(m_plus, style::properties::button::spinboxButton);
	m_plus->setFixedSize(size, size);

	m_minus->setIcon(Style::getPixmap(":/gui/icons/minus.svg", Style::getColor(json::theme::content_inverse)));
	Style::setStyle(m_minus, style::properties::button::spinboxButton);
	m_minus->setFixedSize(size, size);

	if(!m_large_widget) {
		int icon_size = Style::getDimension(json::global::unit_0_5);
		m_plus->setIconSize(QSize(icon_size, icon_size));
		m_minus->setIconSize(QSize(icon_size, icon_size));
	}
}

void MenuSpinbox::layoutHorizontally(bool left)
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

double MenuSpinbox::value() const { return m_value; }

void MenuSpinbox::setValue(double newValue) { setValueForce(newValue, 0); }

void MenuSpinbox::setValueForce(double newValue, bool force)
{
	// when force is true value does not consider min/max limits
	if(force) {
		m_value = newValue;
		populateWidgets();
		Q_EMIT valueChanged(m_value);
		return;
	}

	if(qFuzzyCompare(m_value, newValue)) {
		// check if text in edit changed even if value does not
		if(QString::number(m_value).compare(m_edit->text()) != 0)
			populateWidgets(); // reset value
		return;
	}

	m_value = clamp(newValue, m_min, m_max);
	populateWidgets();
	Q_EMIT valueChanged(m_value);
}

void MenuSpinbox::setValueString(QString s) { userInput(s); }

QString MenuSpinbox::unit() const { return m_scale->unit(); }

void MenuSpinbox::setUnit(const QString &newUnit) { m_scale->setUnit(newUnit); }

void MenuSpinbox::setMinValue(double min) { m_min = min; }

void MenuSpinbox::setMaxValue(double max) { m_max = max; }

IncrementStrategy *MenuSpinbox::incrementStrategy() const { return m_incrementStrategy; }

void MenuSpinbox::setIncrementMode(IncrementMode im)
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

	// when scalling is not enbaled scale will be set to 1
	if(scallingEnabled()) {
		m_incrementStrategy->setScale(m_scale->scaleCb()->currentData().toDouble());
	} else {
		m_incrementStrategy->setScale(1);
	}
}

void MenuSpinbox::setScalingEnabled(bool en)
{
	m_scale->setScalingEnabled(en);
	if(en) {
		m_incrementStrategy->setScale(m_scale->scaleCb()->currentData().toDouble());
	} else {
		m_incrementStrategy->setScale(1);
	}
}

bool MenuSpinbox::scallingEnabled() { return m_scale->scalingEnabled(); }

void MenuSpinbox::userInput(QString s)
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
		populateWidgets(); // reset

	if(m_scale->scalingEnabled()) {
		QString unit = s.mid(i + 1, s.length() - 1); // isolate unit from the whole string (min)
		if(unit.length() > 0) {			     // user wrote a unit
			double scaleValue = m_scale->getScaleForSymbol(unit); // find the unit in the map
			if(scaleValue == -1) {
				populateWidgets(); // inputed prefix is invalid
			} else {
				val = val * scaleValue; // scale accordingly
			}

		} else {
			// Apply current scale to value
			double scaleValue = m_scale->scaleCb()->currentData().toDouble();
			val = val * scaleValue;
		}
	}

	setValue(val);
}

void MenuSpinbox::populateWidgets()
{
	// block all signals that affect value changes before updating widgets
	// update values for edittext
	m_edit->blockSignals(true);

	if(m_scale->scalingEnabled() && m_value != 0) {

		m_scale->scaleCb()->blockSignals(true);
		m_scale->computeScale(m_value);
		double scale = m_scale->scaleCb()->currentData().toDouble();

		// print value based on scale
		m_edit->setText(QString::number(m_value / scale, 'f', m_precision));
		setToolTip(QString::number(m_value, 'f', 6)); // set tooltip

		// update scale for increment strategy
		m_incrementStrategy->setScale(scale);
		m_scale->scaleCb()->blockSignals(false);

	} else {
		// when no scaling is enabled we just update value
		m_edit->setText(QString::number(m_value, 'f', m_precision));
	}
	m_edit->blockSignals(false);

	setToolTip(QString::number(m_value, 'f', 6)); // set tooltip
}

int MenuSpinbox::findLastDigit(QString str)
{
	for(int i = str.length() - 1; i >= 0; --i) {
		if(str[i].isDigit()) {
			return i;
		}
	}
	return -1; // Return -1 if no digit is found
}

double MenuSpinbox::clamp(double val, double min, double max)
{
	val = std::max(val, min);
	val = std::min(val, max);
	minMaxReached(val);
	return val;
}

void MenuSpinbox::minMaxReached(double val)
{
	// disable decrement button if min is reached
	if(val == m_min) {
		m_minus->setEnabled(false);
	} else {
		m_minus->setEnabled(true);
	}

	// disable increment button if max is reached
	if(val == m_max) {
		m_plus->setEnabled(false);
	} else {
		m_plus->setEnabled(true);
	}
}

int MenuSpinbox::precision() const { return m_precision; }

void MenuSpinbox::setPrecision(int newPrecision) { m_precision = newPrecision; }

Scale *MenuSpinbox::scale() const { return m_scale; }

void MenuSpinbox::setScale(Scale *newScale) { m_scale = newScale; }

QString MenuSpinbox::name() const { return m_name; }

void MenuSpinbox::setName(const QString &newName)
{
	if(m_name == newName)
		return;
	m_name = newName;
	m_label->setText(m_name);
	Q_EMIT nameChanged(newName);
}

} // namespace gui
} // namespace scopy

#include "moc_menuspinbox.cpp"
