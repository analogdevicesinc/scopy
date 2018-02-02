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
#include "channel_widget.hpp"
#include "db_click_buttons.hpp"

/* Generated UI */
#include "ui_spectrum_analyzer.h"

#include <boost/make_shared.hpp>
#include <iio.h>
#include <iostream>

using namespace adiscope;
using namespace std;

std::vector<std::pair<QString, FftDisplayPlot::MagnitudeType>>
SpectrumAnalyzer::mag_types = {
	{"dBFS", FftDisplayPlot::DBFS},
	{"dBV", FftDisplayPlot::DBV},
	{"dBu", FftDisplayPlot::DBU},
	{"Vpeak", FftDisplayPlot::VPEAK},
	{"Vrms", FftDisplayPlot::VRMS},
};

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

std::vector<QString> SpectrumAnalyzer::markerTypes = {
	"Manual",
	"Peak",
	"Delta",
	"Fixed",
};

SpectrumAnalyzer::SpectrumAnalyzer(struct iio_context *ctx, Filter *filt,
                                   std::shared_ptr<GenericAdc> adc, QPushButton *runButton,
                                   QJSEngine *engine, ToolLauncher *parent):
	Tool(ctx, runButton, new SpectrumAnalyzer_API(this), "Spectrum Analyzer",
	     parent),
	ui(new Ui::SpectrumAnalyzer),
	marker_selector(new DbClickButtons(this)),
	fft_plot(nullptr),
	settings_group(new QButtonGroup(this)),
	channels_group(new QButtonGroup(this)),
	adc(adc),
	adc_name(ctx ? filt->device_name(TOOL_SPECTRUM_ANALYZER) : ""),
	crt_channel_id(0),
	crt_peak(0),
	max_peak_count(10),
	fft_size(32768),
	bin_sizes(
{
	256, 512, 1024, 2048, 4096, 8192, 16384, 32768
})
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
	ui->btnPreset->hide();

	ui->cmb_units->blockSignals(true);
	ui->cmb_units->clear();

	for (auto it = mag_types.begin(); it != mag_types.end(); ++it) {
		ui->cmb_units->addItem(it->first);
	}

	ui->cmb_units->blockSignals(false);

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
	fft_plot->canvas()->setStyleSheet(QString("QwtPlotCanvas { "
	                                  "background-color: #141416; }"));
	fft_plot->disableLegend();
	fft_plot->setAxisScale(QwtPlot::yLeft, -200, 0, 10);
	// Disable mouse interactions with the axes until they are in a working state
	fft_plot->setXaxisMouseGesturesEnabled(false);

	for (uint i = 0; i < num_adc_channels; i++) {
		fft_plot->setYaxisMouseGesturesEnabled(i, false);
	}

	QGridLayout *gLayout = static_cast<QGridLayout *>
	                       (ui->widgetPlotContainer->layout());
	gLayout->addWidget(fft_plot, 1, 0, 1, 1);

	// Initialize spectrum channels
	for (int i = 0 ; i < num_adc_channels; i++) {
		channel_sptr channel = boost::make_shared<SpectrumChannel>(i,
		                       channel_names[i], fft_plot);
		channel->setColor(fft_plot->getLineColor(i));
		ui->channelsList->addWidget(channel->widget());
		channels.push_back(channel);

		settings_group->addButton(channel->widget()->menuButton());
		channels_group->addButton(channel->widget()->nameButton());

		connect(channel.get()->widget(), SIGNAL(menuToggled(bool)),
		        SLOT(onChannelSettingsToggled(bool)));
		connect(channel.get()->widget(), SIGNAL(selected(bool)),
		        SLOT(onChannelSelected(bool)));
		connect(channel.get()->widget(), SIGNAL(enabled(bool)),
		        SLOT(onChannelEnabled(bool)));

		ch_api.append(new SpectrumChannel_API(this,channel));
	}

	if (num_adc_channels > 0)
		channels[crt_channel_id]->widget()->nameButton()->
		setChecked(true);

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

	// Initialize vertical axis controls
	ui->range->setMinValue(1);
	ui->range->setMaxValue(200);

	// Configure plot peak capabilities
	for (uint i = 0; i < num_adc_channels; i++) {
		fft_plot->setPeakCount(i, max_peak_count);
	}

	// Configure markers
	for (int i = 0; i < num_adc_channels; i++) {
		fft_plot->setMarkerCount(i, 5);

		for (int m = 0; m < 5; m++) {
			fft_plot->setMarkerEnabled(i, m, false);
			marker_api.append(new SpectrumMarker_API(this,i,m));

		}
	}

	// Initialize Marker controls
	ui->hLayout_marker_selector->addWidget(marker_selector);

	connect(marker_selector, SIGNAL(buttonToggled(int, bool)),
	        this, SLOT(onMarkerToggled(int, bool)));
	connect(marker_selector, SIGNAL(buttonSelected(int)),
	        this, SLOT(onMarkerSelected(int)));
	marker_selector->setSelectedButton(0);

	connect(fft_plot, SIGNAL(newMarkerData()),
	        this, SLOT(onPlotNewMarkerData()));
	connect(fft_plot, SIGNAL(markerSelected(uint, uint)),
	        this, SLOT(onPlotMarkerSelected(uint, uint)));

	connect(ui->marker_freq_pos, SIGNAL(valueChanged(double)),
	        this, SLOT(onMarkerFreqPosChanged(double)));

	connect(fft_plot, SIGNAL(sampleRateUpdated(double)),
	        this, SLOT(onPlotSampleRateUpdated(double)));
	connect(fft_plot, SIGNAL(sampleCountUpdated(uint)),
	        this, SLOT(onPlotSampleCountUpdated(uint)));

	if (ctx) {
		build_gnuradio_block_chain();
	} else {
		build_gnuradio_block_chain_no_ctx();
	}

	connect(ui->run_button, SIGNAL(toggled(bool)), this,
	        SLOT(runStopToggled(bool)));
	connect(ui->run_button, SIGNAL(toggled(bool)), runButton,
	        SLOT(setChecked(bool)));
	connect(run_button, SIGNAL(toggled(bool)), ui->run_button,
	        SLOT(setChecked(bool)));
	connect(ui->btnSingle, SIGNAL(toggled(bool)),
	        SLOT(runStopToggled(bool)));
	connect(this, SIGNAL(isRunning(bool)), run_button,
	        SLOT(setChecked(bool)));

	connect(fft_plot, SIGNAL(newData()),
	        SLOT(singleCaptureDone()));

	connect(ui->start_freq, SIGNAL(valueChanged(double)),
	        this, SLOT(onStartStopChanged()));
	connect(ui->stop_freq, SIGNAL(valueChanged(double)),
	        this, SLOT(onStartStopChanged()));
	connect(ui->center_freq, SIGNAL(valueChanged(double)),
	        this, SLOT(onCenterSpanChanged()));
	connect(ui->span_freq, SIGNAL(valueChanged(double)),
	        this, SLOT(onCenterSpanChanged()));

	connect(ui->top, SIGNAL(valueChanged(double)),
	        SLOT(onTopValueChanged(double)));
	connect(ui->range, SIGNAL(valueChanged(double)),
	        SLOT(onRangeValueChanged(double)));

	// UI default
	ui->comboBox_window->setCurrentText("Hamming");
	ui->stackedWidget->setVisible(false);
	ui->start_freq->setValue(0);
	ui->stop_freq->setValue(50e6);

	ui->top->setValue(0);
	ui->range->setValue(200);

	ui->marker_freq_pos->setMaxValue(ui->stop_freq->value());
	ui->marker_freq_pos->setStep(2 * (ui->stop_freq->value() -
	                                  ui->start_freq->value()) / bin_sizes[ui->cmb_rbw->currentIndex()]);

	ui->lblMagUnit->setText(ui->cmb_units->currentText());
	ui->markerTable->hide();

	for (auto ch: channels) {
		ch->setFftWindow(FftWinType::HAMMING, fft_size);
	}


	api->setObjectName(QString::fromStdString(Filter::tool_name(
	                           TOOL_SPECTRUM_ANALYZER)));
	api->load(*settings);
	api->js_register(engine);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
	ui->run_button->setChecked(false);

	if (saveOnExit) {
		api->save(*settings);
	}
	delete api;

	if (iio) {
		bool started = iio->started();

		if (started) {
			iio->lock();
		}

		for (unsigned int i = 0; i < num_adc_channels; i++) {
			iio->disconnect(fft_ids[i]);
		}

		if (started) {
			iio->unlock();
		}

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
	QPushButton *btn = static_cast<QPushButton *>(QObject::sender());
	setDynamicProperty(btn, "running", checked);

	if (checked) {
		if (btn == ui->btnSingle && ui->run_button->isChecked()) {
			ui->run_button->blockSignals(true);
			ui->run_button->setChecked(false);
			ui->run_button->blockSignals(false);
			setDynamicProperty(ui->run_button, "running", false);
		} else if (btn == ui->run_button &&
		           ui->btnSingle->isChecked()) {
			ui->btnSingle->blockSignals(true);
			ui->btnSingle->setChecked(false);
			ui->btnSingle->blockSignals(false);
			setDynamicProperty(ui->btnSingle, "running", false);
		}

		if (iio) {
			writeAllSettingsToHardware();
		}

		fft_plot->presetSampleRate(sample_rate);
		fft_sink->set_samp_rate(sample_rate);
		start_blockchain_flow();
	} else {
		stop_blockchain_flow();
	}

	if (!checked) {
		fft_plot->resetAverageHistory();
	}
}

