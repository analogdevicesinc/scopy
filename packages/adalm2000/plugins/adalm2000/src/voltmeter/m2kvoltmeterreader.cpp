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

#include "m2kvoltmeterreader.h"

#include "m2kcallcommand.h"
#include "m2kcontext.h"

#include "component/capabilityexecutor.h"
#include "component/ping.h"

#include <pluginbase/statusbarmanager.h>

#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/m2k.hpp>

#include <QLoggingCategory>
#include <vector>

Q_LOGGING_CATEGORY(CAT_M2KVM_READER, "M2kVoltmeterReader")

using namespace scopy;
using namespace scopy::adalm2000;
using namespace libm2k::analog;

M2kVoltmeterReader::M2kVoltmeterReader(M2kContext *ctx, QObject *parent)
	: QObject(parent)
	, m_ctx(ctx)
{}

M2kVoltmeterReader::~M2kVoltmeterReader() { requestStop(); }

void M2kVoltmeterReader::start()
{
	if(m_running) {
		return;
	}
	m_running = true;
	m_task = acquisitionLoop();
}

void M2kVoltmeterReader::requestStop() { m_running = false; }

void M2kVoltmeterReader::resetChannels()
{
	for(int c = 0; c < 2; ++c) {
		m_dsp[c].reset();
		m_autoGain[c].reset();
	}
}

void M2kVoltmeterReader::setAutoGainEnabled(int channel, bool enabled)
{
	if(channel < 0 || channel > 1) {
		return;
	}
	if(m_autoGainEnabled[channel] == enabled) {
		return;
	}
	m_autoGainEnabled[channel] = enabled;
	if(enabled) {
		m_autoGain[channel].reset();
	}
}

QCoro::Task<void> M2kVoltmeterReader::acquisitionLoop()
{
	if(!m_ctx || !m_ctx->m2k() || !m_ctx->executor()) {
		m_running = false;
		Q_EMIT finished();
		co_return;
	}

	libm2k::context::M2k *m2k = m_ctx->m2k();
	ICmdExecutor *exec = m_ctx->executor();

	auto *ping = m_ctx->findChild<component::Ping *>(QString(), Qt::FindDirectChildrenOnly);
	m_pingWasMonitoring = ping && ping->isMonitoring();
	if(m_pingWasMonitoring) {
		ping->stopMonitoring();
	}

	const unsigned int n = SAMPLES_PER_CHANNEL;

	auto *openCmd = new M2kCallCommand<void>(m2k, [m2k, n]() {
		M2kAnalogIn *ain = m2k->getAnalogIn();
		for(unsigned int c = 0; c < 2; ++c) {
			ain->enableChannel(c, true);
		}
		ain->startAcquisition(n);
	});
	auto opened = co_await component::runCommand(
		exec, openCmd, [](const scopy::Result<void> &) {},
		[](const scopy::Error &e) {
			qWarning(CAT_M2KVM_READER) << "startAcquisition failed:" << e.message;
			StatusBarManager::pushUrgentMessage("Voltmeter: could not start acquisition");
		});

	if(!opened) {
		m_running = false;
		if(m_pingWasMonitoring && ping) {
			ping->startMonitoring();
		}
		Q_EMIT finished();
		co_return;
	}

	while(m_running) {
		auto *refill = new M2kCallCommand<std::vector<short>>(m2k, [m2k, n]() {
			const short *raw = m2k->getAnalogIn()->getSamplesRawInterleaved(n);
			return std::vector<short>(raw, raw + (n * 2));
		});
		auto r = co_await component::runCommand(
			exec, refill, [](const scopy::Result<std::vector<short>> &) {},
			[](const scopy::Error &e) { qDebug(CAT_M2KVM_READER) << "refill error:" << e.message; });

		if(!r) {
			break;
		}

		const std::vector<short> &buf = r.value();
		M2kAnalogIn *ain = m2k->getAnalogIn();
		for(int c = 0; c < 2; ++c) {
			const unsigned int chn = static_cast<unsigned int>(c);
			for(unsigned int s = 0; s < n; ++s) {
				auto u = m_dsp[c].push(buf[(s * 2) + chn]);
				if(!u) {
					continue;
				}

				const double dcV = ain->convertRawToVolts(chn, static_cast<short>(u->dcCount));
				const double acV = ain->convertRawToVolts(chn, static_cast<short>(u->acCount));
				Q_EMIT readingsUpdated(c, dcV, acV);

				if(!m_autoGainEnabled[c]) {
					continue; // a fixed range is in force for this channel
				}
				const double minV = ain->convertRawToVolts(chn, u->blockMin);
				const double maxV = ain->convertRawToVolts(chn, u->blockMax);
				const bool low = m_autoGain[c].push(minV, maxV) == dsp::AutoGain::Range::Plus2_5V;
				Q_EMIT rangeChangeRequested(c, low);
			}
		}
	}

	auto *closeCmd = new M2kCallCommand<void>(m2k, [m2k]() { m2k->getAnalogIn()->stopAcquisition(); });
	co_await component::runCommand(
		exec, closeCmd, [](const scopy::Result<void> &) {},
		[](const scopy::Error &e) { qWarning(CAT_M2KVM_READER) << "stopAcquisition failed:" << e.message; });

	if(m_pingWasMonitoring && ping) {
		ping->startMonitoring();
	}

	m_running = false;
	Q_EMIT finished();
}

#include "moc_m2kvoltmeterreader.cpp"
