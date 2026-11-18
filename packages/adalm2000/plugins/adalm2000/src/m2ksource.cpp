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

#include "m2ksource.h"

#include "m2kattributeio.h"
#include "m2kcontext.h"
#include "m2kdigitalstreams.h"
#include "m2kinputstream.h"
#include "spectrumsweep.h"

#include <core/acq_engine/datastore.h>

#include <component/channel.h>
#include <component/device.h>
#include <component/streamview.h>

#include <QTimer>

#include <algorithm>
#include <cerrno>

using namespace scopy;
using namespace scopy::adalm2000;
using Severity = scopy::acq::AcquisitionError::Severity;

namespace {
QString inputName(int in) { return in == M2kSource::Analog ? QStringLiteral("analog") : QStringLiteral("digital"); }
} // namespace

M2kSource::M2kSource(M2kContext *ctx, const QString &id, QObject *parent)
	: SourceBlock(id, parent)
{
	auto device = [ctx](const char *name) {
		return ctx ? ctx->findChild<component::Device *>(QString::fromLatin1(name), Qt::FindDirectChildrenOnly)
			   : nullptr;
	};
	m_dev[Analog] = device("analog-in");
	m_dev[Digital] = device("digital");
	m_ain = m_dev[Analog] ? m_dev[Analog]->findChild<M2kInputStream *>(QString(), Qt::FindDirectChildrenOnly)
			      : nullptr;
	m_din = m_dev[Digital]
		? m_dev[Digital]->findChild<M2kDigitalInputStream *>(QString(), Qt::FindDirectChildrenOnly)
		: nullptr;
}

QString M2kSource::channelId(Input in, int i)
{
	return (in == Analog ? QStringLiteral("voltage%1") : QStringLiteral("dio%1")).arg(i);
}

scopy::acq::DataKey M2kSource::rawKey(Input in, int i) const
{
	return scopy::acq::DataKey::raw(id(), channelId(in, i));
}

component::Channel *M2kSource::channel(Input in, int i) const
{
	return m_dev[in] ? component::channelById(m_dev[in], QStringLiteral("voltage%1").arg(i), false) : nullptr;
}

int M2kSource::indexOf(QObject *node, const char *attr) const
{
	for(int i = 0; i < m_entries.size(); ++i) {
		if(m_entries[i].setting.node == node && m_entries[i].setting.attr == attr) {
			return i;
		}
	}
	return -1;
}

void M2kSource::set(QObject *node, const char *attr, const QString &value, Apply apply)
{
	{
		QMutexLocker lock(&m_mutex);
		const int i = indexOf(node, attr);
		if(i < 0) {
			m_entries.append(Entry{Setting{node, attr, value}, apply, true});
		} else {
			Entry &e = m_entries[i];
			e.apply = e.dirty ? std::max(e.apply, apply) : apply;
			e.setting.value = value;
			e.dirty = true;
		}
	}
	const Input in = inputOf(node);
	if(apply != Write && m_started && hasStream(in)) {
		m_interrupted = true;
		cancel(in);
	}
}

QString M2kSource::value(QObject *node, const char *attr) const
{
	QMutexLocker lock(&m_mutex);
	const int i = indexOf(node, attr);
	return i < 0 ? QString() : m_entries[i].setting.value;
}

bool M2kSource::hasStream(Input in) const
{
	return in == Analog ? m_ain != nullptr : in == Digital && m_din != nullptr;
}

void M2kSource::cancel(Input in) { in == Analog ? m_ain->cancel() : m_din->cancel(); }

void M2kSource::invalidate(Input in) { in == Analog ? m_ain->invalidate() : m_din->invalidate(); }

void M2kSource::setKernelBuffers(Input in, unsigned n)
{
	QMutexLocker lock(&m_mutex);
	if(m_kernelBuffers[in] != n) {
		m_kernelBuffers[in] = n;
		m_reopen[in] = true;
	}
}

void M2kSource::setAutoTrigger(Input in, int timeoutMs, const QList<Setting> &untriggered)
{
	QMutexLocker lock(&m_mutex);
	m_autoMs[in] = timeoutMs;
	m_untriggered[in] = untriggered;
}

void M2kSource::watch(QObject *node, const char *attr)
{
	QMutexLocker lock(&m_mutex);
	m_watched.append(Setting{node, attr, QString()});
}

void M2kSource::restoreOnStop(QObject *node, const char *attr)
{
	QMutexLocker lock(&m_mutex);
	m_restored.append(Setting{node, attr, QString()});
}

