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

/* GNU Radio includes */
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/add_blk.h>
#include <scopy/math.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/analog/fastnoise_source.h>

/* Qt includes */
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>
#include <QFileDialog>
#include <QCheckBox>
#include <QTimer>
#include <QDockWidget>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets>

/* Local includes */
#include "logging_categories.h"
#include "spectrum_analyzer.hpp"
#include "filter.hpp"
#include "math.hpp"
#include "fft_block.hpp"
#include "gui/dynamicWidget.hpp"
#include "gui/channel_widget.hpp"
#include "gui/db_click_buttons.hpp"
#include "filemanager.h"
#include "spectrum_analyzer_api.hpp"
#include "stream_to_vector_overlap.h"
#include "tool_launcher.hpp"

#ifdef SPECTRAL_MSR
#include "gui/measure.h"
#include "measurement_gui.h"
#include "gui/measure_settings.h"
#endif

/* Generated UI */
#include "ui_spectrum_analyzer.h"
#include "ui_cursors_settings.h"
#include "ui_cursor_readouts.h"
#include "ui_measure_panel.h"
#include "ui_measure_settings.h"

#include <boost/make_shared.hpp>
#include <iio.h>
#include <iostream>

/* libm2k includes */
#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2kexceptions.hpp>
#include "scopyExceptionHandler.h"

#define TIMER_TIMEOUT_MS 100

static const int MAX_REF_CHANNELS = 4;

using namespace adiscope;
using namespace std;
using namespace libm2k;
using namespace libm2k::context;

void SpectrumAnalyzer::initInstrumentStrings() {

	mag_types = {
	    {tr("dBFS"), FftDisplayPlot::DBFS},
	    {tr("dBV"), FftDisplayPlot::DBV},
	    {tr("dBu"), FftDisplayPlot::DBU},
	    {tr("Vpeak"), FftDisplayPlot::VPEAK},
	    {tr("Vrms"), FftDisplayPlot::VRMS},
	    {tr("V/√Hz"), FftDisplayPlot::VROOTHZ},
	};

	avg_types = {
	    {tr("Sample"), FftDisplayPlot::SAMPLE},
	    {tr("Peak Hold"), FftDisplayPlot::PEAK_HOLD},
	    {tr("Peak Hold Continous"), FftDisplayPlot::PEAK_HOLD_CONTINUOUS},
	    {tr("Min Hold"), FftDisplayPlot::MIN_HOLD},
	    {tr("Min Hold Continous"), FftDisplayPlot::MIN_HOLD_CONTINUOUS},
	    {tr("Linear RMS"), FftDisplayPlot::LINEAR_RMS},
	    {tr("Linear dB"), FftDisplayPlot::LINEAR_DB},
	    {tr("Exponential RMS"), FftDisplayPlot::EXPONENTIAL_RMS},
	    {tr("Exponential dB"), FftDisplayPlot::EXPONENTIAL_DB},
	};

	win_types = {
	    {tr("Flat top"), FftWinType::FLAT_TOP},
	    {tr("Rectangular"), FftWinType::RECTANGULAR},
	    {tr("Triangular (Bartlett)"), FftWinType::TRIANGULAR},
	    {tr("Hamming"), FftWinType::HAMMING},
	    {tr("Hann"), FftWinType::HANN},
	    {tr("Blackman-Harris"), FftWinType::BLACKMAN_HARRIS},
	    {tr("Kaiser"), FftWinType::KAISER},
	};

	markerTypes = {
	    tr("Manual"),
	    tr("Peak"),
	    tr("Delta"),
	    tr("Fixed"),
	};
}

SpectrumAnalyzer::SpectrumAnalyzer(struct iio_context *ctx, Filter *filt,
				   ToolMenuItem *toolMenuItem,
				   QJSEngine *engine, ToolLauncher *parent):
	Tool(ctx, toolMenuItem, new SpectrumAnalyzer_API(this), "Spectrum Analyzer",
	     parent),
	ui(new Ui::SpectrumAnalyzer),
	m_m2k_context(nullptr),
	m_m2k_analogin(nullptr),
	m_generic_context(nullptr),
	m_generic_analogin(nullptr),
	marker_selector(new DbClickButtons(this)),
	fft_plot(nullptr),
	settings_group(new QButtonGroup(this)),
	channels_group(new QButtonGroup(this)),
	adc_name(ctx ? filt->device_name(TOOL_SPECTRUM_ANALYZER) : ""),
	crt_channel_id(0),
	crt_peak(0),
	max_peak_count(10),
	fft_size(32768),
	hCursorsEnabled(true),
	vCursorsEnabled(true),
	searchVisiblePeaks(true),
	m_max_sample_rate(100e6),
	sample_rate_divider(1),
	marker_menu_opened(false),
	bin_sizes({
	256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144
	}), nb_ref_channels(0),
	selected_ch_settings(-1),
	fft_ids(nullptr),
	m_nb_overlapping_avg(1)
{
	initInstrumentStrings();
	// Get the list of names of the available channels
	QList<QString> channel_names;

	if (ctx) {
		auto libm2k_ctx = contextOpen(ctx, "");
		if (libm2k_ctx->toM2k()) {
			m_m2k_context = libm2k_ctx->toM2k();
			m_m2k_analogin = m_m2k_context->getAnalogIn();
			m_adc_nb_channels = m_m2k_analogin->getNbChannels();
			m_max_sample_rate = m_m2k_analogin->getMaximumSamplerate();
		} else {
			m_generic_context = libm2k_ctx->toGeneric();
			m_generic_analogin = m_generic_context->getAnalogIn(0);
			m_adc_nb_channels = m_generic_analogin->getNbChannels();
		}
		iio = iio_manager::get_instance(ctx, adc_name);

		for (unsigned int i = 0; i < m_adc_nb_channels; i++) {
			channel_names.push_back(
				QString("Channel %1").arg(i + 1));
		}
	}
	sample_rate = m_max_sample_rate;

	ui->setupUi(this);
	// Temporarily disable the delta marker button
	ui->pushButton_4->hide();

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
	settings_group->addButton(ui->btnSweep);
	settings_group->addButton(ui->btnMarkers);
	settings_group->addButton(ui->btnAddRef);
	settings_group->addButton(ui->btnCursors);

#ifdef SPECTRAL_MSR
	settings_group->addButton(ui->btnMeasure);
#endif
	settings_group->setExclusive(true);

#ifdef SPECTRAL_MSR
	/* Measure panel */
	measure_panel_init();
#endif

	fft_plot = new FftDisplayPlot(m_adc_nb_channels, this);
	fft_plot->disableLegend();

	// Disable mouse interactions with the axes until they are in a working state
	fft_plot->setXaxisMouseGesturesEnabled(false);

	for (uint i = 0; i < m_adc_nb_channels; i++) {
//		ui->gridLayout_plot->addWidget(measurePanel, 0, 1, 1, 1);
		fft_plot->setYaxisMouseGesturesEnabled(i, false);
	}

	connect(fft_plot, SIGNAL(channelAdded(int)),
		    SLOT(onChannelAdded(int)));

#ifdef SPECTRAL_MSR
	/* Measurements Settings */
	measure_settings_init();
#endif


	// plot widget
	QWidget* centralWidget = new QWidget(this);
	QVBoxLayout* vLayout = new QVBoxLayout(centralWidget);
	vLayout->setContentsMargins(20, 0, 20, 20);
	vLayout->setSpacing(10);
	centralWidget->setLayout(vLayout);

#ifdef SPECTRAL_MSR
	vLayout->addWidget(measurePanel);
#endif

	ui->widgetPlotContainer->layout()->removeWidget(ui->topPlotWidget);
	vLayout->addWidget(ui->topPlotWidget);

	vLayout->addWidget(fft_plot->getPlotwithElements());

	ui->widgetPlotContainer->layout()->removeWidget(ui->markerTable);
	vLayout->addWidget(ui->markerTable);

	fft_plot->setZoomerEnabled();
	fft_plot->setAxisVisible(QwtAxis::XBottom, false);
	fft_plot->setAxisVisible(QwtAxis::YLeft, false);
	fft_plot->setUsingLeftAxisScales(false);

	if(prefPanel->getCurrent_docking_enabled()) {

		// main window for dock widget
		QMainWindow* mainWindow = new QMainWindow(this);
		mainWindow->setCentralWidget(0);
		mainWindow->setWindowFlags(Qt::Widget);
		ui->gridLayout_plot->addWidget(mainWindow, 1, 0, 1, 1);

		QDockWidget* dockWidget = DockerUtils::createDockWidget(mainWindow, centralWidget);

		mainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

#ifdef PLOT_MENU_BAR_ENABLED
		DockerUtils::configureTopBar(dockWidget);
#endif

	} else {
		ui->gridLayout_plot->addWidget(centralWidget, 1, 0, 1, 1);
	}


	fft_plot->enableXaxisLabels();
	fft_plot->enableYaxisLabels();
	setYAxisUnit(ui->cmb_units->currentText());
	fft_plot->setBtmHorAxisUnit("Hz");

	// Initialize spectrum channels
	for (int i = 0 ; i < m_adc_nb_channels; i++) {
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

	if (m_adc_nb_channels > 0)
		channels[crt_channel_id]->widget()->nameButton()->
		setChecked(true);

	// Initialize Sweep controls
	unit_per_div = new PositionSpinButton({
		{" ",1e0},
	}, tr("Scale/Div"), 0.0, 0.0, false, false, this);

	ui->divisionWidget->addWidget(unit_per_div);

	m_mag_min_max = QPair<double, double>(-200, 300);
	top = new PositionSpinButton({
		{" ",1e0},
	}, tr("Top"), -200.0, 300.0, false, false, this);
	top_scale = new ScaleSpinButton({
			{{"pV/√Hz",1e-12},
			 {"nV/√Hz",1e-9},
			 {"μV/√Hz",1e-6},
			 {"mV/√Hz",1e-3},
			 {"V/√Hz",1e0}}
	}, tr("Top"), 1e-12, 10e1, false, false, this);
	ui->topWidget->addWidget(top);
	ui->topWidget->addWidget(top_scale);

	bottom = new PositionSpinButton({
		{" ",1e0},
	}, tr("Bottom"), -200.0, 300.0, false, false, this);
	bottom_scale = new ScaleSpinButton({
			   {{"pV/√Hz",1e-12},
			    {"nV/√Hz",1e-9},
			    {"μV/√Hz",1e-6},
			    {"mV/√Hz",1e-3},
			    {"V/√Hz",1e0}}
	}, tr("Bottom"), 1e-12, 10e1, false, false, this);
	ui->bottomWidget->addWidget(bottom);
	ui->bottomWidget->addWidget(bottom_scale);

	marker_freq_pos = new PositionSpinButton({
	{tr("Hz"),1e0},
	{tr("kHz"),1e3},
	{tr("MHz"),1e6}
	}, tr("Frequency Position"), 0.0, 5e7, true, false, this);
	ui->markerFreqPosLayout->addWidget(marker_freq_pos);
	marker_freq_pos->setFineModeAvailable(false);

	sample_timer = new QTimer();
	connect(sample_timer, SIGNAL(timeout()), this, SLOT(refreshCurrentSampleLabel()));

	startStopRange = new StartStopRangeWidget(0);
	connect(startStopRange, &StartStopRangeWidget::rangeChanged, [=](double start, double stop){
		fft_plot->setStartStop(start, stop);
		fft_plot->setAxisScale(QwtAxis::XBottom, start, stop);
		fft_plot->replot();
		fft_plot->bottomHandlesArea()->repaint();

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
		ui->cmb_rbw->setCurrentIndex((int)bin_sizes.size() / 2);

		marker_freq_pos->setMaxValue(stop);
		marker_freq_pos->setStep(2 * (stop -
						  start) / bin_sizes[ui->cmb_rbw->currentIndex()]);

	});

	connect(ui->cmb_rbw, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[=](int index){
		startStopRange->setMinimumSpanValue(10 * sample_rate / bin_sizes[index]);
	});

	connect(ui->cmbGainMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[=](int index){

		int crt_channel = channelIdOfOpenedSettings();
		if (crt_channel >= m_adc_nb_channels || crt_channel < 0) {
			return;
		}
		channels[crt_channel]->setGainMode(index);

		bool started = isIioManagerStarted();
		if (started) {
			iio->lock();
		}

		try {
			libm2k::analog::ANALOG_IN_CHANNEL chn = static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(crt_channel);
			m_m2k_analogin->setRange(chn, channels[crt_channel]->getGainMode());

			bool canConvRawToVolts = m_m2k_analogin ? true : false;
			if (canConvRawToVolts) {
				fft_plot->setScaleFactor(crt_channel, m_m2k_analogin->getScalingFactor(chn));
			}
		} catch (libm2k::m2k_exception &e) {
			HANDLE_EXCEPTION(e);
			qDebug(CAT_SPECTRUM_ANALYZER) << e.what();
		}

		if (started) {
			iio->unlock();
		}
	});

	ui->cmbGainMode->setCurrentIndex(0);

	// Initialize vertical axis controls
	unit_per_div->setMinValue(0.01);
	unit_per_div->setMaxValue(500/10);

	// Configure plot peak capabilities
	for (uint i = 0; i < m_adc_nb_channels; i++) {
		fft_plot->setPeakCount(i, max_peak_count);
	}

	// Configure markers
	for (int i = 0; i < m_adc_nb_channels; i++) {
		fft_plot->setMarkerCount(i, 5);

		for (int m = 0; m < 5; m++) {
			fft_plot->setMarkerEnabled(i, m, false);
			marker_api.append(new SpectrumMarker_API(this,i,m));

		}
	}

	ui->rightMenu->setMaximumWidth(0);
	ui->pushButton_3->hide();

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

	connect(marker_freq_pos, SIGNAL(valueChanged(double)),
	        this, SLOT(onMarkerFreqPosChanged(double)));

	connect(fft_plot, SIGNAL(sampleRateUpdated(double)),
	        this, SLOT(onPlotSampleRateUpdated(double)));
	connect(fft_plot, SIGNAL(sampleCountUpdated(uint)),
	        this, SLOT(onPlotSampleCountUpdated(uint)));


	connect(this, SIGNAL(selectedChannelChanged(int)),
		fft_plot, SLOT(setSelectedChannel(int)));

	if (ctx) {
		build_gnuradio_block_chain();
	} else {
		build_gnuradio_block_chain_no_ctx();
	}

	connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(validateSpinboxAveraging()));
	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		[=](bool checked){
		auto btn = dynamic_cast<CustomPushButton *>(run_button);
		btn->setChecked(checked);
	});
	connect(run_button, &QPushButton::toggled,
		ui->runSingleWidget, &RunSingleWidget::toggle);
	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		this, &SpectrumAnalyzer::runStopToggled);
	connect(this, &SpectrumAnalyzer::started,
		ui->runSingleWidget, &RunSingleWidget::toggle);


	connect(fft_plot, SIGNAL(newData()),
	        SLOT(singleCaptureDone()));

	connect(fft_plot, SIGNAL(currentAverageIndex(unsigned int, unsigned int)),
		SLOT(onCurrentAverageIndexChanged(unsigned int, unsigned int)));
	const bool visible = (channels[crt_channel_id]->averageType() != FftDisplayPlot::AverageType::SAMPLE);
	setCurrentAverageIndexLabel(crt_channel_id);

