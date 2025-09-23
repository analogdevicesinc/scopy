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

#include "fmcomms5/fmcomms5calibration.h"
#include <QLoggingCategory>
#include <cmath>
#include <QtConcurrent>
#include <QThread>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Q_LOGGING_CATEGORY(CAT_FMCOMMS5_CALIBRATION, "CAT_FMCOMMS5_CALIBRATION")

using namespace scopy;
using namespace ad936x;

Fmcomms5Calibration::Fmcomms5Calibration(iio_context *ctx, QObject *parent)
	: m_ctx(ctx)
	, QObject{parent}
{
	m_mainDevice = iio_context_find_device(m_ctx, "ad9361-phy");
	m_secondDevice = iio_context_find_device(m_ctx, "ad9361-phy-B");

	if(!m_mainDevice) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "No ad9361-phy device found in context!";
		return;
	}

	if(!m_secondDevice) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "No ad9361-phy-B device found in context!";
		return;
	}

	// find capture core
	m_cf_ad9361_lpc = iio_context_find_device(m_ctx, CAP_DEVICE_ALT);
	m_cf_ad9361_hpc = iio_context_find_device(m_ctx, CAP_SLAVE_DEVICE);

	if(!m_cf_ad9361_lpc || !m_cf_ad9361_hpc) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "Could not find capture cores";
		return;
	}

	// find DDS main and second devices
	m_ddsMain = iio_context_find_device(m_ctx, DDS_DEVICE);
	m_ddsSecond = iio_context_find_device(m_ctx, DDS_SLAVE_DEVICE);

	if(!m_ddsMain || !m_ddsSecond) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "Could not find dds cores";
		return;
	}
}

void Fmcomms5Calibration::calibrate() { QtConcurrent::run(this, &Fmcomms5Calibration::doCalibbrationInThread); }

void Fmcomms5Calibration::doCalibbrationInThread()
{
	int ret = 0;
	double rx_phase_lpc = 0, rx_phase_hpc = 0, tx_phase_hpc = 0;
	long long cal_tone = 0, cal_freq = 0;

	iio_channel *in0 = iio_device_find_channel(m_mainDevice, "voltage0", false);
	iio_channel *in0B = iio_device_find_channel(m_secondDevice, "voltage0", false);

	if(!in0 || !in0B) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "Could not find channels";
		ret = -ENODEV;
		calibrationFail(ret);
		return;
	}

	if(!m_cf_ad9361_lpc || !m_cf_ad9361_hpc) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "Could not find capture cores";
		ret = -ENODEV;
		calibrationFail(ret);
		return;
	}

	if(!m_ddsMain || !m_ddsSecond) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "Could not find  dds cores";
		ret = -ENODEV;
		calibrationFail(ret);
		return;
	}

	Q_EMIT updateCalibrationProgress(0);

	////////////////set some logical defaults / assumptions ///////////////////
	ret = defaultDds(getCalTone(), CAL_SCALE);

	if(ret < 0) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "Could not set dds cores";
		calibrationFail(ret);
	}
	////////////////////////////////

	// Read calibration tone and frequency
	iio_channel *dds_ch = iio_device_find_channel(m_ddsMain, "altvoltage0", true);
	if(!dds_ch) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "Could not find DDS channel";
		calibrationFail(ret);
		return;
	}
	iio_channel_attr_read_longlong(dds_ch, "frequency", &cal_tone);
	iio_channel_attr_read_longlong(dds_ch, "sampling_frequency", &cal_freq);

	// Turn off quadrature tracking
	iio_channel_attr_write(in0, "quadrature_tracking_en", "0");
	iio_channel_attr_write(in0B, "quadrature_tracking_en", "0");

	// Reset any Tx rotation to zero
	trxPhaseRottation(m_cf_ad9361_lpc, 0.0);
	trxPhaseRottation(m_cf_ad9361_hpc, 0.0);

	Q_EMIT updateCalibrationProgress(16);

	// Calibration sequence
	// 1. Calibrate RX: TX1B_B (HPC) -> RX1C_B (HPC)
	callSwitchPortsEnableCb(1);
	rx_phase_hpc =
		tuneTrxPhaseOffset(m_cf_ad9361_hpc, // cf_ad9361_hpc
				   &ret,	    // Error/status pointer
				   cal_freq,	    // Calibration frequency
				   cal_tone,	    // Calibration tone
				   1.0,		    // Sign
				   [this](iio_device *dev, double phase) { this->trxPhaseRottation(dev, phase); });

	if(ret < 0) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "Failed to tune phase";
		calibrationFail(ret);
		return;
	}

	Q_EMIT updateCalibrationProgress(40);

	// 2. Calibrate RX: TX1B_B (HPC) -> RX1C_A (LPC)
	callSwitchPortsEnableCb(3);
	trxPhaseRottation(m_mainDevice, 0.0);
	rx_phase_lpc =
		tuneTrxPhaseOffset(m_cf_ad9361_lpc, // cf_ad9361_lpc
				   &ret,	    // Error/status pointer
				   cal_freq,	    // Calibration frequency
				   cal_tone,	    // Calibration tone
				   1.0,		    // Sign
				   [this](iio_device *dev, double phase) { this->trxPhaseRottation(dev, phase); });

	if(ret < 0) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "Failed to tune phase";
		calibrationFail(ret);
		return;
	}

	Q_EMIT updateCalibrationProgress(64);

	// 3. Calibrate TX: TX1B_A (LPC) -> RX1C_A (LPC)
	callSwitchPortsEnableCb(4);
	trxPhaseRottation(m_mainDevice, 0.0);
	tx_phase_hpc =
		tuneTrxPhaseOffset(m_ddsSecond, // m_ddsSecond
				   &ret,	// Error/status pointer
				   cal_freq,	// Calibration frequency
				   cal_tone,	// Calibration tone
				   -1.0,	// Sign
				   [this](iio_device *dev, double phase) { this->trxPhaseRottation(dev, phase); });

	if(ret < 0) {
		qWarning(CAT_FMCOMMS5_CALIBRATION) << "Failed to tune phase";
		calibrationFail(ret);
		return;
	}

	Q_EMIT updateCalibrationProgress(88);

	// Restore phase rotation
	trxPhaseRottation(m_cf_ad9361_hpc, rx_phase_hpc);

	// Restore calibration switch matrix to default
	callSwitchPortsEnableCb(0);

	// Re-enable quadrature tracking
	iio_channel_attr_write(in0, "quadrature_tracking_en", "1");
	iio_channel_attr_write(in0B, "quadrature_tracking_en", "1");

	Q_EMIT updateCalibrationProgress(100);
}

