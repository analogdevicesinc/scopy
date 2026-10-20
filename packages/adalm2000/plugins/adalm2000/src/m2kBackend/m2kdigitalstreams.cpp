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

#include "m2kdigitalstreams.h"

#include "m2kcallcommand.h"

#include "component/capabilityexecutor.h"

#include <libm2k/digital/m2kdigital.hpp>

#include <QLoggingCategory>

#include <cerrno>
#include <memory>
#include <vector>

Q_LOGGING_CATEGORY(CAT_M2K_DIGITAL_STREAMS, "M2kDigitalStreams")

using namespace scopy;
using namespace scopy::adalm2000;

using libm2k::digital::DIO_CHANNEL;
using libm2k::digital::DIO_DIRECTION;
using libm2k::digital::M2kDigital;

namespace {

// Every pin when nothing specific was asked for.
QList<int> normalisePins(const QList<int> &requested)
{
	QList<int> pins;
	for(int pin : requested) {
		if(pin >= 0 && pin < stream::DIGITAL_WORD_BITS && !pins.contains(pin)) {
			pins.append(pin);
		}
	}
	if(pins.isEmpty()) {
		for(int pin = 0; pin < stream::DIGITAL_WORD_BITS; ++pin) {
			pins.append(pin);
		}
	}
	return pins;
}

} // namespace

// ---------------------------------------------------------------- input

M2kDigitalInputStream::M2kDigitalInputStream(void *resource, M2kDigital *digital, ICmdExecutor *executor,
					     QObject *parent)
	: component::InputStream(parent)
	, m_resource(resource)
	, m_digital(digital)
	, m_executor(executor)
{}

M2kDigitalInputStream::~M2kDigitalInputStream()
{
	if(!m_open || !m_digital) {
		return;
	}
	try {
		m_digital->cancelAcquisition();
		m_digital->stopAcquisition();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_DIGITAL_STREAMS) << "input teardown failed:" << e.what();
	}
}

Result<void> M2kDigitalInputStream::setKernelBuffers(unsigned n)
{
	if(m_open) {
		return Result<void>(Unexpected{Error{-EBUSY, QStringLiteral("cannot set kernelBuffers while open")}});
	}
	m_kernelBuffers = n;
	return Result<void>();
}

QCoro::Task<CommandResponse<void>> M2kDigitalInputStream::openAsync(const component::StreamConfig &cfg)
{
	if(m_open) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EBUSY, QStringLiteral("already open")}});
	}

	component::StreamConfig effective = cfg;
	effective.samplesCount = stream::alignSamplesCount(cfg.samplesCount);
	if(effective.samplesCount != cfg.samplesCount) {
		qWarning(CAT_M2K_DIGITAL_STREAMS)
			<< "samplesCount" << cfg.samplesCount << "must be a multiple of 4 above 16; using"
			<< effective.samplesCount;
	}
	effective.enabledChannels = normalisePins(cfg.enabledChannels);

	M2kDigital *digital = m_digital;
	const unsigned kernelBuffers = m_kernelBuffers;
	const unsigned samples = static_cast<unsigned>(effective.samplesCount);

	// No per-channel input enable exists, and none is needed: the capture word
	// always carries all 16 pins. Channel selection is a descriptor concern.
	auto *cmd = new M2kCallCommand<void>(m_resource, [digital, kernelBuffers, samples]() {
		digital->setKernelBuffersCountIn(kernelBuffers);
		digital->startAcquisition(samples);
	});

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [this](const Error &e) { Q_EMIT streamError(e); });

	if(result) {
		m_config = effective;
		m_open = true;
		Q_EMIT openedChanged(true);
	}
	co_return result;
}

QCoro::Task<CommandResponse<void>> M2kDigitalInputStream::closeAsync()
{
	if(!m_open) {
		co_return CommandResponse<void>(QUuid());
	}

	m_open = false;
	m_format = component::StreamFormat{};

	M2kDigital *digital = m_digital;
	auto *cmd = new M2kCallCommand<void>(m_resource, [digital]() {
		digital->cancelAcquisition();
		digital->stopAcquisition();
	});

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [this](const Error &e) { Q_EMIT streamError(e); });

	Q_EMIT openedChanged(false);
	co_return result;
}

