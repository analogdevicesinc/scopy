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

#include <rollingstrategy.h>

using namespace scopy::pqm;

RollingStrategy::RollingStrategy(int samplingFreq)
	: PlottingStrategy(samplingFreq)
{
	m_dataReady = true;
}

RollingStrategy::~RollingStrategy() {}

QMap<QString, QVector<double>> RollingStrategy::processSamples(QMap<QString, QVector<double>> samples)
{
	const QStringList keys = samples.keys();
	for(const auto &key : keys) {
		m_samples[key].append(samples[key]);
		if(m_samples[key].size() > m_samplingFreq) {
			int unnecessarySamples = m_samples[key].size() - m_samplingFreq;
			m_samples[key].erase(m_samples[key].begin(), m_samples[key].begin() + unnecessarySamples);
		}
	}
	return m_samples;
}
