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

#include "m2ksource.h"

#include "sourceregistry.h"

#include <core/acq_engine/DataKey.h>
#include <core/acq_engine/DataStore.h>

#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/contextbuilder.hpp>
#include <libm2k/digital/m2kdigital.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/m2kexceptions.hpp>

#include <iio.h>

#include <QDebug>

#include <algorithm>
#include <stdexcept>

using namespace scopy;
using namespace scopy::adc;

QString M2kSource::digitalChannelId(int index) { return QStringLiteral("DIO%1").arg(index); }

QString M2kSource::analogChannelId(int index) { return QStringLiteral("voltage%1").arg(index); }

unsigned int M2kSource::analogSampleCount() const
{
	// 20 rather than 17 for the floor, so the result stays a multiple of 4 after the clamp.
	const unsigned int n = std::max<unsigned int>(static_cast<unsigned int>(m_bufferSize), 20u);
	return (n + 3u) & ~3u;
}

M2kSource::M2kSource(iio_context *ctx, const QString &id, QObject *parent)
	: SourceBlock(id, parent)
{
	// libm2k wants the uri alongside the context: it keys its own registry by uri, so handing
	// it the wrong one would make a second open of the same device look like a different board.
	// The context carries its own, which is the one the plugin opened with.
	const char *uri = ctx ? iio_context_get_attr_value(ctx, "uri") : nullptr;
	if(ctx && uri) {
		try {
			// Adopts the context without owning it — closing this block's handle must not
			// take the plugin's context down, which is why the destructor passes
			// deinit=false.
			m_m2k = libm2k::context::m2kOpen(ctx, uri);
		} catch(...) {
			// Not an M2K, or one already open elsewhere. Either way the answer is the same
			// as an absent device: isAvailable() reports false and nothing is registered.
			m_m2k = nullptr;
		}
	}

	if(!m_m2k) {
		return;
	}

	m_digital = m_m2k->getDigital();
	m_analog = m_m2k->getAnalogIn();

	// All 18 channels registered but disabled: the rail shows what the board offers and the
	// user picks. A run with nothing enabled starts neither subsystem.
	for(int ch = 0; ch < kDigitalChannels; ++ch) {
		enableChannel(digitalChannelId(ch), false);
	}
	for(int ch = 0; ch < kAnalogChannels; ++ch) {
		enableChannel(analogChannelId(ch), false);
	}
}

M2kSource::~M2kSource()
{
	onStop();

	if(m_m2k) {
		// deinit=false: the iio_context belongs to the plugin, and other tools may still be
		// using the board. This drops libm2k's handle, not the hardware's state.
		libm2k::context::contextClose(m_m2k, false);
		m_m2k = nullptr;
	}
}

void M2kSource::onStart()
{
	SourceBlock::onStart();

	if(!m_m2k) {
		throw std::runtime_error("M2kSource::onStart: no M2K handle");
	}

	// Decided once, here, from the enable flags. acquire() reads these rather than the flags
	// themselves: a channel enabled mid-run would otherwise have it read a buffer that
	// startAcquisition() was never called for.
	m_runDigital = false;
	m_runAnalog = false;
	for(const QString &ch : enabledChannels()) {
		if(ch.startsWith(QStringLiteral("DIO"))) {
			m_runDigital = true;
		} else {
			m_runAnalog = true;
		}
	}

	// Calibrated once per run, not per cycle: without it the analog readings carry the board's
	// raw offset and gain error — libm2k applies the coefficients when converting to volts, and
	// an uncalibrated board simply has none. Skipped when the board says it is already calibrated,
	// which is what Scopy's own M2K plugin checks before calibrating (m2kcontroller.cpp:112).
	// Digital-only runs skip it: it takes seconds and does nothing for the logic lines.
	if(m_runAnalog) {
		try {
			if(!m_m2k->isCalibrated()) {
				m_m2k->calibrateADC();
			}
		} catch(libm2k::m2k_exception &e) {
			// Not fatal: an uncalibrated read is a less accurate read, not a failed one.
			qWarning() << "M2kSource: ADC calibration failed, continuing uncalibrated:" << e.what();
		}
	}

	try {
		if(m_runDigital) {
			// Inputs. The board keeps whatever direction a previous tool left, so a line the
			// pattern generator drove would otherwise read back its own output instead of the
			// signal on the pin.
			for(int ch = 0; ch < kDigitalChannels; ++ch) {
				m_digital->setDirection(static_cast<unsigned int>(ch), libm2k::digital::DIO_INPUT);
			}
			// Per-channel enables mirror the block's, so a disabled channel is not read off
			// the wire at all rather than read and dropped.
			for(int ch = 0; ch < kDigitalChannels; ++ch) {
				m_digital->enableChannel(static_cast<unsigned int>(ch),
							 isChannelEnabled(digitalChannelId(ch)));
			}
			// The device returns what it actually took, which may not be what was asked.
			m_digitalRate.store(m_digital->setSampleRateIn(m_wantDigitalRate),
					    std::memory_order_relaxed);
			m_digital->startAcquisition(static_cast<unsigned int>(m_bufferSize));
		}

		if(m_runAnalog) {
			for(int ch = 0; ch < kAnalogChannels; ++ch) {
				m_analog->enableChannel(static_cast<unsigned int>(ch),
							isChannelEnabled(analogChannelId(ch)));
			}
			m_analogRate.store(m_analog->setSampleRate(m_wantAnalogRate), std::memory_order_relaxed);
			// Rounded: startAcquisition and getSamples must agree on the length, and the
			// analog side constrains it.
			m_analog->startAcquisition(analogSampleCount());
		}
	} catch(libm2k::m2k_exception &e) {
		throw std::runtime_error(QStringLiteral("M2kSource::onStart: %1").arg(e.what()).toStdString());
	}
}

