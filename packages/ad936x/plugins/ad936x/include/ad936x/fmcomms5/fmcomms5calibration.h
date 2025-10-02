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
 *
 */

#ifndef FMCOMMS5CALIBRATION_H
#define FMCOMMS5CALIBRATION_H

#include <QObject>
#include <iio.h>

namespace scopy {
namespace ad936x {

struct MarkerResult
{
	double magnitude;
	double offset;
	int chA;
	int chB;
};

// used devices
#define CAP_DEVICE "cf-ad9361-lpc"
#define CAP_DEVICE_ALT "cf-ad9361-A"
#define CAP_SLAVE_DEVICE "cf-ad9361-B"
#define DDS_DEVICE "cf-ad9361-dds-core-lpc"
#define DDS_SLAVE_DEVICE "cf-ad9361-dds-core-B"

/* 1MHZ tone */
#define CAL_TONE 1000000
#define CAL_SCALE 0.12500
#define MARKER_AVG 3

class Fmcomms5Calibration : public QObject
{
	Q_OBJECT
public:
	explicit Fmcomms5Calibration(iio_context *ctx, QObject *parent = nullptr);

	void calibrate();
	void resetCalibration();
	void callSwitchPortsEnableCb(int val);

Q_SIGNALS:
	void calibrationFailed();
	void updateCalibrationProgress(int progress);

private:
	iio_context *m_ctx;
	iio_device *m_mainDevice = nullptr;
	iio_device *m_secondDevice = nullptr;
	iio_device *m_cf_ad9361_lpc = nullptr;
	iio_device *m_cf_ad9361_hpc = nullptr;
	iio_device *m_ddsMain = nullptr;
	iio_device *m_ddsSecond = nullptr;

	void doCalibbrationInThread();

	const char *ddsChannelNames[8] = {"altvoltage0", "altvoltage1", "altvoltage2", "altvoltage3",
					  "altvoltage4", "altvoltage5", "altvoltage6", "altvoltage7"};

	void calibrationFail(int ret);

	double tuneTrxPhaseOffset(iio_device *ldev, int *ret, long long cal_freq, long long cal_tone, double sign,
				  std::function<void(iio_device *, double)> tune);

	void getMarkers(double *offset, double *mag);
	std::vector<MarkerResult> getMarkersFromCrossCorrelation();
	double calcPhaseOffset(double fsample, double dds_freq, double offset, double mag);
	double scalePhase0360(double val);

	int defaultDds(long long freq, double scale);
	void ddsTxPhaseRotation(struct iio_device *dev, double val);
	void trxPhaseRottation(iio_device *dev, double val);
	unsigned int getCalTone();

	void nearEndLoopbackCtrl(unsigned channel, bool enable);
};
} // namespace ad936x
} // namespace scopy
#endif // FMCOMMS5CALIBRATION_H
