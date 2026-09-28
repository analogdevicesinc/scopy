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

#include "ad4130source.h"

#include "sourceregistry.h"

#include <core/acq_engine/DataKey.h>
#include <core/acq_engine/DataStore.h>

#include <iio.h>

#include <cstdint>
#include <stdexcept>

using namespace scopy;
using namespace scopy::adc;

namespace {

// The Linux IIO ABI reports voltage `scale` in mV per LSB — 0.000149011 over a
// 24-bit code is a 2.5 V full scale — so volts need this divisor.
constexpr double kMilliVoltsPerVolt = 1000.0;

} // namespace

Ad4130Source::Ad4130Source(iio_context *ctx, const QString &id, const QString &devName, QObject *parent)
	: SourceBlock(id, parent)
	, m_ctx(ctx)
	, m_devName(devName)
{
	// Found up front so the enumeration below can run: the rail shows the channel
	// rows before the first run. onStart() looks the device up again and throws
	// there — this is a cache, not the check.
	if(m_ctx) {
		m_dev = iio_context_find_device(m_ctx, m_devName.toLocal8Bit().constData());
	}

	refreshChannels();

	// Every discovered channel on. The mux'd differential pairs the overlay
	// publishes are all this device offers, and a reader who wants fewer turns
	// them off from the rail rows.
	for(const QString &chId : std::as_const(m_channels)) {
		enableChannel(chId, true);
	}
}

Ad4130Source::~Ad4130Source() { onStop(); }

iio_channel *Ad4130Source::findChannel(const QString &channelId) const
{
	if(!m_dev) {
		return nullptr;
	}
	// output=false: every channel on this part is an input.
	return iio_device_find_channel(m_dev, channelId.toLocal8Bit().constData(), false);
}

void Ad4130Source::refreshChannels()
{
	if(!m_dev) {
		return;
	}

	m_channels.clear();
	m_info.clear();

	// Scan elements only, and discovered rather than written out: which pins are
	// muxed into which differential pair is a device-tree decision, so the set is
	// whatever this device reports. `raw` exists per channel too, but reading it
	// once per channel per cycle would be slower than the buffer it stands in for.
	const unsigned int nch = iio_device_get_channels_count(m_dev);
	for(unsigned int i = 0; i < nch; ++i) {
		iio_channel *ch = iio_device_get_channel(m_dev, i);
		if(!ch || iio_channel_is_output(ch) || !iio_channel_is_scan_element(ch)) {
			continue;
		}

		const QString chId = QString::fromLatin1(iio_channel_get_id(ch));
		m_channels.append(chId);

		// Per channel, not device-wide: the part has a PGA per setup, so two
		// channels on one device can convert with different gains and rates.
		ChannelInfo info;
		iio_channel_attr_read_double(ch, "scale", &info.scale);
		iio_channel_attr_read_double(ch, "offset", &info.offset);
		iio_channel_attr_read_double(ch, "sampling_frequency", &info.sampleRate);
		m_info.insert(chId, info);
	}
}

void Ad4130Source::onStart()
{
	SourceBlock::onStart();

	m_dev = iio_context_find_device(m_ctx, m_devName.toLocal8Bit().constData());
	if(!m_dev) {
		throw std::runtime_error(QString("Device '%1' not found").arg(m_devName).toStdString());
	}

	// Another tool may have changed a gain or rate since construction, and both the
	// unit and the timeline every channel is published on come from them.
	refreshChannels();

	for(const QString &chId : enabledChannels()) {
		if(iio_channel *ch = findChannel(chId)) {
			iio_channel_enable(ch);
		}
	}

	iio_buffer *buf = iio_device_create_buffer(m_dev, m_bufferSize, false);
	m_buf.store(buf);
	if(!buf) {
		throw std::runtime_error(
			QString("Failed to create IIO buffer (size %1)").arg(m_bufferSize).toStdString());
	}
}

void Ad4130Source::onStop()
{
	m_stopRequested = true;

	// Cancel before destroy, so a worker blocked in refill returns instead of having
	// the buffer freed underneath it.
	iio_buffer *buf = m_buf.exchange(nullptr);
	if(buf) {
		iio_buffer_cancel(buf);
		iio_buffer_destroy(buf);
	}

	if(m_dev) {
		// Every channel, not just the enabled ones: the GUI may have flipped one off
		// after onStart() enabled it on the device.
		for(const QString &chId : std::as_const(m_channels)) {
			if(iio_channel *ch = findChannel(chId)) {
				iio_channel_disable(ch);
			}
		}
		m_dev = nullptr;
	}
}