void M2kSource::onStop()
{
	m_stopRequested = true;

	// Neither cancelAcquisition() nor stopAcquisition() is called here. Both segfault inside
	// libm2k when the worker is mid-read, which is exactly when a stop arrives — the same
	// reason M2kLogicSource leaves them out. The read returns on its own and m_stopRequested
	// makes acquire() drop the result.
	m_runDigital = false;
	m_runAnalog = false;
}

void M2kSource::acquire(scopy::acq::DataStore *store)
{
	if(m_stopRequested || !store) {
		return;
	}

	if(m_runDigital) {
		acquireDigital(store);
	}
	if(m_runAnalog) {
		acquireAnalog(store);
	}
}

void M2kSource::acquireDigital(scopy::acq::DataStore *store)
{
	const unsigned int count = static_cast<unsigned int>(m_bufferSize);

	// One word per sample, all 16 lines packed into its bits. Owned by libm2k and valid until
	// the next call, so the bits are copied out below rather than held.
	const unsigned short *raw = nullptr;
	try {
		raw = m_digital->getSamplesP(count);
	} catch(libm2k::m2k_exception &e) {
		// A stop unblocks the read by tearing the buffer down, so an exception during one is
		// the expected path out rather than a failure to report.
		if(m_stopRequested) {
			return;
		}
		throw std::runtime_error(QStringLiteral("M2kSource::acquire: digital: %1").arg(e.what()).toStdString());
	}

	if(!raw || m_stopRequested) {
		return;
	}

	for(int ch = 0; ch < kDigitalChannels; ++ch) {
		const QString chId = digitalChannelId(ch);
		if(!isChannelEnabled(chId)) {
			continue;
		}
		QVector<quint8> samples(static_cast<int>(count));
		for(unsigned int i = 0; i < count; ++i) {
			samples[static_cast<int>(i)] = (raw[i] >> ch) & 1u;
		}
		store->write(scopy::acq::DataKey::raw(id(), chId), std::move(samples));
	}
}

void M2kSource::acquireAnalog(scopy::acq::DataStore *store)
{
	const unsigned int count = analogSampleCount();

	// Already scaled to volts by libm2k, which applies the range and the board's calibration —
	// worth the double-to-float narrowing to avoid redoing either here.
	std::vector<std::vector<double>> raw;
	try {
		raw = m_analog->getSamples(count);
	} catch(libm2k::m2k_exception &e) {
		if(m_stopRequested) {
			return;
		}
		throw std::runtime_error(QStringLiteral("M2kSource::acquire: analog: %1").arg(e.what()).toStdString());
	}

	if(m_stopRequested) {
		return;
	}

	for(int ch = 0; ch < kAnalogChannels; ++ch) {
		const QString chId = analogChannelId(ch);
		if(!isChannelEnabled(chId) || ch >= int(raw.size())) {
			continue;
		}
		const std::vector<double> &src = raw[ch];
		QVector<float> samples(int(src.size()));
		for(std::size_t i = 0; i < src.size(); ++i) {
			samples[int(i)] = float(src[i]);
		}
		store->write(scopy::acq::DataKey::raw(id(), chId), std::move(samples));
	}
}

std::optional<scopy::acq::StreamInfo> M2kSource::streamInfo(const scopy::acq::DataKey &key) const
{
	// Full-key comparison, like the base class: it checks the source id and the raw stage too, so
	// another block's identically-named channel cannot pick up this one's description.
	for(int ch = 0; ch < kDigitalChannels; ++ch) {
		const QString chId = digitalChannelId(ch);
		if(scopy::acq::DataKey::raw(id(), chId) != key) {
			continue;
		}
		scopy::acq::StreamInfo info;
		// Digital, not Curve: this is what puts the channel in front of the decoders.
		info.kind = scopy::acq::ReprKind::Digital;
		info.label = chId;
		info.sampleRate = m_digitalRate.load(std::memory_order_relaxed);
		info.colorIndex = ch;
		return info;
	}

	for(int ch = 0; ch < kAnalogChannels; ++ch) {
		if(scopy::acq::DataKey::raw(id(), analogChannelId(ch)) != key) {
			continue;
		}
		scopy::acq::StreamInfo info;
		info.kind = scopy::acq::ReprKind::Curve;
		// "CH1"/"CH2" as the front panel labels them, not the IIO channel name.
		info.label = QStringLiteral("CH%1").arg(ch + 1);
		info.unit = QStringLiteral("V");
		info.sampleRate = m_analogRate.load(std::memory_order_relaxed);
		// After the 16 logic lines, so the two subsystems do not share palette entries.
		info.colorIndex = kDigitalChannels + ch;
		return info;
	}

	return std::nullopt;
}

// The block opens its own M2k handle from the context and reports isAvailable() from it, so this is
// the constructor alone.
static const bool s_m2kRegistered = AcqSourceRegistry::instance().add(
	[](iio_context *ctx, QObject *parent) -> scopy::acq::SourceBlock * {
		return new M2kSource(ctx, QStringLiteral("m2k"), parent);
	});

#include "moc_m2ksource.cpp"
