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

#include "m2kpowersupplycontroller.h"

#include "m2kcallcommand.h"
#include "m2kcontext.h"
#include "powersupplymath.h"

#include "component/attribute.h"
#include "component/capabilityexecutor.h"
#include "component/channel.h"
#include "component/device.h"
#include "component/navigation.h"

#include <libm2k/analog/m2kpowersupply.hpp>
#include <libm2k/m2k.hpp>

#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(CAT_M2KPS_CONTROLLER, "M2kPowerSupplyController")

using namespace scopy;
using namespace scopy::adalm2000;

M2kPowerSupplyController::M2kPowerSupplyController(component::Context *ctx, QObject *parent)
	: QObject(parent)
	, m_ctx(ctx)
{
	m_timer = new QTimer(this);
	m_timer->setSingleShot(true);
	connect(m_timer, &QTimer::timeout, this, [this]() { pollOnce(); });

	if(!m_ctx) {
		qWarning(CAT_M2KPS_CONTROLLER) << "no context";
		return;
	}

	auto *dev = m_ctx->findChild<component::Device *>("power-supply", Qt::FindDirectChildrenOnly);
	if(!dev) {
		qWarning(CAT_M2KPS_CONTROLLER) << "power-supply device not found";
		return;
	}

	for(int ch = 0; ch < 2; ++ch) {
		const QString id = QStringLiteral("voltage%1").arg(ch);
		component::Channel *chn = component::channelById(dev, id, /*isOutput=*/true);
		if(!chn) {
			qWarning(CAT_M2KPS_CONTROLLER) << "power-supply output channel" << id << "not found";
			continue;
		}
		m_voltage[ch] = component::attributeByName(chn, "voltage");
		m_enabled[ch] = component::attributeByName(chn, "enabled");
		m_measured[ch] = component::attributeByName(chn, "measured");
		if(!m_voltage[ch] || !m_enabled[ch] || !m_measured[ch]) {
			qWarning(CAT_M2KPS_CONTROLLER) << id << "is missing one of voltage/enabled/measured";
		}
	}
}

M2kPowerSupplyController::~M2kPowerSupplyController() { shutdown(); }

bool M2kPowerSupplyController::channelValid(int channel) const
{
	return channel >= 0 && channel < 2 && m_voltage[channel] && m_enabled[channel] && m_measured[channel];
}

bool M2kPowerSupplyController::isUsable() const { return channelValid(0) && channelValid(1); }

component::Attribute *M2kPowerSupplyController::voltageAttribute(int channel) const
{
	return (channel >= 0 && channel < 2) ? m_voltage[channel] : nullptr;
}

QCoro::Task<void> M2kPowerSupplyController::initialize()
{
	if(!isUsable()) {
		Q_EMIT failed(QStringLiteral("Power Supply: the device exposes no usable rails"));
		co_return;
	}

	for(int ch = 0; ch < 2; ++ch) {
		co_await setRailEnabled(ch, false);
	}
	for(int ch = 0; ch < 2; ++ch) {
		co_await setRailVoltage(ch, 0.0);
	}
}

QCoro::Task<void> M2kPowerSupplyController::setRailEnabled(int channel, bool on)
{
	if(!channelValid(channel) || !m_enabled[channel]->writeCapability()) {
		Q_EMIT failed(QStringLiteral("Power Supply: rail %1 cannot be switched").arg(channel + 1));
		co_return;
	}

	auto resp = co_await m_enabled[channel]->writeCapability()->writeAsync(on ? QStringLiteral("1")
										  : QStringLiteral("0"));
	if(!resp) {
		qWarning(CAT_M2KPS_CONTROLLER) << "enable write failed:" << resp.error().message;
		Q_EMIT failed(QStringLiteral("Power Supply: could not %1 %2")
				      .arg(on ? QStringLiteral("enable") : QStringLiteral("disable"),
					   channel == 0 ? QStringLiteral("V+") : QStringLiteral("V-")));
		co_return;
	}
	Q_EMIT railEnabledChanged(channel, on);
}

QCoro::Task<void> M2kPowerSupplyController::setRailVoltage(int channel, double volts)
{
	if(!channelValid(channel) || !m_voltage[channel]->writeCapability()) {
		Q_EMIT failed(QStringLiteral("Power Supply: rail %1 is not writable").arg(channel + 1));
		co_return;
	}

	auto resp = co_await m_voltage[channel]->writeCapability()->writeAsync(QString::number(volts, 'f', 7));
	if(!resp) {
		qWarning(CAT_M2KPS_CONTROLLER) << "voltage write failed:" << resp.error().message;
		Q_EMIT failed(QStringLiteral("Power Supply: could not set %1 to %2 V")
				      .arg(channel == 0 ? QStringLiteral("V+") : QStringLiteral("V-"))
				      .arg(volts, 0, 'f', 3));
	}
}

void M2kPowerSupplyController::startPolling()
{
	if(m_polling || !isUsable()) {
		return;
	}
	m_polling = true;
	pollOnce();
}

void M2kPowerSupplyController::stopPolling()
{
	m_polling = false;
	m_timer->stop();
}

QCoro::Task<void> M2kPowerSupplyController::pollOnce()
{
	for(int ch = 0; ch < 2; ++ch) {
		if(!m_polling) {
			co_return;
		}
		if(!channelValid(ch) || !m_measured[ch]->readCapability()) {
			continue;
		}

		auto resp = co_await m_measured[ch]->readCapability()->readAsync();
		if(!m_polling) {
			co_return;
		}
		if(!resp) {
			if(!m_readFailed) {
				m_readFailed = true;
				qWarning(CAT_M2KPS_CONTROLLER) << "read failed:" << resp.error().message;
				Q_EMIT failed(QStringLiteral("Power Supply: rail read-back failed"));
			}
			continue;
		}
		m_readFailed = false;
		Q_EMIT railMeasured(ch, QString::fromUtf8(resp.value()).toDouble());
	}

	if(m_polling) {
		m_timer->start(ps::POLL_INTERVAL_MS);
	}
}

void M2kPowerSupplyController::shutdown()
{
	stopPolling();

	auto *m2kCtx = qobject_cast<M2kContext *>(m_ctx);
	if(!m2kCtx || !m2kCtx->m2k() || !m2kCtx->executor()) {
		return;
	}

	libm2k::context::M2k *m2k = m2kCtx->m2k();
	auto *cmd = new M2kCallCommand<void>(m2k, [m2k]() {
		libm2k::analog::M2kPowerSupply *supply = m2k->getPowerSupply();
		supply->enableChannel(0, false);
		supply->enableChannel(1, false);
		supply->powerDownDacs(true);
	});
	QCoro::waitFor(component::runCommand(
		m2kCtx->executor(), cmd, [](const scopy::Result<void> &) {},
		[](const scopy::Error &e) { qWarning(CAT_M2KPS_CONTROLLER) << "power-down failed:" << e.message; }));
}

#include "moc_m2kpowersupplycontroller.cpp"
