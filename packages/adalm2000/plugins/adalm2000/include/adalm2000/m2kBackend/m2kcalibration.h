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

#include <functional>

#include <qcoro/qcorotask.h>

namespace scopy {
class ICmdExecutor;
}

namespace scopy::adalm2000 {

class M2kCalibration : public QObject
{
	Q_OBJECT
public:
	using Procedure = std::function<bool()>;

	M2kCalibration(void *resource, Procedure isCalibrated, Procedure calibrate, Procedure resetCalibration,
		       scopy::ICmdExecutor *executor, QObject *parent = nullptr);

	bool isRunning() const { return m_running; }

	QCoro::Task<void> run();

Q_SIGNALS:
	void started();
	void finished(bool ok);

private:
	QCoro::Task<bool> call(Procedure procedure);

	void *m_resource;
	Procedure m_isCalibrated;
	Procedure m_calibrate;
	Procedure m_resetCalibration;
	scopy::ICmdExecutor *m_executor;
	bool m_running = false;
};

} // namespace scopy::adalm2000
