/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/* GNU Radio includes */
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/add_ff.h>
#include <gnuradio/iio/math.h>
#include <gnuradio/analog/sig_source_f.h>
#include <gnuradio/analog/fastnoise_source_f.h>

/* Qt includes */
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

/* Local includes */
#include "spectrum_analyzer.hpp"
#include "filter.hpp"
#include "math.hpp"
#include "fft_block.hpp"
#include "adc_sample_conv.hpp"
#include "dynamicWidget.hpp"
#include "hardware_trigger.hpp"

/* Generated UI */
#include "ui_spectrum_analyzer.h"
#include "ui_channel.h"

#include <boost/make_shared.hpp>
#include <iio.h>
#include <iostream>

using namespace adiscope;
using namespace std;

// To fix: the value is temporary. FTT size will depend on other tool settings
#define FFT_SIZE 32768

std::vector<std::pair<QString, FftDisplayPlot::AverageType>>
SpectrumAnalyzer::avg_types = {
	{"Sample", FftDisplayPlot::SAMPLE},
	{"Peak Hold", FftDisplayPlot::PEAK_HOLD},
	{"Peak Hold Continous", FftDisplayPlot::PEAK_HOLD_CONTINUOUS},
	{"Min Hold", FftDisplayPlot::MIN_HOLD},
	{"Min Hold Continous", FftDisplayPlot::MIN_HOLD_CONTINUOUS},
	{"Linear RMS", FftDisplayPlot::LINEAR_RMS},
	{"Linear dB", FftDisplayPlot::LINEAR_DB},
	{"Exponential RMS", FftDisplayPlot::EXPONENTIAL_RMS},
	{"Exponential dB", FftDisplayPlot::EXPONENTIAL_DB},
};

std::vector<std::pair<QString, SpectrumAnalyzer::FftWinType>>
SpectrumAnalyzer::win_types = {
	{"Flat top", FftWinType::FLAT_TOP},
	{"Rectangular", FftWinType::RECTANGULAR},
	{"Triangular (Bartlett)", FftWinType::TRIANGULAR},
	{"Hamming", FftWinType::HAMMING},
	{"Hann", FftWinType::HANN},
	{"Blackman-Harris", FftWinType::BLACKMAN_HARRIS},
	{"Kaiser", FftWinType::KAISER},
};

