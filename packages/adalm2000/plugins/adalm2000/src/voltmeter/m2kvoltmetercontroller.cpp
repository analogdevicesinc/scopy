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

#include "m2kvoltmetercontroller.h"

#include "m2kcallcommand.h"
#include "m2kcontext.h"
#include "m2ktrace.h"
#include "voltmeterdsp.h"

#include "component/capabilityexecutor.h"

#include <component/commandmonitor.h>

#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/m2khardwaretrigger.hpp>

#include <QLoggingCategory>
#include <QPointer>

Q_LOGGING_CATEGORY(CAT_M2KVM_CONTROLLER, "M2kVoltmeterController")

using namespace scopy;
using namespace scopy::adalm2000;
using namespace libm2k::analog;

M2kVoltmeterController::M2kVoltmeterController(M2kContext *ctx, const QString &uri, QObject *parent)
	: QObject(parent)
	, m_ctx(ctx)
	, m_key(QStringLiteral("adalm2000-adc:") + uri)
{}

M2kVoltmeterController::~M2kVoltmeterController() { release(); }

QCoro::Task<bool> M2kVoltmeterController::claimAndConfigure()
{
	QPointer<M2kVoltmeterController> self(this);
	if(!m_ctx || !m_ctx->m2k() || !m_ctx->executor()) {
		qWarning(CAT_M2KVM_CONTROLLER) << "incomplete context";
		co_return false;
	}

	if(!ResourceManager::open(m_key, this, true)) {
		qWarning(CAT_M2KVM_CONTROLLER) << "could not claim" << m_key;
		co_return false;
	}
	m_claimed = true;

	libm2k::context::M2k *m2k = m_ctx->m2k();

	auto *cmd = new M2kCallCommand<void>(m2k, [m2k]() {
		M2kAnalogIn *ain = M2K_CALL(m2k, "M2k::getAnalogIn", m2k->getAnalogIn());
		M2K_CALL(ain, "M2kAnalogIn::setKernelBuffersCount", ain->setKernelBuffersCount(4),
			 M2K_ARGN("count", 4));
		M2K_CALL(ain, "M2kAnalogIn::setSampleRate", ain->setSampleRate(dsp::SAMPLE_RATE_HZ),
			 M2K_ARGN("samplerate", dsp::SAMPLE_RATE_HZ));
		M2K_CALL(ain, "M2kAnalogIn::setOversamplingRatio", ain->setOversamplingRatio(1),
			 M2K_ARGN("oversampling", 1));
		libm2k::M2kHardwareTrigger *trigger = M2K_CALL(ain, "M2kAnalogIn::getTrigger", ain->getTrigger());
		for(unsigned int c = 0; c < 2; ++c) {
			const ANALOG_IN_CHANNEL channel = static_cast<ANALOG_IN_CHANNEL>(c);
			M2K_CALL(ain, "M2kAnalogIn::setVerticalOffset", ain->setVerticalOffset(channel, 0.0),
				 M2K_ARG(channel), M2K_ARGN("vertOffset", 0.0));
			if(trigger) {
				M2K_CALL(trigger, "M2kHardwareTrigger::setAnalogMode",
					 trigger->setAnalogMode(c, libm2k::ALWAYS), M2K_ARGN("chnIdx", c),
					 M2K_ARGN("mode", libm2k::ALWAYS));
			}
		}
	});
	component::CommandMonitor::tag(cmd, m_ctx);

	auto r = co_await component::runCommand(
		m_ctx->executor(), cmd, [](const scopy::Result<void> &) {},
		[](const scopy::Error &e) { qWarning(CAT_M2KVM_CONTROLLER) << "setup failed:" << e.message; });
	if(!self) {
		co_return false;
	}

	if(!r) {
		release();
		co_return false;
	}
	co_return true;
}

void M2kVoltmeterController::release()
{
	if(!m_claimed) {
		return;
	}
	m_claimed = false;

	if(m_ctx && m_ctx->m2k() && m_ctx->executor() && !m_ctx->connectionLost()) {
		libm2k::context::M2k *m2k = m_ctx->m2k();
		auto *cmd = new M2kCallCommand<void>(m2k, [m2k]() {
			M2kAnalogIn *ain = M2K_CALL(m2k, "M2k::getAnalogIn", m2k->getAnalogIn());
			M2K_CALL(ain, "M2kAnalogIn::setKernelBuffersCount", ain->setKernelBuffersCount(1),
				 M2K_ARGN("count", 1));
		});
		component::CommandMonitor::tag(cmd, m_ctx);
		// Unawaited: disconnect must not wait on device I/O; the view drains it before closing.
		component::runCommand(
			m_ctx->executor(), cmd, [](const scopy::Result<void> &) {},
			[](const scopy::Error &e) {
				qWarning(CAT_M2KVM_CONTROLLER) << "buffer-count restore failed:" << e.message;
			});
	}

	ResourceManager::close(m_key);
}

void M2kVoltmeterController::stop()
{
	release();
	Q_EMIT resourceLost();
}

#include "moc_m2kvoltmetercontroller.cpp"
