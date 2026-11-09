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

#include "m2kinputstream.h"

#include "m2kcallcommand.h"

#include "component/capabilityexecutor.h"

#include <libm2k/analog/m2kanalogin.hpp>

#include <qcoro/qcorofuture.h>

#include <QLoggingCategory>
#include <QtConcurrent/QtConcurrentRun>

#include <cerrno>
#include <cmath>
#include <memory>

Q_LOGGING_CATEGORY(CAT_M2K_INPUT_STREAM, "M2kInputStream")

using namespace scopy;
using namespace scopy::adalm2000;
using namespace libm2k::analog;

static constexpr short PROBE_RAW_LOW = 0;
static constexpr short PROBE_RAW_HIGH = 1000;
static constexpr short PROBE_RAW_VERIFY = -2000;
static constexpr double AFFINE_TOLERANCE_V = 1e-6;

M2kInputStream::M2kInputStream(void *resource, M2kAnalogIn *ain, ICmdExecutor *executor, QObject *parent)
	: component::InputStream(parent)
	, m_resource(resource)
	, m_ain(ain)
	, m_executor(executor)
{}

M2kInputStream::~M2kInputStream()
{
	m_cancelFuture.waitForFinished();
	if(!m_open || !m_ain) {
		return;
	}
	try {
		m_ain->cancelAcquisition();
		m_ain->stopAcquisition();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_INPUT_STREAM) << "teardown failed:" << e.what();
	}
}

Result<void> M2kInputStream::setKernelBuffers(unsigned n)
{
	if(m_open) {
		return Result<void>(Unexpected{Error{-EBUSY, QStringLiteral("cannot set kernelBuffers while open")}});
	}
	m_kernelBuffers = n;
	return Result<void>();
}

QCoro::Task<CommandResponse<void>> M2kInputStream::openAsync(const component::StreamConfig &cfg)
{
	if(m_open) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EBUSY, QStringLiteral("already open")}});
	}
	co_await awaitCancel();
	if(!m_ain) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-ENODEV, QStringLiteral("stream detached")}});
	}
	if(m_open) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EBUSY, QStringLiteral("already open")}});
	}

	component::StreamConfig effective = cfg;
	effective.samplesCount = stream::alignSamplesCount(cfg.samplesCount);
	if(effective.samplesCount != cfg.samplesCount) {
		qWarning(CAT_M2K_INPUT_STREAM) << "samplesCount" << cfg.samplesCount
					       << "must be a multiple of 4 above 16; using" << effective.samplesCount;
	}
	if(effective.enabledChannels.isEmpty()) {
		effective.enabledChannels = {0, 1};
	}

	M2kAnalogIn *ain = m_ain;
	const QList<int> channels = effective.enabledChannels;
	const unsigned kernelBuffers = m_kernelBuffers;
	const unsigned samples = static_cast<unsigned>(effective.samplesCount);
	auto scales = std::make_shared<QVector<stream::ChannelScale>>();

	// One command: libm2k documents no thread safety, so dependent calls are grouped
	// rather than raced (D13).
	auto *cmd = new M2kCallCommand<void>(m_resource, [ain, channels, kernelBuffers, samples, scales]() {
		for(int i = 0; i < M2kInputStream::HW_CHANNELS; ++i) {
			ain->enableChannel(static_cast<unsigned>(i), channels.contains(i));
		}
		ain->setKernelBuffersCount(kernelBuffers);

		// Solve raw -> volts as an affine map, then prove it on a third point.
		// StreamFormat can only express scale + offset; if the real conversion is
		// not affine this is where we find out.
		for(int channel : channels) {
			const unsigned ch = static_cast<unsigned>(channel);
			const double low = ain->convertRawToVolts(ch, PROBE_RAW_LOW);
			const double high = ain->convertRawToVolts(ch, PROBE_RAW_HIGH);

			stream::ChannelScale cs;
			cs.scale = (high - low) / static_cast<double>(PROBE_RAW_HIGH - PROBE_RAW_LOW);
			cs.offset = low - cs.scale * static_cast<double>(PROBE_RAW_LOW);

			const double actual = ain->convertRawToVolts(ch, PROBE_RAW_VERIFY);
			const double predicted = cs.scale * static_cast<double>(PROBE_RAW_VERIFY) + cs.offset;
			const double deviation = std::fabs(predicted - actual);
			if(deviation > AFFINE_TOLERANCE_V) {
				qWarning(CAT_M2K_INPUT_STREAM)
					<< "convertRawToVolts is NOT affine on channel" << channel << "- deviation"
					<< deviation << "V at raw" << PROBE_RAW_VERIFY
					<< "; StreamView doubles will be wrong, read raw<int16_t>() instead";
			}
			scales->append(cs);
		}

		ain->startAcquisition(samples);
	});

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [this](const Error &e) { Q_EMIT streamError(e); });

	if(result) {
		m_scales = *scales;
		m_config = effective;
		m_stale = false;
		m_open = true;
		Q_EMIT openedChanged(true);
	}
	co_return result;
}

