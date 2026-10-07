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

#include "m2kdigitaliocontroller.h"

#include "m2kcallcommand.h"
#include "m2kcontext.h"

#include "component/attribute.h"
#include "component/attributereader.h"
#include "component/attributewriter.h"
#include "component/capabilityexecutor.h"
#include "component/channel.h"
#include "component/device.h"
#include "component/navigation.h"

#include <libm2k/digital/m2kdigital.hpp>
#include <libm2k/m2k.hpp>

#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(CAT_M2KDIO_CONTROLLER, "M2kDigitalIoController")

using namespace scopy;
using namespace scopy::adalm2000;

using libm2k::digital::DIO_CHANNEL;
using libm2k::digital::DIO_DIRECTION;
using libm2k::digital::DIO_LEVEL;

M2kDigitalIoController::M2kDigitalIoController(component::Context *ctx, QObject *parent)
	: QObject(parent)
	, m_ctx(ctx)
{
	m_timer = new QTimer(this);
	// Single-shot, rearmed only once a tick has landed, so a slow device throttles
	// the poll instead of queueing commands behind it.
	m_timer->setSingleShot(true);
	connect(m_timer, &QTimer::timeout, this, [this]() { pollOnce(); });

	if(!m_ctx) {
		qWarning(CAT_M2KDIO_CONTROLLER) << "no context";
		return;
	}

	// Device::setName and Channel::setId are the setters that touch objectName,
	// which findChild matches on.
	auto *dev = m_ctx->findChild<component::Device *>("digital", Qt::FindDirectChildrenOnly);
	if(!dev) {
		qWarning(CAT_M2KDIO_CONTROLLER) << "digital device not found";
		return;
	}

	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		const QString id = QStringLiteral("voltage%1").arg(pin);
		// isOutput = false: all 16 are type="input" at the IIO layer.
		component::Channel *chn = component::channelById(dev, id, /*isOutput=*/false);
		if(!chn) {
			qWarning(CAT_M2KDIO_CONTROLLER) << "digital channel" << id << "not found";
			continue;
		}
		m_pinDirection[pin] = component::attributeByName(chn, "direction");
		m_pinValue[pin] = component::attributeByName(chn, "raw");
		if(!m_pinDirection[pin] || !m_pinValue[pin]) {
			qWarning(CAT_M2KDIO_CONTROLLER) << id << "is missing direction or raw";
		}
	}

	for(int group = 0; group < dio::GROUP_COUNT; ++group) {
		const QString id = QStringLiteral("group%1").arg(group);
		component::Channel *chn = component::channelById(dev, id, /*isOutput=*/false);
		if(!chn) {
			qWarning(CAT_M2KDIO_CONTROLLER) << "digital group" << id << "not found";
			continue;
		}
		m_groupDirection[group] = component::attributeByName(chn, "direction");
		m_groupValue[group] = component::attributeByName(chn, "value");
		if(!m_groupDirection[group] || !m_groupValue[group]) {
			qWarning(CAT_M2KDIO_CONTROLLER) << id << "is missing direction or value";
		}
	}

	m_gpiAttr = component::attributeByName(dev, "gpi");
	m_outputEnabledAttr = component::attributeByName(dev, "output_enabled");
	if(!m_gpiAttr || !m_outputEnabledAttr) {
		qWarning(CAT_M2KDIO_CONTROLLER) << "digital device is missing gpi or output_enabled";
	}
}

M2kDigitalIoController::~M2kDigitalIoController() { stopPolling(); }

bool M2kDigitalIoController::isUsable() const
{
	if(!m_gpiAttr || !m_outputEnabledAttr) {
		return false;
	}
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		if(!m_pinDirection[pin] || !m_pinValue[pin]) {
			return false;
		}
	}
	for(int group = 0; group < dio::GROUP_COUNT; ++group) {
		if(!m_groupDirection[group] || !m_groupValue[group]) {
			return false;
		}
	}
	return true;
}

component::Attribute *M2kDigitalIoController::pinDirection(int pin) const
{
	return dio::validPin(pin) ? m_pinDirection[pin] : nullptr;
}

component::Attribute *M2kDigitalIoController::pinValue(int pin) const
{
	return dio::validPin(pin) ? m_pinValue[pin] : nullptr;
}

component::Attribute *M2kDigitalIoController::groupDirection(int group) const
{
	return dio::validGroup(group) ? m_groupDirection[group] : nullptr;
}

component::Attribute *M2kDigitalIoController::groupValue(int group) const
{
	return dio::validGroup(group) ? m_groupValue[group] : nullptr;
}

dio::PinState M2kDigitalIoController::snapshot() const
{
	dio::PinState s;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		if(m_pinDirection[pin]) {
			s.direction = dio::withBit(s.direction, pin,
						   m_pinDirection[pin]->cachedValue() == QLatin1String("out"));
		}
		if(m_pinValue[pin]) {
			s.gpo = dio::withBit(s.gpo, pin, m_pinValue[pin]->cachedValue().toInt() != 0);
		}
	}
	s.outputEnabled = outputEnabled();
	return s;
}

bool M2kDigitalIoController::outputEnabled() const
{
	return m_outputEnabledAttr && m_outputEnabledAttr->cachedValue().toInt() != 0;
}

