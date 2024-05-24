#include "basicscaledraw.h"
#include "qlocale.h"

#include <QPainter>
#include <QRect>
#include <qwt_interval.h>
#include <cmath>
#include <QwtText>

using namespace scopy;

BasicScaleDraw::BasicScaleDraw(const QString &unit)
	: QwtScaleDraw()
	, m_floatPrecision(3)
	, m_unit(unit)
	, m_formatter(NULL)
	, m_color(Qt::gray)
	, m_displayScale(1)
	, m_shouldDrawMiddleDelta(false)
	, m_nrTicks(0)
	, m_delta(false)
{
	enableComponent(QwtAbstractScaleDraw::Backbone, false);
	enableComponent(QwtAbstractScaleDraw::Ticks, false);
}

BasicScaleDraw::BasicScaleDraw(PrefixFormatter *formatter, const QString &unit)
	: BasicScaleDraw(unit)
{
	m_formatter = formatter;
}

void BasicScaleDraw::setFloatPrecision(unsigned int numDigits) { m_floatPrecision = numDigits; }

unsigned int BasicScaleDraw::getFloatPrecison() const { return m_floatPrecision; }

void BasicScaleDraw::setUnitType(const QString &unit)
{
	if(m_unit != unit) {
		m_unit = unit;

		// Trigger a new redraw of scale labels since there's a new unit that needs to be redrawn
		invalidateCache();
	}
}

QString BasicScaleDraw::getUnitType() const { return m_unit; }

void BasicScaleDraw::setColor(QColor color) { m_color = color; }

void BasicScaleDraw::setDisplayScale(double value) { m_displayScale = value; }

void BasicScaleDraw::setFormatter(PrefixFormatter *formatter) { m_formatter = formatter; }

void BasicScaleDraw::enableDeltaLabel(bool enable)
{
	if(enable != m_delta) {
		m_delta = enable;

		// Trigger a new redraw of the scale
		invalidateCache();
	}
}

void BasicScaleDraw::setUnitsEnabled(bool enable) { m_unitsEn = enable; }

void BasicScaleDraw::draw(QPainter *painter, const QPalette &palette) const
{
	int nrMajorTicks = scaleDiv().ticks(QwtScaleDiv::MajorTick).size();

	m_nrTicks = nrMajorTicks;

	QList<double> ticks = scaleDiv().ticks(QwtScaleDiv::MajorTick);
	QList<QRect> labels;

	for(int i = 0; i < ticks.size(); ++i) {
		QRect bounds = boundingLabelRect(painter->font(), ticks[i]);
		int half = painter->font().pointSize() / 4;

		if(orientation() == Qt::Horizontal)
			bounds.adjust(-half, 0, half, 0);
		else
			bounds.adjust(0, -half / 2, 0, half / 2);

		labels.append(bounds);
	}

	bool overlap = false;

	int midLabelPos = nrMajorTicks / 2;

	do {
		overlap = false;
		for(int i = 1; i < labels.size(); ++i) {
			QRect last_rectangle = labels.at(i - 1);
			QRect current_rectangle = labels.at(i);

			if(current_rectangle.intersects(last_rectangle)) {
				overlap = true;
				break;
			}
		}

		if(overlap) {
			if(m_delta) {
				// If the middle delta label is to be drawn we are sure that
				// ticks.size() is an odd number
				int center = midLabelPos;
				for(int i = center - 1; i >= 0; i -= 2) {
					// Remove the tick and make sure to update the center
					// label position
					ticks.removeAt(i);
					labels.removeAt(i);
					--center;
				}
				for(int j = center + 1; j < ticks.size(); j += 1) {
					ticks.removeAt(j);
					labels.removeAt(j);
				}
			} else {
				for(int i = 1; i < ticks.size(); ++i) {
					ticks.removeAt(i);
					labels.removeAt(i);
				}
			}
		}

	} while(overlap);

	double delta = -INFINITY;

	if(m_delta && m_nrTicks > midLabelPos) {
		delta = scaleDiv().ticks(QwtScaleDiv::MajorTick)[midLabelPos];
		drawLabel(painter, delta);
	}

	for(const auto &tick : qAsConst(ticks)) {
		if(tick != delta) {
			drawLabel(painter, tick);
		}
	}
}

QwtText BasicScaleDraw::label(double value) const
{
	QString prefix;
	double scale = 1.0;
	QString sign = "";
	int bonusPrecision = 0;
	bool center = false;

	double lower = scaleDiv().interval().minValue();
	double upper = scaleDiv().interval().maxValue();
	double diff = upper - lower;
	double step = diff / (m_nrTicks ? (m_nrTicks - 1) : 1);

	int mid = (m_nrTicks / 2 + 1);

	if(m_delta) {
		int current = 0;
		while(value > (lower + current * step))
			current++;
		int position = current + 1;

		if(position == mid) {
			// center label with extra precision
			center = true;
			bonusPrecision = 1;
		} else if(position < mid) {
			sign = "-";
			// negative delta label
			value = step * (mid - position);
		} else if(position > mid) {
			sign = "+";
			// positive delta label
			value = step * (position - mid);
		}
	}

	value *= m_displayScale;

	QwtText text;
	if(m_unitsEn) {
		if(m_formatter) {
			m_formatter->getFormatAttributes(value, prefix, scale);
		}

		if(orientation() == Qt::Vertical) {
			double absVal = value > 0 ? value : -value;
			if(absVal > 1e-2 && prefix == "m") {
				scale = 1.0;
				prefix = "";
			} else if(absVal > 1e-5 && prefix == "μ") {
				scale = 1e-3;
				prefix = "m";
			} else if(absVal > 1e-8 && prefix == "n") {
				scale = 1e-6;
				prefix = "μ";
			} else if(absVal > 1e-11 && prefix == "p") {
				scale = 1e-9;
				prefix = "n";
			}
		}

		text = QwtText(sign + QLocale().toString(value / scale, 'f', m_floatPrecision + bonusPrecision) + ' ' +
			       prefix + m_unit);
	} else {
		text = QwtText(sign + QLocale().toString(value / scale, 'f', m_floatPrecision + bonusPrecision));
	}

	if(m_color != Qt::gray)
		text.setColor(m_color);
	if(center) {
		text.setColor(QColor(255, 255, 255));
	}

	return text;
}
