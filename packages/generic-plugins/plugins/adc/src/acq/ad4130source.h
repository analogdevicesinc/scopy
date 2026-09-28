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

#ifndef AD4130SOURCE_H
#define AD4130SOURCE_H

#include <core/acq_engine/SourceBlock.h>

#include <atomic>
#include <QMap>
#include <QStringList>

struct iio_context;
struct iio_device;
struct iio_buffer;
struct iio_channel;

namespace scopy {
namespace adc {

// SourceBlock over an AD4130, as exposed by the Linux ad4130 driver:
//
//   iio:device0: ad4130 (buffer capable)
//     voltage19-voltage19, voltage18-voltage19, voltage16-voltage19,
//     voltage2-voltage3, voltage0-voltage1   (input, be:U24/24>>0)
//     per-channel attrs: scale, offset, sampling_frequency, filter_mode
//
// Nothing about the channel set is written down here. The names above are what
// this particular device tree happens to publish — which pins are muxed into
// which differential pair is a board/overlay decision that changes between
// setups, so the constructor enumerates whatever buffer-capable input channels
// the device reports and registers those. A retargeted overlay needs no edit
// here.
//
// scale and offset are read per channel rather than device-wide: the part has a
// PGA per setup, so two channels on the same device can legitimately convert
// with different gains. Volts are (raw + offset) * scale, with scale in volts
// per LSB as the Linux ABI defines it.
//
// sampling_frequency is per channel too, and streamInfo() reports each
// channel's own, so views get a real time axis.
class Ad4130Source : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	explicit Ad4130Source(iio_context *ctx, const QString &id = "ad4130", const QString &devName = "ad4130",
			      QObject *parent = nullptr);
	~Ad4130Source() override;

	// The buffered channels, in scan-element order, as read off the device.
	QStringList channels() const { return m_channels; }

	// The constructor's device lookup is the answer: it caches m_dev and leaves throwing
	// to onStart(), so a block built against a context without this part is inert rather
	// than broken.
	bool isAvailable() const override { return m_dev != nullptr; }

	void onStart() override;
	void onStop() override;
	void acquire(scopy::acq::DataStore *store) override;

	// Volts on the channel's own timeline. The base class would answer unitless
	// curves on an unknown one.
	std::optional<scopy::acq::StreamInfo> streamInfo(const scopy::acq::DataKey &key) const override;

private:
	// The per-channel conversion terms and rate, as last read off the device.
	struct ChannelInfo
	{
		double scale{0.0};
		double offset{0.0};
		double sampleRate{0.0};
	};

	iio_channel *findChannel(const QString &channelId) const;

	// Enumerates the buffer-capable input channels and reads each one's scale,
	// offset and rate. Called from the constructor so the rail can show the rows
	// before the first run, and again from onStart() so a device reconfigured
	// between runs is picked up.
	void refreshChannels();

	// Publishes one channel from the current buffer contents.
	void writeChannel(scopy::acq::DataStore *store, const QString &channelId, iio_channel *ch, ptrdiff_t step);

	iio_context *m_ctx{nullptr};
	QString      m_devName;
	iio_device  *m_dev{nullptr};

	// Read by acquire() and cleared by onStop() from the GUI thread while the worker
	// may be inside a refill — same reason PlutoIIOSource holds an atomic.
	std::atomic<iio_buffer *> m_buf{nullptr};

	// Discovered, not declared — see the class comment.
	QStringList m_channels;

	// Written by refreshChannels() on the GUI thread between runs, read per cycle
	// by the worker. Keyed by channel id so a changed channel set cannot leave a
	// stale index pointing at another channel's gain.
	QMap<QString, ChannelInfo> m_info;
};

} // namespace adc
} // namespace scopy

#endif // AD4130SOURCE_H
