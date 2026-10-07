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

#include "m2kcomponentbuilder.h"

#include "m2kcontext.h"
#include "m2kping.h"

#include "component/device.h"

#include <libm2k/m2k.hpp>

#include <QLoggingCategory>
#include <QString>

Q_LOGGING_CATEGORY(CAT_M2K_BUILDER, "M2kComponentBuilder")

using namespace scopy;
using namespace scopy::adalm2000;

namespace {

component::Device *addDevice(component::Context *ctx, const QString &id)
{
	auto *dev = new component::Device(ctx);
	dev->setId(id);
	dev->setName(id);
	dev->setLabel(id);
	return dev;
}

} // namespace

bool M2kComponentBuilder::build(component::Context *ctx, ICmdExecutor *executor)
{
	auto *m2kCtx = qobject_cast<M2kContext *>(ctx);
	if(!m2kCtx || !m2kCtx->m2k() || !executor) {
		qWarning(CAT_M2K_BUILDER) << "build() called with an incomplete context";
		return false;
	}
	libm2k::context::M2k *m2k = m2kCtx->m2k();

	m2kCtx->setName(QStringLiteral("ADALM2000"));
	try {
		m2kCtx->setDescription(QStringLiteral("ADALM2000 fw %1 serial %2")
					       .arg(QString::fromStdString(m2k->getFirmwareVersion()),
						    QString::fromStdString(m2k->getSerialNumber())));
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "could not read identity:" << e.what();
		m2kCtx->setDescription(QStringLiteral("ADALM2000"));
	}

	new M2kPing(
		m2k, [m2k]() { return m2k->getLed(); }, executor, m2kCtx);

	addDevice(m2kCtx, QStringLiteral("analog-out"));

	return true;
}