#ifdef SPECTRAL_MSR
	/* Apply measurements for every new batch of data */
	connect(fft_plot, SIGNAL(newData()), SLOT(onNewDataReceived()));

	for (int i = 0; i < m_adc_nb_channels; i++) {
		fft_plot->initChannelMeasurement(i);
	    }
#endif

	connect(top, SIGNAL(valueChanged(double)),
	        SLOT(onTopValueChanged(double)));
	connect(unit_per_div, SIGNAL(valueChanged(double)),
		SLOT(onScalePerDivValueChanged(double)));
	connect(bottom, SIGNAL(valueChanged(double)),
		SLOT(onBottomValueChanged(double)));
	connect(top_scale, SIGNAL(valueChanged(double)),
		SLOT(onTopValueChanged(double)));
	connect(bottom_scale, SIGNAL(valueChanged(double)),
		SLOT(onBottomValueChanged(double)));

	cursor_panel_init();

	connect(fft_plot,
		SIGNAL(cursorReadoutsChanged(struct cursorReadoutsText)),
		SLOT(onCursorReadoutsChanged(struct cursorReadoutsText)));

	// UI default
	ui->comboBox_window->setCurrentText("Hamming");
	ui->comboBox_line_thickness->setCurrentText("1");

	startStopRange->setStartValue(0);
	startStopRange->setStopValue(50e6);

	ui->gridSweepControls->addWidget(startStopRange, 0, 0, 2, 2);

	top->setValue(0);
	bottom->setValue(-200);

	marker_freq_pos->setMinValue(1);
	marker_freq_pos->setMaxValue(startStopRange->getStopValue());
	marker_freq_pos->setStep(2 * (startStopRange->getStopValue() -
					  startStopRange->getStartValue()) / bin_sizes[ui->cmb_rbw->currentIndex()]);

	ui->lblMagUnit->setText(ui->cmb_units->currentText());
	ui->markerTable->hide();


	for (auto ch: qAsConst(channels)) {
		ch->setFftWindow(FftWinType::HAMMING, fft_size);
	}

#ifdef SPECTRAL_MSR
	if (!runButton()->isChecked() && measurementsEnabled()) {
	       measureUpdateValues();
	   }

	for (unsigned int i = 0; i < m_adc_nb_channels; i++) {
	       init_selected_measurements(i, {0, 1, 4, 5});
	   }
#endif

	connect(ui->logBtn, &QPushButton::toggled,
		fft_plot, &FftDisplayPlot::useLogFreq);
	connect(ui->logBtn, &QPushButton::toggled,
		[=](bool use_log_freq){
		startStopRange->setMinimumValue(use_log_freq);
	});

	ui->btnHistory->setEnabled(true);
	ui->btnHistory->setChecked(true);
	ui->btnHistory->setVisible(false);

	api->setObjectName(QString::fromStdString(Filter::tool_name(
	                           TOOL_SPECTRUM_ANALYZER)));
	api->load(*settings);
	api->js_register(engine);

	connect(ui->rightMenu, &MenuAnim::finished, this, &SpectrumAnalyzer::rightMenuFinished);
	menuOrder.push_back(ui->btnSweep);


	connect(ui->btnPrint, &QPushButton::clicked, [=](){
		fft_plot->printWithNoBackground(api->objectName(), false);
	});

	connect(ui->btnSnapshot, &QPushButton::clicked, [=](){
		QwtPlotCurve *curve = fft_plot->Curve(selected_ch_settings);
		if (selected_ch_settings < 2) {
			if (nb_ref_channels == MAX_REF_CHANNELS) {
				return;
			}

			if (!curve->data()->size()) {
				return;
			}

			QVector<double> xData;
			QVector<double> yData;
			for (size_t i = 0; i < curve->data()->size(); ++i) {
				xData.push_back(curve->data()->sample(i).x());
				yData.push_back(curve->data()->sample(i).y());
			}

			channel_sptr sc = channels.at(selected_ch_settings);

			QStringList channelDetails;

			auto iterAvg = std::find_if(avg_types.begin(), avg_types.end(),
			[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
				return p.second == sc->averageType();
			});

			if (iterAvg != avg_types.end()) {
				channelDetails.push_back((*iterAvg).first);
			}

			auto iterWin = std::find_if(win_types.begin(), win_types.end(),
			[&](const std::pair<QString, FftWinType>& p) {
				return p.second == sc->fftWindow();
			});

			if (iterWin != win_types.end()) {
				channelDetails.push_back((*iterWin).first);
			}

			channelDetails.push_back(QString::number(sc->averaging()));

			importedChannelDetails.push_back(channelDetails);

			add_ref_waveform(xData, yData);

		} else {
			// export

			QStringList filter;
			filter += QString(tr("Comma-separated values files (*.csv)"));
			filter += QString(tr("Tab-delimited values files (*.txt)"));
			filter += QString(tr("All Files(*)"));

			QString selectedFilter = filter[0];

			QString fileName = QFileDialog::getSaveFileName(this,
			    tr("Export"), "", filter.join(";;"),
			    &selectedFilter, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

			if (fileName.split(".").size() <= 1) {
				// file name w/o extension. Let's append it
				QString ext = selectedFilter.split(".")[1].split(")")[0];
				fileName += "." + ext;
			}

			if (!fileName.isEmpty()) {
				FileManager fm("Spectrum Analyzer");
				fm.open(fileName, FileManager::EXPORT);

				QVector<double> freq_data, mag_data;

				for (size_t i = 0; i < curve->data()->size(); ++i) {
					freq_data.push_back(curve->sample(i).x());
					mag_data.push_back(curve->sample(i).y());
				}

				QString unit = ui->lblMagUnit->text();
				fm.save(freq_data, "Frequency(Hz)");
				fm.save(mag_data, "REF" + QString::number(selected_ch_settings - m_adc_nb_channels + 1)
					+ "(" + unit + ")");

				QString channelDetails;

				auto iterAvg = std::find_if(avg_types.begin(), avg_types.end(),
				[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
					return p.first == importedChannelDetails[selected_ch_settings - m_adc_nb_channels][0];
				});

				if (iterAvg != avg_types.end()) {
					channelDetails += (*iterAvg).first;
					channelDetails += ",";
				}

				auto iterWin = std::find_if(win_types.begin(), win_types.end(),
				[&](const std::pair<QString, FftWinType>& p) {
					return p.first == importedChannelDetails[selected_ch_settings - m_adc_nb_channels][1];
				});

				if (iterWin != win_types.end()) {
					channelDetails += (*iterWin).first;
					channelDetails += ",";
				}

				channelDetails += importedChannelDetails[selected_ch_settings - m_adc_nb_channels][2];
				channelDetails += ",";

				fm.setAdditionalInformation(channelDetails);

				fm.performWrite();
			}
		}
	});

	connect(ui->btnExport, &QPushButton::clicked,
		this, &SpectrumAnalyzer::btnExportClicked);
	readPreferences();

	ui->btnHelp->setUrl("https://wiki.analog.com/university/tools/m2k/scopy/spectrumanalyzer");

#ifndef SPECTRAL_MSR
	// TODO: enable measurements
	ui->boxMeasure->setChecked(false);
	ui->boxMeasure->setVisible(false);
	ui->btnMeasure->setVisible(false);
#endif

#ifdef __ANDROID__
	ui->btnAddRef->setIconSize(QSize(24, 24));
#endif

}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
	disconnect(prefPanel, &Preferences::notify, this, &SpectrumAnalyzer::readPreferences);
	ui->runSingleWidget->toggle(false);
	setDynamicProperty(runButton(), "disabled", false);

	if (saveOnExit) {
		api->save(*settings);
	}

	delete sample_timer;

	delete api;
	for (auto it = ch_api.begin(); it != ch_api.end(); ++it) {
		delete *it;
	}
	for (auto it = marker_api.begin(); it != marker_api.end(); ++it) {
		delete *it;
	}

#ifdef SPECTRAL_MSR
	for (auto it = d_measureObjs.begin(); it != d_measureObjs.end(); ++it) {
		delete *it;
	    }
