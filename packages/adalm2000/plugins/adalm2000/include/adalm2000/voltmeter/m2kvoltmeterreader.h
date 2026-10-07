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

#include "voltmeterdsp.h"

#include <QObject>

#include <atomic>
#include <optional>
#include <qcoro/qcorotask.h>

namespace scopy::adalm2000 {
class M2kContext;

// Runs the Voltmeter acquisition loop and feeds the DSP chain.
class M2kVoltmeterReader : public QObject
{
	Q_OBJECT
public:
	// 10000 samples/channel at 1e5 = one DSP update (DECIMATION) per refill.
	static constexpr unsigned int SAMPLES_PER_CHANNEL = 10000;

	explicit M2kVoltmeterReader(M2kContext *ctx, QObject *parent = nullptr);
	~M2kVoltmeterReader() override;

	void start();
	void requestStop();
	bool isRunning() const { return m_running; }

	void resetChannels();
	void setAutoGainEnabled(int channel, bool enabled);

Q_SIGNALS:
	void readingsUpdated(int channel, double dcVolts, double acVolts);
	void rangeChangeRequested(int channel, bool lowRange);
	void finished();

private:
	QCoro::Task<void> acquisitionLoop();

	M2kContext *m_ctx;
	std::atomic<bool> m_running{false};
	std::optional<QCoro::Task<void>> m_task;
	bool m_pingWasMonitoring = false;

	dsp::VoltmeterChannel m_dsp[2];
	dsp::AutoGain m_autoGain[2];
	bool m_autoGainEnabled[2] = {true, true};
};

} // namespace scopy::adalm2000
