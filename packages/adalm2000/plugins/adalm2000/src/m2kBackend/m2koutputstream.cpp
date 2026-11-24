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

#include "m2koutputstream.h"

#include "m2kcallcommand.h"

#include "component/capabilityexecutor.h"

#include <libm2k/analog/m2kanalogout.hpp>

#include <QLoggingCategory>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <memory>
#include <vector>

Q_LOGGING_CATEGORY(CAT_M2K_OUTPUT_STREAM, "M2kOutputStream")

using namespace scopy;
using namespace scopy::adalm2000;
using namespace libm2k::analog;

static constexpr short PROBE_RAW_LOW = 0;
static constexpr short PROBE_RAW_HIGH = 1000;
static constexpr short PROBE_RAW_VERIFY = -2000;
// convertRawToVolts quantizes to the DAC's 12-bit step (~2.44 mV over 10 V): allow ~1 LSB.
static constexpr double AFFINE_TOLERANCE_V = 0.003;

M2kOutputStream::M2kOutputStream(void *resource, M2kAnalogOut *aout, ICmdExecutor *executor, QObject *parent)
	: component::OutputStream(parent)
	, m_resource(resource)
	, m_aout(aout)
	, m_executor(executor)
{}

M2kOutputStream::~M2kOutputStream()
{
	if(!m_open || !m_aout) {
		return;
	}
	try {
		m_aout->cancelBuffer();
		m_aout->stop();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_OUTPUT_STREAM) << "teardown failed:" << e.what();
	}
}

Result<void> M2kOutputStream::setCyclic(bool enable)
{
	if(m_open) {
		return Result<void>(Unexpected{Error{-EBUSY, QStringLiteral("cannot set cyclic while open")}});
	}
	m_cyclic = enable;
	return Result<void>();
}

Result<void> M2kOutputStream::setKernelBuffers(unsigned n)
{
	if(m_open) {
		return Result<void>(Unexpected{Error{-EBUSY, QStringLiteral("cannot set kernelBuffers while open")}});
	}
	m_kernelBuffers = n;
	return Result<void>();
}

QCoro::Task<CommandResponse<void>> M2kOutputStream::openAsync(const component::StreamConfig &cfg)
{
	if(!m_aout) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-ENODEV, QStringLiteral("stream detached")}});
	}
	if(m_open) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EBUSY, QStringLiteral("already open")}});
	}

	component::StreamConfig effective = cfg;
	effective.samplesCount = stream::alignOutputSamplesCount(cfg.samplesCount, m_cyclic);
	if(effective.samplesCount != cfg.samplesCount) {
		qWarning(CAT_M2K_OUTPUT_STREAM) << "samplesCount" << cfg.samplesCount
						<< "must be a multiple of 4 above 16 (at least 1024 when cyclic); using"
						<< effective.samplesCount;
	}

	// The two DACs are separate devices: channels are independent (base resolve).
	const Result<QList<int>> resolved = resolveForOpen(cfg.enabledChannels);
	if(!resolved) {
		co_return CommandResponse<void>(QUuid(), Unexpected{resolved.error()});
	}
	const QList<int> channels = resolved.value();
	effective.enabledChannels = channels;

	M2kAnalogOut *aout = m_aout;
	const bool cyclic = m_cyclic;
	const unsigned kernelBuffers = m_kernelBuffers;
	auto scales = std::make_shared<QVector<stream::ChannelScale>>();

	// One grouped command: libm2k documents no thread safety (D13).
	auto *cmd = new M2kCallCommand<void>(m_resource, [aout, channels, cyclic, kernelBuffers, scales]() {
		// Drop any buffer left attached by a previous run, as the old signal generator does.
		aout->cancelBuffer();
		for(int i = 0; i < M2kOutputStream::HW_CHANNELS; ++i) {
			aout->enableChannel(static_cast<unsigned>(i), channels.contains(i));
		}

		for(int channel : channels) {
			const unsigned ch = static_cast<unsigned>(channel);
			aout->setCyclic(ch, cyclic);
			aout->setKernelBuffersCount(ch, kernelBuffers);

			// Solve raw -> volts from the DAC's own converter, NOT from
			// getScalingFactor(): that reports raw-codes-per-volt and is
			// negative, the inverse unit of what ChannelFormat::scale means.
			// Keeping one convention across both directions means the DAC's
			// inverted output shows up simply as a negative scale.
			const double low = aout->convertRawToVolts(ch, PROBE_RAW_LOW);
			const double high = aout->convertRawToVolts(ch, PROBE_RAW_HIGH);

			stream::ChannelScale cs;
			cs.scale = (high - low) / static_cast<double>(PROBE_RAW_HIGH - PROBE_RAW_LOW);
			cs.offset = low - cs.scale * static_cast<double>(PROBE_RAW_LOW);

			const double actual = aout->convertRawToVolts(ch, PROBE_RAW_VERIFY);
			const double predicted = cs.scale * static_cast<double>(PROBE_RAW_VERIFY) + cs.offset;
			const double deviation = std::fabs(predicted - actual);
			if(deviation > AFFINE_TOLERANCE_V) {
				qWarning(CAT_M2K_OUTPUT_STREAM)
					<< "convertRawToVolts is NOT affine on DAC channel" << channel << "- deviation"
					<< deviation << "V at raw" << PROBE_RAW_VERIFY
					<< "; voltsToRawCode will mis-encode";
			}
			scales->append(cs);
		}
	});

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [this](const Error &e) { Q_EMIT streamError(e); });

	if(result) {
		m_scales = *scales;
		commitConfig(effective);
		m_buffer.fill(0, static_cast<qsizetype>(effective.samplesCount) * channels.size());
		m_format = stream::packedInt16Format(m_buffer.data(), effective.samplesCount,
						     static_cast<int>(channels.size()), m_scales, channels);
		m_open = true;
		Q_EMIT openedChanged(true);
	}
	co_return result;
}

