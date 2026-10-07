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

#pragma once

#include "digitaliomath.h"

#include <QObject>
#include <QString>

#include <qcoro/qcorotask.h>

// Keep libm2k and device-controller includes in the .cpp: iioutil/command.h and
// core/command.h each declare scopy::Command behind independent include guards,
// and the tool's translation unit already reaches the former.
class QTimer;

namespace scopy::component {
class Attribute;
class Context;
} // namespace scopy::component

namespace scopy::adalm2000 {

// Writes must go through the component::Attribute capabilities: the direction and
// value shadows live in those lambdas, so a raw libm2k call leaves them stale and
// the bound IIOWidgets showing a state the hardware never took.
class M2kDigitalIoController : public QObject
{
	Q_OBJECT
public:
	M2kDigitalIoController(component::Context *ctx, QObject *parent = nullptr);
	~M2kDigitalIoController() override;

	bool isUsable() const;

	component::Attribute *pinDirection(int pin) const;
	component::Attribute *pinValue(int pin) const;
	component::Attribute *groupDirection(int group) const;
	component::Attribute *groupValue(int group) const;

	QCoro::Task<void> initialize();

	QCoro::Task<bool> setOutputEnabled(bool on);
	bool outputEnabled() const;

	quint16 gpi() const { return m_gpi; }
	quint16 shorted() const { return m_shorted; }

	// Independent of Run: inputs are polled whether or not the drivers are enabled.
	void startPolling();
	void stopPolling();

	// Blocks: it runs at teardown, and the pins must not be left driven.
	void shutdown();

Q_SIGNALS:
	void pinStatesUpdated(quint16 gpi, quint16 shorted);
	void outputEnabledChanged(bool on);
	// Emitted only on the ok -> failed transition, not on every failed poll.
	void failed(const QString &message);

private:
	QCoro::Task<void> pollOnce();
	dio::PinState snapshot() const;

	component::Context *m_ctx;
	component::Attribute *m_pinDirection[dio::PIN_COUNT] = {};
	component::Attribute *m_pinValue[dio::PIN_COUNT] = {};
	component::Attribute *m_groupDirection[dio::GROUP_COUNT] = {};
	component::Attribute *m_groupValue[dio::GROUP_COUNT] = {};
	component::Attribute *m_gpiAttr = nullptr;
	component::Attribute *m_outputEnabledAttr = nullptr;

	QTimer *m_timer = nullptr;
	bool m_polling = false;
	bool m_readFailed = false;
	quint16 m_gpi = 0;
	quint16 m_shorted = 0;
};

} // namespace scopy::adalm2000