SpectrumAnalyzer::SpectrumAnalyzer(struct iio_context *ctx, Filter *filt,
	std::shared_ptr<GenericAdc> adc, QPushButton *runButton,
	ToolLauncher *parent):
	Tool(ctx, runButton, new SpectrumAnalyzer_API(this), "Spectrum Analyzer", parent),
	ui(new Ui::SpectrumAnalyzer),
	fft_plot(nullptr),
	settings_group(new QButtonGroup(this)),
	channels_group(new QButtonGroup(this)),
	adc(adc),
	adc_name(ctx ? filt->device_name(TOOL_SPECTRUM_ANALYZER) : ""),
	crt_channel_id(0),
	crt_peak(0),
	crt_marker(-1),
	max_peak_count(10),
	bin_sizes({256, 512, 1024, 2048, 4096, 8192, 16384, 32768})
{

	// Get the list of names of the available channels
	QList<QString> channel_names;
	if (ctx) {
		iio = iio_manager::get_instance(ctx, adc_name);
		num_adc_channels = adc->numAdcChannels();
		adc_bits_count = adc->numAdcBits();

		auto adc_channels = adc->adcChannelList();
		for (unsigned int i = 0; i < adc_channels.size(); i++) {
			const char *id = iio_channel_get_name(adc_channels[i]);
			if (!id) {
				channel_names.push_back(
				QString("Channel %1").arg(i + 1));
			} else {
				channel_names.push_back(QString(id));
			}
		}
	} else {
		num_adc_channels = 2;
		adc_bits_count = 12;
		for (int i = 0; i < num_adc_channels; i++)
			channel_names.push_back(
				QString("Channel %1").arg(i + 1));
	}

	ui->setupUi(this);
	// Temporarily disable the delta marker button
	ui->pushButton_4->hide();

	// Hide general settings and current settings for now
	ui->btnToolSettings->hide();
	ui->btnSettings->hide();

	// Hide Single and Preset buttons until functionality is added
	ui->btnSingle->hide();
	ui->btnPreset->hide();

	ui->comboBox_type->blockSignals(true);
	ui->comboBox_type->clear();
	for (auto it = avg_types.begin(); it != avg_types.end(); ++it) {
		ui->comboBox_type->addItem(it->first);
	}
	ui->comboBox_type->blockSignals(false);

	ui->comboBox_window->blockSignals(true);
	ui->comboBox_window->clear();
	for (auto it = win_types.begin(); it != win_types.end(); ++it) {
		ui->comboBox_window->addItem(it->first);
	}
	ui->comboBox_window->blockSignals(false);

	settings_group->addButton(ui->btnToolSettings);
	settings_group->addButton(ui->btnSettings);
	settings_group->addButton(ui->btnSweep);
	settings_group->addButton(ui->btnMarkers);
	settings_group->setExclusive(true);

	fft_plot = new FftDisplayPlot(num_adc_channels, this);
	fft_plot->disableLegend();
	fft_plot->setAxisScale(QwtPlot::yLeft, -200, 0, 10);
	fft_plot->setLeftVertAxisUnit(ui->cmb_units->currentText());
	// Disable mouse interactions with the axes until they are in a working state
	fft_plot->setXaxisMouseGesturesEnabled(false);
	for (uint i = 0; i < num_adc_channels; i++)
		fft_plot->setYaxisMouseGesturesEnabled(i, false);

	QGridLayout *gLayout = static_cast<QGridLayout *>
		(ui->widgetPlotContainer->layout());
	gLayout->addWidget(fft_plot, 1, 0, 1, 1);

	// Initialize spectrum channels
	for (int i = 0 ; i < num_adc_channels; i++) {
		channel_sptr channel = boost::make_shared<SpectrumChannel>(i,
			channel_names[i], fft_plot);
		channel->setColor(fft_plot->getLineColor(i));
		ui->channelsList->addWidget(channel->m_widget);
		channels.push_back(channel);

		settings_group->addButton(channel->m_ui->btn);
		channels_group->addButton(channel->m_ui->name);

		connect(channel.get(), SIGNAL(settingsToggled(bool)), this,
			SLOT(onChannelSettingsToggled(bool)));
		connect(channel.get(), SIGNAL(selected(bool)), this,
			SLOT(onChannelSelected(bool)));
		connect(channel.get(), SIGNAL(enabled(bool)), this,
			SLOT(onChannelEnabled(bool)));
	}

	if (num_adc_channels > 0)
		channels[crt_channel_id]->m_ui->name->setChecked(true);

	// Initialize Sweep controls
	double max_sr = 50e6; // TO DO: adc should detect max sampl rate and use that
	ui->start_freq->setMaxValue(max_sr);
	ui->stop_freq->setMaxValue(max_sr);
	ui->center_freq->setMaxValue(max_sr);
	ui->span_freq->setMaxValue(max_sr);

	ui->start_freq->setStep(1e6);
	ui->stop_freq->setStep(1e6);
	ui->center_freq->setStep(1e6);
	ui->span_freq->setStep(1e6);

	// Initialize Marker controls
	mrk_buttons.push_back(ui->mrk1);
	mrk_buttons.push_back(ui->mrk2);
	mrk_buttons.push_back(ui->mrk3);
	mrk_buttons.push_back(ui->mrk4);
	mrk_buttons.push_back(ui->mrk5);
	for (int i = 0; i < mrk_buttons.size(); i++) {
		QPushButton *btn = mrk_buttons[i];
		btn->setProperty("id", QVariant(i));
		connect(btn, SIGNAL(toggled(bool)), this,
			SLOT(onMarkerToggled(bool)));
	}

	// Configure plot peak capabilities
	for (uint i = 0; i < num_adc_channels; i++) {
		fft_plot->setPeakCount(i, max_peak_count);
	}

	// Configure markers
	for (int i = 0; i < num_adc_channels; i++) {
		fft_plot->setMarkerCount(i, 5);
		for (int m = 0; m < 5; m++) {
			fft_plot->setMarkerEnabled(i, m, false);
		}
	}
	setActiveMarker(0);

	connect(fft_plot, SIGNAL(newMarkerData()),
		this, SLOT(onPlotNewMarkerData()));


	if (ctx)
		build_gnuradio_block_chain();
	else
		build_gnuradio_block_chain_no_ctx();

	connect(ui->run_button, SIGNAL(toggled(bool)), this,
		SLOT(runStopToggled(bool)));
	connect(ui->run_button, SIGNAL(toggled(bool)), runButton,
			SLOT(setChecked(bool)));
	connect(run_button, SIGNAL(toggled(bool)), ui->run_button,
		SLOT(setChecked(bool)));

	connect(ui->start_freq, SIGNAL(valueChanged(double)),
		this, SLOT(onStartStopChanged()));
	connect(ui->stop_freq, SIGNAL(valueChanged(double)),
		this, SLOT(onStartStopChanged()));
	connect(ui->center_freq, SIGNAL(valueChanged(double)),
		this, SLOT(onCenterSpanChanged()));
	connect(ui->span_freq, SIGNAL(valueChanged(double)),
		this, SLOT(onCenterSpanChanged()));

	// UI default
	ui->comboBox_window->setCurrentText("Hamming");
	ui->stackedWidget->setVisible(false);
	ui->start_freq->setValue(0);
	ui->stop_freq->setValue(50e6);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
	if (iio) {
		for (unsigned int i = 0; i < num_adc_channels; i++)
			iio->stop(fft_ids[i]);

		bool started = iio->started();
		if (started)
			iio->lock();

		for (unsigned int i = 0; i < num_adc_channels; i++)
			iio->disconnect(fft_ids[i]);

		if (started)
			iio->unlock();

		delete[] fft_ids;
	}

	delete ui;
}