void Fmcomms5Calibration::resetCalibration()
{
	iio_channel *in0 = iio_device_find_channel(m_mainDevice, "voltage0", false);
	iio_channel *in0B = iio_device_find_channel(m_secondDevice, "voltage0", false);

	// Reset calibration corrections to zero/default
	iio_channel_attr_write(in0, "calibphase", "0");
	iio_channel_attr_write(in0B, "calibphase", "0");
	iio_channel_attr_write(in0, "calibscale", "0");
	iio_channel_attr_write(in0B, "calibscale", "0");

	// Reset calibration switch matrix as well
	iio_device_attr_write(m_mainDevice, "calibration_switch_control", "0");
}

void Fmcomms5Calibration::calibrationFail(int ret)
{
	// Restore calibration switch matrix to default
	callSwitchPortsEnableCb(0);

	// Re-enable quadrature tracking
	iio_channel *in0 = iio_device_find_channel(m_mainDevice, "voltage0", false);
	iio_channel *in0B = iio_device_find_channel(m_secondDevice, "voltage0", false);
	if(in0)
		iio_channel_attr_write(in0, "quadrature_tracking_en", "1");
	if(in0B)
		iio_channel_attr_write(in0B, "quadrature_tracking_en", "1");
	// Reset progress
	Q_EMIT updateCalibrationProgress(0);

	qWarning(CAT_FMCOMMS5_CALIBRATION) << QString("Calibration failed with error code: %1").arg(ret);
	Q_EMIT calibrationFailed();
}

