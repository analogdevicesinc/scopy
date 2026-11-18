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
 *
 */

#pragma once

#include <core/acq_engine/sourceblock.h>

#include <QString>

#include <atomic>

namespace scopy::component {
class Channel;
class Device;
} // namespace scopy::component

namespace scopy::adalm2000 {
class M2kContext;
class M2kDigitalInputStream;

// The 16 digital inputs as one 0/1 stream per pin. Sample rate, streaming and buffer size
// are latched at start; trigger values index the trigger:: tables of m2ktriggermath.h.
class M2kLogicSource : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	static constexpr int CHANNEL_COUNT = 16;
	static constexpr double MAX_SAMPLE_RATE = 100e6;

	explicit M2kLogicSource(M2kContext *ctx, QObject *parent = nullptr);

	static QString channelId(int pin) { return QStringLiteral("dio%1").arg(pin); }
	scopy::acq::DataKey rawKey(int pin) const { return scopy::acq::DataKey::raw(id(), channelId(pin)); }

	// The requested rate until a start reads back the one the hardware took.
	void setSampleRate(double hz);
	double sampleRate() const { return m_sampleRate.load(); }

	// Trigger once, then capture gapless buffers; otherwise every buffer waits for a trigger.
	void setStreaming(bool on);
	bool isStreaming() const { return m_streaming.load(); }

	// Applied between buffers, so bufferDelay() is the delay the latest buffer was taken with.
	void setDelay(int samples);
	int bufferDelay() const { return m_bufferDelay.load(); }
	void setTriggerCondition(int pin, int condition);
	void setTriggerMode(int mode);

	bool hasExternalTrigger() const;
	void setExternalTrigger(int source, int condition);

	void onStart() override;
	void acquire(scopy::acq::DataStore *store) override;
	void onStop() override;
	void requestStop() override;
	std::optional<scopy::acq::StreamInfo> streamInfo(const scopy::acq::DataKey &key) const override;

Q_SIGNALS:
	void sampleRateChanged(double hz);

private:
	bool writeTrigger();

	scopy::component::Device *m_dev{nullptr};
	scopy::component::Channel *m_pin[CHANNEL_COUNT]{};
	M2kDigitalInputStream *m_stream{nullptr};

	std::atomic<double> m_sampleRate{1e6};
	std::atomic<bool> m_streaming{false};
	std::atomic<int> m_delay{0};
	std::atomic<bool> m_delayDirty{false};
	std::atomic<int> m_bufferDelay{0};
	int m_armedDelay{0};
	std::atomic<int> m_mode{0};
	std::atomic<int> m_condition[CHANNEL_COUNT];
	std::atomic<int> m_extSource;
	std::atomic<int> m_extCondition;
	std::atomic<bool> m_started{false};
};
} // namespace scopy::adalm2000