void SpectrumAnalyzer::on_btnToolSettings_toggled(bool checked)
{
	ui->stackedWidget->setCurrentWidget(ui->generalSettings);
	ui->stackedWidget->setVisible(checked);
}

void SpectrumAnalyzer::on_btnSettings_pressed()
{
	QPushButton *btn = static_cast<QPushButton *>(QObject::sender());
	bool btnStateToBe = !btn->isChecked();
	settings_group->setExclusive(btnStateToBe);

	ui->stackedWidget->setCurrentWidget(ui->channelSettings);
	ui->stackedWidget->setVisible(btnStateToBe);
}

void SpectrumAnalyzer::on_btnSweep_toggled(bool checked)
{
	ui->stackedWidget->setCurrentWidget(ui->sweepSettings);
	ui->stackedWidget->setVisible(checked);
}

void SpectrumAnalyzer::on_btnMarkers_toggled(bool checked)
{
	ui->stackedWidget->setCurrentWidget(ui->markerSettings);
	ui->stackedWidget->setVisible(checked);
}

void SpectrumAnalyzer::runStopToggled(bool checked)
{
	if (checked) {
		if (iio) {
			writeAllSettingsToHardware();
		}

		fft_plot->presetSampleRate(sample_rate);
		fft_sink->set_samp_rate(sample_rate);
		ui->run_button->setText("Stop");
		start_blockchain_flow();
	} else {
		ui->run_button->setText("Run");
		stop_blockchain_flow();
	}

	if (!checked)
		fft_plot->resetAverageHistory();
}

void SpectrumAnalyzer::build_gnuradio_block_chain()
{
	// TO DO: don't use the 100e6 hardcoded value anymore
	fft_sink = adiscope::scope_sink_f::make(FFT_SIZE, 100e6,
			"Osc Frequency", num_adc_channels,
			(QObject *)fft_plot);
	fft_sink->set_trigger_mode(TRIG_MODE_TAG, 0, "buffer_start");

	bool started = iio->started();
	if (started)
		iio->lock();

	bool canConvRawToVolts = (adc_name == "m2k-adc");

	boost::shared_ptr<adc_sample_conv> adc_samp_conv;
	if (canConvRawToVolts) {
		adc_samp_conv = gnuradio::get_initial_sptr(
			new adc_sample_conv(num_adc_channels));
		auto m2k_adc = dynamic_pointer_cast<M2kAdc>(adc);
		for (int i = 0; i < adc->numAdcChannels(); i++) {
			double corr_gain = 1.0;

			if (m2k_adc)
				corr_gain = m2k_adc->chnCorrectionGain(i);

			adc_samp_conv->setCorrectionGain(i, corr_gain);
		}
	}

	fft_ids = new iio_manager::port_id[num_adc_channels];

	for (int i = 0; i < num_adc_channels; i++) {
		auto fft = gnuradio::get_initial_sptr(
				new fft_block(false, FFT_SIZE));
		auto ctm = gr::blocks::complex_to_mag_squared::make(1);

		// iio(i)->fft->ctm->fft_sink
		fft_ids[i] = iio->connect(fft, i, 0, true, FFT_SIZE);
		iio->connect(fft, 0, ctm, 0);
		iio->connect(ctm, 0, fft_sink, i);

		channels[i]->fft_block = fft;
		channels[i]->ctm_block = ctm;
	}

	if (started)
		iio->unlock();
}