void M2kSource::saveRestored()
{
	QList<Setting> restored;
	{
		QMutexLocker lock(&m_mutex);
		restored = m_restored;
	}
	m_saved.clear();
	for(Setting s : std::as_const(restored)) {
		QString why;
		if(const std::optional<QString> v = attr::readString(s.node, s.attr.constData(), why)) {
			s.value = *v;
			m_saved.append(s);
		} else {
			report(Severity::Warning, why + QStringLiteral("; it will not be restored on stop"));
		}
	}
}

void M2kSource::restore()
{
	for(const Setting &s : std::as_const(m_saved)) {
		try {
			attr::write(s.node, s.attr.constData(), s.value);
		} catch(const std::exception &e) {
			report(Severity::Warning, QStringLiteral("restoring on stop failed: ") + e.what());
		}
	}
	m_saved.clear();
}

M2kSource::Input M2kSource::inputOf(QObject *node) const
{
	for(int in = 0; in < INPUT_COUNT; ++in) {
		if(node && m_dev[in] && (node == m_dev[in] || node->parent() == m_dev[in])) {
			return static_cast<Input>(in);
		}
	}
	return INPUT_COUNT;
}

bool M2kSource::wanted(Input in) const
{
	const QList<QString> enabled = enabledChannels();
	return std::any_of(enabled.begin(), enabled.end(), [in](const QString &ch) {
		return in == Analog ? ch.startsWith(QStringLiteral("voltage")) || ch == timeChannelId()
				    : ch.startsWith(QStringLiteral("dio"));
	});
}

void M2kSource::written(Input in, const Setting &s)
{
	if(s.node != m_dev[in]) {
		return;
	}
	if(s.attr == "trigger_delay") {
		m_armedDelay[in] = s.value.toInt();
	} else if(s.attr == "trigger_streaming_flag") {
		m_armedStreaming[in] = s.value == QStringLiteral("1");
	}
}

void M2kSource::open(Input in)
{
	QList<Setting> writes;
	unsigned kernelBuffers = 1;
	{
		QMutexLocker lock(&m_mutex);
		for(Entry &e : m_entries) {
			if(inputOf(e.setting.node) == in) {
				writes.append(e.setting);
				e.dirty = false;
			}
		}
		kernelBuffers = m_kernelBuffers[in];
		m_reopen[in] = false;
	}
	for(const Setting &s : std::as_const(writes)) {
		attr::write(s.node, s.attr.constData(), s.value);
		written(in, s);
	}

	QString why;
	std::optional<double> rate;
	if(in == Analog) {
		const std::optional<double> sr = attr::readDouble(m_dev[in], "sample_rate", why);
		const std::optional<double> os =
			sr ? attr::readDouble(m_dev[in], "oversampling_ratio", why) : std::nullopt;
		if(os) {
			rate = *sr / std::max(1.0, *os);
		}
	} else {
		rate = attr::readDouble(m_dev[in], "sample_rate_in", why);
	}
	if(rate) {
		m_rate[in] = *rate;
		Q_EMIT sampleRateChanged(in, *rate);
	} else {
		report(Severity::Warning, why + QStringLiteral("; keeping %1 SPS").arg(m_rate[in].load(), 0, 'g', 12));
	}

	const Result<void> r =
		in == Analog ? m_ain->setKernelBuffers(kernelBuffers) : m_din->setKernelBuffers(kernelBuffers);
	if(!r) {
		attr::fail(QStringLiteral("setting %1 kernel buffers failed: %2")
				   .arg(kernelBuffers)
				   .arg(attr::describe(r.error())));
	}
	// No pins listed means all 16.
	const auto resp = QCoro::waitFor(in == Analog ? m_ain->openAsync({{0, 1}, bufferSize()})
						      : m_din->openAsync({{}, bufferSize()}));
	if(!resp) {
		attr::fail(QStringLiteral("opening the stream (%1 samples) failed: %2")
				   .arg(bufferSize())
				   .arg(attr::describe(resp.error())));
	}
	m_open[in] = true;
	m_streamed[in] = 0;
}

void M2kSource::close(Input in)
{
	if(!m_open[in]) {
		return;
	}
	m_open[in] = false;
	const auto resp = QCoro::waitFor(in == Analog ? m_ain->closeAsync() : m_din->closeAsync());
	if(!resp) {
		report(Severity::Warning,
		       QStringLiteral("closing the %1 stream failed: %2")
			       .arg(inputName(in), attr::describe(resp.error())));
	}
}

void M2kSource::readWatched()
{
	QList<Setting> watched;
	{
		QMutexLocker lock(&m_mutex);
		watched = m_watched;
	}
	for(const Setting &w : std::as_const(watched)) {
		QString why;
		if(const std::optional<QString> v = attr::readString(w.node, w.attr.constData(), why)) {
			Q_EMIT attributeRead(w.node, w.attr, *v);
		} else {
			report(Severity::Warning, why + QStringLiteral("; keeping the previous value"));
		}
	}
}