QCoro::Task<CommandResponse<void>> M2kDigitalInputStream::refillAsync()
{
	if(!m_open) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EIO, QStringLiteral("stream not open")}});
	}

	M2kDigital *digital = m_digital;
	const unsigned samples = static_cast<unsigned>(m_config.samplesCount);
	auto samplePtr = std::make_shared<const unsigned short *>(nullptr);

	auto *cmd = new M2kCallCommand<void>(
		m_resource, [digital, samples, samplePtr]() { *samplePtr = digital->getSamplesP(samples); });

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [this](const Error &e) { Q_EMIT refillFailed(e); });

	if(!result) {
		co_return result;
	}
	if(!m_open || *samplePtr == nullptr) {
		co_return result;
	}

	m_format = stream::bitPackedUint16Format(const_cast<unsigned short *>(*samplePtr), m_config.samplesCount,
						 m_config.enabledChannels);
	Q_EMIT refillSucceeded();
	co_return result;
}

// --------------------------------------------------------------- output

M2kDigitalOutputStream::M2kDigitalOutputStream(void *resource, M2kDigital *digital, ICmdExecutor *executor,
					       QObject *parent)
	: component::OutputStream(parent)
	, m_resource(resource)
	, m_digital(digital)
	, m_executor(executor)
{}

M2kDigitalOutputStream::~M2kDigitalOutputStream()
{
	if(!m_open || !m_digital) {
		return;
	}
	try {
		m_digital->cancelBufferOut();
		m_digital->stopBufferOut();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_DIGITAL_STREAMS) << "output teardown failed:" << e.what();
	}
}

Result<void> M2kDigitalOutputStream::setCyclic(bool enable)
{
	if(m_open) {
		return Result<void>(Unexpected{Error{-EBUSY, QStringLiteral("cannot set cyclic while open")}});
	}
	m_cyclic = enable;
	return Result<void>();
}

QCoro::Task<CommandResponse<void>> M2kDigitalOutputStream::openAsync(const component::StreamConfig &cfg)
{
	if(m_open) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EBUSY, QStringLiteral("already open")}});
	}

	component::StreamConfig effective = cfg;
	effective.samplesCount = stream::alignSamplesCount(cfg.samplesCount);
	if(effective.samplesCount != cfg.samplesCount) {
		qWarning(CAT_M2K_DIGITAL_STREAMS)
			<< "samplesCount" << cfg.samplesCount << "must be a multiple of 4 above 16; using"
			<< effective.samplesCount;
	}
	effective.enabledChannels = normalisePins(cfg.enabledChannels);

	M2kDigital *digital = m_digital;
	const QList<int> pins = effective.enabledChannels;
	const bool cyclic = m_cyclic;

	auto *cmd = new M2kCallCommand<void>(m_resource, [digital, pins, cyclic]() {
		// Takes over direction for these pins; see the header's caution about the
		// Digital I/O tool's shadow going stale.
		for(int pin : pins) {
			digital->setDirection(static_cast<DIO_CHANNEL>(pin), DIO_DIRECTION::DIO_OUTPUT);
			digital->enableChannel(static_cast<DIO_CHANNEL>(pin), true);
		}
		digital->setCyclic(cyclic);
	});

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [this](const Error &e) { Q_EMIT streamError(e); });

	if(result) {
		m_config = effective;
		m_buffer.fill(0, static_cast<qsizetype>(effective.samplesCount));
		m_format = stream::bitPackedUint16Format(m_buffer.data(), effective.samplesCount, pins);
		m_open = true;
		Q_EMIT openedChanged(true);
	}
	co_return result;
}

QCoro::Task<CommandResponse<void>> M2kDigitalOutputStream::closeAsync()
{
	if(!m_open) {
		co_return CommandResponse<void>(QUuid());
	}

	m_open = false;
	m_format = component::StreamFormat{};

	M2kDigital *digital = m_digital;
	auto *cmd = new M2kCallCommand<void>(m_resource, [digital]() {
		digital->cancelBufferOut();
		digital->stopBufferOut();
	});

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [this](const Error &e) { Q_EMIT streamError(e); });

	Q_EMIT openedChanged(false);
	co_return result;
}

QCoro::Task<CommandResponse<void>> M2kDigitalOutputStream::pushAsync()
{
	if(!m_open) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EIO, QStringLiteral("stream not open")}});
	}

	M2kDigital *digital = m_digital;
	// Snapshot: the push runs on a worker thread while writeFormat() stays writable.
	auto payload = std::make_shared<std::vector<unsigned short>>(m_buffer.begin(), m_buffer.end());

	auto *cmd = new M2kCallCommand<void>(m_resource, [digital, payload]() { digital->push(*payload); });

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [this](const Error &e) { Q_EMIT pushFailed(e); });

	if(result) {
		Q_EMIT pushSucceeded();
	}
	co_return result;
}
