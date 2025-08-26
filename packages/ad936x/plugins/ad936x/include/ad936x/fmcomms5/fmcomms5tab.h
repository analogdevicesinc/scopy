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

#ifndef FMCOMMS5TAB_H
#define FMCOMMS5TAB_H

#include <QWidget>
#include <QBoxLayout>
#include <iio.h>
#include <QCheckBox>
#include <QProgressBar>
#include <QPushButton>

namespace scopy {
namespace ad936x {
class Fmcomms5Tab : public QWidget
{
	Q_OBJECT
public:
	explicit Fmcomms5Tab(iio_context *ctx, QWidget *parent = nullptr);
	~Fmcomms5Tab();

Q_SIGNALS:
	void readRequested();

private:
	iio_context *m_ctx;
	QVBoxLayout *m_layout;
	iio_device *m_device = nullptr;
	iio_device *m_deviceB = nullptr;
	QCheckBox *m_silentCalibration;
	QProgressBar *m_calibProgressBar;
	QPushButton *m_calibrateBtn;

	void initDevices();

	void calibrate();
	void trxPhaseRottation(iio_device *dev, double val);

	void resetCalibration();
	void callSwitchPortsEnableCb(int val);
	double tuneTrxPhaseOffset(iio_device *ldev, int *ret, long long cal_freq, long long cal_tone, double sign,
				  double abort, std::function<void(iio_device *, double)> tune);

	// helper functions TODO MOVE TO CLASS
	double scalePhase0360(double val);
	double calcPhaseOffset(double fsample, double dds_freq, double offset, double mag);
	void getMarkers(double *offset, double *mag);
	int getCalSamples(long long calTone, long long calFreq);
};
} // namespace ad936x
} // namespace scopy
#endif // FMCOMMS5TAB_H