void M2kSource::onStart()
{
	SourceBlock::onStart();
	saveRestored();
	for(int i = 0; i < INPUT_COUNT; ++i) {
		const Input in = static_cast<Input>(i);
		m_timedOut[in] = false;
		if(!wanted(in)) {
			continue;
		}
		if(!m_dev[in] || !hasStream(in)) {
			stopAll();
			attr::fail(m_dev[in]
					   ? QStringLiteral("the M2K %1 device has no input stream").arg(inputName(in))
					   : QStringLiteral("the M2K context has no %1 device").arg(inputName(in)));
		}
		try {
			open(in);
		} catch(const std::exception &e) {
			stopAll();
			attr::fail(QStringLiteral("could not start the M2K %1 input: %2").arg(inputName(in), e.what()));
		}
	}
	readWatched();
	m_started = true;
}

void M2kSource::stopAll()
{
	for(int in = 0; in < INPUT_COUNT; ++in) {
		close(static_cast<Input>(in));
	}
	restore();
}

void M2kSource::applyChanges()
{
	QList<Setting> writes[INPUT_COUNT];
	Apply apply[INPUT_COUNT]{Write, Write};
	{
		QMutexLocker lock(&m_mutex);
		for(Entry &e : m_entries) {
			const Input in = inputOf(e.setting.node);
			if(!e.dirty || in == INPUT_COUNT) {
				continue;
			}
			e.dirty = false;
			writes[in].append(e.setting);
			apply[in] = std::max(apply[in], e.apply);
		}
		for(int in = 0; in < INPUT_COUNT; ++in) {
			if(m_reopen[in]) {
				apply[in] = Reopen;
			}
		}
	}

	bool any = false;
	for(int i = 0; i < INPUT_COUNT; ++i) {
		const Input in = static_cast<Input>(i);
		if((writes[in].isEmpty() && apply[in] != Reopen) || !m_open[in]) {
			continue;
		}
		any = true;
		if(apply[in] == Reopen) {
			close(in);
			try {
				open(in);
			} catch(const std::exception &e) {
				m_started = false;
				stopAll();
				attr::fail(QStringLiteral("could not restart the M2K %1 input: %2")
						   .arg(inputName(in), e.what()));
			}
			continue;
		}
		for(const Setting &s : std::as_const(writes[in])) {
			try {
				attr::write(s.node, s.attr.constData(), s.value);
				written(in, s);
			} catch(const std::exception &e) {
				report(Severity::Warning,
				       QString::fromUtf8(e.what()) + QStringLiteral("; keeping the previous value"));
			}
		}
		if(apply[in] == Restart) {
			invalidate(in);
		}
	}
	if(any) {
		readWatched();
	}
}

bool M2kSource::refill(Input in)
{
	auto once = [this, in]() { return QCoro::waitFor(in == Analog ? m_ain->refillAsync() : m_din->refillAsync()); };
	int autoMs = 0;
	QList<Setting> untriggered;
	{
		QMutexLocker lock(&m_mutex);
		autoMs = m_autoMs[in];
		untriggered = m_untriggered[in];
	}
	m_timedOut[in] = false;

	auto result = [&]() {
		if(autoMs <= 0) {
			return once();
		}
		// The worker pumps events while it waits, so the timer can cancel the refill.
		bool timedOut = false;
		QTimer watchdog;
		watchdog.setSingleShot(true);
		QObject::connect(&watchdog, &QTimer::timeout, [this, in, &timedOut]() {
			timedOut = true;
			cancel(in);
		});
		const double rate = m_rate[in].load();
		const double bufferMs = rate > 0 ? 1000.0 * bufferSize() / rate : 0.0;
		watchdog.start(autoMs + static_cast<int>(bufferMs));
		auto first = once();
		watchdog.stop();
		if(!timedOut || stopRequested()) {
			return first;
		}
		// No trigger: one buffer without it, then rearm.
		m_timedOut[in] = true;
		for(const Setting &s : std::as_const(untriggered)) {
			attr::write(s.node, s.attr.constData(), s.value);
		}
		auto second = once();
		for(const Setting &s : std::as_const(untriggered)) {
			const QString armed = value(s.node, s.attr.constData());
			if(!armed.isNull()) {
				attr::write(s.node, s.attr.constData(), armed);
			}
		}
		invalidate(in);
		return second;
	}();

	if(stopRequested()) {
		return false;
	}
	if(!result) {
		if(result.error().code == -ECANCELED && m_interrupted) {
			return false;
		}
		const bool timeout = result.error().code == -ETIMEDOUT;
		// Once per distinct error: a lost link fails every cycle
		reportOnce(timeout ? Severity::Info : Severity::Warning,
			   (timeout ? QStringLiteral("no %1 trigger yet: ")
				    : QStringLiteral("reading %1 samples failed: "))
					   .arg(inputName(in)) +
				   attr::describe(result.error()));
		return false;
	}
	m_bufDelay[in] = m_armedDelay[in];
	return true;
}