double Fmcomms5Calibration::tuneTrxPhaseOffset(iio_device *ldev, int *ret, long long cal_freq, long long cal_tone,
					       double sign, std::function<void(iio_device *, double)> tune)
{
	// https://github.com/analogdevicesinc/iio-oscilloscope/blob/7a672e3e3e86aeb4fea2e594acff844010afe6fa/plugins/fmcomms2_adv.c#L755
	double offset = 0.0, mag = 0.0;
	double phase = 0.0, increment = 0.0;

	for(int i = 0; i < 10; i++) {
		getMarkers(&offset, &mag);
		getMarkers(&offset, &mag);

		increment = calcPhaseOffset(cal_freq, cal_tone, offset, mag);
		increment *= sign;

		phase += increment;

		phase = scalePhase0360(phase);
		tune(ldev, phase);

		qDebug(CAT_FMCOMMS5_CALIBRATION) << "Step:" << i << "increment" << increment << "Phase:" << phase;

		if(std::fabs(offset) < 0.001)
			break;
	}

	if(std::fabs(offset) > 0.1)
		*ret = -EFAULT;
	else
		*ret = 0;

	return phase * sign;
}

void Fmcomms5Calibration::getMarkers(double *offset, double *mag)
{
	/// https://github.com/analogdevicesinc/iio-oscilloscope/blob/7a672e3e3e86aeb4fea2e594acff844010afe6fa/plugins/fmcomms2_adv.c#L641
	constexpr int avg_count = MARKER_AVG;
	double acc_offset = 0;
	double acc_mag = 0;

	if(offset)
		*offset = 0;
	if(mag)
		*mag = 0;

	for(int sum = 0; sum < avg_count; ++sum) {
		auto markers = getMarkersFromCrossCorrelation();
		if(!markers.empty()) {
			acc_offset += markers[0].offset;
			acc_mag += markers[0].magnitude;
		}
	}

	if(offset)
		*offset = acc_offset / avg_count;
	if(mag)
		*mag = acc_mag / avg_count;

	qDebug(CAT_FMCOMMS5_CALIBRATION) << "getMarkers: averaged offset =" << *offset << ", mag =" << *mag;
}

std::vector<MarkerResult> Fmcomms5Calibration::getMarkersFromCrossCorrelation()
{
	const char *channel_names[] = {"voltage0", "voltage1", "voltage4", "voltage5"};
	constexpr size_t BUFFER_SAMPLES = 2048;
	std::vector<MarkerResult> results;

	iio_device *dev = m_cf_ad9361_lpc;
	if(!dev) {
		qWarning() << "Device not found!";
		return results;
	}

	// Find and enable channels
	std::vector<iio_channel *> channels;
	for(const char *chname : channel_names) {
		iio_channel *ch = iio_device_find_channel(dev, chname, false);
		if(!ch) {
			qWarning() << "Channel" << chname << "not found!";
			return results;
		}
		iio_channel_enable(ch);
		channels.push_back(ch);
	}

	// Create buffer
	iio_buffer *buf = iio_device_create_buffer(dev, BUFFER_SAMPLES, false);
	if(!buf) {
		qWarning() << "Failed to create buffer:" << strerror(errno) << "(errno:" << errno << ")";
		return results;
	}

	// Refill buffer
	ssize_t nbytes = iio_buffer_refill(buf);
	if(nbytes <= 0) {
		qWarning() << "Buffer refill failed:" << strerror(errno) << "(errno:" << errno << ")";
		iio_buffer_destroy(buf);
		return results;
	}

	// Extract data for each channel
	std::vector<std::vector<double>> data(4, std::vector<double>(BUFFER_SAMPLES));
	for(int c = 0; c < 4; ++c) {
		int16_t *samples = (int16_t *)iio_buffer_first(buf, channels[c]);
		ptrdiff_t step = iio_buffer_step(buf) / sizeof(int16_t);
		for(size_t i = 0; i < BUFFER_SAMPLES; ++i) {
			data[c][i] = (double)samples[i * step];
		}
	}

	// Compute cross-correlation for pairs: (0,1) and (2,3)
	for(int pair = 0; pair < 2; ++pair) {
		const std::vector<double> &dataA = data[pair * 2];
		const std::vector<double> &dataB = data[pair * 2 + 1];
		std::vector<double> xcorr_data(BUFFER_SAMPLES, 0.0);

		for(size_t lag = 0; lag < BUFFER_SAMPLES; ++lag) {
			double sum = 0.0;
			for(size_t n = 0; n < BUFFER_SAMPLES - lag; ++n) {
				sum += dataA[n] * dataB[n + lag];
			}
			xcorr_data[lag] = sum;
		}

		// Find peak in cross-correlation
		double max_val = 0.0;
		size_t max_idx = 0;
		for(size_t j = 0; j < BUFFER_SAMPLES; ++j) {
			double val = std::abs(xcorr_data[j]);
			if(val > max_val) {
				max_val = val;
				max_idx = j;
			}
		}

		double marker_offset = static_cast<double>(max_idx) / BUFFER_SAMPLES;
		double marker_mag = max_val;

		results.push_back({marker_mag, marker_offset, pair * 2, pair * 2 + 1});
		qInfo() << "Marker" << pair << ": magnitude =" << marker_mag << "offset =" << marker_offset
			<< "channels:" << channel_names[pair * 2] << "vs" << channel_names[pair * 2 + 1];
	}

	iio_buffer_destroy(buf);
	return results;
}

