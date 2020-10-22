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

#ifndef CALIBRATION_HPP
#define CALIBRATION_HPP

#include "apiObject.hpp"

#include <cstdint>
#include <cstdlib>
#include <string>
#include <memory>
#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
	struct iio_buffer;
}

namespace adiscope {

class M2kAdc;
class M2kDac;
class Calibration_API;

class Calibration
{
	friend class Calibration_API;

public:
	enum calibration_mode {
		ADC_REF1,
		ADC_REF2,
		ADC_GND,
		DAC,
		NONE
	};

        enum gain_mode {
                LOW,
                HIGH
        };

	Calibration(struct iio_context *ctx, QJSEngine *engine);
	~Calibration();

	bool initialize();
	bool isInitialized() const;
	bool isCalibrated();

	bool calibrateAll();
	bool calibrateAdc();
	bool calibrateDac();

	bool resetCalibration();
	void updateCorrections();

	double getIioDevTemp(const QString& devName) const;

	void cancelCalibration();
private:

	ApiObject *m_api;
	volatile bool m_cancel;

	struct iio_context *m_ctx;
	libm2k::context::M2k *m_m2k;
	bool m_initialized;
};


} // namespace adiscope

#endif /* CALIBRATION_HPP */
