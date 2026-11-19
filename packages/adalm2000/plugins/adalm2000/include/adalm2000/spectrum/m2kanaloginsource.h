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
class M2kInputStream;

class M2kAnalogInSource : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	static constexpr int CHANNEL_COUNT = 2;

	explicit M2kAnalogInSource(M2kContext *ctx, QObject *parent = nullptr);

	static QString channelId(int ch) { return QStringLiteral("voltage%1").arg(ch); }
	scopy::acq::DataKey rawKey(int ch) const { return scopy::acq::DataKey::raw(id(), channelId(ch)); }

	void setOversampling(int ratio);
	int oversampling() const { return m_oversampling.load(); }
	double sampleRate() const;

	void setLowRange(int ch, bool low);
	bool isLowRange(int ch) const { return m_lowRange[ch].load(); }

	void onStart() override;
	void acquire(scopy::acq::DataStore *store) override;
	void onStop() override;
	void requestStop() override;
	std::optional<scopy::acq::StreamInfo> streamInfo(const scopy::acq::DataKey &key) const override;

Q_SIGNALS:
	void fullScaleChanged(int ch, double volts);
	void sampleRateChanged(double hz);

private:
	bool open();
	void readFullScale();

	scopy::component::Device *m_dev{nullptr};
	scopy::component::Channel *m_chan[CHANNEL_COUNT]{};
	M2kInputStream *m_stream{nullptr};
	std::atomic<int> m_oversampling{1};
	std::atomic<bool> m_rateDirty{false};
	std::atomic<bool> m_rangeDirty{false};
	std::atomic<bool> m_lowRange[CHANNEL_COUNT]{false, false};
	std::atomic<bool> m_started{false};
};
} // namespace scopy::adalm2000
