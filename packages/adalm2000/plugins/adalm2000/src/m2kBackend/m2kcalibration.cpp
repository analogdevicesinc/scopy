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

#include "m2kcalibration.h"

#include "m2kcallcommand.h"

#include "component/capabilityexecutor.h"

#include <QLoggingCategory>

#include <utility>

Q_LOGGING_CATEGORY(CAT_M2KCALIBRATION, "M2kCalibration")

using namespace scopy;
using namespace scopy::adalm2000;

M2kCalibration::M2kCalibration(void *resource, Procedure isCalibrated, Procedure calibrate, Procedure resetCalibration,
			       scopy::ICmdExecutor *executor, QObject *parent)
	: QObject(parent)
	, m_resource(resource)
	, m_isCalibrated(std::move(isCalibrated))
	, m_calibrate(std::move(calibrate))
	, m_resetCalibration(std::move(resetCalibration))
	, m_executor(executor)
{}

QCoro::Task<void> M2kCalibration::run()
{
	if(m_running) {
		qWarning(CAT_M2KCALIBRATION) << "calibration already in progress; request refused";
		co_return;
	}
	if(!m_executor || !m_isCalibrated || !m_calibrate) {
		qWarning(CAT_M2KCALIBRATION) << "incomplete calibration setup";
		Q_EMIT finished(false);
		co_return;
	}

	m_running = true;
	Q_EMIT started();

	if(co_await call(m_isCalibrated)) {
		m_running = false;
		Q_EMIT finished(true);
		co_return;
	}

	const bool ok = co_await call(m_calibrate);
	if(!ok && m_resetCalibration) {
		co_await call(m_resetCalibration);
	}

	m_running = false;
	Q_EMIT finished(ok);
}

QCoro::Task<bool> M2kCalibration::call(Procedure procedure)
{
	auto *cmd = new M2kCallCommand<bool>(m_resource, std::move(procedure));
	const auto r = co_await component::runCommand(
		m_executor, cmd, [](const scopy::Result<bool> &) {},
		[](const scopy::Error &e) { qWarning(CAT_M2KCALIBRATION) << e.message; });
	co_return bool(r) && r.value();
}

#include "moc_m2kcalibration.cpp"
