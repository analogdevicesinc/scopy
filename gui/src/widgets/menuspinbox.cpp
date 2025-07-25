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
	: m_min(min)
	, m_max(max)
	, QWidget(parent)
{
	m_large_widget = large_widget;
	m_label = new QLabel(name, parent);
	Style::setStyle(m_label, style::properties::label::subtle);

	m_infoIcon = new InfoIconWidget("", parent);

	m_edit = new QLineEdit("0", parent);
	m_scaleCb = new QComboBox(parent);
	m_plus = new QPushButton("", parent);
	m_minus = new QPushButton("", parent);
	m_mouseWheelGuard = new MouseWheelWidgetGuard(this);

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

	m_incrementStrategy = new IncrementStrategyPower2();

	connect(m_plus, &QAbstractButton::clicked, this, &MenuSpinbox::incrementValue);
	connect(m_minus, &QAbstractButton::clicked, this, &MenuSpinbox::decrementValue);

	connect(m_edit, &QLineEdit::editingFinished, this, [=]() { userInput(m_edit->text()); });

	connect(m_scaleCb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		m_incrementStrategy->setScale(m_scaleCb->itemData(idx).toDouble());
		userInput(m_edit->text());
	});

	m_scales.append({QString("n"), 1e-9});
	m_scales.append({QString("u"), 1e-6});
	m_scales.append({QString("m"), 1e-3});
	m_scales.append({QString(""), 1e0});
	m_scales.append({QString("k"), 1e3});
	m_scales.append({QString("M"), 1e6});
	m_scales.append({QString("G"), 1e9});

	m_name = name;
	m_unit = unit;
	m_min = min;
	m_max = max;
	m_scaleMin = min;
	m_scaleMax = max;
	setScaleRange(m_scaleMin, m_scaleMax);
	setValue(val);
	m_scalingEnabled = true;
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

	QHBoxLayout *labelLayout = new QHBoxLayout();
	labelLayout->setContentsMargins(0, 0, 0, 0);
	labelLayout->setSpacing(2);
	labelLayout->addWidget(m_label);
	labelLayout->addWidget(m_infoIcon);
	labelLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

	editLay->addItem(labelLayout);
	editLay->addWidget(m_edit);

	editLay->addWidget(m_scaleCb);

	if(left) {
		lay->addLayout(btnLay);
		lay->addLayout(editLay);
	} else {
		lay->addLayout(editLay);
		lay->addLayout(btnLay);
	}

	Style::setStyle(m_scaleCb, style::properties::widget::noBorder);
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
	editLay->addWidget(m_infoIcon);
	editLay->addWidget(m_edit);

	editLay->addWidget(m_scaleCb);
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
	if(qFuzzyCompare(m_value, newValue) && !force) {
		// check if text in edit changed even if value does not
		if(QString::number(m_value).compare(m_edit->text()) != 0)
			populateWidgets(); // reset to last valid value
		return;
	}

	m_value = clamp(newValue, m_min, m_max);
	populateWidgets();
	Q_EMIT valueChanged(m_value);
}

void MenuSpinbox::setValueString(QString s) { userInput(s); }

QString MenuSpinbox::unit() const { return m_unit; }

void MenuSpinbox::setUnit(const QString &newUnit)
{
	if(m_unit == newUnit)
		return;
	m_unit = newUnit;
	setScaleRange(m_scaleMin, m_scaleMax);
	Q_EMIT unitChanged(newUnit);
}

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
	m_incrementStrategy->setScale(m_scaleCb->currentData().toDouble());
}

void MenuSpinbox::setScalingEnabled(bool en)
{
	m_scalingEnabled = en;
	m_scaleCb->setVisible(en);
}

void MenuSpinbox::setScaleList(QList<UnitPrefix> scales)
{
	m_scales.clear();
	m_scales = scales;
	setScaleRange(m_min, m_max);
}

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
	if(!ok) {
		setValueForce(m_value, true); // reset
		return;
	}

	if(m_scalingEnabled) {
		QString unit = s.mid(i + 1, 1); // isolate prefix and unit from the whole string (mV)
		if(unit.length() > 0) {		// user wrote a prefix and/or a unit
			double scale = getScaleForPrefix(unit, Qt::CaseSensitive); // find the unit in the map
			if(scale == -1) {
				scale = getScaleForPrefix(
					unit,
					Qt::CaseInsensitive); // the user may have written 30K instead of 30k
			}

			if(scale == -1) {
				scale = 1; // do not scale the value at all
			} else {
				val = val * scale; // scale accordingly
			}
		} else {
			val = val *
				m_scaleCb->currentData()
					.toDouble(); // the user didnt write a scale => use scale in combobox
		}
	}
	setValue(val);
}