double Fmcomms5Calibration::calcPhaseOffset(double fsample, double dds_freq, double offset, double mag)
{
	double val = 360.0 / ((fsample / dds_freq) / offset);

	if(mag < 0)
		val += 180.0;

	return scalePhase0360(val);
}

double Fmcomms5Calibration::scalePhase0360(double val)
{
	if(val >= 360.0)
		val -= 360.0;

	if(val < 0)
		val += 360.0;

	return val;
}

int Fmcomms5Calibration::defaultDds(long long freq, double scale)
{
	int ret = 0;
	iio_device *dds_devs[2] = {m_ddsMain, m_ddsSecond};
	for(int i = 0; i < 2; ++i) {
		iio_device *dev = dds_devs[i];
		if(!dev) {
			qWarning(CAT_FMCOMMS5_CALIBRATION)
				<< "DDS device" << (i == 0 ? DDS_DEVICE : DDS_SLAVE_DEVICE) << "not found!";
			ret |= -ENODEV;
			continue;
		}
		for(int j = 0; j < 8; ++j) {
			iio_channel *ch = iio_device_find_channel(dev, ddsChannelNames[j], true);
			if(!ch) {
				qWarning(CAT_FMCOMMS5_CALIBRATION)
					<< "DDS channel" << ddsChannelNames[j] << "not found on device"
					<< (i == 0 ? DDS_DEVICE : DDS_SLAVE_DEVICE);
				ret |= -ENODEV;
				continue;
			}
			ret |= iio_channel_attr_write_longlong(ch, "frequency", freq);
			ret |= iio_channel_attr_write_double(ch, "scale", scale);
		}
		ddsTxPhaseRotation(dev, 0.0);
		trxPhaseRottation(dev, 0.0);
	}
	return ret;
}

void Fmcomms5Calibration::ddsTxPhaseRotation(iio_device *dev, double val)
{
	// Calculate I and Q phases (scaled by 1000)
	long long i = scalePhase0360(val + 90.0) * 1000;
	long long q = scalePhase0360(val) * 1000;

	// Loop over 8 DDS outputs
	for(int j = 0; j < 8; ++j) {
		iio_channel *ch = iio_device_find_channel(dev, ddsChannelNames[j], true);
		if(!ch)
			continue;

		if(j == 0 || j == 1 || j == 4 || j == 5) {
			iio_channel_attr_write_longlong(ch, "phase", i);
		} else {
			iio_channel_attr_write_longlong(ch, "phase", q);
		}
	}
}

void Fmcomms5Calibration::trxPhaseRottation(iio_device *dev, double val)
{
	double phase = val * 2 * M_PI / 360.0;
	double vcos = std::cos(phase);
	double vsin = std::sin(phase);

	bool output = (dev == m_mainDevice) || (dev == m_secondDevice);

	// Correction factor for output devices
	if(output) {
		double corr = 1.0 /
			std::fmax(std::fabs(std::sin(phase) + std::cos(phase)),
				  std::fabs(std::cos(phase) - std::sin(phase)));
		vcos *= corr;
		vsin *= corr;
	}

	// Set both RX1 and RX2
	for(unsigned offset = 0; offset <= 2; offset += 2) {
		iio_channel *out0 = iio_device_find_channel(dev, offset == 2 ? "voltage2" : "voltage0", output);
		iio_channel *out1 = iio_device_find_channel(dev, offset == 2 ? "voltage3" : "voltage1", output);

		if(out0 && out1) {
			iio_channel_attr_write_double(out0, "calibscale", vcos);
			iio_channel_attr_write_double(out0, "calibphase", -1.0 * vsin);
			iio_channel_attr_write_double(out1, "calibscale", vcos);
			iio_channel_attr_write_double(out1, "calibphase", vsin);
		}
	}
}

