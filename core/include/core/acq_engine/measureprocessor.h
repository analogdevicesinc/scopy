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

#include "scopy-core_export.h"

#include "datakey.h"
#include "measure.h"
#include "processorblock.h"

#include <QElapsedTimer>
#include <QHash>
#include <QMetaType>
#include <QMutex>

#include <array>
#include <atomic>

namespace scopy {
namespace acq {

struct SCOPY_CORE_EXPORT MeasureStat
{
	double average{0.0};
	double min{0.0};
	double max{0.0};
	int count{0};

	void push(double v);
};

struct SCOPY_CORE_EXPORT MeasureSnapshot
{
	DataKey key;
	MeasureResult result;
	std::array<MeasureStat, MEASURE_COUNT> stats;
};

// Measures the newest chunk of every watched key on the worker and keeps running
// statistics. Results go out at most once per interval, per key.
class SCOPY_CORE_EXPORT MeasureProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	explicit MeasureProcessor(QObject *parent = nullptr);

	void setSampleRate(double hz);
	// Sample indices into each chunk; end < 0 turns the gate off.
	void setGate(int start, int end);
	void setEmitInterval(int ms) { m_intervalMs = ms; }
	void clearStats();

	void reset() override;
	void process(DataStore *store) override;

Q_SIGNALS:
	void measured(const scopy::acq::MeasureSnapshot &snapshot);

private:
	std::atomic<double> m_sampleRate{1.0};
	std::atomic<int> m_gateStart{0};
	std::atomic<int> m_gateEnd{-1};
	std::atomic<int> m_intervalMs{16};
	std::atomic<bool> m_clear{false};

	// Worker-thread only.
	QHash<DataKey, std::array<MeasureStat, MEASURE_COUNT>> m_stats;
	QHash<DataKey, QElapsedTimer> m_lastEmit;
};

} // namespace acq
} // namespace scopy

Q_DECLARE_METATYPE(scopy::acq::MeasureSnapshot)