void M2kSource::publish(Input in, scopy::acq::DataStore *store)
{
	const QList<QString> enabled = enabledChannels();
	if(in == Digital) {
		const component::StreamFormat &format = m_din->readFormat();
		const auto *words = static_cast<const quint16 *>(format.data);
		const int n = static_cast<int>(format.sampleCount);
		for(const component::ChannelFormat &cf : format.channels) {
			const int pin = cf.shift;
			if(!enabled.contains(channelId(Digital, pin))) {
				continue;
			}
			QVector<quint8> bits(n);
			for(int s = 0; s < n; ++s) {
				bits[s] = (words[s] >> pin) & 1;
			}
			store->write(rawKey(Digital, pin), std::move(bits));
		}
		return;
	}

	const component::StreamFormat &format = m_ain->readFormat();
	const component::StreamView view(format);
	const int n = static_cast<int>(format.sampleCount);
	if(enabled.contains(timeChannelId())) {
		const double rate = m_rate[Analog].load();
		const qint64 first = m_armedStreaming[Analog] ? m_streamed[Analog] : m_armedDelay[Analog];
		QVector<float> time(n);
		for(int s = 0; s < n; ++s) {
			time[s] = static_cast<float>((first + s) / rate);
		}
		store->write(timeKey(), std::move(time));
	}
	m_streamed[Analog] += n;
	const bool fullScale = m_unit == FullScale;
	for(int ch = 0; ch < ANALOG_CHANNELS && ch < format.channels.size(); ++ch) {
		if(!enabled.contains(channelId(Analog, ch))) {
			continue;
		}
		const component::ChannelFormat &cf = format.channels.at(ch);
		const char *base = static_cast<const char *>(format.data) + cf.offset;
		QVector<float> out(n);
		for(int s = 0; s < n; ++s) {
			out[s] = fullScale
				? *reinterpret_cast<const qint16 *>(base + s * cf.stride) / spec::FULL_SCALE_COUNTS
				: static_cast<float>(view.sampleAsDouble(ch, s));
		}
		store->write(rawKey(Analog, ch), std::move(out));
	}
}

void M2kSource::acquire(scopy::acq::DataStore *store)
{
	if(!m_started || stopRequested()) {
		return;
	}
	bool again = true;
	while(again && !stopRequested()) {
		again = false;
		applyChanges();
		for(int i = 0; i < INPUT_COUNT; ++i) {
			const Input in = static_cast<Input>(i);
			if(!m_open[in]) {
				continue;
			}
			if(!refill(in)) {
				again = m_interrupted.exchange(false);
				if(!again) {
					return;
				}
				break;
			}
			publish(in, store);
		}
	}
}

void M2kSource::onStop()
{
	stopAll();
	m_started = false;
	SourceBlock::onStop();
}

void M2kSource::requestStop()
{
	SourceBlock::requestStop();
	if(!m_started) {
		return;
	}
	for(int in = 0; in < INPUT_COUNT; ++in) {
		if(hasStream(static_cast<Input>(in))) {
			cancel(static_cast<Input>(in));
		}
	}
}

std::optional<scopy::acq::StreamInfo> M2kSource::streamInfo(const scopy::acq::DataKey &key) const
{
	if(key.sourceId() != id()) {
		return std::nullopt;
	}
	scopy::acq::StreamInfo info;
	if(key == timeKey()) {
		info.unit = QStringLiteral("s");
		return info;
	}
	for(int ch = 0; ch < ANALOG_CHANNELS; ++ch) {
		if(key == rawKey(Analog, ch)) {
			info.label = QStringLiteral("CH%1").arg(ch + 1);
			info.unit = m_unit == FullScale ? QStringLiteral("FS") : QStringLiteral("V");
			info.sampleRate = m_rate[Analog].load();
			info.kind = scopy::acq::ReprKind::Curve;
			info.colorIndex = ch;
			if(isChannelEnabled(timeChannelId())) {
				info.xKey = timeKey();
			}
			return info;
		}
	}
	for(int pin = 0; pin < DIGITAL_CHANNELS; ++pin) {
		if(key == rawKey(Digital, pin)) {
			info.label = QStringLiteral("DIO %1").arg(pin);
			info.sampleRate = m_rate[Digital].load();
			info.kind = scopy::acq::ReprKind::Digital;
			info.colorIndex = pin;
			return info;
		}
	}
	return std::nullopt;
}

#include "moc_m2ksource.cpp"