void Ad4130Source::writeChannel(scopy::acq::DataStore *store, const QString &channelId, iio_channel *ch, ptrdiff_t step)
{
	iio_buffer *buf = m_buf.load();
	if(!buf) {
		return;
	}

	const ChannelInfo info = m_info.value(channelId);
	// A missing scale attribute would silently zero the trace, so raw codes are
	// published instead — a plot in the wrong unit is diagnosable, an empty one is
	// not. streamInfo() reports the unit the same way.
	const double factor = (info.scale != 0.0) ? info.scale / kMilliVoltsPerVolt : 1.0;

	const char *p = static_cast<const char *>(iio_buffer_first(buf, ch));
	const char *end = static_cast<const char *>(iio_buffer_end(buf));

	QVector<float> samples;
	samples.reserve(static_cast<int>(m_bufferSize));

	for(; p < end; p += step) {
		// be:U24/24>>0 — 24 bits with no padding, big-endian on a little-endian
		// host, so the three bytes need swapping. iio_channel_convert() does that
		// from the channel's own declared format; hand-rolling it would hardcode
		// this driver's format string. Zeroed first because convert writes only
		// the format's three bytes into the wider destination.
		uint32_t raw = 0;
		iio_channel_convert(ch, &raw, p);
		// Widened before the addition: offset is negative on a bipolar range and
		// must not wrap the unsigned value.
		samples.append(static_cast<float>((static_cast<double>(raw) + info.offset) * factor));
	}

	store->write(scopy::acq::DataKey::raw(id(), channelId), std::move(samples));
}

void Ad4130Source::acquire(scopy::acq::DataStore *store)
{
	iio_buffer *buf = m_buf.load();
	if(!buf || m_stopRequested) {
		return;
	}

	const ssize_t ret = iio_buffer_refill(buf);
	if(ret < 0) {
		// A cancelled buffer surfaces here as an error; a requested stop is not a
		// fault.
		if(m_stopRequested) {
			return;
		}
		throw std::runtime_error(QString("iio_buffer_refill error %1").arg(ret).toStdString());
	}

	const ptrdiff_t step = iio_buffer_step(buf);

	for(const QString &chId : enabledChannels()) {
		if(iio_channel *ch = findChannel(chId)) {
			writeChannel(store, chId, ch, step);
		}
	}
}

std::optional<scopy::acq::StreamInfo> Ad4130Source::streamInfo(const scopy::acq::DataKey &key) const
{
	const int index = m_channels.indexOf(key.channelId());

	// Anything not one of our channels, or shaped as something other than a raw key
	// of ours, belongs to another block.
	if(key.sourceId() != id() || index < 0 || !key.isRaw()) {
		return std::nullopt;
	}

	const ChannelInfo info = m_info.value(key.channelId());

	scopy::acq::StreamInfo streamInfo;
	streamInfo.kind = scopy::acq::ReprKind::Curve;
	// The channel's own rate, read off the device, so a view measures in seconds
	// rather than samples without the instrument having to guess it.
	streamInfo.sampleRate = info.sampleRate;
	// The device's channel name is the label: "voltage0-voltage1" says which pins
	// the pair is across, which a "CH0" would throw away.
	streamInfo.label = key.channelId();
	// Only claim volts when the scale that produces them was actually read — see
	// writeChannel(), which publishes raw codes when it was not.
	streamInfo.unit = (info.scale != 0.0) ? QStringLiteral("V") : QStringLiteral("LSB");
	streamInfo.colorIndex = index;
	return streamInfo;
}

// "ad4130" is both the device name the driver registers and the block's id, which is what
// DataKeys carry. The block enumerates the device's channels and enables them itself.
static const bool s_ad4130Registered = AcqSourceRegistry::instance().add(
	[](iio_context *ctx, QObject *parent) -> scopy::acq::SourceBlock * {
		return new Ad4130Source(ctx, QStringLiteral("ad4130"), QStringLiteral("ad4130"), parent);
	});

#include "moc_ad4130source.cpp"
