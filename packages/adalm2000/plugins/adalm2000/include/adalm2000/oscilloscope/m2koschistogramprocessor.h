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

#pragma once

#include <core/acq_engine/processorblock.h>

#include <atomic>

namespace scopy::adalm2000 {

// How often each level occurs in a channel's buffer, over the range shown for it.
class M2kOscHistogramProcessor : public scopy::acq::ProcessorBlock
{
	Q_OBJECT
public:
	static constexpr int CHANNEL_COUNT = 2;
	static constexpr int BINS = 256;

	explicit M2kOscHistogramProcessor(const QList<scopy::acq::DataKey> &inputs, QObject *parent = nullptr);

	scopy::acq::DataKey countsKey(int ch) const;
	scopy::acq::DataKey levelsKey(int ch) const;

	void setRange(int ch, double min, double max);

	void process(scopy::acq::DataStore *store) override;
	QList<scopy::acq::DataKey> outputKeys() const override;
	std::optional<scopy::acq::StreamInfo> streamInfo(const scopy::acq::DataKey &key) const override;

private:
	const QList<scopy::acq::DataKey> m_inputs;
	std::atomic<double> m_min[CHANNEL_COUNT]{-5.0, -5.0};
	std::atomic<double> m_max[CHANNEL_COUNT]{5.0, 5.0};
};

} // namespace scopy::adalm2000