void SpectrumAnalyzer::build_gnuradio_block_chain_no_ctx()
{
	// TO DO: don't use the 100e6 hardcoded value anymore
	fft_sink = adiscope::scope_sink_f::make(FFT_SIZE, 100e6,
			"Osc Frequency", num_adc_channels,
			(QObject *)fft_plot);

	top_block = gr::make_top_block("spectrum_analyzer");

	for (int i = 0; i < num_adc_channels; i++) {
		auto fft = gnuradio::get_initial_sptr(
				new fft_block(false, FFT_SIZE));
		auto ctm = gr::blocks::complex_to_mag_squared::make(1);

		auto siggen = gr::analog::sig_source_f::make(100e6,
			gr::analog::GR_SIN_WAVE, 5e6 + i * 5e6, 2048);
		auto noise = gr::analog::fastnoise_source_f::make(
			gr::analog::GR_GAUSSIAN, 1, 0, 8192);
		auto add = gr::blocks::add_ff::make();

		//siggen->|
		//        |->add->fft->ctm->fft_sink
		//noise-->|
		top_block->connect(siggen, 0, add, 0);
		top_block->connect(noise, 0, add, 1);
		top_block->connect(add, 0, fft, 0);
		top_block->connect(fft, 0, ctm, 0);
		top_block->connect(ctm, 0, fft_sink, i);

		channels[i]->fft_block = fft;
	}
}

void SpectrumAnalyzer::start_blockchain_flow()
{
	if (iio) {
		for (int i = 0; i < num_adc_channels; i++)
			iio->start(fft_ids[i]);
	} else {
		fft_sink->reset();
		top_block->start();
	}
}

void SpectrumAnalyzer::stop_blockchain_flow()
{
	if (iio) {
		for (int i = 0; i < num_adc_channels; i++)
			iio->stop(fft_ids[i]);
	} else {
		top_block->stop();
	}
}

void SpectrumAnalyzer::on_comboBox_type_currentIndexChanged(const QString& s)
{
	auto it = std::find_if(avg_types.begin(), avg_types.end(),
		[&](const std::pair<QString, FftDisplayPlot::AverageType>& p){
			return p.first == s;
		});
	if (it == avg_types.end())
		return;

	int crt_channel = channelIdOfOpenedSettings();
	if (crt_channel < 0) {
		qDebug() << "invalid channel ID for the opened Settings menu";
		return;
	}

	auto avg_type = (*it).second;
	if (avg_type != channels[crt_channel]->averageType())
		channels[crt_channel]->setAverageType(avg_type);
}

void SpectrumAnalyzer::on_comboBox_window_currentIndexChanged(const QString& s)
{
	auto it = std::find_if(win_types.begin(), win_types.end(),
		[&](const std::pair<QString, FftWinType>& p){
			return p.first == s;
		});
	if (it == win_types.end())
		return;

	int crt_channel = channelIdOfOpenedSettings();
	if (crt_channel < 0) {
		qDebug() << "invalid channel ID for the opened Settings menu";
		return;
	}

	if (!channels[crt_channel]->fft_block)
		return;
	auto win_type = (*it).second;
	if (win_type != channels[crt_channel]->fftWindow())
		channels[crt_channel]->setFftWindow((*it).second, fft_size);
}