#endif

	if (iio) {
		bool started = isIioManagerStarted();

		if (started) {
			iio->lock();
		}

		for (unsigned int i = 0; i < m_adc_nb_channels; i++) {
			iio->disconnect(fft_ids[i]);
		}

		if (started) {
			iio->unlock();
		}

		delete[] fft_ids;
	}

	delete ui;
}

void SpectrumAnalyzer::setNativeDialogs(bool nativeDialogs)
{
	Tool::setNativeDialogs(nativeDialogs);
	fft_plot->setUseNativeDialog(nativeDialogs);
}

void SpectrumAnalyzer::readPreferences() {
	bool showFps = prefPanel->getShow_plot_fps();
	fft_plot->setVisibleFpsLabel(showFps);
	fft_plot->setVisiblePeakSearch(prefPanel->getSpectrum_visible_peak_search());
	ui->instrumentNotes->setVisible(prefPanel->getInstrumentNotesActive());
}

void SpectrumAnalyzer::btnExportClicked()
{
	QStringList filter;
	filter += QString(tr("Comma-separated values files (*.csv)"));
	filter += QString(tr("Tab-delimited values files (*.txt)"));
	filter += QString(tr("All Files(*)"));

	QString selectedFilter = filter[0];

	QString fileName = QFileDialog::getSaveFileName(this,
	    tr("Export"), "", filter.join(";;"),
	    &selectedFilter, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	if (fileName.split(".").size() <= 1) {
		// file name w/o extension. Let's append it
		QString ext = selectedFilter.split(".")[1].split(")")[0];
		fileName += "." + ext;
	}

	if (!fileName.isEmpty()) {
		FileManager fm("Spectrum Analyzer");
		fm.open(fileName, FileManager::EXPORT);

		QVector<double> frequency_data;
		int nr_samples = fft_plot->Curve(0)->data()->size();
		for (int i = 0; i < nr_samples; ++i) {
			frequency_data.push_back(fft_plot->Curve(0)->sample(i).x());
		}

		fm.save(frequency_data, "Frequency(Hz)");

		QString channelDetails = "";

		for (int i = 0; i < channels.size(); ++i) {
			QVector<double> data;
			for (int j = 0; j < nr_samples; ++j) {
				data.push_back(fft_plot->Curve(i)->sample(j).y());
			}
			QString unit = ui->lblMagUnit->text();
			fm.save(data, "Amplitude CH" + QString::number(i + 1)
				+ "(" + unit + ")");

			/* Save information about the channels averaging type, window and
			* averaging value */
			channel_sptr sc = channels.at(i);

			auto iterAvg = std::find_if(avg_types.begin(), avg_types.end(),
			[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
				return p.second == sc->averageType();
			});

			if (iterAvg != avg_types.end()) {
				channelDetails += (*iterAvg).first;
				channelDetails += ",";
			}

			auto iterWin = std::find_if(win_types.begin(), win_types.end(),
			[&](const std::pair<QString, FftWinType>& p) {
				return p.second == sc->fftWindow();
			});

			if (iterWin != win_types.end()) {
				channelDetails += (*iterWin).first;
				channelDetails += ",";
			}

			channelDetails += QString::number(sc->averaging());
			channelDetails += ",";

		}

		fm.setAdditionalInformation(channelDetails);

		fm.performWrite();
	}

}

void SpectrumAnalyzer::triggerRightMenuToggle(CustomPushButton *btn, bool checked)
{
	if (ui->rightMenu->animInProgress()) {
		menuButtonActions.enqueue(
			QPair<CustomPushButton *, bool>(btn, checked));
	} else {
		toggleRightMenu(btn, checked);
	}
}

void SpectrumAnalyzer::toggleRightMenu(CustomPushButton *btn, bool checked)
{
	int index = 0;
	bool chSettings = false;
	int id = -1;

	if (btn != ui->btnSettings && btn != ui->btnToolSettings) {
		if (!menuOrder.contains(btn)) {
			menuOrder.push_back(btn);
		} else {
			menuOrder.removeOne(btn);
			menuOrder.push_back(btn);
		}
	}

	for (int i = 0; i < channels.size(); ++i) {
		if (channels.at(i).get()->widget()->menuButton() == btn) {
			chSettings = true;
			id = channels.at(i).get()->widget()->id();
		}
	}

	/* Also check if one of the reference channels settings btn
	 * was toggled */
	for (unsigned int i = 0; i < nb_ref_channels; ++i) {
		ChannelWidget *cw = referenceChannels.at(i);
		if (cw->menuButton() == btn) {
			chSettings = true;
			id = cw->id();
		}
	}

	if (!chSettings) {
		if (btn == ui->btnToolSettings) {
			index = 1;
		} else if (btn == ui->btnSweep) {
			index = 2;
		} else if (btn == ui->btnMarkers) {
			index = 3;
		} else if (btn == ui->btnAddRef) {
			index = 4;

#ifdef SPECTRAL_MSR
		} else if (btn == ui->btnMeasure) {
			index = 5;
		} else if (btn == ui->btnCursors) {
			index = 6;
		}
#else
		} else if (btn == ui->btnCursors) {
			index = 5;
		}
#endif
	}

	if (id != -1) {
		updateChannelSettingsPanel(id);
	}

	if (marker_menu_opened) {
		updateMarkerMenu(crt_channel_id);
	}

	if (!ui->btnToolSettings->isChecked()) {
		ui->btnSettings->setChecked(!!this->settings_group->checkedButton());
	}

	if (checked) {
		ui->stackedWidget->setCurrentIndex(index);
	}

	ui->rightMenu->toggleMenu(checked);
}

void SpectrumAnalyzer::rightMenuFinished(bool opened)
{
	Q_UNUSED(opened)

	while(menuButtonActions.size()) {
		auto pair = menuButtonActions.dequeue();
		toggleRightMenu(pair.first, pair.second);
	}
}

void SpectrumAnalyzer::on_boxCursors_toggled(bool on)
{
	fft_plot->setHorizCursorsEnabled(
				on ? cr_ui->vCursorsEnable->isChecked() : false);
	fft_plot->setVertCursorsEnabled(
				on ? cr_ui->hCursorsEnable->isChecked() : false);

	fft_plot->trackModeEnabled(on ? cr_ui->btnNormalTrack->isChecked() : true);

	if (on) {
		fft_plot->setCursorReadoutsVisible(on);
	} else {
		if (ui->btnCursors->isChecked())
			ui->btnCursors->setChecked(false);

		menuOrder.removeOne(ui->btnCursors);
	}
}

void SpectrumAnalyzer::on_btnToolSettings_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
	if (checked) {
		ui->btnSettings->setChecked(!checked);
	}
}

void SpectrumAnalyzer::on_btnSettings_clicked(bool checked)
{
	CustomPushButton *btn = nullptr;
	if (checked && !menuOrder.isEmpty()) {
		btn = menuOrder.back();
	} else {
		btn = static_cast<CustomPushButton *>(
			settings_group->checkedButton());
	}

	if (btn) {
		btn->setChecked(checked);
	}
}

