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

#ifndef PLOTTINGSTRATEGY_H
#define PLOTTINGSTRATEGY_H

#include <QString>
#include <QMap>

namespace scopy::pqm {

class PlottingStrategy
{

public:
	PlottingStrategy(int samplingFreq) { m_samplingFreq = samplingFreq; }
	virtual ~PlottingStrategy() { m_samples.clear(); }

	virtual QMap<QString, QVector<double>> processSamples(QMap<QString, QVector<double>> samples) = 0;
	bool dataReady() const;
	void setSamplingFreq(int newSamplingFreq);
	void clearSamples();

protected:
	int m_samplingFreq;
	bool m_dataReady = false;
	QMap<QString, QVector<double>> m_samples;
};

inline bool PlottingStrategy::dataReady() const { return m_dataReady; }

inline void PlottingStrategy::setSamplingFreq(int newSamplingFreq) { m_samplingFreq = newSamplingFreq; }

inline void PlottingStrategy::clearSamples() { m_samples.clear(); }

} // namespace scopy::pqm

#endif // PLOTTINGSTRATEGY_H
