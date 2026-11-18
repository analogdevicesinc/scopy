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

#include "m2klogicsource.h"

#include "m2kattributeio.h"
#include "m2kcontext.h"
#include "m2kdigitalstreams.h"
#include "m2ktriggermath.h"

#include <core/acq_engine/datastore.h>

#include <component/channel.h>
#include <component/device.h>

#include <algorithm>

using namespace scopy;
using namespace scopy::adalm2000;

namespace {
constexpr unsigned kStreamingKernelBuffers = 4;
constexpr int kSourceNone = 2;
} // namespace

M2kLogicSource::M2kLogicSource(M2kContext *ctx, QObject *parent)
	: SourceBlock(QStringLiteral("logic"), parent)
	, m_extSource(kSourceNone)
	, m_extCondition(trigger::DIGITAL_CONDITION_NONE)
{
	m_dev = ctx ? ctx->findChild<component::Device *>(QStringLiteral("digital"), Qt::FindDirectChildrenOnly)
		    : nullptr;
	m_stream = m_dev ? m_dev->findChild<M2kDigitalInputStream *>(QString(), Qt::FindDirectChildrenOnly) : nullptr;
	for(int pin = 0; pin < CHANNEL_COUNT; ++pin) {
		m_pin[pin] =
			m_dev ? component::channelById(m_dev, QStringLiteral("voltage%1").arg(pin), false) : nullptr;
		m_condition[pin] = trigger::DIGITAL_CONDITION_NONE;
		enableChannel(channelId(pin), true);
	}
}

void M2kLogicSource::setSampleRate(double hz) { m_sampleRate = std::clamp(hz, 1.0, MAX_SAMPLE_RATE); }

void M2kLogicSource::setStreaming(bool on) { m_streaming = on; }

void M2kLogicSource::setDelay(int samples)
{
	m_delay = samples;
	m_delayDirty = true;
}

void M2kLogicSource::setTriggerCondition(int pin, int condition)
{
	m_condition[pin] = condition;
	attr::post(attr::find(m_pin[pin], "trigger_condition"),
		   trigger::toName(trigger::DIGITAL_CONDITION_OPTIONS, condition));
}

void M2kLogicSource::setTriggerMode(int mode)
{
	m_mode = mode;
	attr::post(attr::find(m_dev, "trigger_mode"), trigger::toName(trigger::DIGITAL_MODE_OPTIONS, mode));
}

bool M2kLogicSource::hasExternalTrigger() const { return attr::find(m_dev, "trigger_source") != nullptr; }

void M2kLogicSource::setExternalTrigger(int source, int condition)
{
	m_extSource = source;
	m_extCondition = condition;
	attr::post(attr::find(m_dev, "trigger_source"), trigger::toName(trigger::DIGITAL_SOURCE_OPTIONS, source));
	attr::post(attr::find(m_dev, "trigger_external_condition"),
		   trigger::toName(trigger::DIGITAL_CONDITION_OPTIONS, condition));
}

bool M2kLogicSource::writeTrigger()
{
	const bool streaming = m_streaming;
	m_delayDirty = false;
	m_armedDelay = streaming ? 0 : m_delay.load();
	bool ok = attr::write(attr::find(m_dev, "trigger_streaming_flag"),
			      streaming ? QStringLiteral("1") : QStringLiteral("0")) &&
		attr::write(attr::find(m_dev, "trigger_delay"), QString::number(m_armedDelay)) &&
		attr::write(attr::find(m_dev, "trigger_mode"), trigger::toName(trigger::DIGITAL_MODE_OPTIONS, m_mode));
	for(int pin = 0; pin < CHANNEL_COUNT; ++pin) {
		ok = ok &&
			attr::write(attr::find(m_pin[pin], "trigger_condition"),
				    trigger::toName(trigger::DIGITAL_CONDITION_OPTIONS, m_condition[pin]));
	}
	if(hasExternalTrigger()) {
		ok = ok &&
			attr::write(attr::find(m_dev, "trigger_source"),
				    trigger::toName(trigger::DIGITAL_SOURCE_OPTIONS, m_extSource)) &&
			attr::write(attr::find(m_dev, "trigger_external_condition"),
				    trigger::toName(trigger::DIGITAL_CONDITION_OPTIONS, m_extCondition));
	}
	return ok;
}

void M2kLogicSource::onStart()
{
	SourceBlock::onStart();
	if(!m_dev || !m_stream) {
		throw std::runtime_error("no M2K digital input");
	}
	// One hertz over, as the old instrument did, so 100 MHz / n does not round to n + 1.
	component::Attribute *rate = attr::find(m_dev, "sample_rate_in");
	if(!attr::write(rate, QString::number(m_sampleRate.load() + 1, 'g', 12)) || !writeTrigger()) {
		throw std::runtime_error("could not configure the M2K digital input");
	}
	m_sampleRate = attr::readDouble(rate).value_or(m_sampleRate.load());
	Q_EMIT sampleRateChanged(m_sampleRate);

	m_stream->setKernelBuffers(m_streaming ? kStreamingKernelBuffers : 1);
	// No pins listed means all 16.
	if(!QCoro::waitFor(m_stream->openAsync({{}, bufferSize()}))) {
		throw std::runtime_error("could not start the M2K digital input");
	}
	m_started = true;
}

void M2kLogicSource::acquire(scopy::acq::DataStore *store)
{
	if(!m_started || stopRequested()) {
		return;
	}
	if(m_delayDirty.exchange(false) && !m_streaming) {
		m_armedDelay = m_delay;
		attr::write(attr::find(m_dev, "trigger_delay"), QString::number(m_armedDelay));
	}
	if(!QCoro::waitFor(m_stream->refillAsync()) || stopRequested()) {
		return;
	}
	m_bufferDelay = m_armedDelay;

	const component::StreamFormat &format = m_stream->readFormat();
	const QList<QString> enabled = enabledChannels();
	const auto *words = static_cast<const quint16 *>(format.data);
	const int n = static_cast<int>(format.sampleCount);
	for(const component::ChannelFormat &cf : format.channels) {
		const int pin = cf.shift;
		if(!enabled.contains(channelId(pin))) {
			continue;
		}
		QVector<quint8> bits(n);
		for(int s = 0; s < n; ++s) {
			bits[s] = (words[s] >> pin) & 1;
		}
		store->write(rawKey(pin), std::move(bits));
	}
}

void M2kLogicSource::onStop()
{
	if(m_stream) {
		QCoro::waitFor(m_stream->closeAsync());
	}
	m_started = false;
	SourceBlock::onStop();
}

void M2kLogicSource::requestStop()
{
	SourceBlock::requestStop();
	if(m_started && m_stream) {
		m_stream->cancel();
	}
}

std::optional<scopy::acq::StreamInfo> M2kLogicSource::streamInfo(const scopy::acq::DataKey &key) const
{
	for(int pin = 0; pin < CHANNEL_COUNT; ++pin) {
		if(key == rawKey(pin)) {
			scopy::acq::StreamInfo info;
			info.label = QStringLiteral("DIO %1").arg(pin);
			info.sampleRate = m_sampleRate;
			info.kind = scopy::acq::ReprKind::Digital;
			info.colorIndex = pin;
			return info;
		}
	}
	return std::nullopt;
}

#include "moc_m2klogicsource.cpp"
