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
#include <algorithm>
#include <QPainter>
#include <QWheelEvent>
#include <QtMath>

namespace scopy {
namespace gui {

MenuSpinboxCircle::MenuSpinboxCircle(QWidget *parent)
	: QAbstractButton(parent)
{
	setCheckable(true);
	setFocusPolicy(Qt::NoFocus);
	setCursor(Qt::PointingHandCursor);
	setToolTip(tr("Click to toggle fine adjustment"));
}

void MenuSpinboxCircle::setRatio(double ratio)
{
	m_ratio = ratio;
	update();
}

void MenuSpinboxCircle::paintEvent(QPaintEvent *)
{
	const double penWidth = 3;
	const double d = qMin(width(), height()) - penWidth;
	const QRectF ring((width() - d) / 2, (height() - d) / 2, d, d);
	const QColor rail = Style::getColor(json::theme::interactive_subtle_idle);
	const QColor fill = Style::getColor(isEnabled() ? json::theme::interactive_primary_idle
							: json::theme::interactive_primary_disabled);

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(QPen(rail, penWidth));
	p.drawEllipse(ring);

	// clockwise from 12 o'clock
	p.setPen(QPen(fill, penWidth, Qt::SolidLine, Qt::FlatCap));
	p.drawArc(ring, 90 * 16, qRound(-m_ratio * 360 * 16));

	// Tick inside the ring at the arc's end
	const double angle = qDegreesToRadians(90 - m_ratio * 360);
	const QPointF dir(std::cos(angle), -std::sin(angle));
	const double r = d / 2;
	p.setPen(QPen(Style::getColor(isEnabled() ? json::theme::interactive_accent_idle : json::theme::content_silent),
		      2, Qt::SolidLine, Qt::RoundCap));
	p.drawLine(ring.center() + dir * r * 0.5, ring.center() + dir * r * 0.75);

	const double dot = d / 8;
	p.setPen(Qt::NoPen);
	p.setBrush(isChecked() ? fill : rail);
	p.drawEllipse(ring.center(), dot, dot);
}

void MenuSpinboxCircle::wheelEvent(QWheelEvent *e)
{
	// Accumulate so a high resolution touchpad steps once per notch, not per event
	m_wheelDelta += e->angleDelta().y();
	const int steps = m_wheelDelta / QWheelEvent::DefaultDeltasPerStep;
	m_wheelDelta -= steps * QWheelEvent::DefaultDeltasPerStep;
	for(int i = 0; i < qAbs(steps); i++) {
		Q_EMIT stepped(steps > 0);
	}
	e->accept();
}

MenuSpinbox::MenuSpinbox(QString name, double val, QString unit, double min, double max, bool vertical, bool left,
			 bool large_widget, QWidget *parent)
	: m_min(min)
	, m_max(max)
	, m_rangeLimits(true)
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
	m_circle = new MenuSpinboxCircle(this);
	m_circle->setVisible(false);
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
	connect(m_circle, &MenuSpinboxCircle::stepped, this,
		[this](bool up) { up ? incrementValue() : decrementValue(); });

	connect(m_edit, &QLineEdit::editingFinished, this, [this]() { userInput(m_edit->text()); });