QCoro::Task<CommandResponse<void>> M2kInputStream::closeAsync()
{
	if(!m_open) {
		co_return CommandResponse<void>(QUuid());
	}

	// Invalidate up front: cancelAcquisition unblocks an in-flight refill, and the
	// co_await below pumps the event loop, so a refill completing mid-close must not
	// decode a buffer that is being torn down. Same fencing as IIOInputStream::closeAsync.
	m_open = false;
	m_format = component::StreamFormat{};
	m_cancelRequested = false;
	startCancel();
	co_await awaitCancel();
	if(!m_ain) {
		Q_EMIT openedChanged(false);
		co_return CommandResponse<void>(QUuid());
	}

	M2kAnalogIn *ain = m_ain;
	auto *cmd = new M2kCallCommand<void>(m_resource, [ain]() { ain->stopAcquisition(); });

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [this](const Error &e) { Q_EMIT streamError(e); });

	Q_EMIT openedChanged(false);
	co_return result;
}

QCoro::Task<CommandResponse<void>> M2kInputStream::refillAsync()
{
	if(!m_open) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EIO, QStringLiteral("stream not open")}});
	}
	co_await awaitCancel();
	if(!m_open) {
		co_return CommandResponse<void>(QUuid(),
						Unexpected{Error{-ECANCELED, QStringLiteral("refill cancelled")}});
	}

	M2kAnalogIn *ain = m_ain;
	const unsigned samples = static_cast<unsigned>(m_config.samplesCount);
	const unsigned kernelBuffers = m_kernelBuffers;
	const bool restart = m_stale;
	m_stale = false;
	auto samplePtr = std::make_shared<const short *>(nullptr);

	auto *cmd = new M2kCallCommand<void>(m_resource, [ain, samples, kernelBuffers, restart, samplePtr]() {
		if(restart) {
			ain->stopAcquisition();
			ain->setKernelBuffersCount(kernelBuffers);
			ain->startAcquisition(samples);
		}
		*samplePtr = ain->getSamplesRawInterleaved(samples);
	});

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [](const Error &) {});

	const bool cancelled = m_cancelRequested || !m_open;
	m_cancelRequested = false;
	if(!result) {
		m_stale = true;
		if(cancelled) {
			const Error error{-ECANCELED, QStringLiteral("refill cancelled")};
			Q_EMIT refillFailed(error);
			co_return CommandResponse<void>(result.commandId(), Unexpected{error});
		}
		Q_EMIT refillFailed(result.error());
		co_return result;
	}
	// close() may have run while the refill was in flight.
	if(!m_open || *samplePtr == nullptr) {
		co_return result;
	}

	// Zero-copy: libm2k owns the buffer and keeps it valid until the next
	// acquisition call, so the descriptor only views it.
	m_format = stream::interleavedInt16Format(const_cast<short *>(*samplePtr), m_config.samplesCount, HW_CHANNELS,
						  m_config.enabledChannels, m_scales);
	Q_EMIT refillSucceeded();
	co_return result;
}

void M2kInputStream::invalidate()
{
	if(m_open) {
		m_stale = true;
	}
}

void M2kInputStream::cancel()
{
	if(!m_open) {
		return;
	}
	m_cancelRequested = true;
	m_stale = true;
	startCancel();
}

void M2kInputStream::startCancel()
{
	if(!m_ain || !m_cancelFuture.isFinished()) {
		return;
	}
	M2kAnalogIn *ain = m_ain;
	m_cancelFuture = QtConcurrent::run([ain]() {
		try {
			ain->cancelAcquisition();
		} catch(const std::exception &e) {
			qWarning(CAT_M2K_INPUT_STREAM) << "cancelAcquisition failed:" << e.what();
		}
	});
}

QCoro::Task<void> M2kInputStream::awaitCancel()
{
	QFuture<void> pending = m_cancelFuture;
	if(!pending.isFinished()) {
		co_await pending;
	}
}

void M2kInputStream::detach()
{
	m_cancelFuture.waitForFinished();
	if(m_open && m_ain) {
		try {
			m_ain->stopAcquisition();
		} catch(const std::exception &e) {
			qWarning(CAT_M2K_INPUT_STREAM) << "stopAcquisition failed:" << e.what();
		}
	}
	m_open = false;
	m_format = component::StreamFormat{};
	m_ain = nullptr;
}

QCoro::Task<CommandResponse<short>> M2kInputStream::convertVoltsToRawAsync(unsigned channel, double volts)
{
	if(!m_ain) {
		co_return CommandResponse<short>(QUuid(),
						 Unexpected{Error{-ENODEV, QStringLiteral("stream detached")}});
	}
	if(channel >= static_cast<unsigned>(HW_CHANNELS)) {
		co_return CommandResponse<short>(QUuid(),
						 Unexpected{Error{-EINVAL, QStringLiteral("no such channel")}});
	}
	M2kAnalogIn *ain = m_ain;
	auto *cmd = new M2kCallCommand<short>(
		m_resource, [ain, channel, volts]() { return ain->convertVoltsToRaw(channel, volts); });
	co_return co_await component::runCommand(
		m_executor, cmd, [](const Result<short> &) {}, [](const Error &) {});
}