void SpectrumAnalyzer::build_gnuradio_block_chain()
{
	// TO DO: don't use the 100e6 hardcoded value anymore
	fft_sink = adiscope::scope_sink_f::make(fft_size, 100e6,
	                                        "Osc Frequency", num_adc_channels,
	                                        (QObject *)fft_plot);
	fft_sink->set_trigger_mode(TRIG_MODE_TAG, 0, "buffer_start");

	bool started = iio->started();

	if (started) {
		iio->lock();
	}

	bool canConvRawToVolts = (adc_name == "m2k-adc");

	if (canConvRawToVolts) {
		auto m2k_adc = dynamic_pointer_cast<M2kAdc>(adc);

		for (int i = 0; i < adc->numAdcChannels(); i++) {
			double corr_gain = 1.0;
			double hw_gain = 1.0;

			if (m2k_adc) {
				corr_gain = m2k_adc->chnCorrectionGain(i);
				hw_gain = m2k_adc->gainAt(
				                  m2k_adc->chnHwGainMode(i));
			}

			// Calculate the VLSB for current channel
			double vlsb = adc_sample_conv::convSampleToVolts(1,
			                corr_gain, 1, 0, hw_gain);
			fft_plot->setScaleFactor(i, vlsb);
		}
	}

	fft_ids = new iio_manager::port_id[num_adc_channels];

	for (int i = 0; i < num_adc_channels; i++) {
		auto fft = gnuradio::get_initial_sptr(
		                   new fft_block(false, fft_size));
		auto ctm = gr::blocks::complex_to_mag_squared::make(1);

		// iio(i)->fft->ctm->fft_sink
		fft_ids[i] = iio->connect(fft, i, 0, true, fft_size);
		iio->connect(fft, 0, ctm, 0);
		iio->connect(ctm, 0, fft_sink, i);

		channels[i]->fft_block = fft;
		channels[i]->ctm_block = ctm;
	}

	if (started) {
		iio->unlock();
	}
}

