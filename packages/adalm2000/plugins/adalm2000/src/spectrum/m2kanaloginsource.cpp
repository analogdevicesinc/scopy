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

#include <optional>
#include <utility>

using namespace scopy;
using namespace scopy::adalm2000;

namespace {
constexpr unsigned kKernelBuffers = 4;

component::Attribute *attribute(QObject *node, const char *name)
{
	return node ? component::attributeByName(node, QString::fromLatin1(name)) : nullptr;
}

QString describe(const Error &e)
{
	const QString s = e.errorString();
	return s.isEmpty() ? QStringLiteral("error %1").arg(e.code) : s;
}

[[noreturn]] void fail(const QString &msg) { throw std::runtime_error(msg.toStdString()); }

QString attrPath(QObject *node, const char *name)
{
	return QStringLiteral("%1/%2").arg(node ? node->objectName() : QStringLiteral("?"), QString::fromLatin1(name));
}

// Throws with the node, attribute and backend error, so a failed start says which step broke
void write(QObject *node, const char *name, const QString &value)
{
	const QString what = attrPath(node, name);
	component::Attribute *attr = attribute(node, name);
	if(!attr || !attr->writeCapability()) {
		fail(what + QStringLiteral(" is missing or not writable"));
	}
	const auto resp = QCoro::waitFor(attr->writeCapability()->writeAsync(value));
	if(!resp) {
		fail(QStringLiteral("writing %1 = %2 failed: %3").arg(what, value, describe(resp.error())));
	}
}

// nullopt with `why` set to the node, attribute and backend error, for the caller to report
std::optional<QString> readString(QObject *node, const char *name, QString &why)
{
	component::Attribute *attr = attribute(node, name);
	if(!attr || !attr->readCapability()) {
		why = attrPath(node, name) + QStringLiteral(" is missing or not readable");
		return std::nullopt;
	}
	const auto resp = QCoro::waitFor(attr->readCapability()->readAsync());
	if(!resp) {
		why = QStringLiteral("reading %1 failed: %2").arg(attrPath(node, name), describe(resp.error()));
		return std::nullopt;
	}
	return QString::fromUtf8(resp.value()).trimmed();
}

std::optional<double> readDouble(QObject *node, const char *name, QString &why)
{
	const std::optional<QString> s = readString(node, name, why);
	if(!s) {
		return std::nullopt;
	}
	bool ok = false;
	const double value = s->toDouble(&ok);
	if(!ok) {
		why = QStringLiteral("%1 reads \"%2\", not a number").arg(attrPath(node, name), *s);
		return std::nullopt;
	}
	return value;
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
	// Applied by the next open() or acquire(), so a failed write is reported
	m_lowRange[ch] = low;
	m_rangeDirty = true;
}

void M2kAnalogInSource::writeRanges()
{
	m_rangeDirty = false;
	for(int ch = 0; ch < CHANNEL_COUNT; ++ch) {
		write(m_chan[ch], "range", QLatin1String(m_lowRange[ch] ? PLUS_2_5V : PLUS_25V));
	}
}

void M2kAnalogInSource::open()
{
	QString why;
	const std::optional<double> readRate = readDouble(m_dev, "maximum_samplerate", why);
	if(!readRate) {
		reportOnce(scopy::acq::AcquisitionError::Severity::Warning,
			   QStringLiteral("%1; assuming %2 SPS").arg(why).arg(spec::MAX_SAMPLE_RATE, 0, 'g', 12));
	}
	const double maxRate = readRate.value_or(spec::MAX_SAMPLE_RATE);
	write(m_dev, "sample_rate", QString::number(maxRate, 'g', 12));
	write(m_dev, "oversampling_ratio", QString::number(m_oversampling.load()));
	// Another tool may have left an external or digital source, which would stall every refill
	write(m_dev, "trigger_source", QStringLiteral("CHANNEL_1"));
	writeRanges();
	for(int ch = 0; ch < CHANNEL_COUNT; ++ch) {
		write(m_chan[ch], "vertical_offset", QStringLiteral("0"));
		write(m_chan[ch], "trigger_mode", QStringLiteral("ALWAYS"));
	}
	if(const Result<void> r = m_stream->setKernelBuffers(kKernelBuffers); !r) {
		fail(QStringLiteral("setting %1 kernel buffers failed: %2")
			     .arg(kKernelBuffers)
			     .arg(describe(r.error())));
	}
	const int oversampling = m_oversampling.load();
	const auto resp = QCoro::waitFor(m_stream->openAsync({{0, 1}, bufferSize()}));
	if(!resp) {
		fail(QStringLiteral("opening the stream (%1 samples) failed: %2")
			     .arg(bufferSize())
			     .arg(describe(resp.error())));
	}
	m_openOversampling = oversampling;
}

void M2kAnalogInSource::closeStream()
{
	if(const auto resp = QCoro::waitFor(m_stream->closeAsync()); !resp) {
		report(scopy::acq::AcquisitionError::Severity::Warning,
		       QStringLiteral("closing the stream failed: ") + describe(resp.error()));
	}
}

// The trigger is shared with the other analog-in tools, so it is put back as it was
void M2kAnalogInSource::saveTrigger()
{
	m_savedTrigger.clear();
	const std::pair<QObject *, const char *> attrs[] = {
		{m_dev, "trigger_source"}, {m_chan[0], "trigger_mode"}, {m_chan[1], "trigger_mode"}};
	for(const auto &[node, name] : attrs) {
		QString why;
		if(const std::optional<QString> value = readString(node, name, why)) {
			m_savedTrigger.append(SavedAttr{node, name, *value});
		} else {
			report(scopy::acq::AcquisitionError::Severity::Warning,
			       why + QStringLiteral("; it will not be restored on stop"));
		}
	}
}

void M2kAnalogInSource::restoreTrigger()
{
	for(const SavedAttr &a : std::as_const(m_savedTrigger)) {
		try {
			write(a.node, a.name, a.value);
		} catch(const std::exception &e) {
			report(scopy::acq::AcquisitionError::Severity::Warning,
			       QStringLiteral("restoring the trigger failed: ") + e.what());
		}
	}
	m_savedTrigger.clear();
}

void M2kAnalogInSource::readFullScale()
{
	for(int ch = 0; ch < CHANNEL_COUNT; ++ch) {
		QString why;
		if(const std::optional<double> lsb = readDouble(m_chan[ch], "scaling_factor", why)) {
			Q_EMIT fullScaleChanged(ch, *lsb * spec::FULL_SCALE_COUNTS);
		} else {
			report(scopy::acq::AcquisitionError::Severity::Warning,
			       why + QStringLiteral("; volts are scaled with the previous full scale"));
		}
	}
}

void M2kAnalogInSource::onStart()
{
	SourceBlock::onStart();
	if(!m_dev || !m_stream) {
		fail(m_dev ? QStringLiteral("the M2K analog-in device has no input stream")
			   : QStringLiteral("the M2K context has no analog-in device"));
	}
	m_rateDirty = false;
	saveTrigger();
	try {
		open();
	} catch(const std::exception &e) {
		restoreTrigger();
		fail(QStringLiteral("could not start the M2K analog input: ") + e.what());
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
		closeStream();
		try {
			open();
		} catch(const std::exception &e) {
			m_started = false;
			fail(QStringLiteral("could not restart the M2K analog input: ") + e.what());
		}
	}
	if(m_rangeDirty) {
		writeRanges();
		readFullScale();
		// The queued kernel buffers hold samples taken at the old range
		m_stream->invalidate();
	}
	const auto refill = QCoro::waitFor(m_stream->refillAsync());
	if(stopRequested()) {
		return;
	}
	if(!refill) {
		// Once per distinct error: a lost link fails every cycle
		reportOnce(scopy::acq::AcquisitionError::Severity::Warning,
			   QStringLiteral("reading samples failed: ") + describe(refill.error()));
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
		closeStream();
	}
	m_started = false;
	restoreTrigger();
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
			// The rate the running stream was opened at, so a frame captured before a
			// pending rate change is not labelled with the new one
			info.sampleRate =
				spec::MAX_SAMPLE_RATE / (m_started ? m_openOversampling : m_oversampling).load();
			info.kind = scopy::acq::ReprKind::Curve;
			info.colorIndex = ch;
			return info;
		}
	}
	return std::nullopt;
}

#include "moc_m2kanaloginsource.cpp"
