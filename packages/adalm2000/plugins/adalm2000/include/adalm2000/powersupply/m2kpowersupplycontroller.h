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

#include <QObject>
#include <QString>

#include <qcoro/qcorotask.h>

class QTimer;

namespace scopy::component {
class Attribute;
class Context;
} // namespace scopy::component

namespace scopy::adalm2000 {

class M2kPowerSupplyController : public QObject
{
	Q_OBJECT
public:
	M2kPowerSupplyController(component::Context *ctx, QObject *parent = nullptr);
	~M2kPowerSupplyController() override;

	bool isUsable() const;

	component::Attribute *voltageAttribute(int channel) const;

	QCoro::Task<void> initialize();

	QCoro::Task<void> setRailEnabled(int channel, bool on);
	QCoro::Task<void> setRailVoltage(int channel, double volts);

	void startPolling();
	void stopPolling();

	void shutdown();

Q_SIGNALS:
	void railMeasured(int channel, double volts);
	void railEnabledChanged(int channel, bool on);
	void failed(const QString &message);

private:
	QCoro::Task<void> pollOnce();
	bool channelValid(int channel) const;

	component::Context *m_ctx;
	component::Attribute *m_voltage[2] = {nullptr, nullptr};
	component::Attribute *m_enabled[2] = {nullptr, nullptr};
	component::Attribute *m_measured[2] = {nullptr, nullptr};

	QTimer *m_timer = nullptr;
	bool m_polling = false;
	bool m_readFailed = false;
};

} // namespace scopy::adalm2000