void SpectrumAnalyzer::on_spinBox_averaging_valueChanged(int n)
{
	int crt_channel = channelIdOfOpenedSettings();
	if (crt_channel < 0) {
		qDebug() << "invalid channel ID for the opened Settings menu";
		return;
	}

	if (n != channels[crt_channel]->averaging())
		channels[crt_channel]->setAveraging(n);
}

void SpectrumAnalyzer::onChannelSettingsToggled(bool en)
{
	SpectrumChannel *sc = static_cast<SpectrumChannel *>(QObject::sender());

	QString style = QString("border: 2px solid %1").arg(sc->color().name());
	ui->lineChannelSettingsTitle->setStyleSheet(style);

	auto it = std::find_if(avg_types.begin(), avg_types.end(),
		[&](const std::pair<QString, FftDisplayPlot::AverageType>& p){
			return p.second == sc->averageType();
		});
	if (it != avg_types.end()) {
		ui->comboBox_type->setCurrentText((*it).first);
	}

	auto it2 = std::find_if(win_types.begin(), win_types.end(),
		[&](const std::pair<QString, FftWinType>& p){
			return p.second == sc->fftWindow();
		});
	if (it2 != win_types.end()) {
		ui->comboBox_window->setCurrentText((*it2).first);
	}

	ui->spinBox_averaging->setValue(sc->averaging());

	ui->stackedWidget->setCurrentWidget(ui->channelSettings);
	ui->stackedWidget->setVisible(en);
}

void SpectrumAnalyzer::onChannelSelected(bool en)
{
	SpectrumChannel *sc = static_cast<SpectrumChannel *>(QObject::sender());
	crt_channel_id = sc->id();

	if (!ui->run_button->isChecked()) {
		fft_plot->replot();
	}
}

void SpectrumAnalyzer::onChannelEnabled(bool en)
{
	SpectrumChannel *sc = static_cast<SpectrumChannel *>(QObject::sender());

	if (en) {
		channels_group->addButton(sc->m_ui->name);
		sc->m_ui->name->setChecked(true);
		sc->m_ui->btn->setDisabled(false);
	} else {
		sc->m_ui->btn->setChecked(false);
		sc->m_ui->btn->setDisabled(true);
		channels_group->removeButton(sc->m_ui->name);
		sc->m_ui->name->setChecked(false);
		if (channels_group->buttons().size() > 0)
			channels_group->buttons()[0]->setChecked(true);
	}
}

void SpectrumAnalyzer::onStartStopChanged()
{
	double start = ui->start_freq->value();
	double stop = ui->stop_freq->value();
	double span = stop - start;
	double center = start + (span / 2);

	if (start > stop) {
		start = stop;
		span = stop - start;
		center = start + (span / 2);

		ui->start_freq->blockSignals(true);
		ui->start_freq->setValue(start);
		ui->start_freq->blockSignals(false);
	}

	// Update Center/Span
	ui->span_freq->blockSignals(true);
	ui->span_freq->setValue(span);
	ui->span_freq->blockSignals(false);
	ui->center_freq->blockSignals(true);
	ui->center_freq->setValue(center);
	ui->center_freq->blockSignals(false);

	// Configure plot
	fft_plot->setAxisScale(QwtPlot::xBottom, start, stop);
	fft_plot->replot();

	setSampleRate(2 * stop);

	/* Re-populate the RBW list with the new available values */
	ui->cmb_rbw->blockSignals(true);
	ui->cmb_rbw->clear();
	int i = 0;
	for (; i < bin_sizes.size(); i++) {
		ui->cmb_rbw->addItem(freq_formatter.format(
		sample_rate / bin_sizes[i], "Hz", 2));
	}
	ui->cmb_rbw->blockSignals(false);
	ui->cmb_rbw->setCurrentIndex(i - 1);
}