void SpectrumAnalyzer::build_gnuradio_block_chain_no_ctx()
{
	// TO DO: don't use the 100e6 hardcoded value anymore
	fft_sink = adiscope::scope_sink_f::make(fft_size, 100e6,
	                                        "Osc Frequency", num_adc_channels,
	                                        (QObject *)fft_plot);

	top_block = gr::make_top_block("spectrum_analyzer");

	for (int i = 0; i < num_adc_channels; i++) {
		auto fft = gnuradio::get_initial_sptr(
		                   new fft_block(false, fft_size));
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
		for (int i = 0; i < num_adc_channels; i++) {
			iio->start(fft_ids[i]);
		}
	} else {
		fft_sink->reset();
		top_block->start();
	}
}

void SpectrumAnalyzer::stop_blockchain_flow()
{
	if (iio) {
		for (int i = 0; i < num_adc_channels; i++) {
			iio->stop(fft_ids[i]);
		}
	} else {
		top_block->stop();
	}
}

void SpectrumAnalyzer::on_comboBox_type_currentIndexChanged(const QString& s)
{
	auto it = std::find_if(avg_types.begin(), avg_types.end(),
	[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
		return p.first == s;
	});

	if (it == avg_types.end()) {
		return;
	}

	int crt_channel = channelIdOfOpenedSettings();

	if (crt_channel < 0) {
		qDebug() << "invalid channel ID for the opened Settings menu";
		return;
	}

	auto avg_type = (*it).second;

	if (avg_type != channels[crt_channel]->averageType()) {
		channels[crt_channel]->setAverageType(avg_type);
	}
}

