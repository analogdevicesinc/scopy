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

#include "fmcomms5/fmcomms5tab.h"

#include <style.h>
#include <iiowidgetbuilder.h>
#include <iiowidgetutils.h>
#include <QLoggingCategory>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CAP_DEVICE "cf-ad9361-lpc"
#define CAP_DEVICE_ALT "cf-ad9361-A"
#define CAP_SLAVE_DEVICE "cf-ad9361-B"
#define DDS_DEVICE "cf-ad9361-dds-core-lpc"
#define DDS_SLAVE_DEVICE "cf-ad9361-dds-core-B"

Q_LOGGING_CATEGORY(CAT_FMCOMMS5_TAB, "FMCOMMS5_TAB")

using namespace scopy;
using namespace ad936x;

Fmcomms5Tab::Fmcomms5Tab(iio_context *ctx, QWidget *parent)
	: m_ctx(ctx)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);

	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);

	m_layout->addWidget(widget);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("CAL Switch Control", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	initDevices();

	QMap<QString, QString> *calSwitchControlOptions = new QMap<QString, QString>();
	calSwitchControlOptions->insert("0", "DISABLE");
	calSwitchControlOptions->insert("1", "TX1B_B->RX1C_B");
	calSwitchControlOptions->insert("2", "TX1B_A->RX1C_B");
	calSwitchControlOptions->insert("3", "TX1B_B->RX1C_A");
	calSwitchControlOptions->insert("4", "TX1B_A->RX1C_A");

	auto values = calSwitchControlOptions->values();
	QString optionasData = "";
	for(int i = 0; i < values.size(); i++) {
		optionasData += " " + values.at(i);
	}

	IIOWidget *calSwitchControl = IIOWidgetBuilder(widget)
					      .device(m_device)
					      .attribute("calibration_switch_control")
					      .uiStrategy(IIOWidgetBuilder::ComboUi)
					      .optionsValues(optionasData)
					      .title("")
					      .buildSingle();
	layout->addWidget(calSwitchControl);

	calSwitchControl->setUItoDataConversion([this, calSwitchControlOptions](QString data) {
		return IIOWidgetUtils::comboUiToDataConversionFunction(data, calSwitchControlOptions);
	});
	calSwitchControl->setDataToUIConversion([this, calSwitchControlOptions](QString data) {
		return IIOWidgetUtils::comboDataToUiConversionFunction(data, calSwitchControlOptions);
	});

	m_calibrateBtn = new QPushButton("Calibrate", this);
	Style::setStyle(m_calibrateBtn, style::properties::button::basicButton);
	connect(m_calibrateBtn, &QPushButton::clicked, this, &Fmcomms5Tab::calibrate);

	QPushButton *resetCalibrationBtn = new QPushButton("Reset Calibration", this);
	Style::setStyle(resetCalibrationBtn, style::properties::button::basicButton);
	connect(resetCalibrationBtn, &QPushButton::clicked, this, &Fmcomms5Tab::resetCalibration);

	QHBoxLayout *calibBtnLayout = new QHBoxLayout();
	calibBtnLayout->setSpacing(10);

	calibBtnLayout->addWidget(m_calibrateBtn);
	calibBtnLayout->addWidget(resetCalibrationBtn);
	layout->addLayout(calibBtnLayout);

	// ??? seems to be related to how calibration affects ui
	// in iio-osc calls  osc_plot_set_visible(plot_xcorr_4ch, false);
	m_silentCalibration = new QCheckBox("Silent Calibration", this);
	layout->addWidget(m_silentCalibration);

	m_calibProgressBar = new QProgressBar(this);
	m_calibProgressBar->setRange(0, 100);
	m_calibProgressBar->setValue(0);
	m_calibProgressBar->setVisible(false);

	connect(m_silentCalibration, &QCheckBox::toggled, m_calibProgressBar, &QProgressBar::setVisible);

	layout->addWidget(m_calibProgressBar);

	// TODO REMOVE
	m_silentCalibration->setChecked(true);

	// TX Phase
	IIOWidget *txPhase = IIOWidgetBuilder(widget)
				     .device(m_device)
				     .attribute("calibration_switch_control")
				     .uiStrategy(IIOWidgetBuilder::RangeUi)
				     .optionsValues("[0 0.1 360]")
				     .title("TX Phase")
				     .buildSingle();
	layout->addWidget(txPhase);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

Fmcomms5Tab::~Fmcomms5Tab() {}

void Fmcomms5Tab::initDevices()
{
	iio_device *plutoDevice = nullptr;
	iio_device *plutoDeviceB = nullptr;
	int foundDevices = 0;
	int device_count = iio_context_get_devices_count(m_ctx);
	for(int i = 0; i < device_count; ++i) {
		iio_device *dev = iio_context_get_device(m_ctx, i);
		const char *dev_name = iio_device_get_name(dev);
		if(dev_name && QString(dev_name).compare("ad9361-phy", Qt::CaseInsensitive) == 0) {
			plutoDevice = dev;
			foundDevices++;
		}

		if(dev_name && QString(dev_name).compare("ad9361-phy-B", Qt::CaseInsensitive) == 0) {
			plutoDeviceB = dev;
			foundDevices++;
		}

		if(foundDevices == 2) {
			break;
		}
	}
	if(plutoDevice == nullptr) {
		qWarning(CAT_FMCOMMS5_TAB) << "No ad9361-phy device found in context!";
		return;
	}

	if(plutoDeviceB == nullptr) {
		qWarning(CAT_FMCOMMS5_TAB) << "No ad9361-phy-B device found in context!";
		return;
	}

	m_device = plutoDevice;
	m_deviceB = plutoDeviceB;
}

void Fmcomms5Tab::calibrate()
{
	// https://github.com/analogdevicesinc/iio-oscilloscope/blob/7a672e3e3e86aeb4fea2e594acff844010afe6fa/plugins/fmcomms2_adv.c#L897C13-L897C23

	m_calibrateBtn->setEnabled(false);
	int ret = 0;
	double rx_phase_lpc = 0, rx_phase_hpc = 0, tx_phase_hpc = 0;
	long long cal_tone = 0, cal_freq = 0;
	int samples = 0;

	iio_channel *in0 = iio_device_find_channel(m_device, "voltage0", false);
	iio_channel *in0B = iio_device_find_channel(m_deviceB, "voltage0", false);

	if(!in0 || !in0B) {
		qWarning(CAT_FMCOMMS5_TAB) << "Could not find channels";
		return;
	}

	// find caputre core
	iio_device *cf_ad9361_lpc = iio_context_find_device(m_ctx, CAP_DEVICE_ALT);
	iio_device *cf_ad9361_hpc = iio_context_find_device(m_ctx, CAP_SLAVE_DEVICE);

	if(!cf_ad9361_lpc || !cf_ad9361_hpc) {
		qWarning(CAT_FMCOMMS5_TAB) << "Could not find capture cores";
		return;
	}

	// Find DDS master/slave devices
	iio_device *dev_dds_master = iio_context_find_device(m_ctx, DDS_DEVICE);
	iio_device *dev_dds_B = iio_context_find_device(m_ctx, DDS_SLAVE_DEVICE);

	if(!dev_dds_master || !dev_dds_B) {
		qWarning(CAT_FMCOMMS5_TAB) << "Could not find  dds cores";
		return;
	}

	m_calibProgressBar->setValue(0);

	// Read calibration tone and frequency
	iio_channel *dds_ch = iio_device_find_channel(dev_dds_master, "altvoltage0", true);
	if(!dds_ch) {
		if(!m_silentCalibration->isChecked())
			qWarning(CAT_FMCOMMS5_TAB) << "Could not find DDS channel";
		return;
	}
	iio_channel_attr_read_longlong(dds_ch, "frequency", &cal_tone);
	iio_channel_attr_read_longlong(dds_ch, "sampling_frequency", &cal_freq);

	samples = getCalSamples(cal_tone, cal_freq); // Implement this helper

	// Turn off quadrature tracking
	iio_channel_attr_write(in0, "quadrature_tracking_en", "0");
	iio_channel_attr_write(in0B, "quadrature_tracking_en", "0");

	// Reset any Tx rotation to zero
	trxPhaseRottation(cf_ad9361_lpc, 0.0);
	trxPhaseRottation(cf_ad9361_hpc, 0.0);

	m_calibProgressBar->setValue(16);

	// Calibration sequence
	// 1. Calibrate RX: TX1B_B (HPC) -> RX1C_B (HPC)
	callSwitchPortsEnableCb(1);
	rx_phase_hpc =
		tuneTrxPhaseOffset(cf_ad9361_hpc, // cf_ad9361_hpc
				   &ret,	  // Error/status pointer
				   cal_freq,	  // Calibration frequency
				   cal_tone,	  // Calibration tone
				   1.0,		  // Sign
				   0.01,	  // Abort threshold
				   [this](iio_device *dev, double phase) { this->trxPhaseRottation(dev, phase); });

	if(ret < 0) {
		qWarning(CAT_FMCOMMS5_TAB) << "Failed to tune phase";
		return;
	}

	m_calibProgressBar->setValue(40);

	// 2. Calibrate RX: TX1B_B (HPC) -> RX1C_A (LPC)
	callSwitchPortsEnableCb(3);
	trxPhaseRottation(m_device, 0.0);
	rx_phase_lpc =
		tuneTrxPhaseOffset(cf_ad9361_lpc, // cf_ad9361_lpc
				   &ret,	  // Error/status pointer
				   cal_freq,	  // Calibration frequency
				   cal_tone,	  // Calibration tone
				   1.0,		  // Sign
				   0.01,	  // Abort threshold
				   [this](iio_device *dev, double phase) { this->trxPhaseRottation(dev, phase); });

	if(ret < 0) {
		qWarning(CAT_FMCOMMS5_TAB) << "Failed to tune phase";
		return;
	}

	m_calibProgressBar->setValue(64);

	// 3. Calibrate TX: TX1B_A (LPC) -> RX1C_A (LPC)
	callSwitchPortsEnableCb(4);
	trxPhaseRottation(m_device, 0.0);
	tx_phase_hpc =
		tuneTrxPhaseOffset(dev_dds_B, // dev_dds_B
				   &ret,      // Error/status pointer
				   cal_freq,  // Calibration frequency
				   cal_tone,  // Calibration tone
				   -1.0,      // Sign
				   0.001,     // Abort threshold
				   [this](iio_device *dev, double phase) { this->trxPhaseRottation(dev, phase); });

	if(ret < 0) {
		qWarning(CAT_FMCOMMS5_TAB) << "Failed to tune phase";
		return;
	}

	m_calibProgressBar->setValue(88);

	// Restore phase rotation
	trxPhaseRottation(cf_ad9361_hpc, rx_phase_hpc);

	// Restore calibration switch matrix to default
	callSwitchPortsEnableCb(0);

	// Re-enable quadrature tracking
	iio_channel_attr_write(in0, "quadrature_tracking_en", "1");
	iio_channel_attr_write(in0B, "quadrature_tracking_en", "1");

	m_calibProgressBar->setValue(100);

	m_calibrateBtn->setEnabled(true);

	//////TODO FIUGRE THIS OUT ?????
	/// calibrate_fail:

	//        osc_plot_xcorr_revert(plot_xcorr_4ch, false);
	// __cal_switch_ports_enable_cb(0);

	// if (in0 && in0_slave) {
	// 	iio_channel_attr_write(in0, "quadrature_tracking_en", "1");
	// 	iio_channel_attr_write(in0_slave, "quadrature_tracking_en", "1");
	// }

	// calib_failed_param = malloc(sizeof(*calib_failed_param));
	// calib_failed_param->button = button;
	// calib_failed_param->ret = ret;
	// gdk_threads_add_idle(calibration_failed_ui, (gpointer) calib_failed_param);

	// /* reset progress bar */
	// gtk_progress_bar_set_fraction(calib_progress, 0.0);
	// gtk_progress_bar_set_text(calib_progress, "Calibration Progress");

	// /* Disable the channels that were enabled at the beginning of the calibration */
	// struct iio_device *iio_dev;
	// iio_dev = iio_context_find_device(get_context_from_osc(), CAP_DEVICE_ALT);
	// if (iio_dev && cap_device_channels_enabled) {
	// 	iio_channels_change_shadow_of_enabled(iio_dev, false);
	// 	cap_device_channels_enabled = false;
	// }
}

void Fmcomms5Tab::trxPhaseRottation(iio_device *dev, double val)
{
	// https://github.com/analogdevicesinc/iio-oscilloscope/blob/7a672e3e3e86aeb4fea2e594acff844010afe6fa/plugins/fmcomms2_adv.c#L526
	double phase = val * 2 * M_PI / 360.0;
	double vcos = std::cos(phase);
	double vsin = std::sin(phase);

	bool output = (dev == m_device) || (dev == m_deviceB);

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

int Fmcomms5Tab::getCalSamples(long long calTone, long long calFreq)
{
	int samples, env_samples;
	/// ????
	const char *cal_samples = getenv("CAL_SAMPLES");

	samples = std::exp2(std::ceil(log2(calFreq / calTone)) + 2);

	if(!cal_samples)
		return samples;

	env_samples = std::atoi(cal_samples);

	if(env_samples < samples)
		return samples;

	return env_samples;
}

void Fmcomms5Tab::resetCalibration()
{
	iio_channel *in0 = iio_device_find_channel(m_device, "voltage0", false);
	iio_channel *in0B = iio_device_find_channel(m_deviceB, "voltage0", false);

	// Reset calibration corrections to zero/default
	iio_channel_attr_write(in0, "calibphase", "0");
	iio_channel_attr_write(in0B, "calibphase", "0");
	iio_channel_attr_write(in0, "calibscale", "0");
	iio_channel_attr_write(in0B, "calibscale", "0");

	// Optionally reset calibration switch matrix as well
	iio_device_attr_write(m_device, "calibration_switch_control", "0");
}

void Fmcomms5Tab::callSwitchPortsEnableCb(int val)
{
	// unsigned lp_slave, lp_master, sw;
	//         char *rx_port, *tx_port;

	//         /*
	//         *  0 DISABLE
	//         *  1 TX1B_B (HPC) -> RX1C_B (HPC) : BIST_LOOPBACK on A
	//         *  2 TX1B_A (LPC) -> RX1C_B (HPC) : BIST_LOOPBACK on A
	//         *  3 TX1B_B (HPC) -> RX1C_A (LPC) : BIST_LOOPBACK on B
	//         *  4 TX1B_A (LPC) -> RX1C_A (LPC) : BIST_LOOPBACK on B
	//         *
	//         */
	//         switch (val) {
	//         default:
	//         case 0:
	//                 lp_slave = 0;
	//                 lp_master = 0;
	//                 sw = 0;
	//                 tx_port = "A";
	//                 rx_port = "A_BALANCED";
	//                 break;
	//         case 1:
	//         case 2:
	//                 lp_slave = 0;
	//                 lp_master = 1;
	//                 sw = val - 1;
	//                 tx_port = "B";
	//                 rx_port = "C_BALANCED";
	//                 break;
	//         case 3:
	//         case 4:
	//                 lp_slave = 1;
	//                 lp_master = 0;
	//                 sw = val - 1;
	//                 tx_port = "B";
	//                 rx_port = "C_BALANCED";
	//                 break;
	//         }

	// #if 0
	//         iio_device_debug_attr_write_bool(dev, "loopback", lp_master);
	//         iio_device_debug_attr_write_bool(dev_slave, "loopback", lp_slave);
	// #else
	//         near_end_loopback_ctrl(0, lp_slave); /* HPC */
	//         near_end_loopback_ctrl(1, lp_slave); /* HPC */

	//         near_end_loopback_ctrl(4, lp_master); /* LPC */
	//         near_end_loopback_ctrl(5, lp_master); /* LPC */
	// #endif
	//         iio_device_debug_attr_write_longlong(dev, "calibration_switch_control", sw);
	//         iio_channel_attr_write(iio_device_find_channel(dev, "voltage0", false),
	//                                "rf_port_select", rx_port);
	//         iio_channel_attr_write(iio_device_find_channel(dev, "voltage0", true),
	//                                "rf_port_select", tx_port);

	//         if (dev_slave) {
	//                 iio_channel_attr_write(iio_device_find_channel(dev_slave, "voltage0", false),
	//                                 "rf_port_select", rx_port);
	//                 iio_channel_attr_write(iio_device_find_channel(dev_slave, "voltage0", true),
	//                                 "rf_port_select", tx_port);
	//         }
}

double Fmcomms5Tab::tuneTrxPhaseOffset(iio_device *ldev, int *ret, long long cal_freq, long long cal_tone, double sign,
				       double abort, std::function<void(iio_device *, double)> tune)
{
	// https://github.com/analogdevicesinc/iio-oscilloscope/blob/7a672e3e3e86aeb4fea2e594acff844010afe6fa/plugins/fmcomms2_adv.c#L755
	double offset = 0.0, mag = 0.0;
	double phase = 0.0, increment = 0.0;

	for(int i = 0; i < 10; i++) {
		getMarkers(&offset, &mag); // Implement this helper
		getMarkers(&offset, &mag);

		increment = calcPhaseOffset(cal_freq, cal_tone, offset, mag); // Implement this helper
		increment *= sign;

		phase += increment;

		phase = scalePhase0360(phase); // Implement this helper
		tune(ldev, phase);

		qDebug() << "Step:" << i << "increment" << increment << "Phase:" << phase;

		if(std::fabs(offset) < 0.001)
			break;
	}

	if(std::fabs(offset) > 0.1)
		*ret = -EFAULT;
	else
		*ret = 0;

	return phase * sign;
}

void Fmcomms5Tab::getMarkers(double *offset, double *mag)
{
	/// ????
	/// https://github.com/analogdevicesinc/iio-oscilloscope/blob/7a672e3e3e86aeb4fea2e594acff844010afe6fa/plugins/fmcomms2_adv.c#L641
}

double Fmcomms5Tab::scalePhase0360(double val)
{
	if(val >= 360.0)
		val -= 360.0;

	if(val < 0)
		val += 360.0;

	return val;
}

double Fmcomms5Tab::calcPhaseOffset(double fsample, double dds_freq, double offset, double mag)
{
	double val = 360.0 / ((fsample / dds_freq) / offset);

	if(mag < 0)
		val += 180.0;

	return scalePhase0360(val);
}