void SpectrumAnalyzer::on_btnCursors_toggled(bool checked)
{
	triggerRightMenuToggle(
				static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void SpectrumAnalyzer::on_btnSweep_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void SpectrumAnalyzer::on_btnMarkers_toggled(bool checked)
{
	marker_menu_opened = checked;

	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

#ifdef SPECTRAL_MSR
void SpectrumAnalyzer::on_btnMeasure_toggled(bool checked) {
    triggerRightMenuToggle(static_cast<CustomPushButton *>(QObject::sender()),
			   checked);
}

void SpectrumAnalyzer::measure_settings_init()
{
    measure_settings = new MeasureSettings(&d_measureObjs, this, false);
    int measure_panel = ui->stackedWidget->insertWidget(5, measure_settings);

    ui->btnMeasure->setDisabled(true);

    connect(measure_settings,
	SIGNAL(measurementActivated(int, int)),
	SLOT(onMeasurementActivated(int, int)));

    connect(measure_settings,
	SIGNAL(measurementDeactivated(int, int)),
	SLOT(onMeasurementDeactivated(int, int)));

    connect(measure_settings,
	SIGNAL(measurementSelectionListChanged()),
	SLOT(onMeasurementSelectionListChanged()));

    connect(fft_plot, SIGNAL(channelAdded(int)),
	measure_settings, SLOT(onChannelAdded(int)));

    connect(this, SIGNAL(selectedChannelChanged(int)),
	measure_settings, SLOT(setSelectedChannel(int)));

    connect(ui->boxMeasure, SIGNAL(toggled(bool)),
	 SLOT(setMeasuremensEnabled(bool)));

}

void SpectrumAnalyzer::onChannelAdded(int chnIdx)
{
    Measure *measure = nullptr;

    if(fft_plot->isReferenceWaveform(chnIdx))
    {
	 int idx = chnIdx - fft_plot->getYdata_size();
	 size_t curve_size = fft_plot->getCurveSize(chnIdx);
	 double* data = new double [curve_size]();
	 measure = new Measure(chnIdx, data, curve_size,
			       nullptr, false);
    }
    else
    {
	int64_t numPoints = fft_plot->getNumPoints() / 2;
	std::vector<double> scale_factor = fft_plot->getScaleFactor();

	//std::vector<double*> data = fft_plot->getOrginal_data();
	//int count = fft_plot->countReferenceWaveform(chnIdx);
	double* data = new double [numPoints]();
	measure = new Measure(chnIdx, data,
		numPoints, nullptr, false);
    }
    measure->setAdcBitCount(12);
    d_measureObjs.push_back(measure);

}


void SpectrumAnalyzer::update_measure_for_channel(int ch_idx) {
    ChannelWidget *chn_widget = getChannelWidgetAt(ch_idx);

    measure_settings->setChannelName(chn_widget->fullName());
    measure_settings->setChannelUnderlineColor(chn_widget->color());
}


void SpectrumAnalyzer::measure_panel_init() {
    measurePanel = new QWidget(this);
    measure_panel_ui = new Ui::MeasurementsPanel();
    measure_panel_ui->setupUi(measurePanel);

    connect(measure_panel_ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged,
	measure_panel_ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::setRange);

    connect(measure_panel_ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::valueChanged,
	measure_panel_ui->scrollArea->horizontalScrollBar(), &QScrollBar::setValue);
    connect(measure_panel_ui->scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged,
	measure_panel_ui->scrollArea_2->horizontalScrollBar(), &QScrollBar::setValue);

    connect(measure_panel_ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged,
	[=](double v1, double v2){
	measure_panel_ui->scrollArea_2->widget()->setFixedWidth(measure_panel_ui->scrollAreaWidgetContents->width());
    });

    measurePanel->hide();

    connect(this, SIGNAL(measurementsAvailable()),
	    SLOT(onMeasuremetsAvailable()));

}

void SpectrumAnalyzer::init_selected_measurements(int chnIdx,
			  std::vector<int> measureIdx)
{
    auto measurements_val = measurements(chnIdx);
    for (int i = 0; i < measureIdx.size(); i++) {
	measurements_val[measureIdx[i]]->setEnabled(true);
	measure_settings->onMeasurementActivated(
		    chnIdx, measureIdx[i], true);
    }
    measure_settings->loadMeasurementStatesFromData();
    onMeasurementSelectionListChanged();
}

std::shared_ptr<MeasurementData> SpectrumAnalyzer::measurement(int id, int chnIdx)
{
    Measure *measure = measureOfChannel(chnIdx);
    if (measure)
	return measure->measurement(id);
    else
	return std::shared_ptr<MeasurementData>();
}

void SpectrumAnalyzer::onMeasurementActivated(int id, int chnIdx)
{
    int oldActiveMeasCount = activeMeasurementsCount(chnIdx);

    auto mList = measurements(chnIdx);
    mList[id]->setEnabled(true);
    measurements_data.push_back(mList[id]);
    measureCreateAndAppendGuiFrom(*mList[id]);

    if (oldActiveMeasCount == 0) {
	measure();
    }

    measureLabelsRearrange();
}

void SpectrumAnalyzer::onMeasurementDeactivated(int id, int chnIdx)
{
    auto mList = measurements(chnIdx);
    QString name = mList[id]->name();

    mList[id]->setEnabled(false);

    auto it = find_if(measurements_data.begin(), measurements_data.end(),
	[&](std::shared_ptr<MeasurementData> const& p)
	{ return  (p->name() == name) && (p->channel() == chnIdx); });
    if (it != measurements_data.end()) {
	int i = it - measurements_data.begin();
	measurements_data.removeAt(i);
	measurements_gui.removeAt(i);
	measureLabelsRearrange();
    }
}

void SpectrumAnalyzer::onMeasurementSelectionListChanged()
{
    // Clear all measurements in list
    for (int i = 0; i < measurements_data.size(); i++) {
	measurements_data[i]->setEnabled(false);
    }
    measurements_data.clear();
    measurements_gui.clear();

    // Use the new list from MeasureSettings
    auto newList = measure_settings->measurementSelection();
    for (int i = 0; i < newList.size(); i++) {
	auto pMeasurement = measurement(newList[i].id(),
	    newList[i].channel_id());
	if (pMeasurement) {
	    pMeasurement->setEnabled(true);
	    measurements_data.push_back(pMeasurement);
	    measureCreateAndAppendGuiFrom(*pMeasurement);
	}
    }
    measureLabelsRearrange();
}

void SpectrumAnalyzer::measureCreateAndAppendGuiFrom(const MeasurementData&
	measurement)
{
    std::shared_ptr<MeasurementGui> p;

    switch(measurement.unitType()) {

    case MeasurementData::DECIBELS:
	p = std::make_shared<DecibelsMeasurementGui>();
	break;
    case MeasurementData::DECIBELS_TO_CARRIER:
	p = std::make_shared<DecibelstoCarrierMeasurementGui>();
	break;
    case MeasurementData::DIMENSIONLESS:
	p = std::make_shared<DimensionlessMeasurementGui>();
	break;
    default:
	break;
    }
    if (p)
	measurements_gui.push_back(p);
}

void SpectrumAnalyzer::measureLabelsRearrange()
{
    QWidget *container = measure_panel_ui->measurements->
		    findChild<QWidget *>("container");

    if (container) {
	measure_panel_ui->measurements->layout()->removeWidget(container);
	delete container;
    }

    container = new QWidget();
    container->setObjectName("container");
    if (!measure_panel_ui->measurements->layout()) {
	QVBoxLayout *measurementsLayout = new
		QVBoxLayout(measure_panel_ui->measurements);
	measurementsLayout->addWidget(container);
	measurementsLayout->setContentsMargins(0, 0, 0, 0);
    } else {
	measure_panel_ui->measurements->layout()->addWidget(container);
    }

    QGridLayout *gLayout = new QGridLayout(container);

    gLayout->setContentsMargins(0, 0, 0, 0);
    gLayout->setVerticalSpacing(5);
    gLayout->setHorizontalSpacing(5);
    int max_rows = 4;
    int nb_meas_added = 0;

    for (int i = 0; i < measurements_data.size(); i++) {

	int channel = measurements_data[i]->channel();
	if (channel >=  m_adc_nb_channels + nb_ref_channels) {
	    continue;
	}

	ChannelWidget *chn_widget = getChannelWidgetAt(channel);
	if (!chn_widget->enableButton()->isChecked()) {
	    continue;
	}

	QLabel *name = new QLabel();
	QLabel *value = new QLabel();


	int row = nb_meas_added % max_rows;
	int col = nb_meas_added / max_rows;

	gLayout->addWidget(name, row, 2 * col);

	QHBoxLayout *value_layout = new QHBoxLayout();
	value_layout->setContentsMargins(0, 0, 10, 0);
	value_layout->addWidget(value);
	gLayout->addLayout(value_layout, row, 2 * col + 1);

	measurements_gui[i]->init(name, value);
	double pb_atten = 1;
	measurements_gui[i]->update(*(measurements_data[i]),
			pb_atten);
	measurements_gui[i]->setLabelsColor(fft_plot->getLineColor(channel));

	nb_meas_added++;
    }
}

QList<std::shared_ptr<MeasurementData>> SpectrumAnalyzer::measurements(int chnIdx)
{
    //POATE O SA POT PUNE DIRECT MEASURE OF CHANNALE, IN LOC DE FCT ASTA??
    Measure *measure = measureOfChannel(chnIdx);

    if (measure)
	return measure->measurments();
    else
	return QList<std::shared_ptr<MeasurementData>>();
}

Measure* SpectrumAnalyzer::measureOfChannel(int chnIdx) const
{
    Measure *measure = nullptr;

    auto it = std::find_if(d_measureObjs.begin(), d_measureObjs.end(),
	[&](Measure *m) { return m->channel() == chnIdx; });
    if (it != d_measureObjs.end())
	measure = *it;

    return measure;
}

int SpectrumAnalyzer::activeMeasurementsCount(int chnIdx)
{
    int count = -1;
    Measure *measure = measureOfChannel(chnIdx);

    if (measure)
	count = measure->activeMeasurementsCount();

    return count;
}

bool SpectrumAnalyzer::measurementsEnabled()
{
    return d_measurementsEnabled;
}

void SpectrumAnalyzer::setMeasuremensEnabled(bool en)
{
    d_measurementsEnabled = en;

    if (en) {
	ui->btnMeasure->setEnabled(true);
    } else {
	    if (ui->btnMeasure->isChecked())
		    ui->btnMeasure->setChecked(false);

	    ui->btnMeasure->setEnabled(false);

	    menuOrder.removeOne(ui->btnMeasure);
    }
}

void SpectrumAnalyzer::computeMeasurementsForChannel(unsigned int chnIdx, unsigned int sampleRate)
{
    if (chnIdx >= d_measureObjs.size()) {
	return;
    }

    Measure *measure = d_measureObjs[chnIdx];
    measure->setSampleRate(sampleRate);
    measure->measure();

    Q_EMIT measurementsAvailable();
}

void SpectrumAnalyzer::measure()
{
    for (int i = 0; i < d_measureObjs.size(); i++) {
	Measure *measure = d_measureObjs[i];
	if (measure->activeMeasurementsCount() > 0) {
	    measure->setSampleRate(fft_plot->sampleRate());
	    measure->measure();
	}
    }
}

void SpectrumAnalyzer::onNewDataReceived()
{
    int ref_idx = 0;
    for (int i = 0; i < d_measureObjs.size(); i++) {
	Measure *measure = d_measureObjs[i];
	int chn = measure->channel();
	if (fft_plot->isReferenceWaveform(chn)) {
	    size_t curve_size = fft_plot->getCurveSize(chn);
	    measure->setDataSource(fft_plot->getRef_data()[ref_idx],
			   curve_size / 2);
	    ref_idx++;
	}
	else {
	    int64_t numPoints = fft_plot->getNumPoints() / 2;
	    std::vector<double*> data = fft_plot->getOrginal_data();
	    std::vector<double> scale_factor = fft_plot->getScaleFactor();

	    //int count = fft_plot->countReferenceWaveform(chn);
	    //chn = chn - count;
	    for (int s = 0; s < numPoints; s++) {
		data[chn][s] = sqrt(data[chn][s]) * scale_factor[chn] / numPoints;
	    }
	    measure->setDataSource(data[chn], numPoints);
	}
	measure->setSampleRate(sample_rate);
	measure->measure();
    }

    Q_EMIT measurementsAvailable();
}

void SpectrumAnalyzer::onMeasuremetsAvailable()
{
    measureUpdateValues();
}

void SpectrumAnalyzer::measureUpdateValues()
{

    for (int i = 0; i < measurements_data.size(); i++) {
	int channel = measurements_data[i]->channel();
	ChannelWidget *chn_widget = getChannelWidgetAt(channel);
	if (!chn_widget->enableButton()->isChecked()) {
	    continue;
	}
	measurements_gui[i]->update(*(measurements_data[i]),
			1.0f); // de modificat scale-ul
    }
}

void SpectrumAnalyzer::on_boxMeasure_toggled(bool checked) {
  if (checked) {
      update_measure_for_channel(crt_channel_id);
  } else {
    if (ui->btnMeasure->isChecked())
      ui->btnMeasure->setChecked(false);
    menuOrder.removeOne(ui->btnMeasure);
  }
  measurePanel->setVisible(checked);
}
#endif

void SpectrumAnalyzer::on_btnAddRef_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void SpectrumAnalyzer::on_btnBrowseFile_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this,
	    tr("Export"), "", tr("Comma-separated values files (*.csv);;"
				       "Tab-delimited values files (*.txt)"),
	    nullptr, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	FileManager fm("Spectrum Analyzer");

	ui->importSettings->clear();
	import_data.clear();

	try {
		fm.open(fileName, FileManager::IMPORT);

		for (int i = 0; i < fm.getNrOfChannels(); ++i) {
			/* Amplitude CHX UNIT => mid(10, 3) strip CHX from column name */
			QString chn_name = fm.getColumnName(i).mid(10, 3);
			chn_name = (chn_name == "") ? "CH" + i : chn_name;
			ui->importSettings->addChannel(i, chn_name);
		}

		QVector<QVector<double>> data = fm.read();
		for (int i = 0; i < data.size(); ++i) {
			import_data.push_back(data[i]);
		}

		QStringList channelDetails = fm.getAdditionalInformation();
		for (int i = 0; i < channelDetails.size() / 3; ++i) {
			QStringList currentChannelDetails;
			for (int j = 0; j < 3; ++j) {
				currentChannelDetails.push_back(channelDetails[i * 3 + j]);
			}
			importedChannelDetails.push_back(currentChannelDetails);
		}

		ui->fileLineEdit->setText(fileName);
		ui->fileLineEdit->setToolTip(fileName);

		ui->btnImport->setEnabled(true);
		ui->importSettings->setEnabled(true);


	} catch (FileManagerException &e) {
		ui->fileLineEdit->setText(e.what());
		ui->fileLineEdit->setToolTip("");
		ui->btnImport->setDisabled(true);
		ui->importSettings->setDisabled(true);
	}
}

void SpectrumAnalyzer::on_btnImport_clicked()
{
	QMap<int, bool> import_map = ui->importSettings->getExportConfig();

	auto keys = import_map.keys();
	for (int key : qAsConst(keys)) {
		if (import_map[key]) {
			add_ref_waveform(key);
		}
	}
}

void SpectrumAnalyzer::cursor_panel_init()
{
	cr_ui = new Ui::CursorsSettings;
	cr_ui->setupUi(ui->cursorsSettings);
	setDynamicProperty(cr_ui->btnLockHorizontal, "use_icon", true);
	setDynamicProperty(cr_ui->btnLockVertical, "use_icon", true);

	connect(cr_ui->btnLockHorizontal, &QPushButton::toggled,
		fft_plot, &FftDisplayPlot::setHorizCursorsLocked);
	connect(cr_ui->btnLockVertical, &QPushButton::toggled,
		fft_plot, &FftDisplayPlot::setVertCursorsLocked);

	cursorsPositionButton = new CustomPlotPositionButton(cr_ui->posSelect);

	connect(cr_ui->hCursorsEnable, SIGNAL(toggled(bool)),
		fft_plot, SLOT(setVertCursorsEnabled(bool)));
	connect(cr_ui->vCursorsEnable, SIGNAL(toggled(bool)),
		fft_plot, SLOT(setHorizCursorsEnabled(bool)));
	connect(cr_ui->btnNormalTrack, &QPushButton::toggled,
		this, &SpectrumAnalyzer::toggleCursorsMode);

	cr_ui->horizontalSlider->setMaximum(100);
	cr_ui->horizontalSlider->setMinimum(0);
	cr_ui->horizontalSlider->setSingleStep(1);

	connect(cr_ui->horizontalSlider, &QSlider::valueChanged, [=](int value){
		cr_ui->transLabel->setText(tr("Transparency ") + QString::number(value) + "%");
		fft_plot->setCursorReadoutsTransparency(value);
	});
	cr_ui->horizontalSlider->setSliderPosition(0);

	connect(cursorsPositionButton, &CustomPlotPositionButton::positionChanged,
		[=](CustomPlotPositionButton::ReadoutsPosition position){
		fft_plot->moveCursorReadouts(position);
	});

}


void SpectrumAnalyzer::toggleCursorsMode(bool toggled)
{
	cr_ui->hCursorsEnable->setEnabled(toggled);
	cr_ui->vCursorsEnable->setEnabled(toggled);

	if (toggled) {
		fft_plot->setVertCursorsEnabled(hCursorsEnabled);
		fft_plot->setHorizCursorsEnabled(vCursorsEnabled);
//		cursor_readouts_ui->TimeCursors->setVisible(vCursorsEnabled);
//		cursor_readouts_ui->VoltageCursors->setVisible(hCursorsEnabled);
	} else {
		hCursorsEnabled = cr_ui->hCursorsEnable->isChecked();
		vCursorsEnabled = cr_ui->vCursorsEnable->isChecked();
		fft_plot->setVertCursorsEnabled(true);
		fft_plot->setHorizCursorsEnabled(true);
//		cursor_readouts_ui->TimeCursors->setVisible(true);
//		cursor_readouts_ui->VoltageCursors->setVisible(true);
	}

	cr_ui->btnLockVertical->setEnabled(toggled);
	fft_plot->trackModeEnabled(toggled);
}

void SpectrumAnalyzer::onCursorReadoutsChanged(struct cursorReadoutsText data)
{
	fillCursorReadouts(data);
}

void SpectrumAnalyzer::fillCursorReadouts(const struct cursorReadoutsText& data)
{
	//needs to be filled when measure ui is added
	//    cursor_readouts_ui->cursorT1->setText(data.t1);
	//    cursor_readouts_ui->cursorT2->setText(data.t2);
	//    cursor_readouts_ui->timeDelta->setText(data.tDelta);
	//    cursor_readouts_ui->frequencyDelta->setText(data.freq);
	//    cursor_readouts_ui->cursorV1->setText(data.v1);
	//    cursor_readouts_ui->cursorV2->setText(data.v2);
	//    cursor_readouts_ui->voltageDelta->setText(data.vDelta);
}

QString SpectrumAnalyzer::getReferenceChannelName() const
{
	int current = 1;
	for (; current <= MAX_REF_CHANNELS; ++current) {
		bool isOk = true;
		for (const auto &ref_channel : referenceChannels) {
			QString shortName = ref_channel->shortName();
			int channel_counter = shortName.midRef(shortName.size() - 1).toInt();
			if (current == channel_counter) {
				isOk = false;
				break;
			}
		}
		if (isOk) {
			return QString("REF %1").arg(current);
		}
	}

	return QString("REF %1").arg(current);
}

void SpectrumAnalyzer::setYAxisUnit(const QString& unit)
{
	if (unit == "dBFS" || unit == "dBu" || unit == "dBV") {
		fft_plot->setLeftVertAxisUnit("dB");
	} else if (unit == "Vpeak" || unit == "Vrms" || unit == "V/√Hz") {
		fft_plot->setLeftVertAxisUnit("V");
	}
}

void SpectrumAnalyzer::add_ref_waveform(QVector<double> xData, QVector<double> yData)
{
	if (nb_ref_channels == MAX_REF_CHANNELS) {
		return;
	}

	unsigned int curve_id = m_adc_nb_channels + nb_ref_channels;

	QString qname = getReferenceChannelName();

	fft_plot->registerReferenceWaveform(qname, xData, yData);

	auto channelWidget = new ChannelWidget(curve_id, true, false, fft_plot->getLineColor(curve_id));
	channelWidget->setShortName(qname);
	channelWidget->nameButton()->setText(channelWidget->shortName());
	channelWidget->setReferenceChannel(true);
	channelWidget->deleteButton()->setProperty(
		"curve_name", QVariant(qname));

	settings_group->addButton(channelWidget->menuButton());
	channels_group->addButton(channelWidget->nameButton());
	referenceChannels.push_back(channelWidget);

	connect(channelWidget, &ChannelWidget::menuToggled,
		this, &SpectrumAnalyzer::onChannelSettingsToggled);
	connect(channelWidget, &ChannelWidget::selected,
		this, &SpectrumAnalyzer::onChannelSelected);
	connect(channelWidget, &ChannelWidget::deleteClicked,
		this, &SpectrumAnalyzer::onReferenceChannelDeleted);
	connect(channelWidget, &ChannelWidget::enabled,
		this, &SpectrumAnalyzer::onChannelEnabled);

	ui->channelsList->addWidget(channelWidget);

	/* Increase ref channels count */
	nb_ref_channels++;

	if (!ui->btnMarkers->isEnabled()) {
		QSignalBlocker(ui->btnMarkers);
		ui->btnMarkers->setEnabled(true);
	}

	if (nb_ref_channels == MAX_REF_CHANNELS) {
		ui->btnAddRef->hide();
	}

#ifdef SPECTRAL_MSR
	init_selected_measurements(curve_id, {0, 1, 4, 5});

	computeMeasurementsForChannel(curve_id, sample_rate);
#endif
}

void SpectrumAnalyzer::add_ref_waveform(unsigned int chIdx)
{
	QVector<double> xData;
	QVector<double> yData;

	for (int i = 0; i < import_data.size(); ++i) {
		xData.push_back(import_data[i][0]);
		yData.push_back(import_data[i][chIdx + 1]);
	}

	add_ref_waveform(xData, yData);
}

#ifdef SPECTRAL_MSR
void SpectrumAnalyzer::cleanUpMeasurementsBeforeChannelRemoval(int chnIdx)
{
    Measure *measure = measureOfChannel(chnIdx);
    if (measure) {
	int pos = d_measureObjs.indexOf(measure);
	for (int i = pos + 1; i < d_measureObjs.size(); i++) {
	    d_measureObjs[i]->setChannel(
		d_measureObjs[i]->channel() - 1);
	}
	d_measureObjs.removeOne(measure);
	delete measure;
    }
}
#endif

void SpectrumAnalyzer::onReferenceChannelDeleted()
{
	if (nb_ref_channels - 1 < MAX_REF_CHANNELS) {
		ui->btnAddRef->show();
	}

	ChannelWidget *channelWidget = static_cast<ChannelWidget *>(QObject::sender());
	QAbstractButton *delBtn = channelWidget->deleteButton();
	QString qname = delBtn->property("curve_name").toString();
	int curve_id = channelWidget->id();

	/*If there are no more channels enabled, we should
	disable the measurements.*/
	bool shouldDisable = true;

	for (unsigned int i = 0; i < m_adc_nb_channels + nb_ref_channels; i++) {
		ChannelWidget *cw = static_cast<ChannelWidget *>(
		    ui->channelsList->itemAt(i)->widget());
		if (curve_id == cw->id())
		    continue;
		if (cw->enableButton()->isChecked())
		    shouldDisable = false;
	    }

#ifdef SPECTRAL_MSR
	if (shouldDisable)
		measure_settings->disableDisplayAll();

	measure_settings->onChannelRemoved(channelWidget->id());

	cleanUpMeasurementsBeforeChannelRemoval(channelWidget->id());
#endif

	fft_plot->unregisterReferenceWaveform(qname);
	ui->channelsList->removeWidget(channelWidget);

	/* Check if there are other channel widgets that are enabled */
	bool channelsEnabled = false;

	referenceChannels.erase(std::find(referenceChannels.begin(),
					  referenceChannels.end(),
					  channelWidget));

	/* Update the id of the remaining channels */
	int id = m_adc_nb_channels;
	for (auto iter = referenceChannels.begin();
	     iter != referenceChannels.end(); ++iter) {
		(*iter)->setId(id++);
	}

	nb_ref_channels--;

	if (channelWidget->id() < crt_channel_id) {
		crt_channel_id--;
		Q_EMIT selectedChannelChanged(crt_channel_id);

#ifdef SPECTRAL_MSR
		update_measure_for_channel(crt_channel_id);
#endif

	} else if (channelWidget->id() == crt_channel_id) {
		for (int i = 0; i < m_adc_nb_channels + nb_ref_channels; ++i) {
			auto cw = getChannelWidgetAt(i);
			if (cw == channelWidget) {
				continue;
			}
			if (cw->enableButton()->isChecked()) {
				channelsEnabled = true;
				Q_EMIT selectedChannelChanged(0);

#ifdef SPECTRAL_MSR
				update_measure_for_channel(0);
#endif

				cw->nameButton()->setChecked(true);
				Q_EMIT selectedChannelChanged(cw->id());
#ifdef SPECTRAL_MSR
				update_measure_for_channel(cw->id());
#endif

				break;
			}
		}
		if (!channelsEnabled) {
		    crt_channel_id = 0;
		    Q_EMIT selectedChannelChanged(0);
		}
	}

	if (ui->btnMarkers->isChecked() && !channelsEnabled) {
		ui->btnMarkers->setChecked(false);
		menuButtonActions.removeAll(QPair<CustomPushButton*, bool>
					    (ui->btnMarkers, true));
		menuOrder.removeAll(ui->btnMarkers);
		ui->btnMarkers->blockSignals(true);
		ui->btnMarkers->setDisabled(true);
		ui->btnMarkers->blockSignals(false);
	}

	if (channelWidget->menuButton()->isChecked()) {
		channelWidget->menuButton()->setChecked(false);
		menuButtonActions.removeAll(QPair<CustomPushButton*, bool>
					    (static_cast<CustomPushButton*>(channelWidget->menuButton()), true));
		menuOrder.removeAll(static_cast<CustomPushButton *>(channelWidget->menuButton()));
	}

#ifdef SPECTRAL_MSR
	onMeasurementSelectionListChanged();
#endif
	delete channelWidget;
}

void SpectrumAnalyzer::run()
{
	runStopToggled(true);
}

void SpectrumAnalyzer::stop()
{
	runStopToggled(false);
}

void SpectrumAnalyzer::runStopToggled(bool checked)
{
	if (checked) {
		if (iio) {
			writeAllSettingsToHardware();
		}

		fft_plot->presetSampleRate(sample_rate);
		fft_sink->set_samp_rate(sample_rate);
		m_time_start = std::chrono::system_clock::now();
		start_blockchain_flow();
		sample_timer->start(TIMER_TIMEOUT_MS);
	} else {
		stop_blockchain_flow();
		sample_timer->stop();
	}

	if (!checked) {
		fft_plot->resetAverageHistory();
	}
	fft_plot->startStop(checked);
	m_running = checked;
}

void SpectrumAnalyzer::build_gnuradio_block_chain()
{
	fft_sink = adiscope::scope_sink_f::make(fft_size, m_max_sample_rate,
						"Osc Frequency", m_adc_nb_channels,
	                                        (QObject *)fft_plot);
	fft_sink->set_trigger_mode(TRIG_MODE_TAG, 0, "buffer_start");

	double targetFps = getScopyPreferences()->getTarget_fps();
	fft_sink->set_update_time(1.0/targetFps);

	bool started = isIioManagerStarted();

	if (started) {
		iio->lock();
	}

	bool canConvRawToVolts = m_m2k_analogin ? true : false;

	if (canConvRawToVolts) {
		for (int i = 0; i < m_adc_nb_channels; i++) {
			libm2k::analog::ANALOG_IN_CHANNEL chn = static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(i);
			fft_plot->setScaleFactor(i, m_m2k_analogin->getScalingFactor(chn));
		}
	}

	fft_ids = new iio_manager::port_id[m_adc_nb_channels];

	for (int i = 0; i < m_adc_nb_channels; i++) {
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
	fft_sink = adiscope::scope_sink_f::make(fft_size, m_max_sample_rate,
						"Osc Frequency", m_adc_nb_channels,
	                                        (QObject *)fft_plot);

	double targetFps = getScopyPreferences()->getTarget_fps();
	fft_sink->set_update_time(1.0/targetFps);

	top_block = gr::make_top_block("spectrum_analyzer");

	for (int i = 0; i < m_adc_nb_channels; i++) {
		auto fft = gnuradio::get_initial_sptr(
		                   new fft_block(false, fft_size));
		auto ctm = gr::blocks::complex_to_mag_squared::make(1);

		auto siggen = gr::analog::sig_source_f::make(m_max_sample_rate,
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
		for (int i = 0; i < m_adc_nb_channels; i++) {
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
		for (int i = 0; i < m_adc_nb_channels; i++) {
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

	const bool visible = ((*it).second != FftDisplayPlot::AverageType::SAMPLE);
	if (!visible) {
		ui->spinBox_averaging->setValue(1);
	}
	ui->spinBox_averaging->setVisible(visible);
	ui->label_averaging->setVisible(visible);

	int crt_channel = channelIdOfOpenedSettings();

	if (crt_channel < 0) {
		qDebug(CAT_SPECTRUM_ANALYZER) << "invalid channel ID for the opened Settings menu";
		return;
	}

	auto avg_type = (*it).second;

	auto history_visible = canSwitchAverageHistory(avg_type);
	ui->btnHistory->setVisible(history_visible);
	ui->btnHistory->setChecked(true);
	ui->lbl_history->setVisible(history_visible);
	ui->btnApply->setVisible(visible);
	setCurrentAverageIndexLabel(crt_channel);

	if (avg_type != channels[crt_channel]->averageType()) {
		channels[crt_channel]->setAverageType(avg_type);
		if (channels[crt_channel]->canStoreAverageHistory()) {
			if (ui->btnHistory->isChecked()) {
				ui->spinBox_averaging->setMaximum(10000);
			} else {
				ui->spinBox_averaging->setMaximum(1000000);
			}
		} else {
			ui->spinBox_averaging->setMaximum(1000000);
		}
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
		qDebug(CAT_SPECTRUM_ANALYZER) << "invalid channel ID for the opened Settings menu";
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

void SpectrumAnalyzer::on_comboBox_line_thickness_currentIndexChanged(int index)
{
	int crt_channel = channelIdOfOpenedSettings();

	if (crt_channel < 0) {
		qDebug(CAT_SPECTRUM_ANALYZER) << "invalid channel ID for the opened Settings menu";
		return;
	}

	qreal width = 0.5 * (index + 1);

	if (width != channels[crt_channel]->lineWidth()) {
		channels[crt_channel]->setLinewidth(width);
		fft_plot->setLineWidth(crt_channel, width);
		fft_plot->replot();
	}
}

void SpectrumAnalyzer::on_spinBox_averaging_valueChanged(int n)
{
	int crt_channel = channelIdOfOpenedSettings();

	if (crt_channel < 0) {
		qDebug(CAT_SPECTRUM_ANALYZER) << "invalid channel ID for the opened Settings menu";
		return;
	}

	if (n != channels[crt_channel]->averaging()) {
		channels[crt_channel]->setAveraging(n);
	}
}

void SpectrumAnalyzer::onChannelSettingsToggled(bool en)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());

	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(cw->menuButton()), en);
}

void SpectrumAnalyzer::updateChannelSettingsPanel(unsigned int id)
{
	ChannelWidget *cw = getChannelWidgetAt(id);

	selected_ch_settings = id;

	if (cw->isReferenceChannel()) {
		ui->btnSnapshot->setText(tr("Export"));
	} else {
		ui->btnSnapshot->setText(tr("Snapshot"));
	}

	if (id < m_adc_nb_channels) {
		channel_sptr sc = channels.at(id);

		QString style = QString("border: 2px solid %1").arg(cw->color().name());
		ui->lineChannelSettingsTitle->setStyleSheet(style);
		ui->channelSettingsTitle->setText(sc->name());

		/* Might be disabled */
		ui->comboBox_type->setEnabled(true);
		ui->comboBox_window->setEnabled(true);
		ui->spinBox_averaging->setEnabled(true);
		ui->btnHistory->setEnabled(true);
		ui->comboBox_line_thickness->setEnabled(true);

		/* Migh be hidden */
		ui->comboBox_type->setVisible(true);
		ui->comboBox_window->setVisible(true);
		ui->spinBox_averaging->setVisible(true);
		ui->btnHistory->setVisible(true);
		ui->comboBox_line_thickness->setVisible(true);

		ui->lbl_history->setVisible(true);
		ui->label_type->setVisible(true);
		ui->label_window->setVisible(true);
		ui->label_averaging->setVisible(true);
		ui->label_thickness->setVisible(true);

		auto it = std::find_if(avg_types.begin(), avg_types.end(),
		[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
			return p.second == sc->averageType();
		});

		const bool visible = (sc->averageType() != FftDisplayPlot::AverageType::SAMPLE);
		ui->spinBox_averaging->setVisible(visible);
		ui->label_averaging->setVisible(visible);
		ui->btnApply->setVisible(visible);

		auto history_visible = canSwitchAverageHistory(sc->averageType());
		ui->btnHistory->setVisible(history_visible);
		ui->btnHistory->setChecked(sc->isAverageHistoryEnabled());
		ui->lbl_history->setVisible(history_visible);

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
		int gainIdx = (sc->getGainMode() == libm2k::analog::PLUS_MINUS_25V) ? 0 : 1;
		ui->cmbGainMode->setCurrentIndex(gainIdx);

        int thicknessIdx = (int)(sc->lineWidth() / 0.5) - 1;
        ui->comboBox_line_thickness->setCurrentIndex(thicknessIdx);
	} else {
		QString style = QString("border: 2px solid %1").arg(cw->color().name());
		ui->lineChannelSettingsTitle->setStyleSheet(style);
		ui->channelSettingsTitle->setText(cw->nameButton()->text());


		if ((id - m_adc_nb_channels) < importedChannelDetails.size() &&
				importedChannelDetails[id - m_adc_nb_channels].size() == 3) {
			ui->comboBox_type->setCurrentText(importedChannelDetails[id - m_adc_nb_channels][0]);
			ui->comboBox_window->setCurrentText(importedChannelDetails[id - m_adc_nb_channels][1]);
			ui->spinBox_averaging->setValue(importedChannelDetails[id - m_adc_nb_channels][2].toInt());

			ui->comboBox_type->setDisabled(true);
			ui->comboBox_window->setDisabled(true);
			ui->spinBox_averaging->setDisabled(true);
			ui->btnHistory->setDisabled(true);
			ui->btnApply->setDisabled(true);
			ui->comboBox_line_thickness->setDisabled(true);
			ui->cmbGainMode->setDisabled(true);
		} else {
			ui->comboBox_type->setVisible(false);
			ui->comboBox_window->setVisible(false);
			ui->spinBox_averaging->setVisible(false);
			ui->btnHistory->setVisible(false);

			ui->lbl_history->setVisible(false);
			ui->label_type->setVisible(false);
                        ui->comboBox_line_thickness->setDisabled(false);

                        ui->label_type->setVisible(false);
			ui->label_window->setVisible(false);
			ui->label_averaging->setVisible(false);
			ui->label_thickness->setVisible(false);
		}
	}

}

ChannelWidget * SpectrumAnalyzer::getChannelWidgetAt(unsigned int id)
{
	for (int i = 0; i < channels.size(); ++i) {
		if (channels.at(i).get()->widget()->id() == id)
			return channels.at(i).get()->widget();
	}

	/* Also check if the id corresponds to one of the reference
	 * channel widgets and get an iterator pointing to that widget */
	auto cwIter = std::find_if(referenceChannels.begin(),
			       referenceChannels.end(),
			       [&id](ChannelWidget *referenceChannelWidget){
		return referenceChannelWidget->id() == id;
	});

	if (cwIter != referenceChannels.end()) {
		return *cwIter;
	}

	return nullptr;
}

void SpectrumAnalyzer::onChannelSelected(bool en)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());
	int chIdx = cw->id();

	if(crt_channel_id != chIdx)
	{
		crt_channel_id = chIdx;
		Q_EMIT selectedChannelChanged(chIdx);
	}

	if (!cw->isReferenceChannel()) {
		const bool visible = (channels[crt_channel_id]->averageType() != FftDisplayPlot::AverageType::SAMPLE);
		setCurrentAverageIndexLabel(crt_channel_id);
	} else {
		ui->lbl_crtAvgSample->setVisible(false);
	}

	fft_plot->bringCurveToFront(chIdx);
	if (marker_menu_opened) {
		triggerRightMenuToggle(
			static_cast<CustomPushButton *>(ui->btnMarkers), en);
	}

#ifdef SPECTRAL_MSR
	if (measurementsEnabled()) {
		update_measure_for_channel(chIdx);
	}
#endif
}

void SpectrumAnalyzer::updateMarkerMenu(unsigned int id)
{
	ChannelWidget *cw = getChannelWidgetAt(id);

	// Is this if branch required?
	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->replot();
	}

	// Update markers settings menu based on current channel
	if (cw->nameButton()->isChecked()) {
		ui->labelMarkerSettingsTitle->setText(cw->fullName());
		QString stylesheet = QString("border: 2px solid %1"
					    ).arg(cw->color().name());
		ui->lineMarkerSettingsTitle->setStyleSheet(stylesheet);


		marker_selector->blockSignals(true);

		for (int i = 0; i < fft_plot->markerCount(id); i++) {
			marker_selector->setButtonChecked(i,
							  fft_plot->markerEnabled(id, i));
		}

		marker_selector->blockSignals(false);

		updateCrtMrkLblVisibility();
	}
}

bool SpectrumAnalyzer::isIioManagerStarted() const
{
	return iio->started() && (ui->runSingleWidget->singleButtonChecked()
						  || ui->runSingleWidget->runButtonChecked());
}

void SpectrumAnalyzer::onChannelEnabled(bool en)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());

	if (en) {
		fft_plot->AttachCurve(cw->id());
		if (!ui->btnMarkers->isEnabled()) {
			ui->btnMarkers->blockSignals(true);
			ui->btnMarkers->setEnabled(true);
			ui->btnMarkers->blockSignals(false);
		}

		bool shouldActivate = true;
		if (cw->enableButton()->isChecked()) {
		    shouldActivate = false;
		}

		if (shouldActivate) {
		    //nu trebe asta
		    Q_EMIT selectedChannelChanged(cw->id());

#ifdef SPECTRAL_MSR
		    update_measure_for_channel(cw->id());
		    measure_settings->activateDisplayAll();
#endif
		}

	} else {
		bool allDisabled = true;
		bool shouldDisable = true;
		for (int i = 0; i < channels.size() + nb_ref_channels; i++) {
			ChannelWidget *cw = getChannelWidgetAt(i);

			if (cw->enableButton()->isChecked()) {
				cw->nameButton()->setChecked(true);
				allDisabled = false;
				shouldDisable = false;
				break;
			}
		}

#ifdef SPECTRAL_MSR
		if (shouldDisable) {
		    measure_settings->disableDisplayAll();
		}
#endif

		if (allDisabled) {
			QSignalBlocker(ui->btnMarkers);
			if (!ui->btnSweep->isChecked()) {
				QSignalBlocker(ui->btnSettings);
				ui->btnSettings->setChecked(false);
			}
			ui->btnMarkers->setChecked(false);
			ui->btnMarkers->setDisabled(true);
			menuButtonActions.removeAll(QPair<CustomPushButton*, bool>
						    (ui->btnMarkers, false));
			menuOrder.removeAll(ui->btnMarkers);
		}
		fft_plot->DetachCurve(cw->id());
	}

	for (int i = 0; i < fft_plot->markerCount(cw->id()); i++) {
		if (fft_plot->markerEnabled(cw->id(), i)) {
			fft_plot->setMarkerVisible(cw->id(), i, en);
		}
	}

#ifdef SPECTRAL_MSR
	measureLabelsRearrange();
#endif

	fft_plot->replot();
	updateRunButton(en);
}

void SpectrumAnalyzer::updateRunButton(bool ch_en)
{
	for (unsigned int i = 0; !ch_en && i < m_adc_nb_channels; i++) {
		QWidget *parent = ui->channelsList->itemAt(i)->widget();
		QCheckBox *box = parent->findChild<QCheckBox *>("box");
		ch_en = box->isChecked();
	}
	ui->runSingleWidget->setEnabled(ch_en);
	runButton()->setEnabled(ch_en);
	setDynamicProperty(runButton(), "disabled", !ch_en);


	if (!ch_en) {
		runButton()->setChecked(false);
		ui->runSingleWidget->toggle(false);
	}
}

void SpectrumAnalyzer::writeAllSettingsToHardware()
{
	try {
		if (m_m2k_analogin) {
			bool canConvRawToVolts = m_m2k_analogin ? true : false;
			m_m2k_analogin->setSampleRate(m_max_sample_rate);

			for (unsigned int i = 0; i < m_adc_nb_channels; i++) {
				libm2k::analog::ANALOG_IN_CHANNEL chn = static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(i);
				m_m2k_analogin->setVerticalOffset(chn, 0.0);
				m_m2k_analogin->setRange(chn, channels[i]->getGainMode());
				if (canConvRawToVolts) {
					fft_plot->setScaleFactor(i, m_m2k_analogin->getScalingFactor(chn));
				}
			}
			m_m2k_analogin->setOversamplingRatio(sample_rate_divider);
			auto trigger = m_m2k_analogin->getTrigger();
			if (trigger) {
				for (unsigned int i = 0; i < m_adc_nb_channels; i++) {
					trigger->setAnalogMode(i, libm2k::ALWAYS);
				}
			}
		} else {
			for (unsigned int i = 0; i < m_adc_nb_channels; i++) {
				m_generic_analogin->setSampleRate(i, m_generic_analogin->getMaximumSamplerate(i));
			}
		}
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e);
		qDebug(CAT_SPECTRUM_ANALYZER) << "Can't write settings to hardware: " << e.what();
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

	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::on_btnRightPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_higher_freq_peak(crt_channel_id, crt_marker);

	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::on_btnMaxPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_max_peak(crt_channel_id, crt_marker);

	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::on_cmb_rbw_currentIndexChanged(int index)
{
	double update_time = 1.0/getScopyPreferences()->getTarget_fps();
	switch(bin_sizes[index]) {
	case 1<<17:
		fft_sink->set_update_time(update_time * 2);
	break;
	case 1<<18:
		fft_sink->set_update_time(update_time * 4);
	break;
	default:
		fft_sink->set_update_time(update_time);
		break;
	}

	uint new_fft_size = bin_sizes[index];

	if (new_fft_size != fft_size) {
		setFftSize(new_fft_size);
	}

	marker_freq_pos->setMinValue(startStopRange->getStartValue());
	marker_freq_pos->setMaxValue(startStopRange->getStopValue());
	marker_freq_pos->setStep(2 * (startStopRange->getStopValue() -
					  startStopRange->getStartValue()) / bin_sizes[ui->cmb_rbw->currentIndex()]);
}

void SpectrumAnalyzer::setSampleRate(double sr)
{
	sample_rate_divider = (int)(m_max_sample_rate / sr);
	double new_sr = m_max_sample_rate / sample_rate_divider;

	if (new_sr == sample_rate) {
		return;
	}

	sample_rate = new_sr;
	if (isIioManagerStarted()) {
		stop_blockchain_flow();

		if (m_m2k_analogin) {
			try {
				m_m2k_analogin->setOversamplingRatio(sample_rate_divider);
			} catch (libm2k::m2k_exception &e) {
				HANDLE_EXCEPTION(e);
				qDebug(CAT_SPECTRUM_ANALYZER) << "Can't write oversampling ratio: " << e.what();
			}
		} else {
			try {
				for (unsigned int i = 0; i < m_adc_nb_channels; i++) {
					m_generic_analogin->setSampleRate(i, sr);
				}
			} catch (libm2k::m2k_exception &e) {
				HANDLE_EXCEPTION(e);
				qDebug(CAT_SPECTRUM_ANALYZER) << "Can't write sampling frequency: " << e.what();
			}
		}

		fft_plot->presetSampleRate(new_sr);
		fft_plot->resetAverageHistory();
		fft_sink->set_samp_rate(new_sr);

		sample_timer->stop();
		m_time_start = std::chrono::system_clock::now();

		start_blockchain_flow();
		sample_timer->start(TIMER_TIMEOUT_MS);
	}
}

void SpectrumAnalyzer::setFftSize(uint size)
{
	// TO DO: This is cumbersome. We shouldn't have to rebuild the entire
	//        block chain every time we need to change the FFT size. A
	//        spectrum_sink block similar to scope_sink_f would be better

	bool started = isIioManagerStarted();

	if (started) {
		iio->lock();
	}

	fft_size = size;
	fft_sink->set_nsamps(size);

	if (fft_plot->magnitudeType() == FftDisplayPlot::VROOTHZ) {
		fft_plot->setNbOverlappingAverages(m_nb_overlapping_avg);
	}

	for (int i = 0; i < channels.size(); i++) {
		auto fft = gnuradio::get_initial_sptr(
		                   new fft_block(false, size));

		iio->disconnect(fft_ids[i]);

		fft_ids[i] = iio->connect(fft, i, 0, true, fft_size * m_nb_overlapping_avg);

		iio->connect(fft, 0, channels[i]->ctm_block, 0);
		iio->connect(channels[i]->ctm_block, 0, fft_sink, i);

		if (started) {
			iio->start(fft_ids[i]);
		}

		channels[i]->fft_block = fft;
		channels[i]->setFftWindow(channels[i]->fftWindow(), size);

		iio->set_buffer_size(fft_ids[i], size * m_nb_overlapping_avg);
	}

	if (started) {
		iio->unlock();
	}

	sample_timer->stop();
	setCurrentSampleLabel(0.0);
	m_time_start = std::chrono::system_clock::now();
	sample_timer->start(TIMER_TIMEOUT_MS);
}


void SpectrumAnalyzer::refreshCurrentSampleLabel()
{
	if (!isIioManagerStarted()) {
		sample_timer->stop();
		return;
	}

	double time_acquisition = fft_size / sample_rate;

	auto time_now = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_done = time_now - m_time_start;

	if (time_acquisition < elapsed_done.count()) {
		setCurrentSampleLabel(100);
		return;
	}
	double time_percentage = elapsed_done.count() / time_acquisition * 100;
	if (time_percentage > 100) {
		time_percentage = 100;
		sample_timer->stop();
	}
	setCurrentSampleLabel(time_percentage);
}

void SpectrumAnalyzer::validateSpinboxAveraging()
{
	on_spinBox_averaging_valueChanged(ui->spinBox_averaging->value());
}

void SpectrumAnalyzer::setCurrentSampleLabel(double percentage)
{
	QString percentage_str = QString::number(percentage, 'f', 2);
	QString txt = QString(tr("Sample:") + " %1 % / %2 ").arg(percentage_str).arg(fft_size);
	ui->lbl_crtSampleNb->setText(txt);
}

bool SpectrumAnalyzer::canSwitchAverageHistory(FftDisplayPlot::AverageType avg_type)
{
	switch (avg_type) {
	case FftDisplayPlot::LINEAR_RMS:
	case FftDisplayPlot::LINEAR_DB:
		return true;
	default:
		return false;
	}
}

void SpectrumAnalyzer::on_btnDnAmplPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_lower_mag_peak(crt_channel_id, crt_marker);

	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::on_btnUpAmplPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_higher_mag_peak(crt_channel_id, crt_marker);

	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::onMarkerToggled(int id, bool on)
{
	setMarkerEnabled(crt_channel_id, id, on);

	// Add/remove the marker from the marker table
	if (on) {
		int mkType = fft_plot->markerType(crt_channel_id, id);
		if(mkType < 0)
			return;

		bool mkExists = ui->markerTable->isMarker(id, crt_channel_id);
		if (!mkExists) {
			ui->markerTable->addMarker(id, crt_channel_id, QString("M%1").arg(id + 1),
						   fft_plot->markerFrequency(crt_channel_id, id),
						   fft_plot->markerMagnitude(crt_channel_id, id),
						   markerTypes[mkType]);
		} else {
			ui->markerTable->updateMarker(id, crt_channel_id,
						      fft_plot->markerFrequency(crt_channel_id, id),
						      fft_plot->markerMagnitude(crt_channel_id, id),
						      markerTypes[mkType]);
		}
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
		double cf = startStopRange->getCenterValue();
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
				 fft_plot->markerMagnitude(chIdx, mkIdx), 'f', 3) +
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
	for (int c = 0; c < m_adc_nb_channels; c++) {
		for (int m = 0; m < fft_plot->markerCount(c); m++) {
			if (fft_plot->markerEnabled(c, m)) {
				int mkType = fft_plot->markerType(c, m);
				ui->markerTable->updateMarker(m, c,
				                              fft_plot->markerFrequency(c, m),
							      fft_plot->markerMagnitude(c, m),
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

	marker_freq_pos->silentSetValue(actual_freq);

	fft_plot->replot();
}

void SpectrumAnalyzer::updateMrkFreqPosSpinBtnLimits()
{
	marker_freq_pos->setMaxValue(startStopRange->getStopValue());
	marker_freq_pos->setStep(2 * startStopRange->getStopValue() /
	                             bin_sizes[ui->cmb_rbw->currentIndex()]);

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::updateMrkFreqPosSpinBtnValue()
{
	int crt_marker = marker_selector->selectedButton();

	if (!fft_plot->markerEnabled(crt_channel_id, crt_marker)) {
		return;
	}

//	if (!fft_plot->markerType(crt_channel_id, crt_marker) == 0) {
//		return;
//	}

	double freq = fft_plot->markerFrequency(crt_channel_id, crt_marker);

	if (freq != marker_freq_pos->value()) {
		marker_freq_pos->silentSetValue(freq);
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
	if (ui->runSingleWidget->singleButtonChecked()) {
		Q_EMIT started(false);
	} else {
		sample_timer->stop();
		m_time_start = std::chrono::system_clock::now();
		sample_timer->start(TIMER_TIMEOUT_MS);
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
	unsigned int stackedWidgetCurrentIdx = 0;

	bool started = isIioManagerStarted();
	if (started) {
		iio->lock();
	}
	for (unsigned int i = 0; i < m_adc_nb_channels; i++) {
		auto ov_factor = SpectrumChannel::win_overlap_factor(channels[i]->fftWindow());
		if (magType == FftDisplayPlot::VROOTHZ) {
			channels[i]->fft_block->set_overlap_factor(ov_factor);
		} else {
			channels[i]->fft_block->set_overlap_factor(0.0);
		}
	}

	if (started) {
		iio->unlock();
	}

	switch (magType) {
	case FftDisplayPlot::VPEAK:
	case FftDisplayPlot::VRMS:
		ui->divisionWidget->setVisible(true);
		fft_plot->useLogScaleY(false);
		ui->topWidget->setCurrentIndex(stackedWidgetCurrentIdx);
		ui->bottomWidget->setCurrentIndex(stackedWidgetCurrentIdx);
		ui->divisionWidget->setCurrentIndex(stackedWidgetCurrentIdx);
		break;
	case FftDisplayPlot::VROOTHZ:
		ui->divisionWidget->setVisible(false);
		fft_plot->useLogScaleY(true);
		stackedWidgetCurrentIdx = 1;
		ui->topWidget->setCurrentIndex(stackedWidgetCurrentIdx);
		ui->bottomWidget->setCurrentIndex(stackedWidgetCurrentIdx);
		ui->divisionWidget->setCurrentIndex(stackedWidgetCurrentIdx);
		top_scale->setValue(2.5E1);
		bottom_scale->setValue(1E-12);
		fft_plot->setAxisScale(QwtAxis::YLeft, bottom_scale->value(), top_scale->value());

		fft_plot->replot();
		fft_plot->setYaxisMajorTicksPos(fft_plot->axisScaleDiv(QwtAxis::YLeft).ticks(2));
		fft_plot->leftHandlesArea()->repaint();

		break;
	default:
		ui->divisionWidget->setVisible(true);
		fft_plot->useLogScaleY(false);
		ui->topWidget->setCurrentIndex(stackedWidgetCurrentIdx);
		ui->bottomWidget->setCurrentIndex(stackedWidgetCurrentIdx);
		ui->divisionWidget->setCurrentIndex(stackedWidgetCurrentIdx);
		break;
	}
	fft_plot->setMagnitudeType((*it).second);
	fft_plot->recalculateMagnitudes();

	setYAxisUnit(unit);

	fft_plot->replot();
	fft_plot->leftHandlesArea()->repaint();

	ui->lblMagUnit->setText(unit);

	// TODO: Check a different fix for this.
	// The QwtPlot and ticks are not correctly repainted.
	// This seems to be a timing issue.
	// Need more investigation, this is a temporary hack.
	QTimer::singleShot(20, [=](){
		switch (magType) {
		case FftDisplayPlot::VPEAK:
		case FftDisplayPlot::VRMS:
			bottom->setValue(-10);
			unit_per_div->setValue(2);
			fft_plot->setAxisScale(QwtAxis::YLeft, bottom->value(), top->value());
			break;
		case FftDisplayPlot::VROOTHZ:
			top_scale->setValue(2.5E1);
			bottom_scale->setValue(1E-12);
			fft_plot->setAxisScale(QwtAxis::YLeft, bottom_scale->value(), top_scale->value());
			break;
		default:
			top->setValue(0);
			bottom->setValue(-200);
			fft_plot->setAxisScale(QwtAxis::YLeft, bottom->value(), top->value());
			break;
		}
	});
}

void SpectrumAnalyzer::on_btnHistory_toggled(bool checked)
{
	int crt_channel = channelIdOfOpenedSettings();

	if (crt_channel < 0) {
		qDebug(CAT_SPECTRUM_ANALYZER) << "invalid channel ID for the opened Settings menu";
		return;
	}

	channels[crt_channel]->setAverageHistoryEnabled(checked);

	if (channels[crt_channel]->canStoreAverageHistory()) {
		if (ui->btnHistory->isChecked()) {
			ui->spinBox_averaging->setMaximum(10000);
		} else {
			ui->spinBox_averaging->setMaximum(1000000);
		}
	} else {
		ui->spinBox_averaging->setMaximum(1000000);
	}
}

void SpectrumAnalyzer::on_btnMarkerTable_toggled(bool checked)
{
	ui->markerTable->setVisible(checked);

	// Set the Plot 3 times taller than the Marker Table (when visible)
//	QGridLayout *layout = static_cast<QGridLayout *>(
//				ui->widgetPlotContainer->layout());
//	int row1 = getGridLayoutPosFromIndex(layout,
//					     layout->indexOf(ui->markerTable)).first;
//	int row2 = getGridLayoutPosFromIndex(layout,
//					     layout->indexOf(ui->gridLayout_plot)).first;

//	if (checked) {
//		layout->setRowStretch(row1, 1);
//		layout->setRowStretch(row2, 3);
//	} else {
//		layout->setRowStretch(row1, 0);
//		layout->setRowStretch(row2, 0);
//	}
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

void SpectrumAnalyzer::onTopValueChanged(double top_value)
{
	bool isScaleBtn = ui->topWidget->currentIndex();
	double bottom_value;

	if (!isScaleBtn) {
		bottom_value = bottom->value();

		unit_per_div->blockSignals(true);
		unit_per_div->setValue(abs((top_value - bottom_value)/10));
		unit_per_div->blockSignals(false);

	} else {
		bottom_value = bottom_scale->value();
	}

	fft_plot->setAxisScale(QwtAxis::YLeft, bottom_value, top_value);

	fft_plot->replot();
	auto div = fft_plot->axisScaleDiv(QwtAxis::YLeft);
	fft_plot->setYaxisMajorTicksPos(div.ticks(2));
	fft_plot->leftHandlesArea()->repaint();
}

void SpectrumAnalyzer::onScalePerDivValueChanged(double perDiv)
{
	bool isScaleBtn = ui->topWidget->currentIndex();
	double topValue, bottomValue;

	if (isScaleBtn) {

		bottomValue = bottom_scale->value();
		topValue = bottomValue + perDiv * 10;

		top_scale->blockSignals(true);
		top_scale->setValue(topValue);
		top_scale->blockSignals(false);

	} else {
		bottom->setMaxValue(m_mag_min_max.second - perDiv * 10);
		top->setMinValue(m_mag_min_max.first + perDiv * 10);

		bottomValue = bottom->value();
		topValue = bottomValue + perDiv * 10;

		top->blockSignals(true);
		top->setValue(topValue);
		top->blockSignals(false);
	}

	fft_plot->setAxisScale(QwtAxis::YLeft, bottomValue, topValue);

	fft_plot->replot();
	auto div = fft_plot->axisScaleDiv(QwtAxis::YLeft);
	fft_plot->setYaxisMajorTicksPos(div.ticks(2));
	fft_plot->leftHandlesArea()->repaint();
}

void SpectrumAnalyzer::onBottomValueChanged(double bottom_value)
{
	bool isScaleBtn = ui->topWidget->currentIndex();
	double top_value;

	if (!isScaleBtn) {
		top_value = top->value();

		unit_per_div->blockSignals(true);
		unit_per_div->setValue(abs((top_value - bottom_value)/10));
		unit_per_div->blockSignals(false);

	} else {
		top_value = top_scale->value();
	}

	fft_plot->setAxisScale(QwtAxis::YLeft, bottom_value, top_value);

	fft_plot->replot();
	auto div = fft_plot->axisScaleDiv(QwtAxis::YLeft);
	fft_plot->setYaxisMajorTicksPos(div.ticks(2));
	fft_plot->leftHandlesArea()->repaint();
}

void SpectrumAnalyzer::onCurrentAverageIndexChanged(uint chnIdx, uint avgIdx)
{
	channels[chnIdx]->setAverageIdx(avgIdx);
	setCurrentAverageIndexLabel(chnIdx);
}

void SpectrumAnalyzer::setCurrentAverageIndexLabel(uint chnIdx)
{
	if (chnIdx == crt_channel_id) {
		if (channels[chnIdx]->averaging() != 1) {
			ui->lbl_crtAvgSample->setVisible(true);
			QString txt = QString(tr("Average Sample:")+" %1 / %2 ").arg(channels[chnIdx]->averageIdx())
					.arg(channels[chnIdx]->averaging());
			ui->lbl_crtAvgSample->setText(txt);
			ui->lbl_crtAvgSample->setStyleSheet(QString("QLabel {"
								    "color: %1;"
								    "font-weight: bold;"
								    "}").arg(fft_plot->getLineColor(chnIdx).name()));
		} else {
			ui->lbl_crtAvgSample->setVisible(false);
		}
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
	m_average_current_index(0),
	m_avg_type(FftDisplayPlot::SAMPLE),
	m_fft_win(SpectrumAnalyzer::HAMMING),
	m_plot(plot),
	m_widget(new ChannelWidget(id, false, false, m_color)),
	m_average_history(true),
	m_gain_mode(libm2k::analog::PLUS_MINUS_25V)
{
	m_widget->setFullName(name);
	m_widget->setShortName(QString("CH %1").arg(id + 1));
	m_widget->nameButton()->setText(m_widget->shortName());
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
	m_plot->setAverage(m_id, m_avg_type, averaging, m_average_history);
}

uint SpectrumChannel::averageIdx() const
{
	return m_average_current_index;
}

void SpectrumChannel::setAverageIdx(uint avg)
{
	m_average_current_index = avg;
}

FftDisplayPlot::AverageType SpectrumChannel::averageType() const
{
	return m_avg_type;
}

bool SpectrumChannel::canStoreAverageHistory() const
{
	switch (m_avg_type) {
	case FftDisplayPlot::LINEAR_RMS:
	case FftDisplayPlot::LINEAR_DB:
	case FftDisplayPlot::PEAK_HOLD:
	case FftDisplayPlot::MIN_HOLD:
		return true;
	default:
		return false;
	}
}

void SpectrumChannel::setGainMode(int index)
{
	m_gain_mode = index == 0 ? libm2k::analog::PLUS_MINUS_25V
				: libm2k::analog::PLUS_MINUS_2_5V;
}

libm2k::analog::M2K_RANGE  SpectrumChannel::getGainMode()
{
	return m_gain_mode;
}

bool SpectrumChannel::isAverageHistoryEnabled() const
{
	return m_average_history;
}

void SpectrumChannel::setAverageHistoryEnabled(bool enabled)
{
	m_average_history = enabled;
	m_plot->setAverage(m_id, m_avg_type, m_averaging, enabled);
}

void SpectrumChannel::setAverageType(FftDisplayPlot::AverageType avg_type)
{
	m_avg_type = avg_type;
	m_average_history = canStoreAverageHistory();
	m_plot->setAverage(m_id, avg_type, m_averaging, m_average_history);
}

void SpectrumChannel::setFftWindow(SpectrumAnalyzer::FftWinType win, int taps)
{
	m_fft_win = win;

	std::vector<float> window = build_win(win, taps);
	float sum = 0;
	float sqr_sum = 0;
	for (auto elem : window) {
		sum += elem;
		sqr_sum += (elem * elem);
	}
	m_plot->setWindowCoefficientSum(m_id, round(sum), round(sqr_sum));


	/* We scale the window coefficients using the
	 * coeff average in order to compensate for the
	 * loss in the FFT.
	 * After the FFT we no longer have to compensate
	 * for the changes determined by the window.*/
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

double SpectrumChannel::win_overlap_factor(SpectrumAnalyzer::FftWinType type)
{
	switch (type) {
	case SpectrumAnalyzer::FLAT_TOP:
		return 0.656;

	case SpectrumAnalyzer::RECTANGULAR:
		return 0.0;

	case SpectrumAnalyzer::TRIANGULAR:
	case SpectrumAnalyzer::HAMMING:
	case SpectrumAnalyzer::HANN:
		return 0.5;

	case SpectrumAnalyzer::BLACKMAN_HARRIS:
		return 0.66;

	case SpectrumAnalyzer::KAISER:
		return 0.6;

	default:
		return 0.0;
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