void MenuSpinbox::populateWidgets()
{
	QSignalBlocker sb1(m_edit);
	QSignalBlocker sb2(m_scaleCb);

	// TODO: Review this function
	if(!m_scalingEnabled) {
		m_edit->setText(Util::doubleToQString(m_value));
		setToolTip(QString::number(m_value, 'f', 6)); // set tooltip
		return;
	}

	int i = 0;
	double scale = 1;
	double absvalue = abs(m_value);
	if(qFuzzyCompare(absvalue, 0)) {
		i = m_scaleCb->currentIndex();
		scale = m_scaleCb->itemData(i).toDouble();

	} else {
		for(i = m_scaleCb->count() - 1; i >= 0; i--) { // find most suitable scale
			scale = m_scaleCb->itemData(i).toDouble();
			if(absvalue / scale >= 1)
				break;
		}
		if(i < 0) {
			i = 0;
			scale = m_scaleCb->itemData(i).toDouble();
		}
	}

	// only change  the scale for non 0 values
	if(!qFuzzyCompare(m_value, 0.0)) {
		m_edit->setText(QString::number(m_value / scale)); // reduce number to a meaningful value
		m_scaleCb->setCurrentIndex(i);			   // set appropriate  scale in combobox
		m_incrementStrategy->setScale(m_scaleCb->currentData().toDouble());
	} else {
		m_edit->setText(QString::number(m_value));
	}

	setToolTip(QString::number(m_value, 'f', 6)); // set tooltip
}

void MenuSpinbox::setScaleRange(double min, double max)
{
	m_scaleCb->clear();
	for(int i = 0; i < m_scales.count(); i++) {
		auto scale = m_scales[i].scale;
		if(scale >= min && scale <= max) {
			m_scaleCb->addItem(m_scales[i].prefix + m_unit, scale);
		}
	}
	m_incrementStrategy->setScale(m_scaleCb->currentData().toDouble());
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

	// disable decrement button if min is reached
	if((val == m_min) && (m_scaleCb->currentIndex() == 0)) {
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

	return val;
}

void MenuSpinbox::incrementValue()
{
	if(qFuzzyCompare(m_value, 0.0)) {
		// On increment from zero, set to 1 * current scale
		double scale = m_scaleCb->itemData(m_scaleCb->currentIndex()).toDouble();
		setValue(1 * scale);
	} else {
		setValue(m_incrementStrategy->increment(m_value));
	}
}

void MenuSpinbox::decrementValue()
{
	if(qFuzzyCompare(m_value, 0.0)) {
		int idx = m_scaleCb->currentIndex();
		if(idx > 0) {
			// Go to max of lower scale - 1
			double lowerScale = m_scaleCb->itemData(idx - 1).toDouble();
			double currentScale = m_scaleCb->itemData(idx).toDouble();
			double maxLower = (currentScale / lowerScale) - 1;
			setValue(maxLower * lowerScale);
		} else {
			// Already at lowest scale, go to -1 * current scale
			double scale = m_scaleCb->itemData(idx).toDouble();
			setValue(-1 * scale);
		}
	} else {
		double newValue = m_incrementStrategy->decrement(m_value);
		// If decrement would result in 0 and there is a lower scale, scale down instead
		if(qFuzzyCompare(newValue, 0.0)) {
			int idx = m_scaleCb->currentIndex();
			if(idx > 0) {
				double lowerScale = m_scaleCb->itemData(idx - 1).toDouble();
				double currentScale = m_scaleCb->itemData(idx).toDouble();
				double maxLower = (currentScale / lowerScale) - 1;
				setValue(maxLower * lowerScale);
				return;
			}
		}
		setValue(newValue);
	}
}

QString MenuSpinbox::name() const { return m_name; }

QLabel *MenuSpinbox::label() const { return m_label; }

void MenuSpinbox::setName(const QString &newName)
{
	if(m_name == newName)
		return;
	m_name = newName;
	m_label->setText(m_name);
	Q_EMIT nameChanged(newName);
}

void MenuSpinbox::setInfoMessage(QString infoMessage) { m_infoIcon->setInfoMessage(infoMessage); }

double MenuSpinbox::getScaleForPrefix(QString prefix, Qt::CaseSensitivity s)
{
	for(int i = 0; i < m_scales.count(); i++) {
		if(s == Qt::CaseSensitive) {
			if(m_scales[i].prefix == prefix) {
				return m_scales[i].scale;
			}
		} else {
			if(m_scales[i].prefix.toLower() == prefix.toLower()) {
				return m_scales[i].scale;
			}
		}
	}
	return -1;
}

} // namespace gui
} // namespace scopy

#include "moc_menuspinbox.cpp"
