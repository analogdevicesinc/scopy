/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "adrv9009.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ADRV9009, "Adrv9009")
using namespace scopy::adrv9009;

Adrv9009::Adrv9009(iio_context *ctx, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
{
	if(!m_ctx) {
		qWarning(CAT_ADRV9009) << "No IIO context provided";
		return;
	}

	// Find ADRV9009 PHY device
	m_iio_dev = iio_context_find_device(m_ctx, "adrv9009-phy");
	if(!m_iio_dev) {
		qWarning(CAT_ADRV9009) << "ADRV9009 PHY device not found in context";
		return;
	}

	qDebug(CAT_ADRV9009) << "ADRV9009 tool initialized with device:" << iio_device_get_name(m_iio_dev);
}

Adrv9009::~Adrv9009() {}