void SpectrumAnalyzer::onCenterSpanChanged()
{
	double span = ui->span_freq->value();
	double center = ui->center_freq->value();
	double start = center - (span / 2);
	double stop =center + (span / 2);

	if (start < 0) {
		start = 0;
		span = stop - start;
		center = start + (span / 2);

		ui->span_freq->blockSignals(true);
		ui->span_freq->setValue(span);
		ui->span_freq->blockSignals(false);
		ui->center_freq->blockSignals(true);
		ui->center_freq->setValue(center);
		ui->center_freq->blockSignals(false);
	}
	if (stop > ui->stop_freq->maxValue()) {
		stop = ui->stop_freq->maxValue();
		span = stop - start;
		center = start + (span / 2);

		ui->span_freq->blockSignals(true);
		ui->span_freq->setValue(span);
		ui->span_freq->blockSignals(false);
		ui->center_freq->blockSignals(true);
		ui->center_freq->setValue(center);
		ui->center_freq->blockSignals(false);
	}

	// Update Start/Stop
	ui->start_freq->blockSignals(true);
	ui->start_freq->setValue(start);
	ui->start_freq->blockSignals(false);
	ui->stop_freq->blockSignals(true);
	ui->stop_freq->setValue(stop);
	ui->stop_freq->blockSignals(false);

	// Configure plot
	fft_plot->setAxisScale(QwtPlot::xBottom, start, stop);
	fft_plot->replot();
}

void SpectrumAnalyzer::writeAllSettingsToHardware()
{
	adc->setSampleRate(100e6);

	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc);
	if (m2k_adc) {
		for (uint i = 0; i < adc->numAdcChannels(); i++) {
			m2k_adc->setChnHwOffset(i, 0.0);
			m2k_adc->setChnHwGainMode(i, M2kAdc::LOW_GAIN_MODE);
		}

		iio_device_attr_write_longlong(adc->iio_adc_dev(),
			"oversampling_ratio", sample_rate_divider);
	}

	auto trigger = adc->getTrigger();
	if (trigger) {
		for (uint i = 0; i < trigger->numChannels(); i++)
			trigger->setTriggerMode(i, HardwareTrigger::ALWAYS);
	}
}

int SpectrumAnalyzer::channelIdOfOpenedSettings() const
{
	int chId = -1;
	for (int i = 0; i < channels.size(); i++) {
		if (channels[i]->isSettingsOn()) {
			chId = channels[i]->id();
			break;
		}
	}

	return chId;
}

void SpectrumAnalyzer::on_btnLeftPeak_clicked()
{
	fft_plot->marker_to_next_lower_freq_peak(crt_channel_id, crt_marker);
	if (!ui->run_button->isChecked()) {
			fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
			fft_plot->replot();
	}
}

void SpectrumAnalyzer::on_btnRightPeak_clicked()
{
	fft_plot->marker_to_next_higher_freq_peak(crt_channel_id, crt_marker);
	if (!ui->run_button->isChecked()) {
			fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
			fft_plot->replot();
	}
}

void SpectrumAnalyzer::on_btnMaxPeak_clicked()
{
	fft_plot->marker_to_max_peak(crt_channel_id, crt_marker);
	if (!ui->run_button->isChecked()) {
			fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
			fft_plot->replot();
	}
}

void SpectrumAnalyzer::on_cmb_rbw_currentIndexChanged(int index)
{
	uint new_fft_size = bin_sizes[index];

	if (new_fft_size != fft_size) {
		setFftSize(new_fft_size);
	}
}

void SpectrumAnalyzer::setSampleRate(double sr)
{
	double max_sr = 100E6; // TO DO: make OscAdc figure out the max sr of an ADC

	sample_rate_divider = (int)(max_sr / sr);
	sample_rate = max_sr / sample_rate_divider;

	if (iio->started()) {
		auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc);
		if (m2k_adc) {
			iio_device_attr_write_longlong(adc->iio_adc_dev(),
				"oversampling_ratio", sample_rate_divider);
		} else {
			adc->setSampleRate(sr);
		}

		fft_plot->presetSampleRate(sample_rate);
		fft_sink->set_samp_rate(sample_rate);
	}
}

