/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 */

#include "measureprocessor.h"

#include "datastore.h"

#include <cmath>

using namespace scopy::acq;

void MeasureStat::push(double v)
{
	if(std::isnan(v)) {
		return;
	}
	if(count == 0) {
		min = max = average = v;
	} else {
		min = std::min(min, v);
		max = std::max(max, v);
		average += (v - average) / (count + 1);
	}
	++count;
}

MeasureProcessor::MeasureProcessor(QObject *parent)
	: ProcessorBlock(QStringLiteral("measure"), parent)
{
	qRegisterMetaType<scopy::acq::MeasureSnapshot>("scopy::acq::MeasureSnapshot");
}

void MeasureProcessor::setSampleRate(double hz)
{
	m_sampleRate = hz;
	clearStats();
}

void MeasureProcessor::setGate(int start, int end)
{
	m_gateStart = start;
	m_gateEnd = end;
	clearStats();
}

void MeasureProcessor::clearStats() { m_clear = true; }

void MeasureProcessor::reset()
{
	m_stats.clear();
	m_lastEmit.clear();
	m_clear = false;
}

void MeasureProcessor::process(DataStore *store)
{
	if(m_clear.exchange(false)) {
		m_stats.clear();
	}
	MeasureParams p;
	p.sampleRate = m_sampleRate;
	p.gateStart = m_gateStart;
	p.gateEnd = m_gateEnd;
	for(const DataKey &key : watchedKeys()) {
		const std::optional<QVector<float>> data = store->latestFloat(key);
		if(!data || data->isEmpty()) {
			continue;
		}
		MeasureSnapshot snap;
		snap.key = key;
		snap.result = measure(data->constData(), data->size(), p);
		std::array<MeasureStat, MEASURE_COUNT> &stats = m_stats[key];
		for(int m = 0; m < MEASURE_COUNT; ++m) {
			stats[m].push(snap.result.values[m]);
		}
		QElapsedTimer &last = m_lastEmit[key];
		if(last.isValid() && last.elapsed() < m_intervalMs) {
			continue;
		}
		last.start();
		snap.stats = stats;
		Q_EMIT measured(snap);
	}
}

#include "moc_measureprocessor.cpp"