QCoro::Task<void> M2kDigitalIoController::initialize()
{
	auto *m2kCtx = qobject_cast<M2kContext *>(m_ctx);
	if(!m2kCtx || !m2kCtx->m2k() || !m2kCtx->executor()) {
		qWarning(CAT_M2KDIO_CONTROLLER) << "incomplete context";
		co_return;
	}

	// One command, so the 32 writes cannot interleave with a UI write. Raw rather
	// than through the attributes: the builder's shadow is already zero, so this is
	// what makes the hardware agree with it.
	libm2k::context::M2k *m2k = m2kCtx->m2k();
	auto *cmd = new M2kCallCommand<void>(m2k, [m2k]() {
		libm2k::digital::M2kDigital *dig = m2k->getDigital();
		for(int i = 0; i < dio::PIN_COUNT; ++i) {
			dig->setDirection(static_cast<DIO_CHANNEL>(i), DIO_DIRECTION::DIO_INPUT);
			dig->setValueRaw(static_cast<DIO_CHANNEL>(i), DIO_LEVEL::LOW);
		}
	});

	auto r = co_await component::runCommand(
		m2kCtx->executor(), cmd, [](const scopy::Result<void> &) {},
		[](const scopy::Error &e) { qWarning(CAT_M2KDIO_CONTROLLER) << "reset failed:" << e.message; });

	if(!r) {
		Q_EMIT failed(QStringLiteral("Digital I/O: could not reset the pins"));
		co_return;
	}

	// Settle every bound widget: the shadow did not change, so
	// Attribute::valueChanged will not fire on its own.
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		if(m_pinDirection[pin] && m_pinDirection[pin]->readCapability()) {
			m_pinDirection[pin]->readCapability()->readAsync();
		}
		if(m_pinValue[pin] && m_pinValue[pin]->readCapability()) {
			m_pinValue[pin]->readCapability()->readAsync();
		}
	}
	for(int group = 0; group < dio::GROUP_COUNT; ++group) {
		if(m_groupDirection[group] && m_groupDirection[group]->readCapability()) {
			m_groupDirection[group]->readCapability()->readAsync();
		}
		if(m_groupValue[group] && m_groupValue[group]->readCapability()) {
			m_groupValue[group]->readCapability()->readAsync();
		}
	}
}

QCoro::Task<bool> M2kDigitalIoController::setOutputEnabled(bool on)
{
	if(!m_outputEnabledAttr || !m_outputEnabledAttr->writeCapability()) {
		Q_EMIT failed(QStringLiteral("Digital I/O: the output enable is not writable"));
		co_return false;
	}

	// The writer flushes every buffered direction and value in one command.
	auto r = co_await m_outputEnabledAttr->writeCapability()->writeAsync(on ? QStringLiteral("1")
										: QStringLiteral("0"));
	if(!r) {
		qWarning(CAT_M2KDIO_CONTROLLER) << "output enable failed:" << r.error().message;
		Q_EMIT failed(QStringLiteral("Digital I/O: could not %1 the outputs")
				      .arg(on ? QStringLiteral("enable") : QStringLiteral("disable")));
		co_return false;
	}

	Q_EMIT outputEnabledChanged(on);
	co_return true;
}

void M2kDigitalIoController::startPolling()
{
	if(m_polling || !m_gpiAttr) {
		return;
	}
	m_polling = true;
	pollOnce();
}

void M2kDigitalIoController::stopPolling()
{
	m_polling = false;
	if(m_timer) {
		m_timer->stop();
	}
}

QCoro::Task<void> M2kDigitalIoController::pollOnce()
{
	if(!m_polling || !m_gpiAttr || !m_gpiAttr->readCapability()) {
		co_return;
	}

	auto resp = co_await m_gpiAttr->readCapability()->readAsync();
	// Stopped while the read was in flight: drop the result rather than rearm.
	if(!m_polling) {
		co_return;
	}

	if(!resp) {
		if(!m_readFailed) { // only on the ok -> failed transition
			m_readFailed = true;
			qWarning(CAT_M2KDIO_CONTROLLER) << "gpi read failed:" << resp.error().message;
			Q_EMIT failed(QStringLiteral("Digital I/O: pin read-back failed"));
		}
		m_timer->start(dio::POLL_INTERVAL_MS);
		co_return;
	}
	m_readFailed = false;

	m_gpi = static_cast<quint16>(QString::fromUtf8(resp.value()).toUInt());

	const dio::PinState state = snapshot();
	quint16 shorted = 0;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		shorted = dio::withBit(shorted, pin, dio::isShorted(m_gpi, state, pin));
	}
	m_shorted = shorted;

	Q_EMIT pinStatesUpdated(m_gpi, m_shorted);

	if(m_polling) {
		m_timer->start(dio::POLL_INTERVAL_MS);
	}
}

void M2kDigitalIoController::shutdown()
{
	stopPolling();

	auto *m2kCtx = qobject_cast<M2kContext *>(m_ctx);
	if(!m2kCtx || !m2kCtx->m2k() || !m2kCtx->executor()) {
		return;
	}

	// All pins back to input: at disconnect they must not be left driving against
	// whatever is wired to them.
	libm2k::context::M2k *m2k = m2kCtx->m2k();
	auto *cmd = new M2kCallCommand<void>(m2k, [m2k]() {
		libm2k::digital::M2kDigital *dig = m2k->getDigital();
		for(int i = 0; i < dio::PIN_COUNT; ++i) {
			dig->setDirection(static_cast<DIO_CHANNEL>(i), DIO_DIRECTION::DIO_INPUT);
		}
	});
	QCoro::waitFor(component::runCommand(
		m2kCtx->executor(), cmd, [](const scopy::Result<void> &) {},
		[](const scopy::Error &e) {
			qWarning(CAT_M2KDIO_CONTROLLER) << "releasing the pins failed:" << e.message;
		}));
}

#include "moc_m2kdigitaliocontroller.cpp"