void SpectrumAnalyzer::setFftSize(uint size)
{
	// TO DO: This is cumbersome. We shouldn't have to rebuild the entire
	//        block chain every time we need to change the FFT size. A
	//        spectrum_sink block similar to scope_sink_f would be better

	bool started = iio->started();

	if (started)
		iio->lock();

	fft_size = size;
	fft_sink->set_nsamps(size);
	for (int i = 0; i < channels.size(); i++) {
		auto fft = gnuradio::get_initial_sptr(
			new fft_block(false, size));

		iio->disconnect(fft_ids[i]);
		fft_ids[i] = iio->connect(fft, i, 0, true, size);
		iio->connect(fft, 0, channels[i]->ctm_block, 0);
		iio->connect(channels[i]->ctm_block, 0, fft_sink, i);

		if (started)
			iio->start(fft_ids[i]);

		channels[i]->fft_block = fft;
		channels[i]->setFftWindow(channels[i]->fftWindow(), size);

		iio->set_buffer_size(fft_ids[i], size);
	}

	if (started)
		iio->unlock();
}

void SpectrumAnalyzer::on_btnPrevMrk_clicked()
{
	int m = crt_marker - 1;

	if (m < 0)
		m = mrk_buttons.size() - 1;

	setActiveMarker(m % mrk_buttons.size());
}

void SpectrumAnalyzer::on_btnNextMrk_clicked()
{
	int m = crt_marker + 1;

	setActiveMarker(m % mrk_buttons.size());
}

void SpectrumAnalyzer::on_btnDnAmplPeak_clicked()
{
	fft_plot->marker_to_next_lower_mag_peak(crt_channel_id, crt_marker);
	if (!ui->run_button->isChecked()) {
			fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
			fft_plot->replot();
	}
}

void SpectrumAnalyzer::on_btnUpAmplPeak_clicked()
{
	fft_plot->marker_to_next_higher_mag_peak(crt_channel_id, crt_marker);
	if (!ui->run_button->isChecked()) {
			fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
			fft_plot->replot();
	}
}

void SpectrumAnalyzer::onMarkerToggled(bool on)
{
	QPushButton *btn = static_cast<QPushButton *>(QObject::sender());
	uint id = btn->property("id").toUInt();

	setMarkerEnabled(id, on);

	updateCrtMrkLblVisibility();
}

void SpectrumAnalyzer::setMarkerEnabled(int mrk_idx, bool en)
{
	fft_plot->setMarkerEnabled(0, mrk_idx, en);
	if (en) {
		double cf = ui->center_freq->value();
		fft_plot->setMarkerAtFreq(0, mrk_idx, cf);
		fft_plot->updateMarkerUi(0, mrk_idx);
	}
	fft_plot->replot();
}

void SpectrumAnalyzer::setActiveMarker(int mrk_idx)
{
	for (int i = 0; i < mrk_buttons.size(); i++) {
		setDynamicProperty(mrk_buttons[i], "active", false);
	}

	if (mrk_idx < 0 || mrk_idx >= mrk_buttons.size()) {
		return;
	}

	setDynamicProperty(mrk_buttons[mrk_idx], "active", true);
	crt_marker = mrk_idx;

	if (fft_plot->markerEnabled(0, mrk_idx)) {
		setCurrentMarkerLabelData(0, mrk_idx);
	}

	updateCrtMrkLblVisibility();
}

void SpectrumAnalyzer::setCurrentMarkerLabelData(int chIdx, int mkIdx)
{
	QString txtFreq = freq_formatter.format(
			fft_plot->markerFrequency(chIdx, mkIdx), "Hz", 3);
	QString txtMag = QString::number(
		fft_plot->markerMagnutide(chIdx, mkIdx), 'f', 3) +
		QString(" dBFS");
	QString txt = QString("Marker %1 -> ").arg(mkIdx + 1) +
		txtFreq + QString(" ") + txtMag;
	ui->lbl_crtMarkerReading->setText(txt);
}

void SpectrumAnalyzer::updateCrtMrkLblVisibility()
{
	bool visible = false;
	for (int i = 0; i < mrk_buttons.size(); i++) {
		if (mrk_buttons[i]->isChecked() && i == crt_marker) {
			visible = true;
			break;
		}
	}
	ui->lbl_crtMarkerReading->setVisible(visible);
}

void SpectrumAnalyzer::onPlotNewMarkerData()
{
	// Update top-right label holding the reading of the active marker
	if (fft_plot->markerEnabled(0, crt_marker)) {
		setCurrentMarkerLabelData(0, crt_marker);
	}
}


/*
 * class SpectrumChannel
 */
