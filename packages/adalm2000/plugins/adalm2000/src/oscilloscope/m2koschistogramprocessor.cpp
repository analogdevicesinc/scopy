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

#include "m2koschistogramprocessor.h"

#include <core/acq_engine/datastore.h>

#include <cmath>

using namespace scopy;
using namespace scopy::adalm2000;
using scopy::acq::DataKey;

M2kOscHistogramProcessor::M2kOscHistogramProcessor(const QList<DataKey> &inputs, QObject *parent)
	: ProcessorBlock(QStringLiteral("histogram"), parent)
	, m_inputs(inputs)
{}

DataKey M2kOscHistogramProcessor::countsKey(int ch) const
{
	const DataKey in = m_inputs.at(ch);
	return DataKey::withStage(in.sourceId(), in.channelId(), QStringLiteral("hist"));
}

DataKey M2kOscHistogramProcessor::levelsKey(int ch) const
{
	const DataKey in = m_inputs.at(ch);
	return DataKey::withStage(in.sourceId(), in.channelId(), QStringLiteral("histv"));
}

void M2kOscHistogramProcessor::setRange(int ch, double min, double max)
{
	m_min[ch] = min;
	m_max[ch] = max;
}

void M2kOscHistogramProcessor::process(scopy::acq::DataStore *store)
{
	const QList<DataKey> watched = watchedKeys();
	for(int ch = 0; ch < CHANNEL_COUNT; ++ch) {
		if(!watched.contains(m_inputs.at(ch))) {
			continue;
		}
		const std::optional<QVector<float>> data = store->latestFloat(m_inputs.at(ch));
		if(!data) {
			continue;
		}
		const double min = m_min[ch];
		const double width = (m_max[ch] - min) / BINS;
		if(width <= 0) {
			continue;
		}
		QVector<float> counts(BINS, 0.0f);
		for(float v : *data) {
			const int bin = static_cast<int>(std::floor((v - min) / width));
			if(bin >= 0 && bin < BINS) {
				++counts[bin];
			}
		}
		QVector<float> levels(BINS);
		for(int b = 0; b < BINS; ++b) {
			levels[b] = static_cast<float>(min + (b + 0.5) * width);
		}
		store->write(levelsKey(ch), std::move(levels));
		store->write(countsKey(ch), std::move(counts));
	}
}

QList<DataKey> M2kOscHistogramProcessor::outputKeys() const
{
	return {countsKey(0), levelsKey(0), countsKey(1), levelsKey(1)};
}

std::optional<scopy::acq::StreamInfo> M2kOscHistogramProcessor::streamInfo(const DataKey &key) const
{
	for(int ch = 0; ch < CHANNEL_COUNT; ++ch) {
		if(key == countsKey(ch)) {
			scopy::acq::StreamInfo info;
			info.label = QStringLiteral("CH%1").arg(ch + 1);
			info.xKey = levelsKey(ch);
			info.kind = scopy::acq::ReprKind::Curve;
			info.colorIndex = ch;
			return info;
		}
		if(key == levelsKey(ch)) {
			scopy::acq::StreamInfo info;
			info.unit = QStringLiteral("V");
			return info;
		}
	}
	return std::nullopt;
}

#include "moc_m2koschistogramprocessor.cpp"
