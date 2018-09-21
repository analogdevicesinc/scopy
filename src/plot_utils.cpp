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

#include "plot_utils.hpp"
#include <qlocale.h>

#include <cmath>

using namespace adiscope;
using namespace std;

/*
 * PrefixFormatter class implementation
 */

PrefixFormatter::PrefixFormatter(const vector<pair<QString, double>> &prefixes):
	m_prefixes(prefixes),
	m_twoDecimalMode(false)
{
	m_defaultPrefixIndex = find_if( m_prefixes.begin(), m_prefixes.end(),
				[](const pair<QString, double>& element) {return element.second == 1E0;} ) - m_prefixes.begin();
}

void PrefixFormatter::setTwoDecimalMode(bool enable)
{
	m_twoDecimalMode = enable;
}

bool PrefixFormatter::getTwoDecimalMode()
{
	return m_twoDecimalMode;
}

QString PrefixFormatter::buildString(double value, QString prefix, QString unitType, int precision) const
{
	return QLocale().toString(value, 'f', precision) + " " + prefix + unitType;
}

int PrefixFormatter::findPrefixIndex(double value) const
{
	int index = m_defaultPrefixIndex;

	value = fabs(value);
	for (auto it = m_prefixes.rbegin(); it != m_prefixes.rend(); ++it) {
		if (value >= (*it).second) {
			index = m_prefixes.rend() - it - 1;
			if (m_twoDecimalMode)
				if (value / (*it).second >= 100 && it != m_prefixes.rbegin())
					index++;
			break;
		}
	}

	return index;
}

QString PrefixFormatter::format(double value, QString unitType = "", int precision = 0) const
{
	int index = findPrefixIndex(value);

	return buildString(value / m_prefixes[index].second, m_prefixes[index].first, unitType, precision);
}

void PrefixFormatter::getFormatAttributes(double value, QString& prefix, double& scale) const
{
	int index = findPrefixIndex(value);

	prefix = m_prefixes[index].first;
	scale = m_prefixes[index].second;
}

/*
 * MetricPrefixFormatter class implementation
 */
MetricPrefixFormatter::MetricPrefixFormatter():
	PrefixFormatter({
					{"p", 1E-12},
					{"n", 1E-9},
					{"μ", 1E-6},
					{"m", 1E-3},
					{"", 1E0},
					{"k", 1E3},
					{"M", 1E6},
					{"G", 1E9},
					{"T", 1E12} }
				)
{
}

/*
 * TimePrefixFormatter class implementation
 */
TimePrefixFormatter::TimePrefixFormatter():
	PrefixFormatter({
					{"ps", 1E-12},
					{"ns", 1E-9},
					{"μs", 1E-6},
					{"ms", 1E-3},
					{"s", 1E0},
					{"min", 60},
					{"hr", 60 * 60},
					{"days", 24 * 60 * 60} }
				)
{
}


/*
 * NumberSeries class implementation
 */

NumberSeries::NumberSeries(double lower, double upper, unsigned int powerStep,
			   const std::vector<double>& steps):
	m_lowerLimit(qAbs(lower)),
	m_upperLimit(qAbs(upper)),
	m_powerStep(powerStep),
	m_templateSteps(steps)
{
	// Avoid infinite loop
	if (powerStep < 2)
		m_powerStep = 2;
	if (lower == 0)
		m_lowerLimit = m_upperLimit;
	if (lower > upper)
		qSwap(lower, upper);

	buildNumberSeries();
}

NumberSeries::~NumberSeries()
{

}

const std::vector<double>& NumberSeries::getNumbers()
{
	return m_numbers;
}

double NumberSeries::getNumberBefore(double value)
{
	auto numberIt = std::lower_bound(m_numbers.begin(), m_numbers.end(), value);

	if (numberIt != m_numbers.begin())
		numberIt--;

	return (*numberIt);
}

double NumberSeries::getNumberAfter(double value)
{
	auto numberIt = std::upper_bound(m_numbers.begin(), m_numbers.end(), value);

	if (numberIt == m_numbers.end())
		numberIt--;

	return (*numberIt);
}

void NumberSeries::setLower(double value)
{
	if ((value != m_lowerLimit) && (value != 0)) {
		m_lowerLimit = qAbs(value);
		buildNumberSeries();
	}
}
double NumberSeries::lower()
{
	return m_lowerLimit;
}

void NumberSeries::setUpper(double value)
{
	if (value != m_upperLimit) {
		m_upperLimit = qAbs(value);
		buildNumberSeries();
	}
}

double NumberSeries::upper()
{
	return m_upperLimit;
}

void NumberSeries::setPowerStep(unsigned int value)
{
	if (value != m_powerStep) {
		m_powerStep = value;
		buildNumberSeries();
	}
}
unsigned int NumberSeries::stepPower()
{
	return m_powerStep;
}

void NumberSeries::buildNumberSeries()
{
	m_numbers.clear();

	for (double i = m_lowerLimit; i <= m_upperLimit; i *= m_powerStep)
		for (size_t step = 0; step < m_templateSteps.size(); step++) {
			double number = i * m_templateSteps[step];
			if (number > m_upperLimit)
				break;
			m_numbers.push_back(i * m_templateSteps[step]);
		}
}