unsigned int Fmcomms5Calibration::getCalTone()
{
	unsigned freq;
	const char *cal_tone = getenv("CAL_TONE");

	if(!cal_tone)
		return CAL_TONE;

	freq = atoi(cal_tone);

	if(freq > 0 && freq < 31000000)
		return freq;

	return CAL_TONE;
}

void Fmcomms5Calibration::callSwitchPortsEnableCb(int val)
{
	// Map input value to switch settings
	unsigned lp_slave = 0, lp_master = 0, sw = 0;
	QString tx_port = "A";
	QString rx_port = "A_BALANCED";

	/*
	 *  0 DISABLE
	 *  1 TX1B_B (HPC) -> RX1C_B (HPC) : BIST_LOOPBACK on A
	 *  2 TX1B_A (LPC) -> RX1C_B (HPC) : BIST_LOOPBACK on A
	 *  3 TX1B_B (HPC) -> RX1C_A (LPC) : BIST_LOOPBACK on B
	 *  4 TX1B_A (LPC) -> RX1C_A (LPC) : BIST_LOOPBACK on B
	 *
	 */

	switch(val) {
	default:
	case 0:
		lp_slave = 0;
		lp_master = 0;
		sw = 0;
		tx_port = "A";
		rx_port = "A_BALANCED";
		break;
	case 1:
	case 2:
		lp_slave = 0;
		lp_master = 1;
		sw = val - 1;
		tx_port = "B";
		rx_port = "C_BALANCED";
		break;
	case 3:
	case 4:
		lp_slave = 1;
		lp_master = 0;
		sw = val - 1;
		tx_port = "B";
		rx_port = "C_BALANCED";
		break;
	}

	nearEndLoopbackCtrl(0, lp_slave);  // HPC
	nearEndLoopbackCtrl(1, lp_slave);  // HPC
	nearEndLoopbackCtrl(4, lp_master); // LPC
	nearEndLoopbackCtrl(5, lp_master); // LPC

	// Set calibration switch control
	iio_device_attr_write(m_mainDevice, "calibration_switch_control", QString::number(sw).toUtf8().constData());

	// Set RF port select for RX and TX on master device
	iio_channel *rx_ch = iio_device_find_channel(m_mainDevice, "voltage0", false);
	iio_channel *tx_ch = iio_device_find_channel(m_mainDevice, "voltage0", true);
	if(rx_ch)
		iio_channel_attr_write(rx_ch, "rf_port_select", rx_port.toUtf8().constData());
	if(tx_ch)
		iio_channel_attr_write(tx_ch, "rf_port_select", tx_port.toUtf8().constData());

	// Set RF port select for RX and TX on slave device
	if(m_secondDevice) {
		iio_channel *rx_chB = iio_device_find_channel(m_secondDevice, "voltage0", false);
		iio_channel *tx_chB = iio_device_find_channel(m_secondDevice, "voltage0", true);
		if(rx_chB)
			iio_channel_attr_write(rx_chB, "rf_port_select", rx_port.toUtf8().constData());
		if(tx_chB)
			iio_channel_attr_write(tx_chB, "rf_port_select", tx_port.toUtf8().constData());
	}
}

void Fmcomms5Calibration::nearEndLoopbackCtrl(unsigned int channel, bool enable)
{
	unsigned tmp;
	struct iio_device *dev = (channel > 3) ? m_cf_ad9361_lpc : m_cf_ad9361_hpc;
	if(!dev)
		return;

	if(channel > 3)
		channel -= 4;

	if(iio_device_reg_read(dev, 0x80000418 + channel * 0x40, &tmp))
		return;

	if(enable)
		tmp |= 0x1;
	else
		tmp &= ~0xF;

	iio_device_reg_write(dev, 0x80000418 + channel * 0x40, tmp);
}
