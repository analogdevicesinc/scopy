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
#include "m2kcontext.h"

#include "component/capabilityexecutor.h"

#include <libm2k/m2k.hpp>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_M2KCALIBRATION, "M2kCalibration")

using namespace scopy;
using namespace scopy::adalm2000;

M2kCalibration::M2kCalibration(M2kContext *ctx, QObject *parent)
	: QObject(parent)
	, m_ctx(ctx)
{}

QCoro::Task<bool> M2kCalibration::run()
{
	if(!m_ctx || !m_ctx->m2k() || !m_ctx->executor()) {
		qWarning(CAT_M2KCALIBRATION) << "incomplete context";
		co_return false;
	}

	libm2k::context::M2k *m2k = m_ctx->m2k();
	auto *cmd = new M2kCallCommand<void>(m2k, [m2k]() { m2k->calibrate(); });

	auto r = co_await component::runCommand(
		m_ctx->executor(), cmd, [](const scopy::Result<void> &) {},
		[](const scopy::Error &e) { qWarning(CAT_M2KCALIBRATION) << "calibration failed:" << e.message; });

	co_return bool(r);
}

#include "moc_m2kcalibration.cpp"
