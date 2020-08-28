/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "logging_categories.h"
#include "calibration.hpp"

#include <errno.h>
#include <QDebug>
#include <QtGlobal>
#include <iio.h>
#include <QThread>

#include "calibration_api.hpp"

using namespace adiscope;

Calibration::Calibration(struct iio_context *ctx, QJSEngine *engine):
	m_api(new Calibration_API(this)),
	m_cancel(false),
	m_ctx(ctx),
	m_initialized(false)
{
	m_api->setObjectName("calib");
	m_api->js_register(engine);
}

Calibration::~Calibration()
{
	delete m_api;
}

bool Calibration::initialize()
{
	m_initialized = false;

	if (!m_ctx) {
		return false;
	}

	m_m2k = libm2k::context::m2kOpen(m_ctx, "");
	if (!m_m2k) {
		return false;
	}

	m_initialized = true;

	return m_initialized;
}

bool Calibration::isInitialized() const
{
	return m_initialized;
}

bool Calibration::isCalibrated()
{
	return m_m2k->isCalibrated();
}


bool Calibration::resetCalibration()
{
	if (!m_initialized) {
		qDebug(CAT_CALIBRATION) << "Rx path is not initialized for calibration.";
		return false;
	}

	bool ok = m_m2k->resetCalibration();

	return ok;
}

bool Calibration::calibrateAll()
{
	if (!m_m2k) {
		return false;
	}

	bool ok = false;
	ok = m_m2k->calibrateADC();
	if(!ok || m_cancel)
		goto calibration_fail;
	ok = m_m2k->calibrateDAC();
	if(!ok || m_cancel)
		goto calibration_fail;

	return true;

calibration_fail:
	m_cancel=false;
	return false;
}

bool Calibration::calibrateAdc()
{
	return m_m2k->calibrateADC();
}

bool Calibration::calibrateDac()
{
	return m_m2k->calibrateDAC();
}

void Calibration::cancelCalibration()
{
	m_cancel = true;
}

/* FIXME: TODO: Move this into a HW class / lib M2k */
double Calibration::getIioDevTemp(const QString& devName) const
{
	double temp = -273.15;

	struct iio_device *dev = iio_context_find_device(m_ctx,
		devName.toLatin1().data());

	if (dev) {
		struct iio_channel *chn = iio_device_find_channel(dev, "temp0",
			false);
		if (chn) {
			double offset;
			double raw;
			double scale;

			iio_channel_attr_read_double(chn, "offset", &offset);
			iio_channel_attr_read_double(chn, "raw", &raw);
			iio_channel_attr_read_double(chn, "scale", &scale);

			temp = (raw + offset) * scale / 1000;
		}
	}

	return temp;
}
