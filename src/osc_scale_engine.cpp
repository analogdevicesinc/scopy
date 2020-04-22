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

#include "osc_scale_engine.h"

#include "qwt_math.h"

#include <limits>

static inline long double qwtIntervalWidthL(const QwtInterval& interval)
{
	if (!interval.isValid())
		return 0.0;

	return static_cast<long double>(interval.maxValue()) - static_cast<long double>(interval.minValue());
}

OscScaleEngine::OscScaleEngine()
	: QwtLinearScaleEngine(10)
	, m_majorTicks(11)
	, m_minorTicks(9)
	, m_showZero(false)
{}

OscScaleEngine::~OscScaleEngine() {}

void OscScaleEngine::setMajorTicksCount(uint majorTicks) { m_majorTicks = majorTicks; }

uint OscScaleEngine::majorTicksCount() { return m_majorTicks; }

void OscScaleEngine::setMinorTicksCount(uint minorTicks) { m_minorTicks = minorTicks; }

uint OscScaleEngine::minorTicksCount() { return m_minorTicks; }
void OscScaleEngine::showZero(bool en) { m_showZero = en; }
void OscScaleEngine::autoScale(int maxSteps, double& x1, double& x2,
			       double& stepSize) const {// No implementation is needed for now
							Q_UNUSED(maxSteps) Q_UNUSED(x1) Q_UNUSED(x2) Q_UNUSED(stepSize)}

QwtScaleDiv OscScaleEngine::divideScale(double x1, double x2, int maxMajorSteps, int maxMinorSteps,
					double stepSize) const
{
	Q_UNUSED(maxMajorSteps);

	QwtInterval interval = QwtInterval(x1, x2).normalized();

	if (qwtIntervalWidthL(interval) > std::numeric_limits<double>::max()) {
		qWarning() << "QwtLinearScaleEngine::divideScale: overflow";
		return QwtScaleDiv();
	}

	if (interval.width() <= 0)
		return QwtScaleDiv();

	stepSize = interval.width() / (double)(m_majorTicks - 1);

	QwtScaleDiv scaleDiv;

	QList<double> ticks[QwtScaleDiv::NTickTypes];
	buildTicks(interval, stepSize, maxMinorSteps, ticks);

	scaleDiv = QwtScaleDiv(interval, ticks);

	if (x1 > x2)
		scaleDiv.invert();

	return scaleDiv;
}

void OscScaleEngine::buildTicks(const QwtInterval& interval, double stepSize, int maxMinorSteps,
				QList<double> ticks[QwtScaleDiv::NTickTypes]) const
{
	ticks[QwtScaleDiv::MajorTick] = buildMajorTicks(interval, stepSize);

	if (maxMinorSteps > 0) {
		buildMinorTicks(ticks[QwtScaleDiv::MajorTick], maxMinorSteps, stepSize, ticks[QwtScaleDiv::MinorTick],
				ticks[QwtScaleDiv::MediumTick]);
	}

	for (int i = 0; i < QwtScaleDiv::NTickTypes; i++) {
		ticks[i] = strip(ticks[i], interval);

		// ticks very close to 0.0 are
		// explicitely set to 0.0

		for (int j = 0; j < ticks[i].count(); j++) {
			if (qwtFuzzyCompare(ticks[i][j], 0.0, stepSize) == 0)
				ticks[i][j] = 0.0;
		}
	}
}

void OscScaleEngine::buildMinorTicks(const QList<double>& majorTicks, int maxMinorSteps, double stepSize,
				     QList<double>& minorTicks, QList<double>& mediumTicks) const
{
	Q_UNUSED(maxMinorSteps)

	int num_steps = m_minorTicks + 1;
	double minStep = stepSize / num_steps;

	// # ticks per interval
	const int numTicks = m_minorTicks;

	int medIndex = -1;
	if (numTicks % 2)
		medIndex = numTicks / 2;

	// calculate minor ticks

	int majorTicksCnt = majorTicks.count();

	if (m_showZero) {
		majorTicksCnt--;
	}

	for (int i = 0; i < majorTicksCnt; i++) {
		double val = majorTicks[i];

		if (m_showZero) {
			double upper = majorTicks[i + 1];
			double diff = upper - val;

			minStep = diff / numTicks;
		}

		for (int k = 0; k < numTicks; k++) {
			val += minStep;

			double alignedValue = val;
			if (qwtFuzzyCompare(val, 0.0, stepSize) == 0)
				alignedValue = 0.0;

			if (k == medIndex)
				mediumTicks += alignedValue;
			else
				minorTicks += alignedValue;
		}
	}
}

QList<double> OscScaleEngine::buildMajorTicks(const QwtInterval& interval, double stepSize) const
{
	if (m_showZero && interval.minValue() < 0 && interval.maxValue() > 0) {
		QList<double> majorTicks;

		double range = interval.maxValue() - interval.minValue();
		double upperRange = interval.maxValue() / range;
		double lowerRange = fabs(interval.minValue()) / range;

		int underTicks = round(lowerRange * (m_majorTicks - 1));
		int overTicks = round(upperRange * (m_majorTicks - 1));

		if (overTicks == 0) {
			overTicks++;
			underTicks--;
		}
		if (underTicks == 0) {
			underTicks++;
			overTicks--;
		}
		if (overTicks + underTicks == m_majorTicks) {
			if (underTicks > overTicks)
				underTicks--;
			else
				overTicks--;
		}

		double overStep = interval.maxValue() / overTicks;
		double underStep = fabs(interval.minValue()) / underTicks;

		double val = interval.maxValue();
		for (int i = 0; i < overTicks; i++) {
			majorTicks.append(val);
			val -= overStep;
		}
		for (int i = 0; i < underTicks + 1; i++) {
			majorTicks.append(val);
			val -= underStep;
		}

		return majorTicks;
	}

	return QwtLinearScaleEngine::buildMajorTicks(interval, stepSize);
}