void SpectrumAnalyzer::on_comboBox_window_currentIndexChanged(const QString& s)
{
	auto it = std::find_if(win_types.begin(), win_types.end(),
	[&](const std::pair<QString, FftWinType>& p) {
		return p.first == s;
	});

	if (it == win_types.end()) {
		return;
	}

	int crt_channel = channelIdOfOpenedSettings();

	if (crt_channel < 0) {
		qDebug() << "invalid channel ID for the opened Settings menu";
		return;
	}

	if (!channels[crt_channel]->fft_block) {
		return;
	}

	auto win_type = (*it).second;

	if (win_type != channels[crt_channel]->fftWindow()) {
		channels[crt_channel]->setFftWindow((*it).second, fft_size);
	}
}

void SpectrumAnalyzer::on_spinBox_averaging_valueChanged(int n)
{
	int crt_channel = channelIdOfOpenedSettings();

	if (crt_channel < 0) {
		qDebug() << "invalid channel ID for the opened Settings menu";
		return;
	}

	if (n != channels[crt_channel]->averaging()) {
		channels[crt_channel]->setAveraging(n);
	}
}

void SpectrumAnalyzer::onChannelSettingsToggled(bool en)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());
	channel_sptr sc = channels.at(cw->id());

	QString style = QString("border: 2px solid %1").arg(cw->color().name());
	ui->lineChannelSettingsTitle->setStyleSheet(style);
	ui->channelSettingsTitle->setText(sc->name());

	auto it = std::find_if(avg_types.begin(), avg_types.end(),
	[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
		return p.second == sc->averageType();
	});

	if (it != avg_types.end()) {
		ui->comboBox_type->setCurrentText((*it).first);
	}

	auto it2 = std::find_if(win_types.begin(), win_types.end(),
	[&](const std::pair<QString, FftWinType>& p) {
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
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());
	int chIdx = cw->id();

	// Is this if branch required?
	if (!ui->run_button->isChecked()) {
		fft_plot->replot();
	}

	// Update markers settings menu based on current channel
	if (en) {
		ui->labelMarkerSettingsTitle->setText(cw->fullName());

		marker_selector->blockSignals(true);

		for (int i = 0; i < fft_plot->markerCount(chIdx); i++) {
			marker_selector->setButtonChecked(i,
			                                  fft_plot->markerEnabled(chIdx, i));
		}

		marker_selector->blockSignals(false);

		updateCrtMrkLblVisibility();
	}

	crt_channel_id = chIdx;
}

void SpectrumAnalyzer::onChannelEnabled(bool en)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());

	if (en) {
		fft_plot->AttachCurve(cw->id());
	} else {
		for (int i = 0; i < channels.size(); i++) {
			ChannelWidget *cw = channels[i]->widget();

			if (cw->enableButton()->isChecked()) {
				cw->nameButton()->setChecked(true);
				break;
			}
		}

		fft_plot->DetachCurve(cw->id());
	}

	fft_plot->replot();
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
		for (uint i = 0; i < trigger->numChannels(); i++) {
			trigger->setTriggerMode(i, HardwareTrigger::ALWAYS);
		}
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
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_lower_freq_peak(crt_channel_id, crt_marker);

	if (!ui->run_button->isChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}
}

void SpectrumAnalyzer::on_btnRightPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_higher_freq_peak(crt_channel_id, crt_marker);

	if (!ui->run_button->isChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}
}

void SpectrumAnalyzer::on_btnMaxPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

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
	double new_sr = max_sr / sample_rate_divider;

	if (new_sr == sample_rate) {
		return;
	}

	if (iio->started()) {
		stop_blockchain_flow();

		auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc);

		if (m2k_adc) {
			iio_device_attr_write_longlong(adc->iio_adc_dev(),
			                               "oversampling_ratio", sample_rate_divider);
		} else {
			adc->setSampleRate(sr);
		}

		fft_plot->presetSampleRate(new_sr);
		fft_plot->resetAverageHistory();
		fft_sink->set_samp_rate(new_sr);

		start_blockchain_flow();
	}

	sample_rate = new_sr;
}

