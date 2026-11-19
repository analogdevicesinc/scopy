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

#include "m2kanaloginsource.h"

#include "m2kcomponentbuilder.h"
#include "m2kcontext.h"
#include "m2kinputstream.h"
#include "spectrumsweep.h"

#include <core/acq_engine/datastore.h>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/attributewriter.h>
#include <component/channel.h>
#include <component/device.h>
#include <component/navigation.h>

#include <qcoro/qcorotask.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_M2K_AIN_SOURCE, "M2kAnalogInSource")

using namespace scopy;
using namespace scopy::adalm2000;

namespace {
constexpr unsigned kKernelBuffers = 4;

component::Attribute *attribute(QObject *node, const char *name)
{
	return node ? component::attributeByName(node, QString::fromLatin1(name)) : nullptr;
}

bool write(component::Attribute *attr, const QString &value)
{
	return attr && attr->writeCapability() && QCoro::waitFor(attr->writeCapability()->writeAsync(value));
}

std::optional<double> readDouble(component::Attribute *attr)
{
	if(!attr || !attr->readCapability()) {
		return std::nullopt;
	}
	const auto resp = QCoro::waitFor(attr->readCapability()->readAsync());
	bool ok = false;
	const double value = resp ? QString::fromUtf8(resp.value()).toDouble(&ok) : 0.0;
	return ok ? std::optional<double>(value) : std::nullopt;
}
} // namespace

M2kAnalogInSource::M2kAnalogInSource(M2kContext *ctx, QObject *parent)
	: SourceBlock(QStringLiteral("m2k"), parent)
{
	m_dev = ctx ? ctx->findChild<component::Device *>(QStringLiteral("analog-in"), Qt::FindDirectChildrenOnly)
		    : nullptr;
	m_stream = m_dev ? m_dev->findChild<M2kInputStream *>(QString(), Qt::FindDirectChildrenOnly) : nullptr;
	for(int ch = 0; ch < CHANNEL_COUNT; ++ch) {
		m_chan[ch] = m_dev ? component::channelById(m_dev, channelId(ch), false) : nullptr;
		enableChannel(channelId(ch), true);
	}
	m_oversampling = spec::oversamplingFor(spec::DEFAULT_STOP_HZ);
}

void M2kAnalogInSource::setOversampling(int ratio)
{
	ratio = std::max(1, ratio);
	if(m_oversampling.exchange(ratio) == ratio) {
		return;
	}
	m_rateDirty = true;
	Q_EMIT sampleRateChanged(sampleRate());
}

double M2kAnalogInSource::sampleRate() const { return spec::MAX_SAMPLE_RATE / m_oversampling.load(); }

void M2kAnalogInSource::setLowRange(int ch, bool low)
{
	if(ch < 0 || ch >= CHANNEL_COUNT) {
		return;
	}
	component::Attribute *attr = attribute(m_chan[ch], "range");
	if(!attr || !attr->writeCapability()) {
		qWarning(CAT_M2K_AIN_SOURCE) << channelId(ch) << "has no writable range attribute";
		return;
	}
	m_lowRange[ch] = low;
	attr->writeCapability()->writeAsync(low ? QLatin1String(PLUS_2_5V) : QLatin1String(PLUS_25V));
	m_rangeDirty = true;
}

bool M2kAnalogInSource::open()
{
	const double maxRate = readDouble(attribute(m_dev, "maximum_samplerate")).value_or(spec::MAX_SAMPLE_RATE);
	bool ok = write(attribute(m_dev, "sample_rate"), QString::number(maxRate, 'g', 12)) &&
		write(attribute(m_dev, "oversampling_ratio"), QString::number(m_oversampling.load()));
	for(int ch = 0; ch < CHANNEL_COUNT; ++ch) {
		ok = ok && write(attribute(m_chan[ch], "vertical_offset"), QStringLiteral("0")) &&
			write(attribute(m_chan[ch], "trigger_mode"), QStringLiteral("always"));
	}
	m_stream->setKernelBuffers(kKernelBuffers);
	return ok && QCoro::waitFor(m_stream->openAsync({{0, 1}, bufferSize()}));
}

void M2kAnalogInSource::readFullScale()
{
	m_rangeDirty = false;
	for(int ch = 0; ch < CHANNEL_COUNT; ++ch) {
		if(const std::optional<double> lsb = readDouble(attribute(m_chan[ch], "scaling_factor"))) {
			Q_EMIT fullScaleChanged(ch, *lsb * spec::FULL_SCALE_COUNTS);
		}
	}
}

void M2kAnalogInSource::onStart()
{
	SourceBlock::onStart();
	if(!m_dev || !m_stream) {
		throw std::runtime_error("no M2K analog input");
	}
	m_rateDirty = false;
	if(!open()) {
		throw std::runtime_error("could not start the M2K analog input");
	}
	m_started = true;
	readFullScale();
	Q_EMIT sampleRateChanged(sampleRate());
}

void M2kAnalogInSource::acquire(scopy::acq::DataStore *store)
{
	if(!m_started || stopRequested()) {
		return;
	}
	if(m_rateDirty.exchange(false)) {
		QCoro::waitFor(m_stream->closeAsync());
		if(!open()) {
			m_started = false;
			throw std::runtime_error("could not restart the M2K analog input");
		}
	}
	if(m_rangeDirty) {
		readFullScale();
	}
	if(!QCoro::waitFor(m_stream->refillAsync()) || stopRequested()) {
		return;
	}

	const component::StreamFormat &format = m_stream->readFormat();
	const QList<QString> enabled = enabledChannels();
	for(int ch = 0; ch < CHANNEL_COUNT && ch < format.channels.size(); ++ch) {
		if(!enabled.contains(channelId(ch))) {
			continue;
		}
		const component::ChannelFormat &cf = format.channels.at(ch);
		const char *base = static_cast<const char *>(format.data) + cf.offset;
		QVector<float> out(static_cast<int>(format.sampleCount));
		for(int s = 0; s < out.size(); ++s) {
			out[s] = *reinterpret_cast<const qint16 *>(base + s * cf.stride) / spec::FULL_SCALE_COUNTS;
		}
		store->write(rawKey(ch), std::move(out));
	}
}

void M2kAnalogInSource::onStop()
{
	if(m_stream) {
		QCoro::waitFor(m_stream->closeAsync());
	}
	m_started = false;
	SourceBlock::onStop();
}

void M2kAnalogInSource::requestStop()
{
	SourceBlock::requestStop();
	if(m_started && m_stream) {
		m_stream->cancel();
	}
}

std::optional<scopy::acq::StreamInfo> M2kAnalogInSource::streamInfo(const scopy::acq::DataKey &key) const
{
	for(int ch = 0; ch < CHANNEL_COUNT; ++ch) {
		if(key == rawKey(ch)) {
			scopy::acq::StreamInfo info;
			info.label = QStringLiteral("CH%1").arg(ch + 1);
			info.unit = QStringLiteral("FS");
			info.sampleRate = sampleRate();
			info.kind = scopy::acq::ReprKind::Curve;
			info.colorIndex = ch;
			return info;
		}
	}
	return std::nullopt;
}

#include "moc_m2kanaloginsource.cpp"
