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

#include <pluginbase/resourcemanager.h>

namespace scopy::adalm2000 {
class M2kContext;

// Owns ADC arbitration and the Voltmeter's fixed start-up configuration.
class M2kVoltmeterController : public QObject, public scopy::ResourceUser
{
	Q_OBJECT
public:
	M2kVoltmeterController(M2kContext *ctx, const QString &uri, QObject *parent = nullptr);
	~M2kVoltmeterController() override;

	QCoro::Task<bool> claimAndConfigure();
	void release();

	void stop() override;

	QString resourceKey() const { return m_key; }

Q_SIGNALS:
	void resourceLost();

private:
	M2kContext *m_ctx;
	QString m_key;
	bool m_claimed = false;
};

} // namespace scopy::adalm2000