QCoro::Task<CommandResponse<void>> M2kOutputStream::closeAsync()
{
	if(!m_open) {
		co_return CommandResponse<void>(QUuid());
	}

	// Invalidate before the co_await, which pumps the event loop: cancelBuffer
	// unblocks a push in flight, and nothing may then touch the retired buffer.
	m_open = false;
	m_format = component::StreamFormat{};
	m_cancelRequested = false;
	try {
		m_aout->cancelBuffer();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_OUTPUT_STREAM) << "cancelBuffer failed:" << e.what();
	}

	M2kAnalogOut *aout = m_aout;
	auto *cmd = new M2kCallCommand<void>(m_resource, [aout]() { aout->stop(); });

	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [this](const Error &e) { Q_EMIT streamError(e); });

	Q_EMIT openedChanged(false);
	co_return result;
}

QCoro::Task<CommandResponse<void>> M2kOutputStream::pushAsync()
{
	if(!m_open) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EIO, QStringLiteral("stream not open")}});
	}

	// Decode the raw buffer to volts and push one buffer per DAC (empty when disabled): the
	// pushRaw / pushRawInterleaved paths intermittently leave the DACs idle on hardware, while
	// push(std::vector<std::vector<double>>) -- what the old signal generator uses -- does not.
	const QList<int> channels = m_config.enabledChannels;
	const size_t samples = m_config.samplesCount;
	const qsizetype width = channels.size();
	auto payload = std::make_shared<std::vector<std::vector<double>>>(HW_CHANNELS);
	for(qsizetype k = 0; k < width && k < m_scales.size(); ++k) {
		const stream::ChannelScale &cs = m_scales.at(k);
		std::vector<double> &out = (*payload)[static_cast<size_t>(channels.at(k))];
		out.resize(samples);
		for(size_t i = 0; i < samples; ++i) {
			out[i] = cs.scale * double(m_buffer.at(static_cast<qsizetype>(i) * width + k)) + cs.offset;
		}
	}

	M2kAnalogOut *aout = m_aout;
	auto *cmd = new M2kCallCommand<void>(m_resource, [aout, payload]() { aout->push(*payload); });

	co_return co_await finishPush(cmd);
}

QCoro::Task<CommandResponse<void>> M2kOutputStream::pushAsync(std::vector<std::vector<double>> perDac)
{
	if(!m_open) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EIO, QStringLiteral("stream not open")}});
	}
	if(perDac.size() > static_cast<size_t>(HW_CHANNELS)) {
		co_return CommandResponse<void>(
			QUuid(), Unexpected{Error{-EINVAL, QStringLiteral("more buffers than DAC channels")}});
	}

	std::vector<size_t> lengths;
	QList<int> pushed;
	for(size_t i = 0; i < perDac.size(); ++i) {
		lengths.push_back(perDac[i].size());
		if(perDac[i].empty()) {
			continue;
		}
		if(!m_config.enabledChannels.contains(static_cast<int>(i))) {
			co_return CommandResponse<void>(
				QUuid(),
				Unexpected{Error{-EINVAL, QStringLiteral("DAC %1 was not enabled at open").arg(i)}});
		}
		pushed.append(static_cast<int>(i));
	}
	const QString invalid = stream::validatePerChannelLengths(lengths, m_cyclic);
	if(!invalid.isEmpty()) {
		co_return CommandResponse<void>(QUuid(), Unexpected{Error{-EINVAL, invalid}});
	}

	M2kAnalogOut *aout = m_aout;
	auto payload = std::make_shared<std::vector<std::vector<double>>>(std::move(perDac));

	auto *cmd = new M2kCallCommand<void>(m_resource, [aout, payload, pushed]() {
		if(pushed.size() == 1) {
			const unsigned ch = static_cast<unsigned>(pushed.first());
			aout->push(ch, payload->at(ch));
			return;
		}
		aout->push(*payload);
	});

	co_return co_await finishPush(cmd);
}

QCoro::Task<CommandResponse<void>> M2kOutputStream::finishPush(M2kCallCommand<void> *cmd)
{
	auto result = co_await component::runCommand(
		m_executor, cmd, [](const Result<void> &) {}, [](const Error &) {});

	const bool cancelled = m_cancelRequested || !m_open;
	m_cancelRequested = false;
	if(!result) {
		if(cancelled) {
			const Error error{-ECANCELED, QStringLiteral("push cancelled")};
			Q_EMIT pushFailed(error);
			co_return CommandResponse<void>(result.commandId(), Unexpected{error});
		}
		Q_EMIT pushFailed(result.error());
		co_return result;
	}
	Q_EMIT pushSucceeded();
	co_return result;
}

void M2kOutputStream::detach()
{
	if(m_open && m_aout) {
		try {
			m_aout->stop();
		} catch(const std::exception &e) {
			qWarning(CAT_M2K_OUTPUT_STREAM) << "stop failed:" << e.what();
		}
	}
	m_open = false;
	m_format = component::StreamFormat{};
	m_aout = nullptr;
}

void M2kOutputStream::cancel()
{
	if(!m_open || !m_aout) {
		return;
	}
	m_cancelRequested = true;
	try {
		m_aout->cancelBuffer();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_OUTPUT_STREAM) << "cancelBuffer failed:" << e.what();
	}
}