void SpectrumAnalyzer::setFftSize(uint size)
{
	// TO DO: This is cumbersome. We shouldn't have to rebuild the entire
	//        block chain every time we need to change the FFT size. A
	//        spectrum_sink block similar to scope_sink_f would be better

	bool started = iio->started();

	if (started) {
		iio->lock();
	}

	fft_size = size;
	fft_sink->set_nsamps(size);

	for (int i = 0; i < channels.size(); i++) {
		auto fft = gnuradio::get_initial_sptr(
		                   new fft_block(false, size));

		iio->disconnect(fft_ids[i]);
		fft_ids[i] = iio->connect(fft, i, 0, true, size);
		iio->connect(fft, 0, channels[i]->ctm_block, 0);
		iio->connect(channels[i]->ctm_block, 0, fft_sink, i);

		if (started) {
			iio->start(fft_ids[i]);
		}

		channels[i]->fft_block = fft;
		channels[i]->setFftWindow(channels[i]->fftWindow(), size);

		iio->set_buffer_size(fft_ids[i], size);
	}

	if (started) {
		iio->unlock();
	}
}

void SpectrumAnalyzer::on_btnDnAmplPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_lower_mag_peak(crt_channel_id, crt_marker);

	if (!ui->run_button->isChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}
}

void SpectrumAnalyzer::on_btnUpAmplPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_higher_mag_peak(crt_channel_id, crt_marker);

	if (!ui->run_button->isChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}
}

void SpectrumAnalyzer::onMarkerToggled(int id, bool on)
{
	setMarkerEnabled(crt_channel_id, id, on);

	// Add/remove the marker from the marker table
	if (on) {
		int mkType = fft_plot->markerType(crt_channel_id, id);
		ui->markerTable->addMarker(id, crt_channel_id, QString("M%1").arg(id + 1),
		                           fft_plot->markerFrequency(crt_channel_id, id),
		                           fft_plot->markerMagnutide(crt_channel_id, id),
		                           markerTypes[mkType]);
	} else {
		ui->markerTable->removeMarker(id, crt_channel_id);
	}

	if (id == marker_selector->selectedButton()) {
		if (on) {
			setCurrentMarkerLabelData(crt_channel_id, id);
		}

		updateMrkFreqPosSpinBtnValue();
	}

	updateCrtMrkLblVisibility();
}

void SpectrumAnalyzer::onMarkerSelected(int id)
{
	fft_plot->selectMarker(crt_channel_id, id);
	updateWidgetsRelatedToMarker(id);
}

void SpectrumAnalyzer::setMarkerEnabled(int ch_idx, int mrk_idx, bool en)
{
	fft_plot->setMarkerEnabled(ch_idx, mrk_idx, en);

	if (en) {
		double cf = ui->center_freq->value();
		fft_plot->setMarkerAtFreq(ch_idx, mrk_idx, cf);
		fft_plot->updateMarkerUi(ch_idx, mrk_idx);
	}

	fft_plot->replot();
}