SpectrumChannel::SpectrumChannel(int id, const QString& name,
	FftDisplayPlot *plot):
	m_id(id),
	m_name(name),
	m_line_width(1.0),
	m_color(plot->getLineColor(id).name()),
	m_averaging(1),
	m_avg_type(FftDisplayPlot::SAMPLE),
	m_fft_win(SpectrumAnalyzer::HAMMING),
	m_plot(plot),
	m_widget(new QWidget()),
	m_ui(new Ui::Channel())
{
	m_ui->setupUi(m_widget);
	m_ui->name->setText(m_name);
	setColor(m_color);

	connect(m_ui->box, SIGNAL(toggled(bool)), this,
		SLOT(onEnableBoxToggled(bool)));
	connect(m_ui->name, SIGNAL(toggled(bool)), this,
		SLOT(onNameButtonToggled(bool)));
	connect(m_ui->btn, SIGNAL(toggled(bool)), this,
		SLOT(onSettingsBtnToggled(bool)));
}

bool SpectrumChannel::isSettingsOn() const
{
	return m_ui->btn->isChecked();
}

void SpectrumChannel::setSettingsOn(bool on)
{
	m_ui->btn->setChecked(on);
}

float SpectrumChannel::lineWidth() const
{
	return m_line_width;
}

void SpectrumChannel::setLinewidth(float width)
{
	m_line_width = width;
}

QColor SpectrumChannel::color() const
{
	return m_color;
}

void SpectrumChannel::setColor(const QColor& color)
{
	m_color = color;

	QString stylesheet(m_ui->box->styleSheet());
	stylesheet += QString("\nQCheckBox::indicator {"
		"\nborder-color: %1;\n}\nQCheckBox::indicator:checked {"
		"\nbackground-color: %1;\n}\n"
		).arg(color.name());

	m_ui->box->setStyleSheet(stylesheet);

}

uint SpectrumChannel::averaging() const
{
	return m_averaging;
}

void SpectrumChannel::setAveraging(uint averaging)
{
	m_averaging = averaging;
	m_plot->setAverage(m_id, m_avg_type, averaging);
}

FftDisplayPlot::AverageType SpectrumChannel::averageType() const
{
	return m_avg_type;
}

void SpectrumChannel::setAverageType(FftDisplayPlot::AverageType avg_type)
{
	m_avg_type = avg_type;
	m_plot->setAverage(m_id, avg_type, m_averaging);
}

void SpectrumChannel::setFftWindow(SpectrumAnalyzer::FftWinType win, int taps)
{
	m_fft_win = win;
	fft_block->set_window(build_win(win, taps));
}

SpectrumAnalyzer::FftWinType SpectrumChannel::fftWindow() const
{
	return m_fft_win;
}

void SpectrumChannel::onEnableBoxToggled(bool en)
{
	if (en) {
		m_plot->AttachCurve(m_id);
	} else {
		m_plot->DetachCurve(m_id);
	}
	m_plot->replot();

	Q_EMIT enabled(en);
}

void SpectrumChannel::onNameButtonToggled(bool en)
{
	setDynamicProperty(m_ui->name->parentWidget(), "selected", en);

	Q_EMIT selected(en);
}

void SpectrumChannel::onSettingsBtnToggled(bool en)
{
	Q_EMIT settingsToggled(en);
}

std::vector<float> SpectrumChannel::build_win(SpectrumAnalyzer::FftWinType type,
		int ntaps)
{
	switch(type) {
		case SpectrumAnalyzer::FLAT_TOP:
			return gr::fft::window::flattop(ntaps);
		case SpectrumAnalyzer::RECTANGULAR:
			return gr::fft::window::rectangular(ntaps);
		case SpectrumAnalyzer::TRIANGULAR:
			return gr::fft::window::bartlett(ntaps);
		case SpectrumAnalyzer::HAMMING:
			return gr::fft::window::hamming(ntaps);
		case SpectrumAnalyzer::HANN:
			return gr::fft::window::hann(ntaps);
		case SpectrumAnalyzer::BLACKMAN_HARRIS:
			return gr::fft::window::blackman_harris(ntaps);
		case SpectrumAnalyzer::KAISER:
			return gr::fft::window::kaiser(ntaps, 0);
		default:
			std::vector<float> v(ntaps, 1.0);
			return v;
	}
}