	connect(m_scaleCb, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx) {
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
	m_incrementStrategy->setScale(stepScale());
	m_mouseWheelGuard->installEventRecursively(this);
}

MenuSpinbox::~MenuSpinbox() { delete m_incrementStrategy; }

void MenuSpinbox::layoutVertically(bool left)
{
	auto lay = new QHBoxLayout(this);
	setLayout(lay);

	lay->setSpacing(0);
	lay->setContentsMargins(0, 0, 0, 0);

	QLayout *btnLay;
	QLayout *editLay;

	btnLay = new QVBoxLayout();
	editLay = new QVBoxLayout();

	btnLay->setSpacing(2);
	btnLay->setContentsMargins(5, 0, 0, 0);
	editLay->setSpacing(2);
	editLay->setContentsMargins(0, 0, 0, 0);

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

	auto circleLay = new QVBoxLayout();
	circleLay->setContentsMargins(left ? 0 : 5, 0, left ? 5 : 0, 0);
	circleLay->addWidget(m_circle, 0, Qt::AlignVCenter);

	if(left) {
		lay->addLayout(circleLay);
		lay->addLayout(btnLay);
		lay->addLayout(editLay);
	} else {
		lay->addLayout(editLay);
		lay->addLayout(btnLay);
		lay->addLayout(circleLay);
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

	int circleSize =
		m_large_widget ? Style::getDimension(json::global::unit_4) : Style::getDimension(json::global::unit_2);
	m_circle->setFixedSize(circleSize, circleSize);

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
	lineLay->setContentsMargins(0, 0, 0, 0);
	setLayout(lineLay);

	// Elements layout
	auto lay = new QHBoxLayout(this);
	lay->setSpacing(5);
	lay->setContentsMargins(0, 0, 0, 0);

	QLayout *btnLay;
	QLayout *editLay;

	btnLay = new QHBoxLayout();
	editLay = new QHBoxLayout();

	btnLay->setSpacing(2);
	btnLay->setContentsMargins(0, 0, 0, 0);
	editLay->setSpacing(8);
	editLay->setContentsMargins(0, 0, 5, 0);

	if(left) {
		btnLay->addWidget(m_circle);
	}
	btnLay->addWidget(m_minus);
	btnLay->addWidget(m_plus);
	if(!left) {
		btnLay->addWidget(m_circle);
	}

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
	m_circle->setFixedSize(size, size);
}

double MenuSpinbox::value() const { return m_value; }

void MenuSpinbox::setValue(double newValue) { setValueForce(newValue, 0); }

void MenuSpinbox::setValueSilent(double newValue)
{
	m_value = clamp(newValue, m_min, m_max);
	populateWidgets();
	updateCircle();
}

void MenuSpinbox::setValueForce(double newValue, bool force)
{
	if(qFuzzyCompare(m_value, newValue) && !force) {
		// check if text in edit changed even if value does not
		if(QString::number(m_value).compare(m_edit->text()) != 0)
			populateWidgets(); // reset to last valid value
		return;
	}

	setValueSilent(newValue);
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

void MenuSpinbox::setMinValue(double min)
{
	m_min = min;
	updateCircle();
}

void MenuSpinbox::setMaxValue(double max)
{
	m_max = max;
	updateCircle();
}

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
	m_incrementStrategy->setScale(stepScale());
	updateCircle();
}

void MenuSpinbox::setScalingEnabled(bool en)
{
	m_scalingEnabled = en;
	m_scaleCb->setVisible(en);
	m_incrementStrategy->setScale(stepScale());
}

// Without prefixes the value is shown and stepped in plain units
double MenuSpinbox::stepScale() const { return m_scalingEnabled ? m_scaleCb->currentData().toDouble() : 1.0; }

void MenuSpinbox::setScaleList(QList<UnitPrefix> scales)
{
	m_scales.clear();
	m_scales = scales;
	setScaleRange(m_min, m_max);
}

void MenuSpinbox::setCircleEnabled(bool en)
{
	m_circle->setVisible(en);
	if(!en) {
		m_circle->setChecked(false);
	}
}

void MenuSpinbox::setCircleLogFloor(double floor)
{
	m_circleLogFloor = floor;
	updateCircle();
}

void MenuSpinbox::updateCircle()
{
	double ratio = 0.0;
	if(m_im == IS_125) {
		// Log scale spreads the 1-2-5 decades evenly
		const double lo = m_min > 0 ? m_min : m_circleLogFloor;
		if(lo > 0 && m_max > lo) {
			ratio = std::log(std::clamp(m_value, lo, m_max) / lo) / std::log(m_max / lo);
		}
	} else if(m_max > m_min) {
		ratio = std::clamp((m_value - m_min) / (m_max - m_min), 0.0, 1.0);
	}
	m_circle->setRatio(ratio);
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
	m_incrementStrategy->setScale(stepScale());
}

void MenuSpinbox::enableRangeLimits(bool en)
{
	m_rangeLimits = en;

	m_plus->setEnabled(true);
	m_minus->setEnabled(true);
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
	if(!m_rangeLimits) {
		return val;
	}

	val = std::max(val, min);
	val = std::min(val, max);

	// disable decrement button if min is reached and no prefix is left to roll down to
	double rolled = 0.0;
	if((val == m_min) && !prefixRollDown(&rolled)) {
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

double MenuSpinbox::nextValue(bool up) const
{
	if(!m_circle->isChecked()) {
		return up ? m_incrementStrategy->increment(m_value) : m_incrementStrategy->decrement(m_value);
	}
	// Fine mode steps by one unit of the shown prefix, or a tenth of a fixed step
	double step = stepScale();
	if(m_im == IS_FIXED) {
		step *= static_cast<IncrementStrategyFixed *>(m_incrementStrategy)->k() / 10;
	}
	const double val = m_value + (up ? step : -step);
	// Drop the residue of repeated decimal steps so zero lands on zero
	return std::abs(val) < step * 1e-9 ? 0.0 : val;
}

void MenuSpinbox::incrementValue()
{
	if(!m_circle->isChecked() && qFuzzyCompare(m_value, 0.0)) {
		// On increment from zero, set to 1 * current scale
		setValue(stepScale());
	} else {
		setValue(nextValue(true));
	}
}

bool MenuSpinbox::prefixRollDown(double *rolled) const
{
	const int idx = m_scaleCb->currentIndex();
	if(idx <= 0) {
		return false;
	}
	const double lowerScale = m_scaleCb->itemData(idx - 1).toDouble();
	const double currentScale = m_scaleCb->itemData(idx).toDouble();
	if(lowerScale <= 0 || currentScale <= 0) {
		return false;
	}

	const double candidate = ((currentScale / lowerScale) - 1) * lowerScale;
	if(m_rangeLimits && (candidate < m_min || candidate > m_max)) {
		return false;
	}

	*rolled = candidate;
	return true;
}

void MenuSpinbox::decrementValue()
{
	double rolled = 0.0;

	if(qFuzzyCompare(m_value, 0.0)) {
		setValue(prefixRollDown(&rolled) ? rolled : nextValue(false));
		return;
	}

	// If decrement would result in 0 and there is a lower scale, scale down instead
	const double newValue = nextValue(false);
	if(qFuzzyCompare(newValue, 0.0) && prefixRollDown(&rolled)) {
		setValue(rolled);
		return;
	}
	setValue(newValue);
}

QString MenuSpinbox::name() const { return m_name; }

QLabel *MenuSpinbox::label() const { return m_label; }

double MenuSpinbox::min() const { return m_min; }

double MenuSpinbox::max() const { return m_max; }

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