void SpectrumAnalyzer::updateWidgetsRelatedToMarker(int mrk_idx)
{
	if (fft_plot->markerEnabled(crt_channel_id, mrk_idx)) {
		setCurrentMarkerLabelData(crt_channel_id, mrk_idx);
	}

	updateCrtMrkLblVisibility();
	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::setCurrentMarkerLabelData(int chIdx, int mkIdx)
{
	QString txtFreq = freq_formatter.format(
	                          fft_plot->markerFrequency(chIdx, mkIdx), "Hz", 3);
	QString txtMag = QString::number(
	                         fft_plot->markerMagnutide(chIdx, mkIdx), 'f', 3) +
	                 QString(" " + ui->cmb_units->currentText());
	QString txt = QString("Marker %1 -> ").arg(mkIdx + 1) +
	              txtFreq + QString(" ") + txtMag;
	ui->lbl_crtMarkerReading->setText(txt);
}

void SpectrumAnalyzer::updateCrtMrkLblVisibility()
{
	int crt = marker_selector->selectedButton();

	if (crt >= 0) {
		bool visible = marker_selector->buttonChecked(crt);
		ui->lbl_crtMarkerReading->setVisible(visible);
	}
}

void SpectrumAnalyzer::onPlotNewMarkerData()
{
	int crt_marker = marker_selector->selectedButton();

	// Update top-right label holding the reading of the active marker
	if (fft_plot->markerEnabled(crt_channel_id, crt_marker)) {
		setCurrentMarkerLabelData(crt_channel_id, crt_marker);
		updateMrkFreqPosSpinBtnValue();
	}

	// Update the markers in the marker table
	for (int c = 0; c < num_adc_channels; c++) {
		for (int m = 0; m < fft_plot->markerCount(c); m++) {
			if (fft_plot->markerEnabled(c, m)) {
				int mkType = fft_plot->markerType(c, m);
				ui->markerTable->updateMarker(m, c,
				                              fft_plot->markerFrequency(c, m),
				                              fft_plot->markerMagnutide(c, m),
				                              markerTypes[mkType]);
			}
		}
	}
}

void SpectrumAnalyzer::onPlotMarkerSelected(uint chIdx, uint mkIdx)
{
	if (crt_channel_id != chIdx) {
		channels[chIdx]->widget()->nameButton()->setChecked(true);
	}

	if (marker_selector->selectedButton() != mkIdx) {
		marker_selector->setSelectedButton(mkIdx);
		updateWidgetsRelatedToMarker(mkIdx);
	}
}

void SpectrumAnalyzer::onMarkerFreqPosChanged(double freq)
{
	int crt_marker = marker_selector->selectedButton();

	if (!fft_plot->markerEnabled(crt_channel_id, crt_marker)) {
		return;
	}

	fft_plot->setMarkerAtFreq(crt_channel_id, crt_marker, freq);
	fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
	double actual_freq = fft_plot->markerFrequency(crt_channel_id,
	                     crt_marker);

	ui->marker_freq_pos->blockSignals(true);
	ui->marker_freq_pos->setValue(actual_freq);
	ui->marker_freq_pos->blockSignals(false);

	fft_plot->replot();
}

void SpectrumAnalyzer::updateMrkFreqPosSpinBtnLimits()
{
	ui->marker_freq_pos->setMaxValue(ui->stop_freq->value());
	ui->marker_freq_pos->setStep(2 * ui->stop_freq->value() /
	                             bin_sizes[ui->cmb_rbw->currentIndex()]);

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::updateMrkFreqPosSpinBtnValue()
{
	int crt_marker = marker_selector->selectedButton();

	if (!fft_plot->markerEnabled(crt_channel_id, crt_marker)) {
		return;
	}

	if (!fft_plot->markerType(crt_channel_id, crt_marker) == 0) {
		return;
	}

	double freq = fft_plot->markerFrequency(crt_channel_id, crt_marker);

	if (freq != ui->marker_freq_pos->value()) {
		ui->marker_freq_pos->blockSignals(true);
		ui->marker_freq_pos->setValue(freq);
		ui->marker_freq_pos->blockSignals(false);
	}
}

void SpectrumAnalyzer::onPlotSampleRateUpdated(double)
{
	updateMrkFreqPosSpinBtnLimits();
}

void SpectrumAnalyzer::onPlotSampleCountUpdated(uint)
{
	updateMrkFreqPosSpinBtnLimits();
}

void SpectrumAnalyzer::singleCaptureDone()
{
	if (ui->btnSingle->isChecked()) {
		ui->btnSingle->setChecked(false);
		Q_EMIT isRunning(false);
	}
}

void SpectrumAnalyzer::on_cmb_units_currentIndexChanged(const QString& unit)
{
	auto it = std::find_if(mag_types.begin(), mag_types.end(),
	[&](const std::pair<QString, FftDisplayPlot::MagnitudeType>& p) {
		return p.first == unit;
	});

	if (it == mag_types.end()) {
		return;
	}

	FftDisplayPlot::MagnitudeType magType = (*it).second;

	switch (magType) {
	case FftDisplayPlot::VPEAK:
	case FftDisplayPlot::VRMS:
		fft_plot->setAxisScale(QwtPlot::yLeft, 0, 25, 10);
		break;

	default:
		fft_plot->setAxisScale(QwtPlot::yLeft, -200, 0, 10);
		break;
	}

	fft_plot->setMagnitudeType((*it).second);
	fft_plot->recalculateMagnitudes();
	fft_plot->replot();

	ui->lblMagUnit->setText(unit);
}

void SpectrumAnalyzer::on_btnMarkerTable_toggled(bool checked)
{
	ui->markerTable->setVisible(checked);

	// Set the Plot 3 times taller than the Marker Table (when visible)
	QGridLayout *layout = static_cast<QGridLayout *>(
	                              ui->widgetPlotContainer->layout());
	int row1 = getGridLayoutPosFromIndex(layout,
	                                     layout->indexOf(ui->markerTable)).first;
	int row2 = getGridLayoutPosFromIndex(layout,
	                                     layout->indexOf(fft_plot)).first;

	if (checked) {
		layout->setRowStretch(row1, 1);
		layout->setRowStretch(row2, 3);
	} else {
		layout->setRowStretch(row1, 0);
		layout->setRowStretch(row2, 0);
	}
}

QPair<int, int> SpectrumAnalyzer::getGridLayoutPosFromIndex(QGridLayout *layout,
                int index) const
{
	QPair<int, int> pos = qMakePair(-1, -1);

	if (layout && index >= 0) {
		int row, col, rowSpan, colSpan;
		layout->getItemPosition(index, &row, &col, &rowSpan, &colSpan);
		pos.first = row;
		pos.second = col;
	}

	return pos;
}

void SpectrumAnalyzer::onTopValueChanged(double top)
{
	double range = ui->range->value();
	fft_plot->setAxisScale(QwtPlot::yLeft, top - range, top);
	fft_plot->replot();
}

void SpectrumAnalyzer::onRangeValueChanged(double range)
{
	double top = ui->top->value();
	fft_plot->setAxisScale(QwtPlot::yLeft, top - range, top);
	fft_plot->replot();
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
	m_widget(new ChannelWidget(id, false, false, m_color))
{
	m_widget->setFullName(name);
	m_widget->setShortName(QString("CH %1").arg(id));
	m_widget->nameButton()->setText(m_widget->fullName());
}

bool SpectrumChannel::isSettingsOn() const
{
	return m_widget->menuButton()->isChecked();
}

void SpectrumChannel::setSettingsOn(bool on)
{
	m_widget->enableButton()->setChecked(on);
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
	m_widget->setColor(color);
	m_color = color;
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

	std::vector<float> window = build_win(win, taps);
	float gain = calcCoherentPowerGain(window);
	scaletFftWindow(window, 1 / gain);
	fft_block->set_window(window);
}

SpectrumAnalyzer::FftWinType SpectrumChannel::fftWindow() const
{
	return m_fft_win;
}

std::vector<float> SpectrumChannel::build_win(SpectrumAnalyzer::FftWinType type,
                int ntaps)
{
	switch (type) {
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

float
SpectrumChannel::calcCoherentPowerGain(const std::vector<float>& win) const
{
	float sum = 0;

	for (int i = 0; i < win.size(); i++) {
		sum += win[i];
	}

	return sum / win.size();
}

void SpectrumChannel::scaletFftWindow(std::vector<float>& win, float gain)
{
	for (int i = 0; i < win.size(); i++) {
		win[i] *= gain;
	}
}

int SpectrumChannel_API::type()
{
	return	spch->averageType();
}

int SpectrumChannel_API::window()
{
	return spch->fftWindow();
}

int SpectrumChannel_API::averaging()
{
	return spch->averaging();
}

bool SpectrumChannel_API::enabled()
{
	return spch->widget()->enableButton()->isChecked();
}

void SpectrumChannel_API::enable(bool en)
{
	spch->widget()->enableButton()->setChecked(en);
}

void SpectrumChannel_API::setType(int type)
{
	spch->setAverageType((adiscope::FftDisplayPlot::AverageType)type);
}

void SpectrumChannel_API::setWindow(int win)
{
	auto taps=sp->fft_size;
	spch->setFftWindow((adiscope::SpectrumAnalyzer::FftWinType)win,taps);
}

void SpectrumChannel_API::setAveraging(int avg)
{
	spch->setAveraging(avg);
}

int SpectrumMarker_API::chId()
{
	return m_chid;
}

void SpectrumMarker_API::setChId(int val)
{
	m_chid=val;
}

int SpectrumMarker_API::mkId()
{
	return m_mkid;
}

void SpectrumMarker_API::setMkId(int val)
{
	m_mkid=val;
}

int SpectrumMarker_API::type()
{
//	return sp->fft_plot->markerType(m_chid,m_mkid);
}

void SpectrumMarker_API::setType(int val)
{
	//sp->fft_plot->
}

double SpectrumMarker_API::freq()
{
	if (sp->fft_plot->markerEnabled(m_chid,m_mkid)) {
		return sp->fft_plot->markerFrequency(m_chid,m_mkid);
	} else {
		return 0;
	}
}

void SpectrumMarker_API::setFreq(double pos)
{
	if (sp->fft_plot->markerEnabled(m_chid,m_mkid)) {
		sp->fft_plot->setMarkerAtFreq(m_chid,m_mkid,pos);
		//sp->crt_channel_id=m_chid;
		sp->updateWidgetsRelatedToMarker(m_mkid);
		sp->fft_plot->updateMarkerUi(m_chid, m_mkid);

		if (sp->crt_channel_id==m_chid) {
			sp->marker_selector->blockSignals(true);
			sp->marker_selector->setButtonChecked(m_mkid, true);
			sp->marker_selector->blockSignals(false);
		}


	}
}

bool SpectrumMarker_API::enabled()
{
	return sp->fft_plot->markerEnabled(m_chid,m_mkid);
}

void SpectrumMarker_API::setEnabled(bool en)
{
	sp->fft_plot->setMarkerEnabled(m_chid,m_mkid,en);
	//sp->crt_channel_id=m_chid;
	sp->updateWidgetsRelatedToMarker(m_mkid);
	sp->fft_plot->updateMarkerUi(m_chid, m_mkid);

}

QVariantList SpectrumAnalyzer_API::getMarkers()
{
	QVariantList list;

	for (SpectrumMarker_API *each : sp->marker_api) {
		list.append(QVariant::fromValue(each));
	}

	return list;
}


bool SpectrumAnalyzer_API::running()
{
	return sp->runButton()->isChecked();
}

void SpectrumAnalyzer_API::run(bool chk)
{
	sp->runButton()->setChecked(chk);
}

QVariantList SpectrumAnalyzer_API::getChannels()
{
	QVariantList list;

	for (SpectrumChannel_API *each : sp->ch_api) {
		list.append(QVariant::fromValue(each));
	}

	return list;
}


int SpectrumAnalyzer_API::currentChannel()
{
	return sp->crt_channel_id;
}

void SpectrumAnalyzer_API::setCurrentChannel(int ch)
{
	sp->channels[ch]->widget()->nameButton()->setChecked(true);
	sp->channels[ch]->widget()->selected(true);
}

double SpectrumAnalyzer_API::startFreq()
{
	return sp->ui->start_freq->value();
}
void SpectrumAnalyzer_API::setStartFreq(double val)
{
	sp->ui->start_freq->setValue(val);
}

double SpectrumAnalyzer_API::stopFreq()
{
	return sp->ui->stop_freq->value();
}
void SpectrumAnalyzer_API::setStopFreq(double val)
{
	sp->ui->stop_freq->setValue(val);
}

QString SpectrumAnalyzer_API::resBW()
{
	return sp->ui->cmb_rbw->currentText();
}
void SpectrumAnalyzer_API::setResBW(QString s)
{
	sp->ui->cmb_rbw->setCurrentText(s);
}


QString SpectrumAnalyzer_API::units()
{
	return sp->ui->cmb_units->currentText();
}

void SpectrumAnalyzer_API::setUnits(QString s)
{
	sp->ui->cmb_units->setCurrentText(s);
}

double SpectrumAnalyzer_API::topScale()
{
	return sp->ui->top->value();
}
void SpectrumAnalyzer_API::setTopScale(double val)
{
	sp->ui->top->setValue(val);
}

double SpectrumAnalyzer_API::range()
{
	return sp->ui->range->value();
}
void SpectrumAnalyzer_API::setRange(double val)
{
	sp->ui->range->setValue(val);
}

bool SpectrumAnalyzer_API::markerTableVisible()
{
	return sp->ui->btnMarkerTable->isChecked();
}

void SpectrumAnalyzer_API::setMarkerTableVisible(bool en)
{
	sp->ui->btnMarkerTable->setChecked(en);
}


