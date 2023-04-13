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

#include "calibration.hpp"
#include <libm2k/m2kexceptions.hpp>

#include <errno.h>
#include <QDebug>
#include <QtGlobal>
#include <iio.h>
#include <QThread>

#include "calibration_api.hpp"
#include <QLoggingCategory>
#include "pluginbase/scopyjs.h"

Q_LOGGING_CATEGORY(CAT_M2K_CALIBRATION,"M2KCalibration");

using namespace scopy;

Calibration::Calibration(struct iio_context *ctx):
	m_api(new Calibration_API(this)),
	m_cancel(false),
	m_ctx(ctx),
	m_initialized(false)
{
	m_api->setObjectName("calib");
	ScopyJS::GetInstance()->registerApi(m_api);
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
		qDebug(CAT_M2K_CALIBRATION) << "Rx path is not initialized for calibration.";
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
	try {
		ok = m_m2k->calibrateADC();
	} catch (libm2k::m2k_exception &e) {
		qDebug(CAT_M2K_CALIBRATION) << e.what();
		ok = false;
	}
	if(!ok || m_cancel)
		goto calibration_fail;

	try {
		ok = m_m2k->calibrateDAC();
	} catch (libm2k::m2k_exception &e) {
		qDebug(CAT_M2K_CALIBRATION) << e.what();
		ok = false;
	}
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

bool Calibration::hasContextCalibration() const
{
	return m_m2k->hasContextCalibration();
}

float Calibration::calibrateFromContext()
{
	return m_m2k->calibrateFromContext();
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
