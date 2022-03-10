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

/* Boost 1.55 requires this explicitly */
#include <boost/make_shared.hpp>

/* GNU Radio includes */
#include <gnuradio/blocks/float_to_complex.h>
#include <scopy/math.h>
#include <gnuradio/blocks/sub.h>
#include <gnuradio/filter/iir_filter_ffd.h>
#include <gnuradio/blocks/nlog10_ff.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/multiply_const.h>

/* Qt includes */
#include <QtWidgets>
#include <QDebug>
#include <QVBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QSignalBlocker>
#include <QComboBox>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QtConcurrent>

/* libm2k includes */
#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2kexceptions.hpp>

/* Local includes */
#include "logging_categories.h"
#include "adc_sample_conv.hpp"
#include "gui/customPushButton.hpp"
#include "oscilloscope.hpp"
#include "gui/dynamicWidget.hpp"
#include "measurement_gui.h"
#include "gui/measure_settings.h"
#include "statistic_widget.h"
#include "state_updater.h"
#include "osc_capture_params.hpp"
#include "buffer_previewer.hpp"
#include "config.h"
#include "gui/customplotpositionbutton.h"
#include "gui/channel_widget.hpp"
#include "signal_sample.hpp"
#include "filemanager.h"
#include "scopyExceptionHandler.h"
#include "oscilloscope_api.hpp"
#include "mixed_signal_sink.h"
#include <tool_launcher.hpp>

#include "gui/runsinglewidget.h"

/* Generated UI */
#include "ui_channel_settings.h"
#include "ui_cursors_settings.h"
#include "ui_osc_general_settings.h"
#include "ui_measure_panel.h"
#include "ui_measure_settings.h"
#include "ui_statistics_panel.h"
#include "ui_cursor_readouts.h"
#include "ui_oscilloscope.h"
#include "ui_trigger_settings.h"

#include <functional>

#include <m2k/mixed_signal_source.h>

#define MAX_MATH_CHANNELS 4
#define MAX_MATH_RANGE SHRT_MAX
#define MIN_MATH_RANGE SHRT_MIN
#define MAX_AMPL 25

using namespace adiscope;
using namespace gr;
using namespace std;
using namespace libm2k;
using namespace libm2k::context;
using namespace std::placeholders;

Oscilloscope::Oscilloscope(struct iio_context *ctx, Filter *filt,
			   ToolMenuItem *toolMenuItem,
			   QJSEngine *engine, ToolLauncher *parent) :
	Tool(ctx, toolMenuItem, new Oscilloscope_API(this), "Oscilloscope", parent),
	m_m2k_context(m2kOpen(ctx, "")),
	m_m2k_analogin(m_m2k_context->getAnalogIn()),
	m_m2k_digital(m_m2k_context->getDigital()),
	nb_channels(m_m2k_analogin->getNbChannels()),
	active_sample_rate(getSampleRate()),
	nb_math_channels(0),
	ui(new Ui::Oscilloscope),
	trigger_settings(m_m2k_analogin),
	measure_settings(nullptr),
	plot(this, false, 16, 10, new TimePrefixFormatter, new MetricPrefixFormatter),
	fft_plot(nb_channels, this),
	xy_plot(nb_channels / 2, this),
	hist_plot(nb_channels, this),
	ids(new iio_manager::port_id[nb_channels]),
	autoset_id(new iio_manager::port_id),
	hist_ids(new iio_manager::port_id[nb_channels]),
	fft_is_visible(false), hist_is_visible(false), xy_is_visible(false),
	statistics_enabled(false),
	trigger_is_forced(false),
	new_data_is_triggered(false),
	triggerUpdater(new StateUpdater(250, this)),
	current_channel(-1), math_chn_counter(0),
	channels_group(new QButtonGroup(this)),
	zoom_level(0),
	current_ch_widget(-1),
	addChannel(true),
	index_x(0),
	index_y(1),
	ftc(nullptr),
	locked(false),
	triggerAcCoupled(false),
	autosetRequested(false),
	autosetEnabled(true),
	memory_adjusted_time_pos(0),
	plot_samples_sequentially(false),
	d_displayOneBuffer(true),
	nb_ref_channels(0),
	lastFunctionValid(false),
	import_error(""),
	hCursorsEnabled(true),
	vCursorsEnabled(true),
	horiz_offset(0),
	reset_horiz_offset(true),
	wheelEventGuard(nullptr),
	miniHistogram(true),
	gatingEnabled(false),
	m_filtering_enabled(true),
	m_logicAnalyzer(nullptr),
	m_mixedSignalViewEnabled(false),
	logic_top_block(nullptr)
{
	ui->setupUi(this);
	int triggers_panel = ui->stackedWidget->insertWidget(-1, &trigger_settings);

	if (m_m2k_analogin) {
		symmBufferMode = make_shared<SymmetricBufferMode>();
		symmBufferMode->setSampleRates(m_m2k_analogin->getAvailableSampleRates());
		symmBufferMode->setEntireBufferMaxSize(64000);
		symmBufferMode->setTriggerBufferMaxSize(8192); // 8192 is what hardware supports
		symmBufferMode->setTimeDivisionCount(plot.xAxisNumDiv());
	}
	dc_cancel.push_back(gnuradio::get_initial_sptr(
			new cancel_dc_offset_block(1, true)));
	dc_cancel.push_back(gnuradio::get_initial_sptr(
			new cancel_dc_offset_block(1, true)));

	/* Measurements Settings */
	measure_settings_init();

	fft_size = 1024;
	fft_plot_size = 1024;
	fft_plot.setNumPoints(0);

	fft_plot.setBtmHorAxisUnit("Hz");
	fft_plot.setLeftVertAxisUnit("dB");

	last_set_sample_count = 0;
	last_set_time_pos = 0;

	/* Gnuradio Blocks */

	this->qt_time_block = adiscope::scope_sink_f::make(0, active_sample_rate,
		"Osc Time", nb_channels, (QObject *)&plot);

	this->qt_fft_block = adiscope::scope_sink_f::make(fft_plot_size, active_sample_rate,
			"Osc Frequency", nb_channels, (QObject *)&fft_plot);

	this->qt_hist_block = adiscope::histogram_sink_f::make(1024, 250, 0, 20,
			"Osc Histogram", nb_channels, (QObject *)&hist_plot);

	this->qt_xy_block = adiscope::xy_sink_c::make(
			400, "Osc XY", nb_channels / 2, (QObject*)&xy_plot);

	this->qt_time_block->set_trigger_mode(TRIG_MODE_TAG, 0, "buffer_start");

	// get target fps from preferences
	double targetFps = getScopyPreferences()->getTarget_fps();
	qt_time_block->set_update_time(1.0/targetFps);
	qt_fft_block->set_update_time(1.0/targetFps);
	qt_hist_block->set_update_time(1.0/targetFps);
	qt_xy_block->set_update_time(1.0/targetFps);

	// Prevent the application from hanging while waiting for a trigger condition
	m_m2k_context->setTimeout(UINT_MAX);

	plot.registerSink(qt_time_block->name(), nb_channels, 0);
	plot.disableLegend();

	fft_plot.disableLegend();
	fft_plot.setXaxisMouseGesturesEnabled(false);
	for (uint i = 0; i < nb_channels; i++)
		fft_plot.setYaxisMouseGesturesEnabled(i, false);

	iio = iio_manager::get_instance(ctx,
			filt->device_name(TOOL_OSCILLOSCOPE));
	gr::hier_block2_sptr hier = iio->to_hier_block2();
	qDebug(CAT_OSCILLOSCOPE) << "Manager created:\n" << gr::dot_graph(hier).c_str();

	for (unsigned int i = 0; i < nb_channels; i++) {
		string name = "Channel " + to_string(i + 1);

		ChannelWidget *ch_widget = new ChannelWidget(i, false, false,
			plot.getLineColor(i).name(), this);

		ch_widget->setFullName(name.c_str());
		ch_widget->setShortName(QString("CH %1").arg(i + 1));
		ch_widget->nameButton()->setText(ch_widget->shortName());

		connect(ch_widget, SIGNAL(enabled(bool)),
			SLOT(onChannelWidgetEnabled(bool)));
		connect(ch_widget, SIGNAL(selected(bool)),
			SLOT(onChannelWidgetSelected(bool)));
		connect(ch_widget, SIGNAL(menuToggled(bool)),
			SLOT(onChannelWidgetMenuToggled(bool)));

		ui->channelsList->addWidget(ch_widget);

		channels_group->addButton(ch_widget->nameButton());
		ui->settings_group->addButton(ch_widget->menuButton());

		QLabel *label= new QLabel(this);
		label->setText(vertMeasureFormat.format(
			plot.VertUnitsPerDiv(i), "V/div", 3));
		label->setStyleSheet(QString("QLabel {"
				"color: %1;"
				"font-weight: bold;"
				"}").arg(plot.getLineColor(i).name()));
		ui->chn_scales->addWidget(label);

		high_gain_modes.push_back(false);
		channel_offset.push_back(0.0);
		chnAcCoupled.push_back(false);

		channels_api.append(new Channel_API(this));
	}
	triggerLevelSink = QPair<boost::shared_ptr<signal_sample>, int>(
				nullptr, -1);

	connect(ui->rightMenu, SIGNAL(finished(bool)), this,
			SLOT(rightMenuFinished(bool)));

	/* Cursors Settings */
	const int cursor_settings = ui->stackedWidget->indexOf(ui->cursorsSettings);
	ui->btnCursors->setProperty("id", QVariant(-cursor_settings));

	/* Trigger Settings */
	ui->btnTrigger->setProperty("id", QVariant(-triggers_panel));

	/* Trigger Status Updater */
	triggerUpdater->setOffState(CapturePlot::Stop);
	onTriggerModeChanged(trigger_settings.triggerMode());
	connect(triggerUpdater, SIGNAL(outputChanged(int)),
		&plot, SLOT(setTriggerState(int)));

	//plot.setZoomerEnabled(true);
	fft_plot.setZoomerEnabled();
	create_add_channel_panel();

	/* Gnuradio Blocks Connections */

	/* Lock the flowgraph if we are already started */
	bool started = isIioManagerStarted();
	if (started)
		iio->lock();

	auto adc_samp_conv = new adc_sample_conv(nb_channels, m_m2k_analogin);
	adc_samp_conv_block = gnuradio::get_initial_sptr(adc_samp_conv);


	for (unsigned int i = 0; i < nb_channels; i++) {
		ids[i] = iio->connect(adc_samp_conv_block, i, i,
				true, qt_time_block->nsamps());

		iio->connect(adc_samp_conv_block, i, qt_time_block, i);

		iio->connect(adc_samp_conv_block, i, qt_hist_block, i);
	}

	const std::function<double(unsigned int, double, bool)> fp = std::bind(
				&adc_sample_conv::conversionWrapper, adc_samp_conv,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	plot.setConversionFunction(fp);

	if (started)
		iio->unlock();

	/* Measure panel */
	measure_panel_init();

	/* Statistics panel */
	statistics_panel_init();

	/* Buffer Previewer widget */
	buffer_previewer = new AnalogBufferPreviewer(40, M_PI / 2);

	buffer_previewer->setVerticalSpacing(6);
	buffer_previewer->setMinimumHeight(20);
	buffer_previewer->setMaximumHeight(20);
	buffer_previewer->setMinimumWidth(480);
	buffer_previewer->setMaximumWidth(480);
	ui->vLayoutBufferSlot->addWidget(buffer_previewer);

	buffer_previewer->setCursorPos(0.5);


	// Build central widget
	QWidget* centralWidget = new QWidget(this);
	QVBoxLayout* vLayout = new QVBoxLayout(centralWidget);
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->setSpacing(0);

	// add the buffer previewer
	ui->plot_and_buffPreviewer->removeWidget(ui->hLayoutBufferPreview);
	vLayout->addWidget(ui->hLayoutBufferPreview);

	// add plot elements
	QWidget* plotWidget = new QWidget(this);
	gridPlot = new QGridLayout(plotWidget);
	gridPlot->setVerticalSpacing(0);
	gridPlot->setHorizontalSpacing(0);
	gridPlot->setContentsMargins(9, 0, 9, 0);
	plotWidget->setLayout(gridPlot);

	QSpacerItem *plotSpacer = new QSpacerItem(0, 5,
		QSizePolicy::Fixed, QSizePolicy::Fixed);

	gridPlot->addWidget(measurePanel, 0, 1, 1, 1);
	gridPlot->addWidget(plot.topArea(), 1, 0, 1, 4);
	gridPlot->addWidget(plot.topHandlesArea(), 2, 0, 1, 4);

	gridPlot->addWidget(plot.leftHandlesArea(), 1, 0, 4, 1);
	gridPlot->addWidget(plot.rightHandlesArea(), 1, 3, 4, 1);

	gridPlot->addWidget(&plot, 3, 1, 1, 1);
	gridPlot->addWidget(&hist_plot, 3, 2, 1, 1);

	gridPlot->addWidget(plot.bottomHandlesArea(), 4, 0, 1, 4);
	gridPlot->addItem(plotSpacer, 5, 0, 1, 4);
	gridPlot->addWidget(statisticsPanel, 6, 1, 1, 1);

	vLayout->addWidget(plotWidget);
	centralWidget->setLayout(vLayout);

	ui->plot_and_scales->removeWidget(ui->scalesWidget);
	vLayout->addWidget(ui->scalesWidget);

	// hide fixed containers
	ui->container_fft_plot->hide();
	ui->xy_plot_container->hide();

	// add space for histogram
	plot.setBonusWidthForHistogram(25);

	if(prefPanel->getCurrent_docking_enabled()) {
		ui->histogram_container->hide();

		// main window for all dock widgets
		QMainWindow* mainWindow = new QMainWindow(this);
		mainWindow->setCentralWidget(0);
		mainWindow->setWindowFlags(Qt::Widget);
		ui->gridLayoutPlot->addWidget(mainWindow, 0, 0, 1, 1);

		// time domain plot dock
		QDockWidget* plotDockWidget = DockerUtils::createDockWidget(mainWindow, centralWidget, "TimeDomain");


		// necessary to move around the dockWidgets
		centralWidget->setMinimumHeight(50);
		centralWidget->setMinimumWidth(50);

		fft_plot.setMinimumHeight(50);
		fft_plot.setMinimumWidth(50);

		xy_plot.setMinimumHeight(50);
		xy_plot.setMinimumWidth(50);


		// histogram widget
		histWidget = new QWidget(this);
		QVBoxLayout* histLayout = new QVBoxLayout(histWidget);
		histWidget->setLayout(histLayout);

		// histogram dock
		histDockWidget = DockerUtils::createDockWidget(mainWindow, histWidget, "Histogram");
		histDockWidget->hide();


		// fft dock
		fftDockWidget = DockerUtils::createDockWidget(mainWindow, &fft_plot, "FFT");
		fftDockWidget->hide();


		// xy widget
		QWidget *xyWidget = new QWidget(this);
		QGridLayout *gridL = new QGridLayout(xyWidget);
		gridL->setVerticalSpacing(0);
		gridL->setHorizontalSpacing(0);
		gridL->setContentsMargins(0, 0, 0, 0);

		gridL->addWidget(&xy_plot, 1, 0);
		gridL->addWidget(gridL->itemAtPosition(1, 0)->widget(), 2, 0);
		xyWidget->setLayout(gridL);

		// xy dock
		xyDockWidget = DockerUtils::createDockWidget(mainWindow, xyWidget, "XY");
		xyDockWidget->hide();


		// arange all dockers
#ifdef __ANDROID__
		mainWindow->addDockWidget(Qt::LeftDockWidgetArea, fftDockWidget);
		mainWindow->addDockWidget(Qt::LeftDockWidgetArea, histDockWidget);
		mainWindow->addDockWidget(Qt::LeftDockWidgetArea, plotDockWidget);
		mainWindow->addDockWidget(Qt::LeftDockWidgetArea, xyDockWidget);

		mainWindow->tabifyDockWidget(plotDockWidget, histDockWidget);
		mainWindow->tabifyDockWidget(plotDockWidget, fftDockWidget);
		mainWindow->tabifyDockWidget(plotDockWidget, xyDockWidget);
#else
		mainWindow->addDockWidget(Qt::RightDockWidgetArea, fftDockWidget);
		mainWindow->addDockWidget(Qt::RightDockWidgetArea, xyDockWidget);
		mainWindow->tabifyDockWidget(fftDockWidget, xyDockWidget);

		mainWindow->addDockWidget(Qt::LeftDockWidgetArea, histDockWidget);
		mainWindow->addDockWidget(Qt::LeftDockWidgetArea, plotDockWidget);
		mainWindow->tabifyDockWidget(plotDockWidget, histDockWidget);
#endif

#ifdef PLOT_MENU_BAR_ENABLED
		DockerUtils::configureTopBar(plotDockWidget);
		DockerUtils::configureTopBar(histDockWidget);
		DockerUtils::configureTopBar(fftDockWidget);
		DockerUtils::configureTopBar(xyDockWidget);
#endif

	} else {
		ui->gridLayoutPlot->addWidget(centralWidget, 1, 0, 1, 1);
		ui->hlayout_fft->addWidget(&fft_plot);

		QGridLayout *gridL = static_cast<QGridLayout *>(ui->xy_plot_container->layout());
		QWidget *w = gridL->itemAtPosition(1, 0)->widget();
		gridL->addWidget(&xy_plot, 1, 0);
		gridL->addWidget(w, 2, 0);

		fft_plot.setMinimumHeight(250);
		fft_plot.setMinimumWidth(500);
	}

		hist_plot.setMinimumHeight(60);
		hist_plot.setMinimumWidth(25);
		hist_plot.setMaximumWidth(25);

	//	xy_plot.setMinimumHeight(50);
	//	xy_plot.setMinimumWidth(50);

	/* Default plot settings */
	plot.setSampleRate(active_sample_rate, 1, "");
	plot.setActiveVertAxis(0);

	started = isIioManagerStarted();
	if (started)
		iio->lock();

	for (unsigned int i = 0; i < nb_channels; i++) {
		plot.Curve(i)->setAxes(
				QwtAxisId(QwtAxis::XBottom, 0),
				QwtAxisId(QwtAxis::YLeft, i));
		plot.addZoomer(i);
		probe_attenuation.push_back(1);
		auto multiply = gr::blocks::multiply_const_ff::make(1);
		auto null_sink = gr::blocks::null_sink::make(sizeof(float));
		math_probe_atten.push_back(multiply);
		iio->connect(adc_samp_conv_block, i, math_probe_atten.at(i), 0);
		iio->connect(math_probe_atten.at(i), 0, null_sink, 0);

		plot.Curve(i)->setTitle("CH " + QString::number(i + 1));
	}

	if (started)
		iio->unlock();


	plot.levelTriggerA()->setMobileAxis(QwtAxisId(QwtAxis::YLeft, 0));
	plot.setTriggerAEnabled(trigger_settings.analogEnabled());
	plot.levelTriggerA()->setPosition(trigger_settings.level());

	// TO DO: Give user the option to make these axes visible
	plot.setAxisVisible(QwtAxis::YLeft, false);
	plot.setAxisVisible(QwtAxis::XBottom, false);
	plot.setUsingLeftAxisScales(false);

	xy_plot.setVertUnitsPerDiv(5);
	xy_plot.setHorizUnitsPerDiv(5);

	xy_plot.disableLegend();

	xy_plot.setLineStyle(0, Qt::SolidLine);
	xy_plot.setLineMarker(0, QwtSymbol::NoSymbol);

	// Disable mouse interactions with the axes until we figure out if we want to use them
	xy_plot.setXaxisMouseGesturesEnabled(false);
	for (uint i = 0; i < nb_channels; i++)
		xy_plot.setYaxisMouseGesturesEnabled(i, false);

	xy_plot.setLineColor(0, QColor("#4a64ff"));

	ui->rightMenu->setMaximumWidth(0);

	// Controls for scale/division and position controls (Horizontal & Vertical)
	timeBase = new ScaleSpinButton({
				{"ns", 1E-9},
				{"μs", 1E-6},
				{"ms", 1E-3},
				{"s", 1E0}
				}, tr("Time Base"), 100e-9, 1E0,
				true, false, this);
	timePosition = new PositionSpinButton({
				{"ns", 1E-9},
				{"μs", 1E-6},
				{"ms", 1E-3},
				{"s", 1E0},
				{"min", 60E0},
				{"h", 36E2}
				}, tr("Position"),
				-timeBase->maxValue() * 5,
				36E2,
				true, false, this);
	voltsPerDiv = new ScaleSpinButton({
//				{"μVolts", 1E-6},
				{"mVolts", 1E-3},
				{"Volts", 1E0}
				}, tr("Volts/Div"), 1e-3, 1e1,
				true, false, this);
	voltsPosition  = new PositionSpinButton({
				{"μVolts", 1E-6},
				{"mVolts", 1E-3},
				{"Volts", 1E0}
				}, tr("Position"),
				-25, 25, true, false, this);

	plot.setOffsetInterval(-25, 25);
	plot.setTimeTriggerInterval(-timeBase->maxValue() * 5,
				     timeBase->maxValue() * 5);

	ch_ui = new Ui::ChannelSettings();
	ch_ui->setupUi(ui->channelSettings);

	ch_ui->filter1->setVisible(prefPanel->getShowADCFilters());
	ch_ui->filter2->setVisible(prefPanel->getShowADCFilters());

	ch_ui->horizontal->insertWidget(1, timeBase, 0, Qt::AlignLeft);
	ch_ui->horizontal->insertWidget(2, timePosition, 0, Qt::AlignLeft);
	ch_ui->vertical->insertWidget(1, voltsPerDiv, 0, Qt::AlignLeft);
	ch_ui->vertical->insertWidget(2, voltsPosition, 0, Qt::AlignLeft);
	Util::retainWidgetSizeWhenHidden(ch_ui->probeWidget);

	init_channel_settings();

	connect(ch_ui->filter_en, &QCheckBox::toggled, [&](bool en) {
			iio->freq_comp_filt[current_ch_widget][0]->set_enable(en);
	});
	connect(ch_ui->filter_TC, &QLineEdit::textChanged, [&](QString str) {
		bool ok;
		float val = str.toFloat(&ok);
		if(!ok)
			return;
		iio->freq_comp_filt[current_ch_widget][0]->set_TC(val);

	});
	connect(ch_ui->filter_gain, &QLineEdit::textChanged, [&](QString str) {
		bool ok;
		float val = str.toFloat(&ok);
		if(!ok)
			return;
		iio->freq_comp_filt[current_ch_widget][0]->set_filter_gain(val);
	});


	connect(ch_ui->filter2_en, &QCheckBox::toggled, [&](bool en) {
			iio->freq_comp_filt[current_ch_widget][1]->set_enable(en);
	});
	connect(ch_ui->filter2_TC, &QLineEdit::textChanged, [&](QString str) {
		bool ok;
		float val = str.toFloat(&ok);
		if(!ok)
			return;
		iio->freq_comp_filt[current_ch_widget][1]->set_TC(val);

	});
	connect(ch_ui->filter2_gain, &QLineEdit::textChanged, [&](QString str) {
		bool ok;
		float val = str.toFloat(&ok);
		if(!ok)
			return;
		iio->freq_comp_filt[current_ch_widget][1]->set_filter_gain(val);
	});

	timeBase->setValue(plot.HorizUnitsPerDiv());
	voltsPerDiv->setValue(plot.VertUnitsPerDiv());
	timePosition->setStep(timeBase->value() / 10);
	voltsPosition->setStep(voltsPerDiv->value() / 10);

	plot.setTimeBaseLabelValue(timeBase->value());

	/* General Settings Menu */
	gsettings_ui = new Ui::OscGeneralSettings();
	gsettings_ui->setupUi(ui->generalSettings);

	for(int i = 0; i < nb_channels + nb_math_channels; i++) {
		ChannelWidget *cw = static_cast<ChannelWidget *>(
			ui->channelsList->itemAt(i)->widget());
		gsettings_ui->cmb_x_channel->addItem(cw->shortName());
		gsettings_ui->cmb_y_channel->addItem(cw->shortName());
	}
	gsettings_ui->cmb_x_channel->setCurrentIndex(0);
	gsettings_ui->cmb_y_channel->setCurrentIndex(1);
	setup_xy_channels();

	connect(gsettings_ui->cmb_x_channel, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setup_xy_channels()));
	connect(gsettings_ui->cmb_y_channel, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setup_xy_channels()));


	int gsettings_panel = ui->stackedWidget->indexOf(ui->generalSettings);
	ui->btnGeneralSettings->setProperty("id", QVariant(-gsettings_panel));

	connect(gsettings_ui->FFT_view, SIGNAL(toggled(bool)),
		SLOT(onFFT_view_toggled(bool)));
	connect(gsettings_ui->XY_view, SIGNAL(toggled(bool)),
		SLOT(onXY_view_toggled(bool)));
	connect(gsettings_ui->Histogram_view, SIGNAL(toggled(bool)),
		SLOT(onHistogram_view_toggled(bool)));

	ch_ui->btnAutoset->setEnabled(false);

	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		ch_ui->btnAutoset, &QPushButton::setEnabled);

	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		[=](bool checked){
		auto btn = dynamic_cast<CustomPushButton *>(run_button);
		btn->setChecked(checked);
	});
	connect(run_button, &QPushButton::toggled,
		ui->runSingleWidget, &RunSingleWidget::toggle);
	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		this, &Oscilloscope::runStopToggled);
	connect(this, &Oscilloscope::startRunning,
		ui->runSingleWidget, &RunSingleWidget::toggle);

	connect(gsettings_ui->xyPlotLineType, SIGNAL(toggled(bool)),
		this, SLOT(on_xyPlotLineType_toggled(bool)));

	// Signal-Slot Connections

	connect(timeBase, SIGNAL(valueChanged(double)),
		SLOT(onHorizScaleValueChanged(double)));
	connect(voltsPerDiv, SIGNAL(valueChanged(double)),
		SLOT(onVertScaleValueChanged(double)));
	connect(timePosition, SIGNAL(valueChanged(double)),
		SLOT(onTimePositionChanged(double)));
	connect(voltsPosition, SIGNAL(valueChanged(double)),
		SLOT(onVertOffsetValueChanged(double)));
	connect(ch_ui->btnCoupled, SIGNAL(toggled(bool)),
		SLOT(onChannelCouplingChanged(bool)));

	connect(ch_ui->cmbChnLineWidth, SIGNAL(currentIndexChanged(int)),
		SLOT(channelLineWidthChanged(int)));

	/* Sync timePosition with plot time trigger bar */
	connect(&plot, SIGNAL(timeTriggerValueChanged(double)),
		this, SLOT(onTimeTriggerDelayChanged(double)));
	connect(this, SIGNAL(triggerPositionChanged(double)),
		timePosition, SLOT(setValue(double)));

	/* Update Timebase label each time the oscilloscope timebase changes */
	connect(timeBase, SIGNAL(valueChanged(double)),
		&plot, SLOT(setTimeBaseLabelValue(double)));

	connect(&plot, SIGNAL(channelOffsetChanged(unsigned int, double)),
		SLOT(onChannelOffsetChanged(unsigned int, double)));

	connect(this, SIGNAL(selectedChannelChanged(int)),
		&plot, SLOT(setSelectedChannel(int)));
	connect(this, &Oscilloscope::selectedChannelChanged,
		&hist_plot, &HistogramDisplayPlot::setSelectedChannel);

	connect(this, SIGNAL(selectedChannelChanged(int)),
		&plot, SLOT(setZoomerVertAxis(int)));

	connect(&plot,
		SIGNAL(cursorReadoutsChanged(struct cursorReadoutsText)),
		SLOT(onCursorReadoutsChanged(struct cursorReadoutsText)));

	// Connections with Trigger Settings
	connect(&trigger_settings, SIGNAL(sourceChanged(int)),
		SLOT(onTriggerSourceChanged(int)));
	connect(&trigger_settings, SIGNAL(analogTriggerEnabled(bool)),
		&plot, SLOT(setTriggerAEnabled(bool)));
	connect(&trigger_settings, SIGNAL(analogTriggerEnabled(bool)),
		this, SLOT(configureAcCouplingTrigger(bool)));
	connect(&trigger_settings, SIGNAL(levelChanged(double)),
		plot.levelTriggerA(), SLOT(setPosition(double)));
	connect(plot.levelTriggerA(), SIGNAL(positionChanged(double)),
		&trigger_settings, SLOT(setTriggerLevel(double)));

	connect(&trigger_settings, SIGNAL(levelChanged(double)),
		SLOT(onTriggerLevelChanged(double)));

	connect(&trigger_settings, SIGNAL(triggerModeChanged(int)),
		this, SLOT(onTriggerModeChanged(int)));

	connect(&*iio, SIGNAL(timeout()),
			&trigger_settings, SLOT(autoTriggerDisable()));
	connect(&plot, SIGNAL(newData()),
			&trigger_settings, SLOT(autoTriggerEnable()));
	connect(&plot, SIGNAL(newData()), this, SLOT(singleCaptureDone()));

	connect(&plot, SIGNAL(filledScreen(bool, unsigned int)), this,
		SLOT(onFilledScreen(bool, unsigned int)));

	connect(&*iio, SIGNAL(timeout()),
			SLOT(onIioDataRefillTimeout()));
	connect(&plot, SIGNAL(newData()), this, SLOT(onPlotNewData()));

	connect(ch_ui->cmbMemoryDepth, SIGNAL(currentTextChanged(QString)),
		this, SLOT(onCmbMemoryDepthChanged(QString)));

	if (nb_channels < 2)
		gsettings_ui->XY_view->hide();

	// Set the first channel to be the selected channel (by default)
	ChannelWidget *chn0_widget = channelWidgetAtId(0);
	if (chn0_widget) {
		chn0_widget->nameButton()->setChecked(true);
	}

	// Default hysteresis levels for measurements
	for (int i = 0; i < nb_channels; i++)
		plot.setPeriodDetectHyst(i, 1.0 / 5);

	// Calculate initial sample count and sample rate
	onHorizScaleValueChanged(timeBase->value());
	onTimePositionChanged(timePosition->value());

	if (m_m2k_analogin) {
		int crt_chn_copy = current_channel;
		int crt_chn_w_copy = current_ch_widget;
		for (int i = 0; i < nb_channels; i++) {
			current_channel = i;
			current_ch_widget = i;
			updateGainMode();

			auto chn = static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(i);
			libm2k::analog::M2K_RANGE adc_range = m_m2k_analogin->getRange(chn);
			auto adc_range_limits = m_m2k_analogin->getRangeLimits(adc_range);
			auto hyst_range_limits = m_m2k_analogin->getHysteresisRange(chn);

			double vscale = plot.VertUnitsPerDiv(i);
			trigger_settings.setTriggerLevelRange(i, adc_range_limits);
			trigger_settings.setTriggerLevelStep(i, vscale);
			trigger_settings.setTriggerHystRange(i, hyst_range_limits);
			trigger_settings.setTriggerHystStep(i, vscale / 10);
		}
		current_channel = crt_chn_copy;
		current_ch_widget = crt_chn_w_copy;
	}

	connect(plot.getZoomer(), &OscPlotZoomer::zoomIn, [=](){
		zoom_level++;
		plot.setTimeBaseZoomed(true);
	});
	connect(plot.getZoomer(), &OscPlotZoomer::zoomOut, [=](){
		if (zoom_level != 0) zoom_level--;
		if (zoom_level == 0)
			plot.setTimeBaseZoomed(false);
	});
	connect(plot.getZoomer(), &OscPlotZoomer::zoomFinished, [=](bool isZoomOut) {
		plot.setTimeBaseLabelValue(plot.HorizUnitsPerDiv());

		for (int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels; ++i) {
			QLabel *label = static_cast<QLabel *>(
			                        ui->chn_scales->itemAt(i)->widget());
			double value = probe_attenuation[i] * plot.VertUnitsPerDiv(i);
			QString str = vertMeasureFormat.format(value, "V/div", 3);
			str.append(getChannelRangeStringVDivHelper(i));
			label->setText(str);
		}

		if (zoom_level == 0) {
			onTimePositionChanged(timePosition->value());
		}

		updateBufferPreviewer();
		horiz_offset = plot.HorizOffset();
		time_trigger_offset = horiz_offset;

		scaleHistogramPlot();
		resetHistogramDataPoints();
	});


	connect(ch_ui->probe_attenuation_value, &QLineEdit::textChanged, this, [=](){
		double value = ch_ui->probe_attenuation_value->text().toDouble();
		probe_attenuation[current_ch_widget] = value;
		if (current_channel == current_ch_widget) {
			plot.setDisplayScale(probe_attenuation[current_ch_widget]);
		}

		bool started = isIioManagerStarted();
		if (started)
			iio->lock();

		// Update the multiplier value for this channel
		if (current_ch_widget < math_probe_atten.size()) {
			math_probe_atten[current_ch_widget]->set_k(value);
		}

		auto max_elem = max_element(probe_attenuation.begin(), probe_attenuation.begin() + nb_channels);

		auto const values = math_rails.values();
		for (auto rail : values) {
			rail->set_lo(MIN_MATH_RANGE);
			rail->set_hi(MAX_MATH_RANGE);
		}

		if (started)
			iio->unlock();

		for (int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels; ++i) {
			QLabel *label = static_cast<QLabel *>(
						ui->chn_scales->itemAt(i)->widget());
			double value = probe_attenuation[i] * plot.VertUnitsPerDiv(i);
			QString str = vertMeasureFormat.format(value, "V/div", 3);
			str.append(getChannelRangeStringVDivHelper(i));
			label->setText(str);
		}

		if (channelWidgetAtId(current_ch_widget)->isMathChannel()) {
			voltsPerDiv->silentSetMaxValue((*max_elem) * 10);
			voltsPosition->silentSetMaxValue((*max_elem) * MAX_AMPL);
			voltsPosition->silentSetMinValue((*max_elem) * (-MAX_AMPL));
		} else {
			voltsPerDiv->silentSetMaxValue(10);
			voltsPosition->silentSetMaxValue(MAX_AMPL);
			voltsPosition->silentSetMinValue(-MAX_AMPL);
		}

		voltsPerDiv->setDisplayScale(probe_attenuation[current_ch_widget]);
		voltsPosition->setDisplayScale(probe_attenuation[current_ch_widget]);

		if (!runButton()->isChecked() && plot.measurementsEnabled()) {
			measureUpdateValues();
		}

		onTriggerSourceChanged(trigger_settings.currentChannel());

	});

	init_buffer_scrolling();

	export_settings_init();
	cursor_panel_init();
	setFFT_params(true);

	for (unsigned int i = 0; i < nb_channels + nb_math_channels; i++) {
		init_selected_measurements(i, {0, 1, 4, 5});
	}

	// The trigger is always available (cannot be disabled) and we add it to
	// the list so we can show in case all other menus are disabled
	menuOrder.push_back(ui->btnTrigger);
	gsettings_ui->xySettings->hide();
	timePosition->setValue(0);

	connect(gsettings_ui->xyLineThickness,SIGNAL(currentIndexChanged(int)),this, SLOT(on_xyLineThickness_currentIndexChanged(int)));

	api->setObjectName(QString::fromStdString(Filter::tool_name(
			TOOL_OSCILLOSCOPE)));
	api->load(*settings);
	api->js_register(engine);

	plot.setDisplayScale(probe_attenuation[current_channel]);
	onTriggerSourceChanged(trigger_settings.currentChannel());
	for (int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels; ++i) {
		QLabel *label = static_cast<QLabel *>(
					ui->chn_scales->itemAt(i)->widget());
		double value = probe_attenuation[i] * plot.VertUnitsPerDiv(i);
		QString str = vertMeasureFormat.format(value, "V/div", 3);
		str.append(getChannelRangeStringVDivHelper(i));
		label->setText(str);
	}

	for (unsigned int i = nb_channels; i < nb_channels + nb_math_channels; i++) {
		init_selected_measurements(i, {0, 1, 4, 5});
	}

	if (!wheelEventGuard)
		wheelEventGuard = new MouseWheelWidgetGuard(ui->mainWidget);
	wheelEventGuard->installEventRecursively(ui->mainWidget);

	current_ch_widget = current_channel;

	readPreferences();

	connect(ui->printBtn, &QPushButton::clicked,
		[=]() {
		plot.printWithNoBackground(api->objectName());
	});

	//workaround for a bug that selected channel settings for disabled channels
	bool found = false;
	for (int i = 0; i < nb_channels + nb_math_channels +
					nb_ref_channels; ++i) {
		ChannelWidget *cw = channelWidgetAtId(i);
		if (cw->enableButton()->isChecked()) {
			cw->menuButton()->setChecked(true);
			found = true;
			break;
		}
	}
	if (!found) {
		ui->btnTrigger->setChecked(true);
		ui->btnTrigger->setChecked(false);
	}

	connect(this, &Tool::detachedState,
		this, &Oscilloscope::toolDetached);
	toolDetached(false);

	connect(&plot,SIGNAL(leftGateChanged(double)),SLOT(onLeftGateChanged(double)));
	connect(&plot,SIGNAL(rightGateChanged(double)),SLOT(onRightGateChanged(double)));

	ui->btnHelp->setUrl("https://wiki.analog.com/university/tools/m2k/scopy/oscilloscope");

#ifdef __ANDROID__
	ui->btnAddMath->setIconSize(QSize(24, 24));
#endif

	plot.replot();
	plot.zoomBaseUpdate(true);
}

int Oscilloscope::binSearchPointOnXaxis(double time)
{
	int n = plot.Curve(0)->data()->size();
	if (time <= plot.Curve(0)->data()->sample(0).x()) {
		return 0;
	}
	if (time >= plot.Curve(0)->data()->sample(n - 1).x()) {
		return n - 1;
	}

	int left = 0;
	int right = n - 1;
	while (left <= right) {
		int mid = (left + right) / 2;
		double xData = plot.Curve(0)->data()->sample(mid).x();
		if (xData == time) {
			return mid;
		} else if (xData < time) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}
	return left;
}

void Oscilloscope::resetHistogramDataPoints()
{
	QwtInterval xBot = plot.axisInterval(QwtAxis::XBottom);
	double zoomMinTime = xBot.minValue();
	double zoomMaxTime = xBot.maxValue();

	int posMin = binSearchPointOnXaxis(zoomMinTime);
	int posMax = binSearchPointOnXaxis(zoomMaxTime);

	hist_plot.setDataInterval(posMin, posMax + 1);
}

bool Oscilloscope::isIioManagerStarted() const
{
	return iio->started() && (ui->runSingleWidget->singleButtonChecked()
						  || ui->runSingleWidget->runButtonChecked());
}

void Oscilloscope::init_buffer_scrolling()
{
	connect(&plot, &CapturePlot::canvasSizeChanged, [=](){
		buffer_previewer->setFixedWidth(plot.width());
	});
	connect(buffer_previewer, &BufferPreviewer::bufferMovedBy, [=](int value) {
		reset_horiz_offset = false;
		double moveTo = 0.0;
		double min = plot.Curve(0)->sample(0).x();
		double max = plot.Curve(0)->sample(plot.Curve(0)->data()->size() - 1).x();
		int width = buffer_previewer->width();
		double xAxisWidth = max - min;

		moveTo = value * xAxisWidth / width;
		plot.setHorizOffset(moveTo + horiz_offset);
		plot.replot();
		updateBufferPreviewer();
		resetHistogramDataPoints();
	});
	connect(buffer_previewer, &BufferPreviewer::bufferStopDrag, [=](){
		horiz_offset = plot.HorizOffset();
		reset_horiz_offset = true;
	});
	connect(buffer_previewer, &BufferPreviewer::bufferResetPosition, [=](){
		plot.setHorizOffset(time_trigger_offset);
		plot.replot();
		updateBufferPreviewer();
		horiz_offset = time_trigger_offset;
		resetHistogramDataPoints();
	});
}


void Oscilloscope::init_selected_measurements(int chnIdx,
					      std::vector<int> measureIdx)
{
	auto measurements = plot.measurements(chnIdx);

	for (int i = 0; i < measureIdx.size(); i++) {
		measurements[measureIdx[i]]->setEnabled(true);
		measure_settings->onMeasurementActivated(
					chnIdx, measureIdx[i], true);
	}
	measure_settings->loadMeasurementStatesFromData();
	onMeasurementSelectionListChanged();
}

void Oscilloscope::remove_ref_waveform(QString name)
{
	int maxChnId = nb_channels + nb_math_channels + nb_ref_channels;
	for (int i = 0; i < maxChnId; ++i) {
		ChannelWidget *cw = channelWidgetAtId(i);
		if (cw->fullName() == name) {
			qDebug() << "deleted channel with name: " << cw->fullName();
			cw->deleteButton()->click();
		}
	}
}

void Oscilloscope::setNativeDialogs(bool nativeDialogs)
{
	Tool::setNativeDialogs(nativeDialogs);
	plot.setUseNativeDialog(nativeDialogs);
}

void Oscilloscope::setLogicAnalyzer(logic::LogicAnalyzer *la)
{
	qDebug() << "Logic Analyzer: " << la;
	m_logicAnalyzer = la;
}

void Oscilloscope::add_ref_waveform(QString name, QVector<double> xData, QVector<double> yData, unsigned int sampleRate)
{
	plot.cancelZoom();

	probe_attenuation.push_back(1);

	plot.registerReferenceWaveform(name, xData, yData);

	int curve_id = nb_channels + nb_math_channels + nb_ref_channels;

	ChannelWidget *channel_widget = new ChannelWidget(curve_id, true, false,
			plot.getLineColor(curve_id).name(), this);

	channel_widget->setFullName(name);
	channel_widget->setShortName(name);
	channel_widget->nameButton()->setText(channel_widget->shortName());
	channel_widget->setReferenceChannel(true);


	channel_widget->setProperty("curve_nb", QVariant(curve_id));
	channel_widget->deleteButton()->setProperty(
		"curve_name", QVariant(name));

	connect(channel_widget, SIGNAL(enabled(bool)),
		SLOT(onChannelWidgetEnabled(bool)));
	connect(channel_widget, SIGNAL(selected(bool)),
		SLOT(onChannelWidgetSelected(bool)));
	connect(channel_widget, SIGNAL(menuToggled(bool)),
		SLOT(onChannelWidgetMenuToggled(bool)));
	connect(channel_widget, SIGNAL(deleteClicked()),
		SLOT(onChannelWidgetDeleteClicked()));
	connect(channel_widget, &ChannelWidget::menuToggled,
	[=](bool on) {
		if (!on) {
			ch_ui->btnEditMath->setChecked(false);
		}
	});

	ui->channelsList->addWidget(channel_widget);

	channels_group->addButton(channel_widget->nameButton());
	ui->settings_group->addButton(channel_widget->menuButton());
	channel_widget->nameButton()->setChecked(true);

	QLabel *label= new QLabel(this);
	label->setText(vertMeasureFormat.format(
			       plot.VertUnitsPerDiv(curve_id), "V/div", 3));
	label->setStyleSheet(QString("QLabel {"
				     "color: %1;"
				     "font-weight: bold;"
				     "}").arg(plot.getLineColor(curve_id).name()));
	ui->chn_scales->addWidget(label);
	probe_attenuation.push_back(1);

	plot.Curve(curve_id)->setAxes(
		QwtAxisId(QwtAxis::XBottom, 0),
		QwtAxisId(QwtAxis::YLeft, curve_id));
	plot.Curve(curve_id)->setTitle("REF " + QString::number(nb_ref_channels + 1));
	plot.addZoomer(curve_id);
	plot.replot();

	nb_ref_channels++;

	plot.setPeriodDetectHyst(2, 1.0 / 5);

	if (nb_math_channels + nb_ref_channels == MAX_MATH_CHANNELS) {
		if (ui->btnAddMath->isChecked()) {
			ui->btnAddMath->setChecked(false);
		}

		ui->btnAddMath->hide();
		menuOrder.removeOne(ui->btnAddMath);
	}

	plot.showYAxisWidget(curve_id, true);
	plot.setVertUnitsPerDiv(1, curve_id); // force v/div to 1
	init_selected_measurements(curve_id, {0, 1, 4, 5});

	plot.computeMeasurementsForChannel(curve_id, sampleRate);
}

void Oscilloscope::add_ref_waveform(unsigned int chIdx)
{
	if (nb_math_channels + nb_ref_channels == MAX_MATH_CHANNELS) {
		return;
	}

	plot.cancelZoom();

	int curve_id = nb_channels + nb_math_channels + nb_ref_channels;

	QString qname = QString("REF %1").arg(nb_ref_channels + 1);

	probe_attenuation.push_back(1);

	QVector<double> xData;
	QVector<double> yData;

	double ref_waveform_timebase = refChannelTimeBase->value();

	int nr_of_samples_in_file = import_data.size();

	double mid_point_on_screen = (timeBase->value() * 8) - ((
					     timeBase->value() * 8) - timePosition->value());

	double x_axis_step_size = (ref_waveform_timebase /
				   (nr_of_samples_in_file / plot.xAxisNumDiv()));

	for (int i = -(nr_of_samples_in_file / 2); i < (nr_of_samples_in_file / 2);
	     ++i) {
		xData.push_back(mid_point_on_screen + ((double)i * x_axis_step_size));
	}

	if (!refChannelTimeBase->isEnabled()) chIdx++;

	for (int i = 0; i < import_data.size(); ++i) {
		if (chIdx >= import_data[i].size()) {
			continue;
		}
		yData.push_back(import_data[i][chIdx]);
	}

	qDebug() << "Added ref waveform with nr of samples: " << yData.size();
	qDebug() << "TimeBase: " << refChannelTimeBase->value();
	qDebug() << "Number of samples in file: " << nr_of_samples_in_file;

	plot.registerReferenceWaveform(qname, xData, yData);

	ChannelWidget *channel_widget = new ChannelWidget(curve_id, true, false,
	                plot.getLineColor(curve_id).name(), this);

	channel_widget->setFullName(qname);
	channel_widget->setShortName(qname);
	channel_widget->nameButton()->setText(channel_widget->shortName());
	channel_widget->setReferenceChannel(true);


	channel_widget->setProperty("curve_nb", QVariant(curve_id));
	channel_widget->deleteButton()->setProperty(
	        "curve_name", QVariant(qname));

	connect(channel_widget, SIGNAL(enabled(bool)),
	        SLOT(onChannelWidgetEnabled(bool)));
	connect(channel_widget, SIGNAL(selected(bool)),
	        SLOT(onChannelWidgetSelected(bool)));
	connect(channel_widget, SIGNAL(menuToggled(bool)),
	        SLOT(onChannelWidgetMenuToggled(bool)));
	connect(channel_widget, SIGNAL(deleteClicked()),
	        SLOT(onChannelWidgetDeleteClicked()));
	connect(channel_widget, &ChannelWidget::menuToggled,
	[=](bool on) {
		if (!on) {
			ch_ui->btnEditMath->setChecked(false);
		}
	});

	ui->channelsList->addWidget(channel_widget);

	channels_group->addButton(channel_widget->nameButton());
	ui->settings_group->addButton(channel_widget->menuButton());
	channel_widget->nameButton()->setChecked(true);

	QLabel *label= new QLabel(this);
	label->setText(vertMeasureFormat.format(
	                       plot.VertUnitsPerDiv(curve_id), "V/div", 3));
	label->setStyleSheet(QString("QLabel {"
	                             "color: %1;"
	                             "font-weight: bold;"
	                             "}").arg(plot.getLineColor(curve_id).name()));
	ui->chn_scales->addWidget(label);
	probe_attenuation.push_back(1);

	plot.Curve(curve_id)->setAxes(
	        QwtAxisId(QwtAxis::XBottom, 0),
	        QwtAxisId(QwtAxis::YLeft, curve_id));
	plot.Curve(curve_id)->setTitle("REF " + QString::number(nb_ref_channels + 1));
	plot.addZoomer(curve_id);
	plot.replot();

	nb_ref_channels++;

	plot.setPeriodDetectHyst(2, 1.0 / 5);

	if (nb_math_channels + nb_ref_channels == MAX_MATH_CHANNELS) {
		if (ui->btnAddMath->isChecked()) {
			ui->btnAddMath->setChecked(false);
		}

		ui->btnAddMath->hide();
		menuOrder.removeOne(ui->btnAddMath);
	}

	plot.showYAxisWidget(curve_id, true);
	plot.setVertUnitsPerDiv(1, curve_id); // force v/div to 1
	init_selected_measurements(curve_id, {0, 1, 4, 5});
}

void Oscilloscope::updateTriggerLevelValue(std::vector<float> value)
{
	if (!triggerLevelSink.first) {
		return;
	}

	triggerLevelSink.first->blockSignals(true);

	float val = dc_cancel.at(triggerLevelSink.second)->get_dc_offset();
	double m_dc_level = trigger_settings.dcLevel();
	if ((int)(val*1e3) == (int)(m_dc_level*1e3)) {
		triggerLevelSink.first->blockSignals(false);
		return;
	}
	if (trigger_settings.analogEnabled()) {
		trigger_settings.setDcLevelCoupled(val);
		trigger_settings.onSpinboxTriggerLevelChanged(
					trigger_settings.level());
	}
	triggerLevelSink.first->blockSignals(false);
}

Oscilloscope::~Oscilloscope()
{

	if (m_mixedSignalViewEnabled) {
		disableMixedSignalView();
	}

	disconnect(prefPanel, &Preferences::notify, this, &Oscilloscope::readPreferences);


	ui->runSingleWidget->toggle(false);
	setDynamicProperty(runButton(), "disabled", false);

	bool started = isIioManagerStarted();
	if (started)
		iio->lock();

	for (unsigned int i = 0; i < nb_channels; i++)
		iio->disconnect(ids[i]);

	if (fft_is_visible) {
		if (fft_channels.size() > 0) {
			fft_channels.clear();
			fft_blocks.clear();
			ctm_blocks.clear();
		}
	}

	if (started)
		iio->unlock();

	gr::hier_block2_sptr hier = iio->to_hier_block2();
	qDebug(CAT_OSCILLOSCOPE) << "OSC disconnected:\n" << gr::dot_graph(hier).c_str();

	if (saveOnExit) {
		api->save(*settings);
	}
	delete api;

	for (auto it = channels_api.begin(); it != channels_api.end(); ++it) {
		delete *it;
	}

	delete math_pair;

	delete[] hist_ids;
	delete[] ids;
	delete autoset_id;
	delete ch_ui;
	delete gsettings_ui;
	delete measure_panel_ui;
	delete cursor_readouts_ui;
	delete statistics_panel_ui;
	delete cr_ui;
	delete ui;
}

void Oscilloscope::settingsLoaded()
{
	for (int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels; ++i)
		if (channelWidgetAtId(i)->menuButton()->isChecked() &&
				!channelWidgetAtId(i)->isReferenceChannel()) {
			current_ch_widget = i;
			channelWidgetAtId(i)->menuButton()->setChecked(true);
			break;
		}

	disconnect(voltsPerDiv, SIGNAL(valueChanged(double)), this,
		SLOT(onVertScaleValueChanged(double)));
	voltsPerDiv->setValue(plot.VertUnitsPerDiv(current_ch_widget));
	connect(voltsPerDiv, SIGNAL(valueChanged(double)),
		SLOT(onVertScaleValueChanged(double)));
}

void Oscilloscope::readPreferences()
{
	bool showFps = prefPanel->getShow_plot_fps();
	plot.setVisibleFpsLabel(showFps);
	xy_plot.setVisibleFpsLabel(showFps);
	fft_plot.setVisibleFpsLabel(showFps);
	hist_plot.setVisibleFpsLabel(showFps);
	plot.setGraticuleEnabled(prefPanel->getOsc_graticule_enabled());
	ui->instrumentNotes->setVisible(prefPanel->getInstrumentNotesActive());

	// enable/disable mini histogram plot
	toggleMiniHistogramPlotVisible(prefPanel->getMini_hist_enabled());

	bool foundChannel = false;
	for (unsigned int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels;
	     i++) {
		ChannelWidget *cw = static_cast<ChannelWidget *>(
					    ui->channelsList->itemAt(i)->widget());

		if (cw->enableButton()->isChecked()) {
			/* At least one channel is enabled,
			 * so we can enable/disable labels */
			enableLabels(prefPanel->getOsc_labels_enabled());
			foundChannel = true;
			break;
		}
	}

	if (!foundChannel) {
		/* No channel is enabled, so we disable the labels */
		enableLabels(false);
	}

	update_chn_settings_panel(current_ch_widget);
	setFilteringEnabled(prefPanel->getOsc_filtering_enabled());
	double fps = prefPanel->getTarget_fps();
	iio->set_data_rate(std::max(fps, 15.0)); // minimum 15 buffers/second
	qt_time_block->set_update_time(1.0/fps);
	qt_fft_block->set_update_time(1.0/fps);;
	qt_xy_block->set_update_time(1.0/fps);;
	qt_hist_block->set_update_time(1.0/fps);;

}


void Oscilloscope::setFilteringEnabled(bool set)
{
	m_filtering_enabled = set;
	setSampleRate(active_sample_rate);
}

void Oscilloscope::enableMixedSignalView(ChannelWidget *cw)
{
	const bool iioStarted = isIioManagerStarted();
	if (iioStarted) {
		iio->lock();
	}

	const bool hasLabels = plot.labelsEnabled();
	if (hasLabels) {
		enableLabels(false);
	}

	m_mixedSignalViewEnabled = true;

	m_mixedSignalViewMenu = m_logicAnalyzer->enableMixedSignalView(&plot, nb_channels +
								       nb_math_channels + nb_ref_channels);

	ui->logicSettingsLayout->addWidget(m_mixedSignalViewMenu[0]);

	QTabWidget *tb = qobject_cast<QTabWidget *>(m_mixedSignalViewMenu[0]);
	showLogicAnalyzerTriggerConnection = connect(&trigger_settings, &TriggerSettings::showLogicAnalyzerTriggerSettings,
						     this, [=](){
		cw->menuButton()->setChecked(true);
		tb->setCurrentIndex(tb->indexOf(m_mixedSignalViewMenu[1]));
	});

	mixed_sink = mixed_signal_sink::make(m_logicAnalyzer, &this->plot, active_sample_count);

	double targetFps = getScopyPreferences()->getTarget_fps();

	// this was buggy from the beginning - when using autotriggering, mixed signal acquisition restarts
	// with wrong trigger settings because autotriggering switches inbetween always and actual trigger mode
	// workaround consists in using 2 kernel buffers before restarting the acquisition
	mixed_source = gr::m2k::mixed_signal_source::make_from(m_m2k_context, active_sample_count, targetFps, 2);

	if (iioStarted) {
		// enable the mixed_source in the iio_manager
		iio->enableMixedSignal(mixed_source);

		boost::shared_ptr<adc_sample_conv> block = dynamic_pointer_cast<adc_sample_conv>(adc_samp_conv_block);

		// connect analog
		for (int i = 0; i < nb_channels; ++i) {
			iio->disconnect(block, i, qt_time_block, i);
			iio->connect(block, i, mixed_sink, i);
		}

		// connect digital
		iio->connect(mixed_source, 2, mixed_sink, 2);


		setDigitalPlotCurvesParams();
		iio->unlock();
	}

	trigger_settings.enableMixedSignalView();

	onHorizScaleValueChanged(timeBase->value());
	onTimePositionChanged(timePosition->value());

	if (hasLabels) {
		enableLabels(true);
	}
}

void Oscilloscope::disableMixedSignalView()
{

	const bool iioStarted = isIioManagerStarted();
	if (iioStarted) {
		iio->lock();
	}

	m_mixedSignalViewEnabled = false;

	disconnect(showLogicAnalyzerTriggerConnection);

	// disable mixed signal from logic
	ui->logicSettingsLayout->removeWidget(m_mixedSignalViewMenu[0]);
	m_logicAnalyzer->disableMixedSignalView();

	m_mixedSignalViewMenu[0]->deleteLater();

	if (iioStarted) {
		// disable the mixed_source in the iio_manager
		iio->disableMixedSignal(mixed_source);

		boost::shared_ptr<adc_sample_conv> block = dynamic_pointer_cast<adc_sample_conv>(adc_samp_conv_block);

		// disconnect analog
		for (int i = 0; i < nb_channels; ++i) {
			iio->disconnect(block, i, mixed_sink, i);
			iio->connect(block, i, qt_time_block, i);
		}

		// disconnect digital
		iio->disconnect(mixed_source, 2, mixed_sink, 2);

		iio->unlock();
	}

	mixed_sink = nullptr;
	mixed_source = nullptr;

	trigger_settings.disableMixedSignalView();
}

void Oscilloscope::setDigitalPlotCurvesParams()
{
	m_m2k_digital->setSampleRateIn(active_sample_rate);
	// active_sample_count
	qDebug() << "Delay: " << -(static_cast<int>(active_sample_count) / 2) + active_time_pos * active_sample_rate;
	m_m2k_digital->getTrigger()->setDigitalStreamingFlag(false);
	m_m2k_digital->getTrigger()->setDigitalDelay(-(static_cast<int>(active_sample_count) / 2) + active_time_pos * active_sample_rate);
//		qDebug() << "time pos: " << active_time_pos << "   active sample rate: " << active_sample_rate;
	for (int i = 0; i < plot.getNrDigitalPlotCurves(); ++i) {
		QwtPlotCurve *curve = plot.getDigitalPlotCurve(i);
		GenericLogicPlotCurve *logic_curve = dynamic_cast<GenericLogicPlotCurve *>(curve);
//		logic_curve->reset();

		logic_curve->setSampleRate(active_sample_rate);
		logic_curve->setBufferSize(active_sample_count);

		logic_curve->setTimeTriggerOffset(-(active_sample_count / 2.0) + (active_time_pos * active_sample_rate));
	}
}

void Oscilloscope::toggleMiniHistogramPlotVisible(bool enabled)
{
	miniHistogram = enabled;
	if (hist_plot.getOrientation() == Qt::Horizontal) {
		hist_plot.setVisible(enabled);
		plot.setBonusWidthForHistogram(enabled ? 25 : 0);
	}
}

void Oscilloscope::init_channel_settings()
{
	connect(ch_ui->btnEditMath, &QPushButton::toggled, this,
		&Oscilloscope::openEditMathPanel);

	connect(ch_ui->btnAutoset, &QPushButton::clicked, this,
		&Oscilloscope::requestAutoset);

	connect(ui->btnAddMath, &QPushButton::toggled, [=](bool on){
		if (on && !addChannel) {
			addChannel = true;
			math_pair->first.btnAddChannel->setText(tr("Add Channel"));
			ch_ui->math_settings_widget->setVisible(false);
			ch_ui->btnAutoset->setVisible(autosetEnabled);
		}
	});
	connect(ch_ui->lineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
		plot.setPlotLineStyle(current_ch_widget, index);
	});

	connect(ch_ui->snapshotBtn, &QPushButton::clicked, [=](){
		ChannelWidget *cw = channelWidgetAtId(current_ch_widget);
		if (current_ch_widget > 1 && cw->isReferenceChannel()) {
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
				FileManager fm("Oscilloscope");
				fm.open(fileName, FileManager::EXPORT);

				QVector<double> time_data, volts_data;

				QwtPlotCurve *curve = plot.Curve(current_ch_widget);

				for (size_t i = 0; i < curve->data()->size(); ++i) {
					time_data.push_back(curve->sample(i).x());
					volts_data.push_back(curve->sample(i).y());
				}

				fm.save(time_data, "Time(S)");
				fm.save(volts_data, "Ref");

				fm.setSampleRate(active_sample_rate);

				fm.performWrite();
			}
		} else {
			// snapshot
			if (nb_math_channels + nb_ref_channels == MAX_MATH_CHANNELS) {
				return;
			}

			QwtPlotCurve *curve = plot.Curve(current_ch_widget);

			if (!curve->data()->size()) {
				return;
			}

			QVector<double> xData;
			QVector<double> yData;
			for (size_t i = 0; i < curve->data()->size(); ++i) {
				xData.push_back(curve->data()->sample(i).x());
				yData.push_back(curve->data()->sample(i).y());
			}

			QString qname = QString("REF %1").arg(nb_ref_channels + 1);

			add_ref_waveform(qname, xData, yData, active_sample_rate);
		}
	});
}


void Oscilloscope::activateAcCoupling(int i)
{
	double alpha, beta;
	bool trigger = (i == trigger_settings.currentChannel())
			&& trigger_settings.analogEnabled();
	if (trigger) {
		trigger_settings.setAcCoupled(true, i);
	}

	bool started = isIioManagerStarted();
	if(started) {
		iio->lock();
	}

	boost::shared_ptr<adc_sample_conv> block =
	dynamic_pointer_cast<adc_sample_conv>(
					adc_samp_conv_block);

	iio->disconnect(block, i, qt_time_block, i);
	iio->disconnect(adc_samp_conv_block, i, math_probe_atten.at(i), 0);

	iio->connect(block, i, dc_cancel.at(i), 0);
	iio->connect(dc_cancel.at(i), 0, qt_time_block, i);
	iio->connect(dc_cancel.at(i), 0, math_probe_atten.at(i), 0);

	iio->disconnect(block, i, qt_hist_block, i);
	iio->connect(dc_cancel.at(i), 0, qt_hist_block, i);

	if (trigger && !triggerLevelSink.first) {
		triggerLevelSink.first = boost::make_shared<signal_sample>();
		triggerLevelSink.second = i;
		keep_one = gr::blocks::keep_one_in_n::make(sizeof(float), active_sample_count);
		connect(&*triggerLevelSink.first, SIGNAL(triggered(std::vector<float>)),
			this, SLOT(updateTriggerLevelValue(std::vector<float>)));
		iio->connect(dc_cancel.at(i), 0, keep_one, 0);
		iio->connect(keep_one, 0, triggerLevelSink.first, 0);
	}

	if (xy_is_visible) {
		iio->disconnect(xy_channels.at(index_x).first, xy_channels.at(index_x).second,
				ftc, 0);
		iio->disconnect(xy_channels.at(index_y).first, xy_channels.at(index_y).second,
				ftc, 1);
		xy_channels.clear();

		for(unsigned int ch = 0; ch < nb_channels; ch++) {
			if (chnAcCoupled.at(ch)) {
				xy_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      dc_cancel.at(ch), 0));
			} else {
				xy_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      adc_samp_conv_block, ch));
			}
		}

		iio->connect(xy_channels.at(index_x).first, xy_channels.at(index_x).second,
			     ftc, 0);
		iio->connect(xy_channels.at(index_y).first, xy_channels.at(index_y).second,
			     ftc, 1);
	}

	if (fft_is_visible) {
		for (unsigned int i = 0; i < nb_channels; i++) {
			iio->disconnect(fft_channels.at(i).first, fft_channels.at(i).second,
					fft_blocks.at(i), 0);
		}
		fft_channels.clear();

		for(unsigned int ch = 0; ch < nb_channels; ch++) {
			if (chnAcCoupled.at(ch)) {
				fft_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      dc_cancel.at(ch), 0));
			} else {
				fft_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      adc_samp_conv_block, ch));
			}

			iio->connect(fft_channels.at(ch).first, fft_channels.at(ch).second,
				     fft_blocks.at(ch), 0);
		}
	}

	for(int ch = 0; ch < nb_channels; ch++) {
		iio->set_buffer_size(ids[ch], active_sample_count);
		dc_cancel.at(ch)->set_buffer_size(active_sample_count);
	}
	if (mixed_source) {
		mixed_source->set_buffer_size(active_sample_count);
		setDigitalPlotCurvesParams();
	}
	if(started) {
		iio->unlock();
	}
}

void Oscilloscope::deactivateAcCoupling(int i)
{
	bool trigger = (i == trigger_settings.currentChannel())
			&& trigger_settings.analogEnabled();
	if (trigger) {
		trigger_settings.setAcCoupled(false, i);
	}

	bool started = isIioManagerStarted();
	if(started) {
		iio->lock();
	}

	boost::shared_ptr<adc_sample_conv> block =
	dynamic_pointer_cast<adc_sample_conv>(
					adc_samp_conv_block);

	iio->disconnect(dc_cancel.at(i), 0, math_probe_atten.at(i), 0);
	iio->connect(adc_samp_conv_block, i, math_probe_atten.at(i), 0);

	iio->disconnect(block, i, dc_cancel.at(i), 0);
	iio->disconnect(dc_cancel.at(i), 0, qt_time_block, i);

	iio->connect(block, i, qt_time_block, i);

	iio->disconnect(dc_cancel.at(i), 0, qt_hist_block, i);
	iio->connect(block, i, qt_hist_block, i);

	if (trigger && triggerLevelSink.first) {
		disconnect(&*triggerLevelSink.first, SIGNAL(triggered(std::vector<float>)),
			this, SLOT(updateTriggerLevelValue(std::vector<float>)));

		iio->disconnect(dc_cancel.at(i), 0, keep_one, 0);
		iio->disconnect(keep_one, 0, triggerLevelSink.first, 0);
		trigger_settings.updateHwVoltLevels(i);
		triggerLevelSink.first = nullptr;
		triggerLevelSink.second = -1;
		keep_one = nullptr;
	}

	if (xy_is_visible) {
		iio->disconnect(xy_channels.at(index_x).first, xy_channels.at(index_x).second,
				ftc, 0);
		iio->disconnect(xy_channels.at(index_y).first, xy_channels.at(index_y).second,
				ftc, 1);
		xy_channels.clear();

		for(unsigned int ch = 0; ch < nb_channels; ch++) {
			if (chnAcCoupled.at(ch) && (ch != i)) {
				xy_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      dc_cancel.at(ch), 0));
			} else {
				xy_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      adc_samp_conv_block, ch));
			}
		}

		iio->connect(xy_channels.at(index_x).first, xy_channels.at(index_x).second,
			     ftc, 0);
		iio->connect(xy_channels.at(index_y).first, xy_channels.at(index_y).second,
			     ftc, 1);
	}

	if (fft_is_visible) {
		for (unsigned int ch = 0; ch < nb_channels; ch++) {
			iio->disconnect(fft_channels.at(ch).first, fft_channels.at(ch).second,
					fft_blocks.at(ch), 0);
		}
		fft_channels.clear();

		for(unsigned int ch = 0; ch < nb_channels; ch++) {
			if (chnAcCoupled.at(ch) && ch != i) {
				fft_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      dc_cancel.at(ch), 0));
			} else {
				fft_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      adc_samp_conv_block, ch));
			}

			iio->connect(fft_channels.at(ch).first, fft_channels.at(ch).second,
				     fft_blocks.at(ch), 0);
		}
	}

	for(int ch = 0; ch < nb_channels; ch++) {
		iio->set_buffer_size(ids[ch], active_sample_count);
		dc_cancel.at(ch)->set_buffer_size(active_sample_count);
	}
	if (mixed_source) {
		mixed_source->set_buffer_size(active_sample_count);
		setDigitalPlotCurvesParams();
	}

	if(started) {
		iio->unlock();
	}
}

void Oscilloscope::configureAcCouplingTrigger(bool enabled)
{
	int chIdx = trigger_settings.currentChannel();
	if (enabled && chnAcCoupled.at(chIdx)) {
		activateAcCouplingTrigger(chIdx);
	} else if (!enabled && chnAcCoupled.at(chIdx)) {
		deactivateAcCouplingTrigger();
	}
	triggerAcCoupled = enabled;
}

void Oscilloscope::activateAcCouplingTrigger(int chIdx)
{
	trigger_settings.setAcCoupled(true, chIdx);
	if (!triggerLevelSink.first) {
		bool started = isIioManagerStarted();
		if (started) {
			iio->lock();
		}
		triggerLevelSink.first = boost::make_shared<signal_sample>();
		triggerLevelSink.second = chIdx;
		keep_one = gr::blocks::keep_one_in_n::make(sizeof(float), 100);
		connect(&*triggerLevelSink.first, SIGNAL(triggered(std::vector<float>)),
			this, SLOT(updateTriggerLevelValue(std::vector<float>)));
		iio->connect(dc_cancel.at(triggerLevelSink.second), 0, keep_one, 0);
		iio->connect(keep_one, 0, triggerLevelSink.first, 0);

		if (started) {
			iio->unlock();
		}
	}
}

void Oscilloscope::deactivateAcCouplingTrigger()
{
	trigger_settings.setAcCoupled(false, triggerLevelSink.second);
	if (triggerLevelSink.first) {
		bool started = isIioManagerStarted();
		if (started) {
			iio->lock();
		}
		/* Disconnect the SLOT */
		disconnect(&*triggerLevelSink.first, SIGNAL(triggered(std::vector<float>)),
			this, SLOT(updateTriggerLevelValue(std::vector<float>)));
		/* Disconnect the GNU Radio block */
		iio->disconnect(dc_cancel.at(triggerLevelSink.second), 0, keep_one, 0);
		iio->disconnect(keep_one, 0, triggerLevelSink.first, 0);
		trigger_settings.updateHwVoltLevels(triggerLevelSink.second);
		if (started) {
			iio->unlock();
		}
	}
	triggerLevelSink.first = nullptr;
	triggerLevelSink.second = -1;
	keep_one = nullptr;
}

void Oscilloscope::configureAcCoupling(int i, bool coupled)
{
	bool tmp = chnAcCoupled[i];
	chnAcCoupled[i] = coupled;

	if (coupled && !tmp) {
		activateAcCoupling(i);
	} else if (!coupled && tmp) {
		deactivateAcCoupling(i);
	} else if (coupled && tmp) {
		deactivateAcCoupling(i);
		activateAcCoupling(i);
	}
	scaleHistogramPlot();
}

void Oscilloscope::enableLabels(bool enable)
{
	plot.setUsingLeftAxisScales(enable);
	plot.enableLabels(enable);

	if (enable) {
		bool allDisabled = true;

		for (unsigned int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels;
		     i++) {
			ChannelWidget *cw = static_cast<ChannelWidget *>(
			                            ui->channelsList->itemAt(i)->widget());

			if (cw->enableButton()->isChecked()) {
				allDisabled = false;
			}
		}

		if (!allDisabled) {
			plot.setActiveVertAxis(current_channel);
			plot.enableAxisLabels(enable);
		}
	}
	if (!enable)
		plot.enableAxisLabels(enable);
}

void Oscilloscope::setTrigger_input(bool value)
{
	trigger_input = value;
}

bool Oscilloscope::getTrigger_input() const
{
	return trigger_input;
}

void Oscilloscope::cursor_panel_init()
{
	cr_ui = new Ui::CursorsSettings;
	cr_ui->setupUi(ui->cursorsSettings);
	setDynamicProperty(cr_ui->btnLockHorizontal, "use_icon", true);
	setDynamicProperty(cr_ui->btnLockVertical, "use_icon", true);
	//cr_ui->posSelect->setStyleSheet("background-color:red;");

	connect(cr_ui->btnLockHorizontal, &QPushButton::toggled,
		&plot, &CapturePlot::setHorizCursorsLocked);
	connect(cr_ui->btnLockVertical, &QPushButton::toggled,
		&plot, &CapturePlot::setVertCursorsLocked);

	cursorsPositionButton = new CustomPlotPositionButton(cr_ui->posSelect);

	connect(cr_ui->hCursorsEnable, SIGNAL(toggled(bool)),
		&plot, SLOT(setVertCursorsEnabled(bool)));
	connect(cr_ui->vCursorsEnable, SIGNAL(toggled(bool)),
		&plot, SLOT(setHorizCursorsEnabled(bool)));

	connect(cr_ui->hCursorsEnable, SIGNAL(toggled(bool)),
		cursor_readouts_ui->TimeCursors,
		SLOT(setVisible(bool)));
	connect(cr_ui->vCursorsEnable, SIGNAL(toggled(bool)),
		cursor_readouts_ui->VoltageCursors,
		SLOT(setVisible(bool)));
	connect(cr_ui->btnNormalTrack, &QPushButton::toggled,
		this, &Oscilloscope::toggleCursorsMode);

	cr_ui->horizontalSlider->setMaximum(100);
	cr_ui->horizontalSlider->setMinimum(0);
	cr_ui->horizontalSlider->setSingleStep(1);
	connect(cr_ui->horizontalSlider, &QSlider::valueChanged, [=](int value){
		cr_ui->transLabel->setText(tr("Transparency ") + QString::number(value) + "%");
		plot.setCursorReadoutsTransparency(value);
	});
	cr_ui->horizontalSlider->setSliderPosition(0);

	connect(cursorsPositionButton, &CustomPlotPositionButton::positionChanged,
		[=](CustomPlotPositionButton::ReadoutsPosition position){
		plot.moveCursorReadouts(position);
	});
}

void Oscilloscope::toggleCursorsMode(bool toggled)
{
	cr_ui->hCursorsEnable->setEnabled(toggled);
	cr_ui->vCursorsEnable->setEnabled(toggled);

	if (toggled) {
		plot.setVertCursorsEnabled(hCursorsEnabled);
		plot.setHorizCursorsEnabled(vCursorsEnabled);
		cursor_readouts_ui->TimeCursors->setVisible(vCursorsEnabled);
		cursor_readouts_ui->VoltageCursors->setVisible(hCursorsEnabled);
	} else {
		hCursorsEnabled = cr_ui->hCursorsEnable->isChecked();
		vCursorsEnabled = cr_ui->vCursorsEnable->isChecked();
		plot.setVertCursorsEnabled(true);
		plot.setHorizCursorsEnabled(true);
		cursor_readouts_ui->TimeCursors->setVisible(true);
		cursor_readouts_ui->VoltageCursors->setVisible(true);
	}

	cr_ui->btnLockVertical->setEnabled(toggled);
	plot.trackModeEnabled(toggled);
}

void Oscilloscope::toolDetached(bool detached)
{
	if (detached) {
		this->setMinimumSize(910, 490);
		this->setSizePolicy(QSizePolicy::Preferred,
				    QSizePolicy::Preferred);
	} else {
		this->setMinimumWidth(0);
		this->setSizePolicy(QSizePolicy::MinimumExpanding,
				    QSizePolicy::MinimumExpanding);
	}
}

void Oscilloscope::pause(bool paused)
{
	if (ui->runSingleWidget->runButtonChecked()){
		toggle_blockchain_flow(!paused);
		trigger_settings.setAdcRunningState(!paused);
	}
}

void Oscilloscope::export_settings_init()
{

	exportSettings = new ExportSettings();
	exportSettings->enableExportButton(false);
	gsettings_ui->export_2->addWidget(exportSettings);

	for (int i = 0; i < nb_channels; ++i){
		exportSettings->addChannel(i, QString("Channel") +
					   QString::number(i + 1));
	}

	connect(exportSettings->getExportButton(), SIGNAL(clicked()), this,
		SLOT(btnExport_clicked()));
	connect(this, &Oscilloscope::activateExportButton,
		[=](){
		exportSettings->enableExportButton(true);
	});
}

void Oscilloscope::btnExport_clicked(){

	exportConfig = exportSettings->getExportConfig();
	pause(true);
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
	bool atleastOneChannelEnabled = false;

	auto keys = exportConfig.keys();
	for (auto x : qAsConst(keys))
		if (exportConfig[x]){
			atleastOneChannelEnabled = true;
			break;
		}
	if (!atleastOneChannelEnabled){
		return;
	}

	if (!fileName.isEmpty()){
		FileManager fm("Oscilloscope");
		fm.open(fileName, FileManager::EXPORT);

		int channels_number = nb_channels + nb_math_channels;
		QVector<double> time_data;

		for (int i = 0; i < plot.Curve(0)->data()->size(); ++i) {
			time_data.push_back(plot.Curve(0)->sample(i).x());
		}

		fm.save(time_data, "Time(S)");

		for (int i = 0; i < channels_number; ++i){
			if (exportConfig[i]){
				QVector<double> data;
				int samples = plot.Curve(i)->data()->size();
				for (int j = 0; j < samples; ++j)
					data.push_back(plot.Curve(i)->data()->sample(j).y());
				QString chNo = (i > 1) ? QString::number(i - 1) : QString::number(i + 1);

				fm.save(data, ((i > 1) ? "M" : "CH") + chNo + "(V)");
			}
		}

		fm.setSampleRate(active_sample_rate);
		fm.performWrite();
	}
	pause(false);
}

void Oscilloscope::create_add_channel_panel()
{
	/* Math stuff */

	QWidget *panel = new QWidget(this);
	Ui::MathPanel math_ui;

	math_ui.setupUi(panel);

	QWidget *logic = new QWidget(panel);

	QVBoxLayout *layout_logic = new QVBoxLayout(logic);
	logic->setLayout(layout_logic);

	QLabel *labelWarningMixedSignal = new QLabel(tr("\"Warning: This feature is not supported on the current firmware version!\""));
	labelWarningMixedSignal->setObjectName(QString::fromUtf8("lblWarningMixedSignal"));
	QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(labelWarningMixedSignal->sizePolicy().hasHeightForWidth());
	labelWarningMixedSignal->setSizePolicy(sizePolicy);
	labelWarningMixedSignal->setWordWrap(true);
	labelWarningMixedSignal->setVisible(false);

	layout_logic->insertSpacerItem(0, new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	layout_logic->setContentsMargins(0,0,0,0);
	layout_logic->insertWidget(1, labelWarningMixedSignal);

	QLabel *infoLabel = new QLabel(tr("* When the Mixed Signal View is enabled the LogicAnalyzer tool will be disabled!\n"
					  "** The trigger can be disabled or set only on the Digital channels or Analog channels, not on both at the same time!"), panel);
	layout_logic->insertWidget(2, infoLabel);
	infoLabel->setWordWrap(true);
	infoLabel->setVisible(true);

	QPushButton *btn = math_ui.btnAddChannel;

	Math *math = new Math(nullptr, nb_channels);

	math_pair = new QPair<Ui::MathPanel, Math *>(math_ui, math);

	connect(math, &Math::functionValid,
	[=](const QString &function) {
		btn->setEnabled(true);
		btn->setProperty("function",
		                 QVariant(function));
		lastFunctionValid = true;
	});

	connect(math, &Math::stateReseted, [=]() {
		btn->setEnabled(false);
		lastFunctionValid = false;
	});

	QHBoxLayout *layout = static_cast<QHBoxLayout *>(panel->layout());
	QSpacerItem *spacerItem = new QSpacerItem(0, 0, QSizePolicy::Minimum,
						  QSizePolicy::Expanding);
	tabWidget = new QTabWidget(panel);
	layout->insertWidget(0, tabWidget);
	//set top margin to 0 and the rest to 9
	layout->setContentsMargins(9, 0, 9, 9);
	tabWidget->addTab(math, tr("Math"));

	ref = new QWidget(panel);

	importSettings = new ImportSettings();

	QVBoxLayout *layout_ref = new QVBoxLayout(ref);
	layout_ref->setContentsMargins(9, 20, 9, 9);
	QLineEdit *fileLineEdit = new QLineEdit(ref);

	QWidget *file_select = new QWidget(ref);
	QVBoxLayout *layout_file_select = new QVBoxLayout(file_select);
	layout_file_select->addWidget(fileLineEdit);
	QPushButton *btnOpenFile = new QPushButton(file_select);
	btnOpenFile->setStyleSheet("QPushButton{"
				   "height:25px;"
				   "background-color: #4A64FF;"
				   "border-radius: 4px;"
				   "font-size: 12px;"
				   "line-height: 14px;"
				   "color: #FFFFFF;}"
				   "QPushButton:hover{"
					"background-color: #4a34ff;"
				   "}");
	btnOpenFile->setText(tr("Browse"));
	layout_file_select->addWidget(btnOpenFile);

	refChannelTimeBase = new ScaleSpinButton({
				{"ns", 1E-9},
				{"μs", 1E-6},
				{"ms", 1E-3},
				{"s", 1E0}
				}, tr("Time Base"), 100e-9, 1E0,
				true, false, this);

	refChannelTimeBase->setValue(1e-3);
	refChannelTimeBase->setDisabled(true);

	connect(refChannelTimeBase, &ScaleSpinButton::valueChanged, [=](double value){
		plot.updatePreview(value, timeBase->value(), timePosition->value());
	});

	layout_ref->addWidget(file_select);
	layout_ref->addWidget(importSettings);
	layout_ref->addWidget(refChannelTimeBase);
	layout_ref->addSpacerItem(spacerItem);


	importSettings->setDisabled(true);

	fileLineEdit->setText(tr("No file selected"));
	fileLineEdit->setDisabled(true);

	tabWidget->addTab(ref, tr("Reference"));

	tabWidget->addTab(logic, tr("Logic"));

	connect(ui->mixedSignalBtn, &QPushButton::clicked, [=](){
		if (!m_mixedSignalViewEnabled) {
			ui->btnAddMath->click();
			tabWidget->setCurrentIndex(tabWidget->indexOf(logic));
		} else {
			for (int i = 0; i < ui->channelsList->count(); ++i) {
				ChannelWidget *cw = qobject_cast<ChannelWidget *>(
							ui->channelsList->itemAt(i)->widget());
				if (cw && cw->fullName() == "Logic Analyzer") {
					cw->deleteButton()->click();
					break;
				}
			}
		}
	});

	connect(btnOpenFile, &QPushButton::clicked, this, &Oscilloscope::import);

	connect(tabWidget, &QTabWidget::currentChanged, [=](int index) {
		btn->setVisible(true);
        if (index == 0) {
            btn->setText(tr("Add channel"));
            btn->setEnabled(lastFunctionValid);
            labelWarningMixedSignal->setVisible(false);
        } else if (index == 1) {
            btn->setText(tr("Import selected channels"));
            btn->setEnabled(importSettings->isEnabled());
            labelWarningMixedSignal->setVisible(false);
        } else if (index == 2) {
            btn->setText(tr("Enable Mixed Signal View"));
            if (m_m2k_context->hasMixedSignal()) {
                btn->setEnabled(true);
            } else {
                btn->setEnabled(false);
                labelWarningMixedSignal->setVisible(true);
            }
        }
	});

	connect(btn, &QPushButton::clicked,
	[=]() {

		if (tabWidget->currentIndex() == 2) {

			qDebug() << "Enable mixed signal view!";
			ui->mixedSignalBtn->setText("Disable Mixed\nSignal View");

			ChannelWidget *logicAnalyzerChannelWidget = new ChannelWidget(-1, true, false,
										      QColor(Qt::yellow), this);
			logicAnalyzerChannelWidget->setFullName("Logic Analyzer");
			logicAnalyzerChannelWidget->setShortName("Logic");
			ui->channelsList->addWidget(logicAnalyzerChannelWidget);

			logicAnalyzerChannelWidget->nameButton()->setText(logicAnalyzerChannelWidget->shortName());

			channels_group->addButton(logicAnalyzerChannelWidget->nameButton());
			ui->settings_group->addButton(logicAnalyzerChannelWidget->menuButton());
			logicAnalyzerChannelWidget->nameButton()->setChecked(true);

			const int logicId = ui->stackedWidget->indexOf(ui->logicSettings);
			logicAnalyzerChannelWidget->menuButton()->setProperty("id", QVariant(-logicId));

			connect(logicAnalyzerChannelWidget->menuButton(), &QAbstractButton::toggled, [=](bool toggled){
				triggerRightMenuToggle(
					static_cast<CustomPushButton *>(logicAnalyzerChannelWidget->menuButton()), toggled);
			});

			const int logicTab = tabWidget->currentIndex();

			tabWidget->setTabEnabled(logicTab, false);

			connect(logicAnalyzerChannelWidget->deleteButton(), &QAbstractButton::clicked, [=](){

				if (logicAnalyzerChannelWidget->menuButton()->isChecked()) {
					menuButtonActions.removeAll(QPair<CustomPushButton*, bool>
								    (static_cast<CustomPushButton*>(logicAnalyzerChannelWidget->menuButton()), true));
					toggleRightMenu(static_cast<CustomPushButton*>(logicAnalyzerChannelWidget->menuButton()), false);
				}

				menuOrder.removeOne(static_cast<CustomPushButton*>(logicAnalyzerChannelWidget->menuButton()));

				ui->channelsList->removeWidget(logicAnalyzerChannelWidget);
				logicAnalyzerChannelWidget->deleteLater();

				tabWidget->setTabEnabled(logicTab, true);
				ui->mixedSignalBtn->setText("Enable Mixed\nSignal View");

				disableMixedSignalView();
			});

			logicAnalyzerChannelWidget->menuButton()->setChecked(true);

			enableMixedSignalView(logicAnalyzerChannelWidget);

			return;
		}

		if (tabWidget->currentIndex() != 0) {
			QMap<int, bool> import_map = importSettings->getExportConfig();

			auto keys = import_map.keys();
			for (int key : qAsConst(keys)) {
				if (import_map[key]) {
					add_ref_waveform(key);
				}
			}
			plot.clearPreview();
			ChannelWidget *cw = channelWidgetAtId(nb_channels + nb_math_channels +
							      nb_ref_channels - 1);
			triggerRightMenuToggle(ui->btnAddMath, false);
			cw->menuButton()->setChecked(true);
			refChannelTimeBase->setDisabled(true);
			fileLineEdit->setText("");
			fileLineEdit->setToolTip("");
			importSettings->setEnabled(false);
			btn->setDisabled(true);
			return;
		}

		if (addChannel) {
			QVariant var = btn->property("function");
			add_math_channel(var.toString().toStdString());
		} else {
			QVariant var = btn->property("function");
			editMathChannelFunction(current_ch_widget,
			                        var.toString().toStdString());
		}
	});

	connect(this, &Oscilloscope::importFileLoaded, [=](bool loaded) {
			btn->setEnabled(loaded);
			importSettings->setEnabled(loaded);
			fileLineEdit->setText(import_error);
			fileLineEdit->setToolTip(loaded ? import_error : "");
			if (!loaded) {
				importSettings->clear();
			}
	});

	int panel_id = ui->stackedWidget->insertWidget(-1, panel);
	ui->btnAddMath->setProperty("id", QVariant(-panel_id));
}

void Oscilloscope::import()
{
	QString fileName = QFileDialog::getOpenFileName(this,
	    tr("Import"), "", tr("Comma-separated values files (*.csv);;"
				       "Tab-delimited values files (*.txt)"),
	    nullptr, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	FileManager fm("Oscilloscope");

	importSettings->clear();
	import_data.clear();

	try {
		fm.open(fileName, FileManager::IMPORT);

		double nrOfSamples = fm.getNrOfSamples();
		double sampRate = fm.getSampleRate();
		double timeBase = (nrOfSamples / 16.0) / sampRate;

		if (fm.getFormat() == FileManager::RAW) {
			refChannelTimeBase->setEnabled(true);
		} else {
			refChannelTimeBase->setEnabled(false);
			refChannelTimeBase->setValue(timeBase);
		}

		QVector<QVector<double>> data = fm.read();
		for (int i = 0; i < data.size(); ++i) {
			import_data.push_back(data[i]);
		}

		import_error = fileName;
		Q_EMIT importFileLoaded(true);

		for (int i = 0; i < fm.getNrOfChannels(); ++i) {
			importSettings->addChannel(i, fm.getColumnName(i).remove("(V)"));
		}

		if (refChannelTimeBase->isEnabled()) {
			plot.addPreview(import_data, refChannelTimeBase->value(),
					this->timeBase->value(), timePosition->value());
		}

	} catch (FileManagerException &ex) {
		import_error = QString(ex.what());
		Q_EMIT importFileLoaded(false);
	}
}

unsigned int Oscilloscope::find_curve_number()
{
	unsigned int id = 0;
	bool found;

	do {
		found = false;

		for (unsigned int i = 0; !found && i < (nb_math_channels + nb_ref_channels); i++) {
			QWidget *parent = ui->channelsList->itemAt(nb_channels + i)->widget();

			ChannelWidget *cw = static_cast<ChannelWidget *>(parent);

			if (cw->isReferenceChannel()) {
				continue;
			}

			found = parent->property("curve_nb").toUInt() == id;
		}

		id++;
	} while (found);

	return id - 1;
}

void Oscilloscope::add_math_channel(const std::string& function)
{
	if (nb_math_channels + nb_ref_channels == MAX_MATH_CHANNELS) {
		return;
	}

	auto rail = gr::analog::rail_ff::make(MIN_MATH_RANGE, MAX_MATH_RANGE);
	auto math = gr::scopy::iio_math::make(function, nb_channels);
	unsigned int curve_id = nb_channels + nb_math_channels + nb_ref_channels;
	unsigned int curve_number = find_curve_number();

	nb_math_channels++;
	probe_attenuation.push_back(1);

	QString qname = QString("Math %1").arg(math_chn_counter++);
	std::string name = qname.toStdString();

	auto math_sink = adiscope::scope_sink_f::make(
			noZoomXAxisWidth * getSampleRate() / m_m2k_analogin->getOversamplingRatio(),
			getSampleRate() / m_m2k_analogin->getOversamplingRatio(), name, 1, (QObject *)&plot);

	double targetFps = getScopyPreferences()->getTarget_fps();
	math_sink->set_update_time(1.0/targetFps);
	/* Add the math block and the math scope sink into a container, so that
	 * we can disconnect them when removing the math channel later */
	auto math_pair = QPair<gr::basic_block_sptr, gr::basic_block_sptr>(
				math, math_sink);
	math_sinks.insert(qname, math_pair);
	math_rails.insert(qname, rail);

	/* Lock the flowgraph if we are already started */
	bool started = isIioManagerStarted();
	if (started)
		iio->lock();

	math_sink->set_trigger_mode(TRIG_MODE_TAG, 0, "buffer_start");

	for (unsigned int i = 0; i < nb_channels; i++) {
		iio->connect(math_probe_atten.at(i), 0, math, i);
	}
	iio->connect(math, 0, rail, 0);
	iio->connect(rail, 0, math_sink, 0);

	if (started)
		iio->unlock();

	plot.registerMathWaveform(name, 1,
			noZoomXAxisWidth *
			getSampleRate());

	ChannelWidget *channel_widget = new ChannelWidget(curve_id, true, false,
		plot.getLineColor(curve_id).name(), this);

	channel_widget->setMathChannel(true);
	channel_widget->setFunction(QString::fromStdString(function));

	channel_widget->setFullName(QString("Math %1").arg(curve_number + 1));
	channel_widget->setShortName(QString("M%1").arg(curve_number + 1));
	channel_widget->nameButton()->setText(channel_widget->shortName());

	exportSettings->addChannel(curve_id - nb_ref_channels,
		channel_widget->fullName());

	channel_widget->setProperty("curve_nb", QVariant(curve_number));
	channel_widget->deleteButton()->setProperty(
		"curve_name", QVariant(qname));

	connect(channel_widget, SIGNAL(enabled(bool)),
		SLOT(onChannelWidgetEnabled(bool)));
	connect(channel_widget, SIGNAL(selected(bool)),
		SLOT(onChannelWidgetSelected(bool)));
	connect(channel_widget, SIGNAL(menuToggled(bool)),
		SLOT(onChannelWidgetMenuToggled(bool)));
	connect(channel_widget, SIGNAL(deleteClicked()),
		SLOT(onChannelWidgetDeleteClicked()));
	connect(channel_widget, &ChannelWidget::menuToggled,
		[=](bool on){
		if (!on)
			ch_ui->btnEditMath->setChecked(false);
	});

	ui->channelsList->addWidget(channel_widget);

	channels_group->addButton(channel_widget->nameButton());
	ui->settings_group->addButton(channel_widget->menuButton());
	channel_widget->nameButton()->setChecked(true);

	QLabel *label= new QLabel(this);
		label->setText(vertMeasureFormat.format(
			plot.VertUnitsPerDiv(curve_id), "V/div", 3));
		label->setStyleSheet(QString("QLabel {"
				"color: %1;"
				"font-weight: bold;"
				"}").arg(plot.getLineColor(curve_id).name()));
		ui->chn_scales->addWidget(label);

	plot.Curve(curve_id)->setAxes(
			QwtAxisId(QwtAxis::XBottom, 0),
			QwtAxisId(QwtAxis::YLeft, curve_id));
	plot.Curve(curve_id)->setTitle("M " + QString::number(curve_number + 1));
	plot.addZoomer(curve_id);
	plot.replot();

	/* We added a Math channel that is enabled by default,
	 * so enable the Run button */
	updateRunButton(true);

	// Default hysteresis levels for measurements of the new channel
	plot.setPeriodDetectHyst(curve_id, 1.0 / 5);

	// Keep the current selected channels curve on top of the other ones
	if (isVisible() && channelWidgetAtId(current_channel)->enableButton()->isChecked())
		plot.bringCurveToFront(current_channel);

	if (nb_math_channels + nb_ref_channels == MAX_MATH_CHANNELS) {
		if (ui->btnAddMath->isChecked()) {
			ui->btnAddMath->setChecked(false);
		}

		ui->btnAddMath->hide();
		menuOrder.removeOne(ui->btnAddMath);
	}

	gsettings_ui->cmb_x_channel->addItem(channel_widget->shortName());
	gsettings_ui->cmb_y_channel->addItem(channel_widget->shortName());
	if(xy_is_visible)
		setup_xy_channels();

	plot.showYAxisWidget(curve_id, true);
	setSinksDisplayOneBuffer(d_displayOneBuffer);
	init_selected_measurements(curve_id, {0, 1, 4, 5});
}

void Oscilloscope::onChannelWidgetDeleteClicked()
{
	if (nb_math_channels + nb_ref_channels - 1 < MAX_MATH_CHANNELS) {
		ui->btnAddMath->show();
	}

	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());
	QAbstractButton *delBtn = cw->deleteButton();
	QString qname = delBtn->property("curve_name").toString();
	unsigned int curve_id = cw->id();

	probe_attenuation.removeAt(curve_id);
	if (curve_id == current_ch_widget &&
			cw->menuButton()->isChecked()) {
		menuButtonActions.removeAll(QPair<CustomPushButton*, bool>
					    (static_cast<CustomPushButton*>(cw->menuButton()), true));
		toggleRightMenu(static_cast<CustomPushButton*>(cw->menuButton()), false);
	}
	menuOrder.removeOne(static_cast<CustomPushButton*>(cw->menuButton()));

	/*If there are no more channels enabled, we should
	disable the measurements.*/
	bool shouldDisable = true;

	for (unsigned int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels;
	     i++) {
		ChannelWidget *cw = static_cast<ChannelWidget *>(
			ui->channelsList->itemAt(i)->widget());
		if (curve_id == cw->id())
			continue;
		if (cw->enableButton()->isChecked())
			shouldDisable = false;
	}

	if (shouldDisable)
		measure_settings->disableDisplayAll();

	measure_settings->onChannelRemoved(curve_id);

	if (cw->isMathChannel()) {
		plot.unregisterMathWaveform(qname.toStdString());

		exportSettings->removeChannel(curve_id - nb_ref_channels);
		exportConfig.remove(curve_id - nb_ref_channels);

		/* Lock the flowgraph if we are already started */
		bool started = isIioManagerStarted();

		if (started) {
			iio->lock();
		}

		locked = true;

		gsettings_ui->cmb_x_channel->blockSignals(true);
		gsettings_ui->cmb_y_channel->blockSignals(true);
		gsettings_ui->cmb_x_channel->removeItem(curve_id);
		gsettings_ui->cmb_y_channel->removeItem(curve_id);

		if (index_x >= curve_id) {
			gsettings_ui->cmb_x_channel->setCurrentIndex(curve_id-1);
		}

		if (index_y >= curve_id) {
			gsettings_ui->cmb_y_channel->setCurrentIndex(curve_id-1);
		}

		gsettings_ui->cmb_x_channel->blockSignals(false);
		gsettings_ui->cmb_y_channel->blockSignals(false);
		setup_xy_channels();

		/* Disconnect the blocks from the running flowgraph */
		auto pair = math_sinks.take(qname);
		auto rail = math_rails.take(qname);

		for (unsigned int i = 0; i < nb_channels; i++) {
			iio->disconnect(math_probe_atten.at(i), 0, pair.first, i);
		}

		iio->disconnect(pair.first, 0, rail, 0);
		iio->disconnect(rail, 0, pair.second, 0);

		if (xy_is_visible) {
			setup_xy_channels();
		}

		locked = false;

		if (started) {
			iio->unlock();
		}

		for (unsigned int i = curve_id + 1;
		     i < nb_channels + nb_math_channels + nb_ref_channels; i++) {
			ChannelWidget *w = static_cast<ChannelWidget *>(
			                           ui->channelsList->itemAt(i)->widget());

			/* Update the IDs */
			w->setId(i - 1);
		}

		nb_math_channels--;
	} else if (cw->isReferenceChannel()) {
		plot.unregisterReferenceWaveform(qname);

		for (unsigned int i = curve_id + 1;
		     i < nb_channels + nb_math_channels + nb_ref_channels; i++) {
			ChannelWidget *w = static_cast<ChannelWidget *>(
			                           ui->channelsList->itemAt(i)->widget());

			/* Update the IDs */
			w->setId(i - 1);
		}

		nb_ref_channels--;
	}

	/* Remove the math channel from the bottom list of channels */
	ui->channelsList->removeWidget(cw);
	delete cw;

	/*If the deleted channel is the current channel, check if we have
	enabled channels and select the first one we find
	else update the plots axis and zoomer properties from channel 0.*/
	bool channelsEnabled = false;

	if (curve_id < current_channel) {
		/*If the deleted math channel is before the current channel,
		we will need to update the current channel to its new value.*/
		current_channel -= 1;
		if (curve_id < current_ch_widget) {
			current_ch_widget -= 1;
		}
		channelsEnabled = true;
		Q_EMIT selectedChannelChanged(current_channel);
		update_measure_for_channel(current_channel);
	} else if (curve_id == current_channel) {
		for (unsigned int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels;
		     i++) {
			ChannelWidget *cw = static_cast<ChannelWidget *>(
			                            ui->channelsList->itemAt(i)->widget());

			if (cw->enableButton()->isChecked()) {
				channelsEnabled = true;
				Q_EMIT selectedChannelChanged(0);
				update_measure_for_channel(0);
				cw->nameButton()->setChecked(true);
				Q_EMIT selectedChannelChanged(cw->id());
				update_measure_for_channel(cw->id());
				break;
			}
		}
		if (channelsEnabled) {
			if (curve_id < current_ch_widget) {
				current_ch_widget -= 1;
			}
		} else {
			current_channel = 0;
			Q_EMIT selectedChannelChanged(0);
			enableLabels(false);
		}
	} else if (curve_id < current_ch_widget) {
		current_ch_widget -= 1;
		channelsEnabled = true;
	}

	plot.setActiveVertAxis(current_channel);

	/* If the removed channel is before the current axis, we update the
	 * current axis to account for the index change */
	int current_axis = plot.activeVertAxis();
	if (channelsEnabled && (current_axis > curve_id))
		plot.setActiveVertAxis(current_axis - 1);

	/* Before removing the axis, remove the offset widgets */
	plot.removeOffsetWidgets(curve_id);

	/* Remove the axis that corresponds to the curve we drop */
	plot.removeLeftVertAxis(curve_id);

	/* Remove scale label */
	QWidget *scale_lbl = ui->chn_scales->itemAt(curve_id)->widget();
	ui->chn_scales->removeWidget(scale_lbl);
	delete scale_lbl;

	for (unsigned int i = nb_channels;
	     i < nb_channels + nb_math_channels + nb_ref_channels; i++) {
		/* Update the curve-to-axis map */
		plot.Curve(i)->setAxes(
		        QwtAxisId(QwtAxis::XBottom, 0),
		        QwtAxisId(QwtAxis::YLeft, i));
	}

	onMeasurementSelectionListChanged();
	plot.removeZoomer(curve_id);
	updateRunButton(false);
	plot.replot();
}

void Oscilloscope::clearMathChannels()
{
	int idx = 2;
	while (nb_math_channels) {
		if (!channelWidgetAtId(idx)->isReferenceChannel()) {
			channelWidgetAtId(idx)->deleteButton()->click();
		} else {
			idx++;
		}

	}
}

void Oscilloscope::on_actionClose_triggered()
{
	this->close();
}

void Oscilloscope::toggle_blockchain_flow(bool en)
{
	if (en) {

		if (m_mixedSignalViewEnabled) {
			// enable the mixed_source in the iio_manager
			iio->enableMixedSignal(mixed_source);

			boost::shared_ptr<adc_sample_conv> block = dynamic_pointer_cast<adc_sample_conv>(adc_samp_conv_block);

			// connect analog
			for (int i = 0; i < nb_channels; ++i) {
				iio->disconnect(block, i, qt_time_block, i);
				iio->connect(block, i, mixed_sink, i);
			}

			// connect digital
			iio->connect(mixed_source, 2, mixed_sink, 2);

			// set digital params; analog should be already set when this method is called
			setDigitalPlotCurvesParams();

			mixed_sink->clean_buffers();
			mixed_sink->set_nsamps(active_sample_count);
		}

		if (autosetRequested) {
			iio->start(autoset_id[0]);
		}

		for (unsigned int i = 0; i < nb_channels; i++)
			iio->start(ids[i]);

		scaleHistogramPlot();

	} else {

		for (unsigned int i = 0; i < nb_channels; i++)
			iio->stop(ids[i]);

		if (autosetRequested) {
			iio->stop(autoset_id[0]);			
		}

		if (m_mixedSignalViewEnabled) {
			iio->disableMixedSignal(mixed_source);

			boost::shared_ptr<adc_sample_conv> block = dynamic_pointer_cast<adc_sample_conv>(adc_samp_conv_block);

			// disconnect analog
			for (int i = 0; i < nb_channels; ++i) {
				iio->disconnect(block, i, mixed_sink, i);
				iio->connect(block, i, qt_time_block, i);
			}

			// disconnect digital
			iio->disconnect(mixed_source, 2, mixed_sink, 2);
		}
	}
}

void Oscilloscope::run()
{
	runStopToggled(true);
}
void Oscilloscope::stop()
{
	runStopToggled(false);
}

void Oscilloscope::runStopToggled(bool checked)
{
	if (m_running == checked) {
		return;
	}

	Q_EMIT activateExportButton();

	if (checked) {
		periodicFlowRestart(true);
		if (symmBufferMode->isEnhancedMemDepth()) {
			onCmbMemoryDepthChanged(ch_ui->cmbMemoryDepth->currentText());
		}

		writeAllSettingsToHardware();

		plot.setSampleRate(active_sample_rate, 1, "");
		plot.setBufferSizeLabelValue(active_plot_sample_count);
		plot.setSampleRatelabelValue(active_sample_rate);
		if(fft_is_visible) {
			setFFT_params();
		}

		last_set_sample_count = active_plot_sample_count;

		if (active_trig_sample_count !=
				trigger_settings.triggerDelay()) {
			trigger_settings.setTriggerDelay(
				active_trig_sample_count);
			last_set_time_pos = active_time_pos;
		}

		if ((timePosition->value() != active_time_pos)
				&& !symmBufferMode->isEnhancedMemDepth()) {
			timePosition->setValue(active_time_pos);
		}

		setTrigger_input(false);

		resetStreamingFlag(symmBufferMode->isEnhancedMemDepth()
				   || plot_samples_sequentially);
		toggle_blockchain_flow(true);

		scaleHistogramPlot();
	} else {
		toggle_blockchain_flow(false);
		resetStreamingFlag(symmBufferMode->isEnhancedMemDepth()
				   || plot_samples_sequentially);
		trigger_settings.setAdcRunningState(false);
	}

	// Update trigger status
	m_running = checked;
	plot.startStop(checked);
	hist_plot.startStop(checked);

	triggerUpdater->setEnabled(checked);
}

void Oscilloscope::setFFT_params(bool force)
{
	if(fft_plot.sampleRate() != getSampleRate() || force) {
		fft_plot.setSampleRate(getSampleRate(), 1, "");
		double start = 0;
		double stop =  getSampleRate() / 2;
		fft_plot.setAxisScale(QwtAxis::XBottom, start, stop);
		fft_plot.setAxisScale(QwtAxis::YLeft, -200, 0, 10);
		fft_plot.zoomBaseUpdate();
	}
}

void Oscilloscope::setChannelWidgetIndex(int chnIdx)
{
	current_ch_widget = chnIdx;
	plot.bringCurveToFront(chnIdx);

	for (unsigned int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels;
	     ++i) {
		if (i == chnIdx) {
			continue;
		}

		plot.showYAxisWidget(i, false);
	}
}

void Oscilloscope::autosetFFT()
{
	auto fft = gnuradio::get_initial_sptr(
				new fft_block(false, autoset_fft_size));

	auto ctm = blocks::complex_to_mag_squared::make(1);
	auto log = blocks::nlog10_ff::make(10);

	if(autoset_id[0]) {
		iio->disconnect(autoset_id[0]);
		autoset_id[0] = nullptr;
	}
	autosetFFTSink = blocks::vector_sink_f::make();

	autoset_id[0] = iio->connect(fft,autosetChannel,0,true);

	iio->connect(fft,0,ctm,0);
	iio->connect(ctm,0,log,0);
	iio->connect(log,0,autosetFFTSink,0);
}

void Oscilloscope::onFFT_view_toggled(bool visible)
{
	/* Lock the flowgraph if we are already started */
	bool started = isIioManagerStarted();
	if (started)
		iio->lock();

	if (visible) {
		qt_fft_block->set_nsamps(fft_plot_size);

		if (fft_is_visible && (fft_channels.size() > 0)) {
			for (unsigned int i = 0; i < nb_channels; i++) {
				/** GNU Radio flow: iio(i) ->  fft -> ctm -> qt_fft_block */
				iio->disconnect(fft_channels.at(i).first, fft_channels.at(i).second,
						fft_blocks.at(i), 0);
				iio->disconnect(fft_blocks.at(i), 0, ctm_blocks.at(i), 0);
				iio->disconnect(ctm_blocks.at(i), 0, qt_fft_block, i);
			}
		}
		setFFT_params();

		for (unsigned int i = 0; i < nb_channels; i++) {
			fft_blocks.push_back(gnuradio::get_initial_sptr(
						     new fft_block(false, fft_plot_size)));
			ctm_blocks.push_back(blocks::complex_to_mag_squared::make(1));

			if (chnAcCoupled.at(i)) {
				fft_channels.push_back(QPair<gr::basic_block_sptr, int>(
							       dc_cancel.at(i), 0));
			} else {
				fft_channels.push_back(QPair<gr::basic_block_sptr, int>(
							       adc_samp_conv_block, i));
			}

			iio->connect(fft_channels.at(i).first, fft_channels.at(i).second,
					fft_blocks.at(i), 0);
			iio->connect(fft_blocks.at(i), 0, ctm_blocks.at(i), 0);
			iio->connect(ctm_blocks.at(i), 0, qt_fft_block, i);
		}

		if(prefPanel->getCurrent_docking_enabled()) {
				fftDockWidget->show();
		} else {
				ui->container_fft_plot->show();
		}
	} else {

		if(prefPanel->getCurrent_docking_enabled()) {
				fftDockWidget->setFloating(false);
				fftDockWidget->hide();
		} else {
				ui->container_fft_plot->hide();
		}

		if (fft_is_visible && (fft_channels.size() > 0)) {
			for (unsigned int i = 0; i < nb_channels; i++) {
				iio->disconnect(fft_channels.at(i).first, fft_channels.at(i).second,
						fft_blocks.at(i), 0);
				iio->disconnect(fft_blocks.at(i), 0, ctm_blocks.at(i), 0);
				iio->disconnect(ctm_blocks.at(i), 0, qt_fft_block, i);
			}
			fft_channels.clear();
			fft_blocks.clear();
			ctm_blocks.clear();
		}
	}

	fft_is_visible = visible;

	if (started)
		iio->unlock();
}

void Oscilloscope::onHistogram_view_toggled(bool visible)
{
	cancelZoom();

	if(!visible) {
		if(prefPanel->getCurrent_docking_enabled()) {
			histDockWidget->setFloating(false);
			histDockWidget->hide();
			histWidget->layout()->removeWidget(&hist_plot);
		} else {
			ui->hist_layout->removeWidget(&hist_plot);
		}

		gridPlot->addWidget(&hist_plot, 3, 2, 1, 1);
		gridPlot->addWidget(plot.rightHandlesArea(), 1, 3, 4, 1);

		hist_plot.setOrientation(Qt::Horizontal);
		hist_plot.setMinimumWidth(25);
		hist_plot.setMaximumWidth(25);
		hist_plot.setMaximumHeight(1000);
		plot.setBonusWidthForHistogram(25);
		scaleHistogramPlot();
		toggleMiniHistogramPlotVisible(miniHistogram);

	} else {
		gridPlot->removeWidget(&hist_plot);
		gridPlot->addWidget(plot.rightHandlesArea(), 1, 2, 3, 1);

		if(prefPanel->getCurrent_docking_enabled()) {
			histWidget->layout()->addWidget(&hist_plot);
			histDockWidget->show();
		} else {
			ui->hist_layout->addWidget(&hist_plot);
		}

		hist_plot.setOrientation(Qt::Vertical);
		hist_plot.setMinimumWidth(plot.width());
		hist_plot.setMaximumHeight(300);
		hist_plot.setMaximumWidth(2000);
		plot.setBonusWidthForHistogram(0);
		scaleHistogramPlot();
		hist_plot.setVisible(true);
	}
	hist_is_visible = visible;
}

void Oscilloscope::onXY_view_toggled(bool visible)
{
	if (visible) {
		gsettings_ui->xySettings->show();
	} else {
		if(prefPanel->getCurrent_docking_enabled()) {
			xyDockWidget->setFloating(false);
		}
		gsettings_ui->xySettings->hide();
	}

	/* Lock the flowgraph if we are already started */
	bool started = isIioManagerStarted();
	if (started && !locked)
		iio->lock();

	if (visible) {
		if(xy_is_visible && index_x == gsettings_ui->cmb_x_channel->currentIndex()
				&& index_y == gsettings_ui->cmb_y_channel->currentIndex())
		{
			xy_is_visible = visible;
			if (started && !locked)
				iio->unlock();
			return;
		}

		boost::shared_ptr<adc_sample_conv> block =
		dynamic_pointer_cast<adc_sample_conv>(
						adc_samp_conv_block);

		if(!ftc)
			ftc = blocks::float_to_complex::make(1);

		if(xy_channels.size() > 0) {
			iio->disconnect(xy_channels.at(index_x).first,
					xy_channels.at(index_x).second,
					ftc, 0);
			iio->disconnect(xy_channels.at(index_y).first,
					xy_channels.at(index_y).second,
					ftc, 1);
			xy_channels.clear();
		}

		index_x = gsettings_ui->cmb_x_channel->currentIndex();
		index_y = gsettings_ui->cmb_y_channel->currentIndex();

		if(xy_channels.size() == 0) {
			for(unsigned int i = 0; i < nb_channels; i++) {
				if (chnAcCoupled.at(i)) {
					xy_channels.push_back(QPair<gr::basic_block_sptr, int>(
								      dc_cancel.at(i), 0));
				} else {
					xy_channels.push_back(QPair<gr::basic_block_sptr, int>(
								      adc_samp_conv_block, i));
				}
			}
			for(const auto &p : qAsConst(math_sinks)) {
				auto math = p.first;
				xy_channels.push_back(QPair<gr::basic_block_sptr, int>(
							      math, 0));
			}
		}

		iio->connect(xy_channels.at(index_x).first, xy_channels.at(index_x).second,
			     ftc, 0);
		iio->connect(xy_channels.at(index_y).first, xy_channels.at(index_y).second,
			     ftc, 1);

		if(!xy_is_visible)
			iio->connect(ftc, 0, this->qt_xy_block, 0);

		if(prefPanel->getCurrent_docking_enabled()) {
			xyDockWidget->show();
		} else {
			ui->xy_plot_container->show();
		}
	} else {
		if(prefPanel->getCurrent_docking_enabled()) {
			xyDockWidget->setFloating(false);
			xyDockWidget->hide();
		} else {
			ui->xy_plot_container->hide();
		}

		// Disconnect the XY section from the running flowgraph

		iio->disconnect(xy_channels.at(index_x).first, xy_channels.at(index_x).second,
				ftc, 0);
		iio->disconnect(xy_channels.at(index_y).first, xy_channels.at(index_y).second,
				ftc, 1);

		iio->disconnect(ftc, 0, this->qt_xy_block, 0);

		xy_channels.clear();
	}

	xy_is_visible = visible;

	if (started && !locked)
		iio->unlock();
}

void adiscope::Oscilloscope::on_boxCursors_toggled(bool on)
{
	plot.setHorizCursorsEnabled(
			on ? cr_ui->vCursorsEnable->isChecked() : false);
	plot.setVertCursorsEnabled(
			on ? cr_ui->hCursorsEnable->isChecked() : false);

	plot.trackModeEnabled(on ? cr_ui->btnNormalTrack->isChecked() : true);

	// Set the visibility of the cursor readouts owned by the Oscilloscope
	if (on) {
		plot.setCursorReadoutsVisible(!ui->boxMeasure->isChecked());
	} else {
		if (ui->btnCursors->isChecked())
			ui->btnCursors->setChecked(false);

		menuOrder.removeOne(ui->btnCursors);
	}

	measure_panel_ui->cursorReadouts->setVisible(on);
}

void adiscope::Oscilloscope::on_boxMeasure_toggled(bool on)
{
	if (on) {
		update_measure_for_channel(current_channel);
	} else {
		if (ui->btnMeasure->isChecked())
			ui->btnMeasure->setChecked(false);

		menuOrder.removeOne(ui->btnMeasure);
	}

	measurePanel->setVisible(on);
	statisticsPanel->setVisible(on && statistics_enabled);

	// Set the visibility of the cursor readouts owned by the plot
	if (ui->boxCursors->isChecked())
		plot.setCursorReadoutsVisible(!on);
	// set the gates enable / disable
	plot.setGatingEnabled(on && gatingEnabled);
	buffer_previewer->setGatingEnabled(on && gatingEnabled);

}

void Oscilloscope::onTriggerSourceChanged(int chnIdx)
{
	plot.levelTriggerA()->setMobileAxis(QwtAxisId(QwtAxis::YLeft, chnIdx));
	trigger_settings.setChannelAttenuation(probe_attenuation[chnIdx]);
	if (chnAcCoupled.at(chnIdx)) {
		deactivateAcCouplingTrigger();
		activateAcCouplingTrigger(chnIdx);
	}
	plot.replot();

}

void Oscilloscope::onTimeTriggerDelayChanged(double value)
{
	if (timePosition->value() != value)
		Q_EMIT triggerPositionChanged(value);
}

void Oscilloscope::onTriggerLevelChanged(double value)
{
	int trigger_chn = trigger_settings.currentChannel();

	if (trigger_chn > -1)
		plot.setPeriodDetectLevel(trigger_chn, value);
}

void Oscilloscope::comboBoxUpdateToValue(QComboBox *box, double value, std::vector<double>list)
{
	int i = find_if( list.begin(), list.end(),
				[&value](const double element) {return element == value;} ) - list.begin();
	if (i < list.size())
		box->setCurrentIndex(i);
}

void adiscope::Oscilloscope::updateRunButton(bool ch_enabled)
{
	for (unsigned int i = 0; !ch_enabled &&
	     i < nb_channels + nb_math_channels + nb_ref_channels; i++) {
		if (channelWidgetAtId(i)->isReferenceChannel()) continue;
		QWidget *parent = ui->channelsList->itemAt(i)->widget();
		QCheckBox *box = parent->findChild<QCheckBox *>("box");
		ch_enabled = box->isChecked();
	}

	ui->runSingleWidget->setEnabled(ch_enabled);
	run_button->setEnabled(ch_enabled);
	setDynamicProperty(run_button, "disabled", !ch_enabled);

	if (!ch_enabled) {
		run_button->setChecked(false);
		ui->runSingleWidget->toggle(false);
	}
}

void adiscope::Oscilloscope::onChannelWidgetEnabled(bool en)
{
	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	int id = w->id();

	hist_plot.enableChannel(id, en);

	if (id < nb_channels) {
		runInHwThreadPool( m_m2k_analogin->enableChannel(id, en););
	}

	if (en) {
		plot.AttachCurve(id);
		fft_plot.AttachCurve(id);
		plot.showYAxisWidget(id, en);
		bool shouldActivate = true;

		for (unsigned int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels;
		     i++) {
			ChannelWidget *cw = static_cast<ChannelWidget *>(
			                            ui->channelsList->itemAt(i)->widget());

			if (id == cw->id()) {
				continue;
			}

			if (cw->enableButton()->isChecked()) {
				shouldActivate = false;
			}
		}

		if (shouldActivate) {
			plot.setActiveVertAxis(id);
			Q_EMIT selectedChannelChanged(id);
			update_measure_for_channel(id);
			measure_settings->activateDisplayAll();
		}

	} else {
		plot.DetachCurve(id);
		fft_plot.DetachCurve(id);
		plot.showYAxisWidget(id, en);
		bool shouldDisable = true;

		for (unsigned int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels;
		     i++) {
			ChannelWidget *cw = static_cast<ChannelWidget *>(
			                            ui->channelsList->itemAt(i)->widget());

			if (id == cw->id()) {
				continue;
			}

			if (cw->enableButton()->isChecked()) {
				shouldDisable = false;
			}
		}

		if (shouldDisable) {
			measure_settings->disableDisplayAll();
			plot.setActiveVertAxis(0);
			enableLabels(false);
		}

		if (current_channel == id) {
			for (int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels; i++) {
				ChannelWidget *cw = static_cast<ChannelWidget *>(
				                            ui->channelsList->itemAt(i)->widget());

				if (cw->enableButton()->isChecked()) {
					cw->nameButton()->setChecked(true);
					break;
				}
			}
		}

		menuOrder.removeOne(static_cast<CustomPushButton *>(
			w->menuButton()));
	}

	measureLabelsRearrange();

	plot.setOffsetWidgetVisible(id, en);

	plot.replot();
	fft_plot.replot();
	if (!w->isReferenceChannel()) {
		updateRunButton(en);
	}
}

void adiscope::Oscilloscope::onChannelWidgetSelected(bool checked)
{
	if (!checked) {
		return;
	}

	if (isVisible() && plot.labelsEnabled() != prefPanel->getOsc_labels_enabled()) {
		enableLabels(prefPanel->getOsc_labels_enabled());
	}

	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	int id = w->id();

	if (id != current_channel) {
		current_channel = id;
		Q_EMIT selectedChannelChanged(id);
		plot.bringCurveToFront(id);
		plot.setActiveVertAxis(id);
		plot.setDisplayScale(probe_attenuation[id]);
	}

	if (plot.measurementsEnabled()) {
		update_measure_for_channel(id);
	}
}

void adiscope::Oscilloscope::onChannelWidgetMenuToggled(bool checked)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());

	current_ch_widget = cw->id();

	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(cw->menuButton()), checked);
}

void Oscilloscope::cancelZoom()
{
	zoom_level = 0;
	plot.cancelZoom();

	for (int i = 0; i < nb_channels + nb_math_channels + nb_ref_channels; ++i) {
		QLabel *label = static_cast<QLabel *>(
					ui->chn_scales->itemAt(i)->widget());
		double value = probe_attenuation[i] * plot.VertUnitsPerDiv(i);
		QString str = vertMeasureFormat.format(value, "V/div", 3);
		str.append(getChannelRangeStringVDivHelper(i));
		label->setText(str);

	}
}

void adiscope::Oscilloscope::onChannelCouplingChanged(bool en)
{
	if (en && chnAcCoupled.at(current_ch_widget))
		return;
	configureAcCoupling(current_ch_widget, en);
}

QString adiscope::Oscilloscope::getChannelRangeStringVDivHelper(int ch)
{
	QString str ="";
	if(ch >= 0 && ch < nb_channels)
	{
		str = " (±";
		double range = (high_gain_modes[ch])? 2.5 : 25;
		range = range * probe_attenuation[ch];
		str += QString::number(range,'f',1) + ")";
	}
	return str;
}

void adiscope::Oscilloscope::onVertScaleValueChanged(double value)
{
	cancelZoom();
	if (value != plot.VertUnitsPerDiv(current_ch_widget)) {
		plot.setVertUnitsPerDiv(value, current_ch_widget);
	}
	voltsPosition->setStep(value / 10);

	scaleHistogramPlot();
	updateXyPlotScales();

	// TO DO: refactor this once the source of the X and Y axes can be configured
	if (current_ch_widget == index_x) {
		xy_plot.setHorizUnitsPerDiv(value);
	}
	if (current_ch_widget == index_y) {
		xy_plot.setVertUnitsPerDiv(value, QwtAxis::YLeft);
	}
	xy_plot.zoomBaseUpdate();

	if (current_ch_widget < nb_channels) {
		trigger_settings.setTriggerLevelStep(current_ch_widget, value);
		trigger_settings.setTriggerHystStep(current_ch_widget, value / 10);
	}

	// Send scale information to the measure object
	plot.setPeriodDetectHyst(current_ch_widget, value / 5);

	QLabel *label = static_cast<QLabel *>(
			ui->chn_scales->itemAt(current_ch_widget)->widget());
	double labelValue = probe_attenuation[current_ch_widget]
			* plot.VertUnitsPerDiv(current_ch_widget);
	QString str = vertMeasureFormat.format(labelValue, "V/div", 3);

	// Switch between high and low gain modes only for the M2K channels
	if (m_m2k_analogin && current_ch_widget < nb_channels) {
		updateGainMode();
		setChannelHwOffset(current_ch_widget, voltsPosition->value());
		trigger_settings.updateHwVoltLevels(current_ch_widget);
	}
	str.append(getChannelRangeStringVDivHelper(current_ch_widget));
	label->setText(str);
}


void Oscilloscope::onCmbMemoryDepthChanged(QString value)
{
	bool ok, started;
	unsigned long bufferSize = value.toInt(&ok);
	if (!ok) {
		ch_ui->cmbMemoryDepth->setCurrentIndex(0);
		return;
	}

	if (ch_ui->cmbMemoryDepth->currentIndex() == 0) {
		onHorizScaleValueChanged(timeBase->value());
		return;
	}

	started = isIioManagerStarted();
	if (started) {
		toggle_blockchain_flow(false);
	}

	symmBufferMode->setCustomBufferSize(bufferSize);
	SymmetricBufferMode::capture_parameters params = symmBufferMode->captureParameters();
	memory_adjusted_time_pos = params.timePos;
	timePosition->setValue(params.timePos);
	active_time_pos = params.timePos;
	active_sample_rate = params.sampleRate;
	active_plot_sample_count = params.entireBufferSize;
	active_sample_count = params.maxBufferSize;
	active_trig_sample_count = -(long long)params.triggerBufferSize;
	plot_samples_sequentially = false;

	if (timeBase->value() < TIMEBASE_THRESHOLD) {
		active_sample_count = active_plot_sample_count;
		setSinksDisplayOneBuffer(true);
	} else  {
		setSinksDisplayOneBuffer(false);
	}

	plot.setXAxisNumPoints(bufferSize);
	plot.setHorizOffset(params.timePos);
	plot.setDataStartingPoint(active_trig_sample_count);
	plot.resetXaxisOnNextReceivedData();
	plot.cancelZoom();

	if (zoom_level == 0) {
		noZoomXAxisWidth = plot.axisInterval(QwtAxis::XBottom).width();
	}

	setFFT_params(true);
	setAllSinksSampleCount(active_plot_sample_count);

	plot.setSampleRate(active_sample_rate, 1, "");
	plot.setBufferSizeLabelValue(active_plot_sample_count);
	plot.setSampleRatelabelValue(active_sample_rate);

	setSampleRate(active_sample_rate);
	trigger_settings.setTriggerDelay(active_trig_sample_count);
	last_set_time_pos = active_time_pos;
	last_set_sample_count = active_plot_sample_count;

	for (unsigned int i = 0; i < nb_channels; i++) {
		iio->set_buffer_size(ids[i], active_sample_count);
		dc_cancel.at(i)->set_buffer_size(active_sample_count);
	}
	if (mixed_source) {
		mixed_source->set_buffer_size(active_sample_count);
		setDigitalPlotCurvesParams();
	}

	for(int ch = 0; ch < nb_channels; ch++) {
		configureAcCoupling(ch, chnAcCoupled.at(ch));
	}

	// Compute the appropriate value for fft_size
	double power;
	if (symmBufferMode->isEnhancedMemDepth() || plot_samples_sequentially) {
		power = floor(log2(active_plot_sample_count));
	} else {
		power = ceil(log2(active_plot_sample_count));
	}
	fft_plot_size = pow(2, power);
	fft_size = active_sample_count;
	onFFT_view_toggled(fft_is_visible);

	updateBufferPreviewer();
	if (started) {
		writeAllSettingsToHardware();
		toggle_blockchain_flow(true);
	}
	resetStreamingFlag(true);
}

void Oscilloscope::setSinksDisplayOneBuffer(bool val)
{
	d_displayOneBuffer = val;
	qt_time_block->set_displayOneBuffer(val);

	if (mixed_sink) {
		mixed_sink->set_displayOneBuffer(val);
	}

	auto it = math_sinks.constBegin();
	while (it != math_sinks.constEnd()) {
		scope_sink_f::sptr math_sink = dynamic_pointer_cast<
				scope_sink_f>(it.value().second);
		math_sink->set_displayOneBuffer(val);
		++it;
	}
	qt_fft_block->set_displayOneBuffer(val);
}

void adiscope::Oscilloscope::onHorizScaleValueChanged(double value)
{
	cancelZoom();
	symmBufferMode->setTimeBase(value);
	SymmetricBufferMode::capture_parameters params = symmBufferMode->captureParameters();
	active_sample_rate = params.sampleRate;
	active_sample_count = params.entireBufferSize;
	active_plot_sample_count = active_sample_count;
	active_trig_sample_count = -(long long)params.triggerBufferSize;
	active_time_pos = -params.timePos;
	memory_adjusted_time_pos = -params.timePos;

	ch_ui->cmbMemoryDepth->blockSignals(true);
	ch_ui->cmbMemoryDepth->clear();
	for (auto item : params.availableBufferSizes) {
		ch_ui->cmbMemoryDepth->addItem(QString::number(item));
	}
	ch_ui->cmbMemoryDepth->blockSignals(false);

	// Realign plot data based on the new sample count and trigger position
	plot.setHorizUnitsPerDiv(value);
	plot.replot();
	plot.setDataStartingPoint(active_trig_sample_count);
	plot.resetXaxisOnNextReceivedData();
	plot.setXAxisNumPoints(0);

	hist_plot.replot();

	ch_ui->cmbMemoryDepth->setCurrentIndex(0);
	if (timeBase->value() >= TIMEBASE_THRESHOLD) {
		plot_samples_sequentially = true;
		if (active_sample_count != -active_trig_sample_count) {
			active_sample_count = ((-active_trig_sample_count + 3) / 4) * 4;
			setSinksDisplayOneBuffer(false);
			plot.setXAxisNumPoints(active_plot_sample_count);
			resetStreamingFlag(true);
		}
	} else {
		plot_samples_sequentially = false;
		setSinksDisplayOneBuffer(true);
		plot.setXAxisNumPoints(0);
		resetStreamingFlag(false);
	}

	if (zoom_level == 0) {
		noZoomXAxisWidth = plot.axisInterval(QwtAxis::XBottom).width();
	}

	/* Reconfigure the GNU Radio block to receive a different number of samples  */
	bool started = isIioManagerStarted();
	if (started)
		iio->lock();
	setAllSinksSampleCount(active_plot_sample_count);

	if (started) {
		plot.setSampleRate(active_sample_rate, 1, "");
		plot.setBufferSizeLabelValue(active_plot_sample_count);
		plot.setSampleRatelabelValue(active_sample_rate);
		last_set_sample_count = active_plot_sample_count;

		setSampleRate(active_sample_rate);
		for(auto i=0;i<nb_channels;i++)
		{
			iio->freq_comp_filt[i][0]->set_sample_rate(active_sample_rate);
			iio->freq_comp_filt[i][1]->set_sample_rate(active_sample_rate);
		}
		trigger_settings.setTriggerDelay(active_trig_sample_count);
		last_set_time_pos = active_time_pos;

		// Time base changes can limit the time position value
		if (timePosition->value() != -params.timePos)
			timePosition->setValue(-params.timePos);
	}

	for (unsigned int i = 0; i < nb_channels; i++) {
		iio->set_buffer_size(ids[i], active_sample_count);
		dc_cancel.at(i)->set_buffer_size(active_sample_count);
	}

	if (mixed_source) {
		mixed_source->set_buffer_size(active_sample_count);
		setDigitalPlotCurvesParams();
	}


	iio->set_device_timeout((active_sample_count / active_sample_rate) * 1000 + 100);


	if (started) {
		iio->unlock();
	}

	for(int ch = 0; ch < nb_channels; ch++) {
		configureAcCoupling(ch, chnAcCoupled.at(ch));
	}


	// Compute the appropriate value for fft_size
	double power;
	if (symmBufferMode->isEnhancedMemDepth() || plot_samples_sequentially) {
		power = floor(log2(active_plot_sample_count));
	} else {
		power = ceil(log2(active_plot_sample_count));
	}
	fft_plot_size = pow(2, power);
	fft_size = active_sample_count;
	onFFT_view_toggled(fft_is_visible);

	// Change the sensitivity of time position control
	timePosition->setStep(value / 10);

	updateBufferPreviewer();
}

bool adiscope::Oscilloscope::gainUpdateNeeded()
{
	double offset = plot.VertOffset(current_ch_widget);
	QwtInterval hw_input_itv(-2.5 + offset, 2.5 + offset);
	QwtInterval plot_vert_itv = plot.axisScaleDiv(
		QwtAxisId(QwtAxis::YLeft, current_ch_widget)).interval();

	if (plot_vert_itv.minValue() < hw_input_itv.minValue() ||
			plot_vert_itv.maxValue() > hw_input_itv.maxValue() ||
			std::abs(offset) > 5.0) {
		if (high_gain_modes[current_ch_widget]) {
			return true;
		}
		return false;
	} else if (!high_gain_modes[current_ch_widget]) {
		return true;
	}

	return false;

}

void Oscilloscope::updateXyPlotScales()
{
	int x = gsettings_ui->cmb_x_channel->currentIndex();
	int y = gsettings_ui->cmb_y_channel->currentIndex();
	auto xInterval = plot.axisInterval(QwtAxisId(QwtAxis::YLeft, x));
	auto yInterval = plot.axisInterval(QwtAxisId(QwtAxis::YLeft, y));
	xy_plot.set_axis(xInterval.minValue(), xInterval.maxValue(),
			 yInterval.minValue(), yInterval.maxValue());
}

void adiscope::Oscilloscope::onVertOffsetValueChanged(double value)
{
	cancelZoom();

	if (value != -plot.VertOffset(current_ch_widget)) {
		plot.setVertOffset(-value, current_ch_widget);
	}

	scaleHistogramPlot();
	updateXyPlotScales();

	// Switch between high and low gain modes only for the M2K channels
	if (m_m2k_analogin && current_ch_widget < nb_channels) {
		if (gainUpdateNeeded()) {
			updateGainMode();
		}
		setChannelHwOffset(current_ch_widget, value);

		trigger_settings.updateHwVoltLevels(current_ch_widget);
	}
}

void adiscope::Oscilloscope::onTimePositionChanged(double value)
{
	cancelZoom();

	bool started = isIioManagerStarted();
	bool enhancedMemDepth = symmBufferMode->isEnhancedMemDepth();

	unsigned long oldSampleCount = symmBufferMode->captureParameters().entireBufferSize;
	symmBufferMode->setTriggerPos(-value);

	/* If the time position changes during enhanced memory depth mode,
	 * the default settings are restored. (No enhanced memory depth, buffer size
	 * is computed again, based on the current time trigger position.)
	 * If the current timebase is greater than a threshold, the buffer size needs to
	 * be computed again, in order to properly display samples.
	 */
	if ((enhancedMemDepth || (timeBase->value() >= TIMEBASE_THRESHOLD))
			&& (value != memory_adjusted_time_pos)) {
		onHorizScaleValueChanged(timeBase->value());
		if (enhancedMemDepth) {
			return;
		}
	}

	SymmetricBufferMode::capture_parameters params = symmBufferMode->captureParameters();
	active_sample_rate = params.sampleRate;
	active_sample_count = params.entireBufferSize;
	active_plot_sample_count = active_sample_count;
	active_trig_sample_count = -(long long)params.triggerBufferSize;
	active_time_pos = -params.timePos;
	memory_adjusted_time_pos = -params.timePos;

	// Realign plot data based on the new time position
	plot.setHorizOffset(value);
	time_trigger_offset = value;

	plot.setXAxisNumPoints(plot_samples_sequentially ? active_plot_sample_count : 0);
	plot.realignReferenceWaveforms(timeBase->value(), timePosition->value());
	plot.replot();
	plot.setDataStartingPoint(active_trig_sample_count);
	plot.resetXaxisOnNextReceivedData();


	if (started) {
		trigger_settings.setTriggerDelay(active_trig_sample_count);
		last_set_time_pos = active_time_pos;

		if (mixed_source) {
			setDigitalPlotCurvesParams();
		}
	}
	updateBufferPreviewer();
	if (reset_horiz_offset) {
		horiz_offset = value;
	}

	if (active_sample_rate == getSampleRate() &&
			(active_plot_sample_count == oldSampleCount)) {
		return;
	}

	/* Reconfigure the GNU Radio block to receive a different number of samples  */
	auto chnCoupled = chnAcCoupled;
	for(int ch = 0; ch < nb_channels; ch++) {
		if (chnAcCoupled.at(ch)) {
			configureAcCoupling(ch, !chnAcCoupled.at(ch));
		}
	}

	if (started)
		iio->lock();
	setAllSinksSampleCount(active_plot_sample_count);

	if (started) {
		plot.setSampleRate(active_sample_rate, 1, "");
		plot.setBufferSizeLabelValue(active_plot_sample_count);
		plot.setSampleRatelabelValue(active_sample_rate);

		last_set_sample_count = active_plot_sample_count;

		setSampleRate(active_sample_rate);
		for(auto i=0;i<nb_channels;i++)
		{
			iio->freq_comp_filt[i][0]->set_sample_rate(active_sample_rate);
			iio->freq_comp_filt[i][1]->set_sample_rate(active_sample_rate);
		}
	}

	for (unsigned int i = 0; i < nb_channels; i++) {
		iio->set_buffer_size(ids[i], active_sample_count);
		dc_cancel.at(i)->set_buffer_size(active_sample_count);
	}
	if (mixed_source) {
		mixed_source->set_buffer_size(active_sample_count);
		setDigitalPlotCurvesParams();
	}

	if (started) {
		iio->unlock();
	}

	for(int ch = 0; ch < nb_channels; ch++) {
		if (chnCoupled.at(ch)) {
			configureAcCoupling(ch, chnCoupled.at(ch));
		}
	}

	// Compute the appropriate value for fft_size
	double power;
	if (symmBufferMode->isEnhancedMemDepth() || plot_samples_sequentially) {
		power = floor(log2(active_plot_sample_count));
	} else {
		power = ceil(log2(active_plot_sample_count));
	}
	fft_plot_size = pow(2, power);
	fft_size = active_sample_count;
	onFFT_view_toggled(fft_is_visible);
}

void adiscope::Oscilloscope::rightMenuFinished(bool opened)
{
	Q_UNUSED(opened)

	// At the end of each animation, check if there are other button check
	// actions that might have happened while animating and execute all
	// these queued actions
	while (menuButtonActions.size()) {
		auto pair = menuButtonActions.dequeue();
		toggleRightMenu(pair.first, pair.second);
	}
}

void adiscope::Oscilloscope::toggleRightMenu(CustomPushButton *btn, bool checked)
{
	int id = btn->property("id").toInt();

	if (id != -ui->stackedWidget->indexOf(ui->generalSettings)){
		if (!menuOrder.contains(btn)){
			menuOrder.push_back(btn);
		} else {
			menuOrder.removeOne(btn);
			menuOrder.push_back(btn);
		}
	}

	if (checked)
		settings_panel_update(id);

	if (id >= 0) {
		if (checked) {
			update_chn_settings_panel(id);
		}
	}

	// Update Settings button state
	if(!ui->btnGeneralSettings->isChecked())
		ui->btnSettings->setChecked(!!ui->settings_group->checkedButton());

	ui->rightMenu->toggleMenu(checked);
}

void Oscilloscope::triggerRightMenuToggle(CustomPushButton *btn, bool checked)
{
	// Queue the action, if right menu animation is in progress. This way
	// the action will be remembered and performed right after the animation
	// finishes
	if (ui->rightMenu->animInProgress()) {
		menuButtonActions.enqueue(
			QPair<CustomPushButton *, bool>(btn, checked));
	} else {
		toggleRightMenu(btn, checked);
	}
}

void Oscilloscope::settings_panel_update(int id)
{
        if (id >= 0)
        ui->stackedWidget->setCurrentIndex(0);
        else
        ui->stackedWidget->setCurrentIndex(-id);

        settings_panel_size_adjust();
}

void Oscilloscope::settings_panel_size_adjust()
{
	for (int i = 0; i < ui->stackedWidget->count(); i++) {
		QSizePolicy::Policy policy = QSizePolicy::Ignored;

		if (i == ui->stackedWidget->currentIndex()) {
			policy = QSizePolicy::Expanding;
		}
		QWidget *widget = ui->stackedWidget->widget(i);
		widget->setSizePolicy(policy, policy);
	}
	ui->stackedWidget->adjustSize();
}

void Oscilloscope::onChannelOffsetChanged(unsigned int chnIdx, double value)
{
	voltsPosition->setValue(-plot.VertOffset(current_ch_widget));

	scaleHistogramPlot();
	updateXyPlotScales();
}

ChannelWidget *Oscilloscope::channelWidgetAtId(int id)
{
	ChannelWidget *w = nullptr;
	bool found = false;

	for (unsigned int i = 0; !found &&
	     i < nb_channels + nb_math_channels + nb_ref_channels; i++) {

		w = static_cast<ChannelWidget *>(
		            ui->channelsList->itemAt(i)->widget());
		found = w->id() == id;
	}

	return w;
}

void Oscilloscope::update_chn_settings_panel(int id)
{
	ChannelWidget *chn_widget = channelWidgetAtId(id);
	if (!chn_widget)
		return;

	disconnect(voltsPerDiv, SIGNAL(valueChanged(double)), this,
		SLOT(onVertScaleValueChanged(double)));
	voltsPerDiv->setValue(plot.VertUnitsPerDiv(id));
	connect(voltsPerDiv, SIGNAL(valueChanged(double)),
		SLOT(onVertScaleValueChanged(double)));
	voltsPosition->blockSignals(true);
	voltsPosition->setValue(-plot.VertOffset(id));
	voltsPosition->blockSignals(false);

	QString name = chn_widget->fullName();
	ch_ui->label_channelName->setText(name);
	QString stylesheet = QString("border: 2px solid %1"
					).arg(plot.getLineColor(id).name());
	ch_ui->line_channelColor->setStyleSheet(stylesheet);
	int cmbIdx = (int)(plot.getLineWidthF(id) / 0.5) - 1;
	ch_ui->cmbChnLineWidth->setCurrentIndex(cmbIdx);

	if (chn_widget->isMathChannel()) {
		ch_ui->filter1->setVisible(false);
		ch_ui->filter2->setVisible(false);
		ch_ui->math_settings_widget->setVisible(true);
		ch_ui->btnAutoset->setVisible(false);
		ch_ui->function_2->setText(chn_widget->function());
		ch_ui->function_2->setText(chn_widget->function());
		ch_ui->wCoupling->setVisible(false);
		timePosition->setEnabled(true);

		ch_ui->snapshotBtn->setText(tr("Snapshot"));

	} else if (chn_widget->isReferenceChannel()) {
		ch_ui->filter1->setVisible(false);
		ch_ui->filter2->setVisible(false);

		ch_ui->snapshotBtn->setText(tr("Export"));

		timePosition->setEnabled(false);

		ch_ui->math_settings_widget->setVisible(false);
		ch_ui->wCoupling->setVisible(false);


	} else {
		ch_ui->filter1->setVisible(prefPanel->getShowADCFilters());
		ch_ui->filter2->setVisible(prefPanel->getShowADCFilters());
		ch_ui->math_settings_widget->setVisible(false);
		ch_ui->btnAutoset->setVisible(autosetEnabled);
		ch_ui->wCoupling->setVisible(true);

		if (ch_ui->btnCoupled->isChecked() != chnAcCoupled.at(id)) {
			ch_ui->btnCoupled->setChecked(chnAcCoupled.at(id));
		}

		timePosition->setEnabled(true);

		ch_ui->snapshotBtn->setText(tr("Snapshot"));
	}

	if (chn_widget->isMathChannel() || chn_widget->isReferenceChannel()) {
		ch_ui->probe_attenuation_value->setVisible(false);
		ch_ui->probe_label->setVisible(false);
		ch_ui->label_3->setVisible(false);
		ch_ui->cmbMemoryDepth->setVisible(false);
		ch_ui->btnAutoset->setVisible(false);
	} else {

		ch_ui->probe_attenuation_value->setText(QString::number(probe_attenuation[current_ch_widget]));
		ch_ui->probe_attenuation_value->setVisible(true);
		ch_ui->probe_label->setVisible(true);
		ch_ui->label_3->setVisible(true);
		ch_ui->cmbMemoryDepth->setVisible(true);
		ch_ui->btnAutoset->setVisible(true);
		ch_ui->filter_TC->setText(QString::number(iio->freq_comp_filt[current_ch_widget][0]->get_TC()));
		ch_ui->filter2_TC->setText(QString::number(iio->freq_comp_filt[current_ch_widget][1]->get_TC()));
		ch_ui->filter_gain->setText(QString::number(iio->freq_comp_filt[current_ch_widget][0]->get_filter_gain()));
		ch_ui->filter2_gain->setText(QString::number(iio->freq_comp_filt[current_ch_widget][1]->get_filter_gain()));
		ch_ui->filter_en->setText(tr("Filter 1 - Enable - ") + getChannelRangeStringVDivHelper(id));
		ch_ui->filter2_en->setText(tr("Filter 2 - Enable - ") + getChannelRangeStringVDivHelper(id));
		ch_ui->filter_en->setChecked(iio->freq_comp_filt[current_ch_widget][0]->get_enable());
		ch_ui->filter2_en->setChecked(iio->freq_comp_filt[current_ch_widget][1]->get_enable());
	}

	auto max_elem = max_element(probe_attenuation.begin(), probe_attenuation.begin() + nb_channels);
	if (chn_widget->isMathChannel()) {
		voltsPerDiv->silentSetMaxValue((*max_elem) * 10);
		voltsPosition->silentSetMaxValue((*max_elem) * MAX_AMPL);
		voltsPosition->silentSetMinValue((*max_elem) * (-MAX_AMPL));
	} else {
		voltsPerDiv->silentSetMaxValue(10);
		voltsPosition->silentSetMaxValue(MAX_AMPL);
		voltsPosition->silentSetMinValue(-MAX_AMPL);
	}

	voltsPerDiv->setDisplayScale(probe_attenuation[id]);
	voltsPosition->setDisplayScale(probe_attenuation[id]);

	ch_ui->lineStyle->setCurrentIndex(plot.getPlotLineStyle(id));
}

void Oscilloscope::openEditMathPanel(bool on)
{
	ChannelWidget *chn_widget = channelWidgetAtId(current_ch_widget);

	if (on) {
		addChannel = false;
		triggerRightMenuToggle(
			static_cast<CustomPushButton* >(chn_widget->menuButton()), false);
		math_pair->first.btnAddChannel->setText(tr("Save"));
		math_pair->second->setFunction(chn_widget->function());
		triggerRightMenuToggle(
			static_cast<CustomPushButton* >(ui->btnAddMath), true);

		tabWidget->removeTab(1);
		tabWidget->tabBar()->hide();
		math_pair->first.btnAddChannel->setText(tr("Save"));
	} else {
		tabWidget->addTab(ref, tr("Reference"));
		tabWidget->tabBar()->show();
	}
}

void Oscilloscope::editMathChannelFunction(int id, const std::string& new_function)
{
	ChannelWidget *chn_widget = channelWidgetAtId(id);
	int current_x = index_x;
	int current_y = index_y;

	triggerRightMenuToggle(
				static_cast<CustomPushButton* >(ui->btnAddMath), false);
	triggerRightMenuToggle(
				static_cast<CustomPushButton* >(chn_widget->menuButton()), true);
	math_pair->first.btnAddChannel->setText(tr("Add Channel"));
	math_pair->second->setFunction("");
	addChannel = true;
	ch_ui->btnEditMath->setChecked(false);

	if (chn_widget->function().toStdString() == new_function)
		return;

	QString qname = chn_widget->deleteButton()->property("curve_name").toString();
	std::string name = qname.toStdString();

	auto rail = gr::analog::rail_ff::make(MIN_MATH_RANGE, MAX_MATH_RANGE);
	auto math = gr::scopy::iio_math::make(new_function, nb_channels);

	bool started = isIioManagerStarted();
	if (started)
		iio->lock();
	locked = true;
	if(xy_is_visible) {
		gsettings_ui->cmb_x_channel->blockSignals(true);
		gsettings_ui->cmb_y_channel->blockSignals(true);
		if(index_x == id)
			gsettings_ui->cmb_x_channel->setCurrentIndex(id-1);
		if(index_y == id)
			gsettings_ui->cmb_y_channel->setCurrentIndex(id-1);
		gsettings_ui->cmb_x_channel->blockSignals(false);
		gsettings_ui->cmb_y_channel->blockSignals(false);
		setup_xy_channels();
	}
	auto pair = math_sinks.value(qname);
	auto rail_old = math_rails.value(qname);
	for (unsigned int i = 0; i < nb_channels; ++i) {
		iio->disconnect(math_probe_atten.at(i), 0, pair.first, i);
	}
	iio->disconnect(pair.first, 0, rail_old, 0);
	iio->disconnect(rail_old, 0, pair.second, 0);

	auto math_pair = QPair<gr::basic_block_sptr, gr::basic_block_sptr>(
				math, pair.second);

	math_sinks.insert(qname, math_pair);
	math_rails.insert(qname, rail);

	for (unsigned int i = 0; i < nb_channels; ++i) {
		iio->connect(math_probe_atten.at(i), 0, math, i);
	}
	iio->connect(math, 0, rail, 0);
	iio->connect(rail, 0, pair.second, 0);

	if(xy_is_visible) {
		gsettings_ui->cmb_x_channel->blockSignals(true);
		gsettings_ui->cmb_y_channel->blockSignals(true);
		gsettings_ui->cmb_x_channel->setCurrentIndex(current_x);
		gsettings_ui->cmb_y_channel->setCurrentIndex(current_y);
		gsettings_ui->cmb_x_channel->blockSignals(false);
		gsettings_ui->cmb_y_channel->blockSignals(false);
		setup_xy_channels();
	}
	locked = false;
	if (started)
		iio->unlock();

	// If the oscilloscope is not running, create a single run situation only for
	// the channel that is edited .
//	else {
//		QList<QString> keys = math_sinks.keys();
//		for (const QString &key : keys)
//			if (key != qname) {
//				pair = math_sinks.value(key);
//				for (unsigned int i = 0; i < nb_channels; ++i)
//					iio->disconnect(adc_samp_conv_block, i, pair.first, i);
//				iio->disconnect(pair.first, 0, pair.second, 0);
//			}

//		ui->pushButtonSingle->setChecked(true);

//		for (const QString &key : keys)
//			if (key != qname) {
//				pair = math_sinks.value(key);
//				for (unsigned int i = 0; i < nb_channels; ++i)
//					iio->connect(adc_samp_conv_block, i, pair.first, i);
//				iio->connect(pair.first, 0, pair.second, 0);
//			}
//	}

	chn_widget->setFunction(QString::fromStdString(new_function));
	setSinksDisplayOneBuffer(d_displayOneBuffer);
}

void Oscilloscope::onMeasuremetsAvailable()
{
	measureUpdateValues();

	if (statistics_enabled) {
		statisticsUpdateValues();
		statisticsUpdateGui();
	}
}

void Oscilloscope::update_measure_for_channel(int ch_idx)
{
	ChannelWidget *chn_widget = channelWidgetAtId(ch_idx);

	measure_settings->setChannelName(chn_widget->fullName());
	measure_settings->setChannelUnderlineColor(chn_widget->color());
}

void Oscilloscope::measureCreateAndAppendGuiFrom(const MeasurementData&
		measurement)
{
	std::shared_ptr<MeasurementGui> p;

	switch(measurement.unitType()) {

	case MeasurementData::METRIC:
		p = std::make_shared<MetricMeasurementGui>();
		break;
	case MeasurementData::TIME:
		p = std::make_shared<TimeMeasurementGui>();
		break;
	case MeasurementData::PERCENTAGE:
		p = std::make_shared<PercentageMeasurementGui>();
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

void Oscilloscope::measureLabelsRearrange()
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
		if (channel >= nb_channels + nb_math_channels + nb_ref_channels) {
			continue;
		}

		ChannelWidget *chn_widget = channelWidgetAtId(channel);
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
		if (channel < probe_attenuation.size()) {
			pb_atten = probe_attenuation[channel];
		}
		measurements_gui[i]->update(*(measurements_data[i]),
					    pb_atten);
		measurements_gui[i]->setLabelsColor(plot.getLineColor(channel));

		nb_meas_added++;
	}
}

void Oscilloscope::measureUpdateValues()
{

	for (int i = 0; i < measurements_data.size(); i++) {
		int channel = measurements_data[i]->channel();
		ChannelWidget *chn_widget = channelWidgetAtId(channel);
		if (!chn_widget->enableButton()->isChecked()) {
			continue;
		}
		measurements_gui[i]->update(*(measurements_data[i]),
					    probe_attenuation[channel]);
	}
}

void Oscilloscope::measure_settings_init()
{
	QList<Measure *>* measure_obj = plot.getMeasurements();
	measure_settings = new MeasureSettings(measure_obj, this);

	int measure_panel = ui->stackedWidget->insertWidget(-1, measure_settings);

	connect(measure_settings,
		SIGNAL(measurementActivated(int, int)),
		SLOT(onMeasurementActivated(int, int)));

	connect(measure_settings,
		SIGNAL(measurementDeactivated(int, int)),
		SLOT(onMeasurementDeactivated(int, int)));

	connect(measure_settings,
		SIGNAL(measurementSelectionListChanged()),
		SLOT(onMeasurementSelectionListChanged()));

	connect(measure_settings,
		SIGNAL(statisticActivated(int, int)),
		SLOT(onStatisticActivated(int, int)));
	connect(measure_settings,
		SIGNAL(statisticDeactivated(int, int)),
		SLOT(onStatisticDeactivated(int, int)));

	connect(measure_settings, SIGNAL(statisticsEnabled(bool)),
		SLOT(onStatisticsEnabled(bool)));

	connect(measure_settings, SIGNAL(statisticsReset()),
		SLOT(onStatisticsReset()));

	connect(measure_settings, SIGNAL(gatingEnabled(bool)),SLOT(onGatingEnabled(bool)));

	connect(&plot, SIGNAL(channelAdded(int)),
		measure_settings, SLOT(onChannelAdded(int)));

	connect(this, SIGNAL(selectedChannelChanged(int)),
		measure_settings, SLOT(setSelectedChannel(int)));

	connect(measure_settings,
		SIGNAL(statisticSelectionListChanged()),
		SLOT(onStatisticSelectionListChanged()));

	ui->btnMeasure->setProperty("id", QVariant(-measure_panel));

	connect(ui->boxMeasure, SIGNAL(toggled(bool)),
		&plot, SLOT(setMeasuremensEnabled(bool)));
	connect(ui->boxMeasure, &QCheckBox::toggled, [=](bool on){
		cr_ui->readoutsWidget->setVisible(!on);
	});
}

void Oscilloscope::onMeasurementActivated(int id, int chnIdx)
{
	int oldActiveMeasCount = plot.activeMeasurementsCount(chnIdx);

	auto mList = plot.measurements(chnIdx);
	mList[id]->setEnabled(true);
	measurements_data.push_back(mList[id]);
	measureCreateAndAppendGuiFrom(*mList[id]);

	// Even if a measurement had been added after data was captured, it
	// should display the measurement value corresponding to that data
	if (oldActiveMeasCount == 0) {
		plot.measure();
	}

	measureLabelsRearrange();
}

void Oscilloscope::onMeasurementDeactivated(int id, int chnIdx)
{
	auto mList = plot.measurements(chnIdx);
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

void Oscilloscope::onMeasurementSelectionListChanged()
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
		auto pMeasurement = plot.measurement(newList[i].id(),
			newList[i].channel_id());
		if (pMeasurement) {
			pMeasurement->setEnabled(true);
			measurements_data.push_back(pMeasurement);
			measureCreateAndAppendGuiFrom(*pMeasurement);
		}
	}
	measureLabelsRearrange();
}

void Oscilloscope::onCursorReadoutsChanged(struct cursorReadoutsText data)
{
	fillCursorReadouts(data);
}

void Oscilloscope::fillCursorReadouts(const struct cursorReadoutsText& data)
{
	cursor_readouts_ui->cursorT1->setText(data.t1);
	cursor_readouts_ui->cursorT2->setText(data.t2);
	cursor_readouts_ui->timeDelta->setText(data.tDelta);
	cursor_readouts_ui->frequencyDelta->setText(data.freq);
	cursor_readouts_ui->cursorV1->setText(data.v1);
	cursor_readouts_ui->cursorV2->setText(data.v2);
	cursor_readouts_ui->voltageDelta->setText(data.vDelta);
}

void Oscilloscope::measure_panel_init()
{
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

	connect(&plot, SIGNAL(measurementsAvailable()),
		SLOT(onMeasuremetsAvailable()));

	// The second CursorReadouts belongs to the Measure panel. The first
	// one is drawn on top of the plot canvas.
	cursorReadouts = new QWidget(measure_panel_ui->cursorReadouts);
	cursor_readouts_ui = new Ui::CursorReadouts();
	cursor_readouts_ui->setupUi(cursorReadouts);

	cursor_readouts_ui->TimeCursors->setStyleSheet("QWidget {"
		"background-color: transparent;}"
						       );
	cursor_readouts_ui->VoltageCursors->setStyleSheet("QWidget {"
		"background-color: transparent;}"
							  );

	// Avoid labels jumping around to left or right by imposing a min width
	QLabel *label = new QLabel(this);
	label->setStyleSheet("font-size: 14px");
	label->setText("-999.999 ns");
	double minWidth = label->minimumSizeHint().width();
	cursor_readouts_ui->cursorT1->setMinimumWidth(minWidth);
	cursor_readouts_ui->cursorT2->setMinimumWidth(minWidth);
	cursor_readouts_ui->timeDelta->setMinimumWidth(minWidth);
	label->setText("-999.999 MHz");
	minWidth = label->minimumSizeHint().width();
	cursor_readouts_ui->frequencyDelta->setMinimumWidth(minWidth);
	label->setText("-999.999 mV");
	minWidth = label->minimumSizeHint().width();
	cursor_readouts_ui->cursorV1->setMinimumWidth(minWidth);
	cursor_readouts_ui->cursorV2->setMinimumWidth(minWidth);
	cursor_readouts_ui->voltageDelta->setMinimumWidth(minWidth);
	measure_panel_ui->scrollArea->setMinimumHeight(label->height() * 4);

	delete label;

	QHBoxLayout *hLayout = static_cast<QHBoxLayout *>(
		measure_panel_ui->cursorReadouts->layout());
	if (hLayout)
		hLayout->insertWidget(0, cursorReadouts);

	fillCursorReadouts(plot.allCursorReadouts());
	measure_panel_ui->cursorReadouts->hide();
}

void Oscilloscope::statistics_panel_init()
{
	statisticsPanel = new QWidget(this);
	statistics_panel_ui = new Ui::StatisticsPanel();
	statistics_panel_ui->setupUi(statisticsPanel);
	QHBoxLayout *hLayout = new QHBoxLayout(statistics_panel_ui->statistics);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(25);

	// Make sure the scroll area knows beforehand the height of the content
	// that will be added to the statistics widget
	StatisticWidget *dummyStat = new StatisticWidget();
	statistics_panel_ui->statistics->setMinimumHeight(dummyStat->height());
	delete dummyStat;

	statisticsPanel->hide();
}

void Oscilloscope::statisticsUpdateValues()
{
	for (int i = 0; i < statistics_data.size(); i++) {
		double meas_data = statistics_data[i].first->value();
		statistics_data[i].second.pushNewData(meas_data);
	}
}

void Oscilloscope::statisticsReset()
{
	for (int i = 0; i < statistics_data.size(); i++)
		statistics_data[i].second.clear();
}

void Oscilloscope::statisticsUpdateGui()
{
	QList<StatisticWidget *>statistics = statistics_panel_ui->
		statistics->findChildren<StatisticWidget *>(
			QString("Statistic"), Qt::FindDirectChildrenOnly);
	for (int i = 0; i < statistics.size(); i++)
		statistics[i]->updateStatistics(statistics_data[i].second);

	if (!ui->boxMeasure->isChecked())
		statisticsPanel->hide();
}

void Oscilloscope::statisticsUpdateGuiTitleColor()
{
	QList<StatisticWidget *>statistics = statistics_panel_ui->
		statistics->findChildren<StatisticWidget *>(
			QString("Statistic"), Qt::FindDirectChildrenOnly);
	for (int i = 0; i < statistics.size(); i++)
		statistics[i]->setTitleColor(plot.getLineColor(
			statistics[i]->channelId()));
}

void Oscilloscope::statisticsUpdateGuiPosIndex()
{
	QList<StatisticWidget *>statistics = statistics_panel_ui->
		statistics->findChildren<StatisticWidget *>(
			QString("Statistic"), Qt::FindDirectChildrenOnly);
	for (int i = 0; i < statistics.size(); i++)
		statistics[i]->setPositionIndex(i + 1);
}

void Oscilloscope::onStatisticActivated(int id, int chnIdx)
{
	std::shared_ptr<MeasurementData> pmd = plot.measurement(id, chnIdx);
	if (!pmd)
		return;

	statistics_data.push_back(QPair<std::shared_ptr<MeasurementData>,
		Statistic>(pmd, Statistic()));

	/* Add a widget for the new statistic */
	QWidget *statisticContainer = statistics_panel_ui->statistics;
	QHBoxLayout *hLayout = static_cast<QHBoxLayout *>
		(statisticContainer->layout());

	StatisticWidget *statistic = new StatisticWidget(statisticContainer);
	statistic->initForMeasurement(*pmd);
	statistic->setTitleColor(plot.getLineColor(chnIdx));
	statistic->setPositionIndex(statistics_data.size());
	statistic->updateStatistics(statistics_data.last().second);
	hLayout->addWidget(statistic);
}

void Oscilloscope::onStatisticDeactivated(int id, int chnIdx)
{
	std::shared_ptr<MeasurementData> pmd = plot.measurement(id, chnIdx);
	if (!pmd)
		return;

	QString name = pmd->name();

	auto it = std::find_if(statistics_data.begin(), statistics_data.end(),
		[&](QPair<std::shared_ptr<MeasurementData>, Statistic> pair) {
			return pair.first->name() == name &&
				pair.first->channel() == chnIdx;
		});
	if (it != statistics_data.end()) {
		statistics_data.erase(it);
	}

	/* remove the widget corresponding to the statistic we deleted */
	QList<StatisticWidget *>statistics = statistics_panel_ui->statistics->
		findChildren<StatisticWidget *>(QString("Statistic"),
		Qt::FindDirectChildrenOnly);

	auto stat_it = std::find_if(statistics.begin(), statistics.end(),
		[=](StatisticWidget *statistic){
			return statistic->title() == name &&
				statistic->channelId() == chnIdx;
		});
	if (stat_it != statistics.end()) {
		QWidget *w = static_cast<QWidget *> (*stat_it);
		if (statistics_enabled) // Avoid flickers when panel is visible
			statisticsPanel->hide(); // Avoid flickers
		statistics_panel_ui->statistics->layout()->removeWidget(w);
		if (statistics_enabled) // Avoid flickers when panel is visible
			statisticsPanel->show();
		delete w;

		statisticsUpdateGuiPosIndex();
	}
}

void Oscilloscope::onStatisticSelectionListChanged()
{
	// Clear all statistics in list
	statistics_data.clear();

	QList<StatisticWidget *>statistics = statistics_panel_ui->statistics->
		findChildren<StatisticWidget *>(QString("Statistic"),
		Qt::FindDirectChildrenOnly);
	for (int i = 0; i < statistics.size(); i++) {
		statistics_panel_ui->statistics->layout()->removeWidget(
			statistics[i]);
		delete statistics[i];
	}

	// Use the new list from MeasureSettings
	auto newList = measure_settings->statisticSelection();
	for (int i = 0; i < newList.size(); i++)
		onStatisticActivated(newList[i].id(), newList[i].channel_id());
	statisticsUpdateGui();

}

void Oscilloscope::onStatisticsEnabled(bool on)
{
	statistics_enabled = on;
	statisticsPanel->setVisible(on);

	if (!on)
		statisticsReset();
	else
		statisticsUpdateGui();
}

void Oscilloscope::onStatisticsReset()
{
	statisticsReset();
	statisticsUpdateGui();
}

void Oscilloscope::onGatingEnabled(bool on){
//enable / disable gatings
	gatingEnabled = on;
	buffer_previewer->setGatingEnabled(on && ui->boxMeasure->isChecked());
	plot.setGatingEnabled(on && ui->boxMeasure->isChecked());
}

void Oscilloscope::onLeftGateChanged(double width)
{
	buffer_previewer->setLeftGateWidth(width);
}

void Oscilloscope::onRightGateChanged(double width)
{
	buffer_previewer->setRightGateWidth(width);
}

void Oscilloscope::setupAutosetFreqSweep()
{	bool started = isIioManagerStarted();
	if(started)		
		iio->lock();
	qt_time_block->reset();
	qt_time_block->clean_buffers();
	if (m_m2k_analogin) {
		runInHwThreadPool({
		high_gain_modes[autosetChannel] = libm2k::analog::PLUS_MINUS_25V;
		trigger_settings.autoTriggerDisable();
		trigger_settings.setTriggerEnable(false);
		});
	}
	autosetSampleRateCnt--;
	active_sample_rate = m_m2k_analogin->getAvailableSampleRates()[autosetSampleRateCnt];
	qt_time_block->set_samp_rate(active_sample_rate);
	active_sample_count = autosetFFTSize;
	setAllSinksSampleCount(active_sample_count);
	autoset_fft_size = active_sample_count;
	writeAllSettingsToHardware();
	last_set_sample_count = active_sample_count;
	autosetFFT();
	iio->set_buffer_size(autoset_id[0], autoset_fft_size);
	for (unsigned int i = 0; i < nb_channels; i++) {
		iio->set_buffer_size(ids[i], autoset_fft_size);
		dc_cancel.at(i)->set_buffer_size(active_sample_count);
	}
	if (mixed_source) {
		mixed_source->set_buffer_size(active_sample_count);
		setDigitalPlotCurvesParams();
	}
	if (keep_one) {
		iio->disconnect(dc_cancel.at(triggerLevelSink.second), 0, keep_one, 0);
		iio->disconnect(keep_one, 0, triggerLevelSink.first, 0);
		keep_one = gr::blocks::keep_one_in_n::make(sizeof(float), active_sample_count);
		iio->connect(dc_cancel.at(triggerLevelSink.second), 0, keep_one, 0);
		iio->connect(keep_one, 0, triggerLevelSink.first, 0);
	}
	if(started)
		iio->unlock();
}

bool Oscilloscope::autosetFindFrequency()
{

	toggle_blockchain_flow(false);
	double max=-INFINITY;
	size_t maxindex=1;
	// try skipping the DC offset tones and last few tones that cause
	// weird results

	for(int j=autosetNrOfSkippedTones ;j<((autoset_fft_size/2)-5);j++) {
		if(autosetFFTSink->data()[j] > max) {
			max = autosetFFTSink->data()[j];
			maxindex=j;
		}
	}

	double frequency = (maxindex) * (active_sample_rate/autoset_fft_size);
	autosetFFTIndex = maxindex;
	if(autosetMaxIndexAmpl < max /*&& (autosetFFTIndex > autosetValidTone)*/) {
		autosetFrequency = frequency;
		autosetMaxIndexAmpl = max;
		return true; // found valid frequency
	}
	return false;
}

void Oscilloscope::autosetFindPeaks()
{
		for(auto j=autosetSkippedTimeSamples;j<active_sample_count;j++) {
			auto data = plot.Curve(autosetChannel)->data()->sample(j).y();
			if(data > autosetMaxAmpl)
				autosetMaxAmpl = data;
			if(data < autosetMinAmpl)
				autosetMinAmpl = data;
		}
		qDebug()<<"autoset min-max" << autosetMinAmpl <<autosetMaxAmpl;
}



void Oscilloscope::requestAutoset()
{
	if(!autosetRequested && current_ch_widget != -1 && current_ch_widget < nb_channels){

		toggle_blockchain_flow(false);
		autosetChannel = current_ch_widget;
		autosetSampleRateCnt = m_m2k_analogin->getAvailableSampleRates().size();
		autosetRequested = true;
		autosetFFTIndex = 0;
		autosetMaxIndexAmpl = 0;
		setupAutosetFreqSweep();
		toggle_blockchain_flow(true);
	}
}

void Oscilloscope::periodicFlowRestart(bool force)
{
	static uint64_t restartFlowCounter = 0;
	const uint64_t NO_FLOW_BUFFERS = 1024;
	if(force) {
		restartFlowCounter = NO_FLOW_BUFFERS;
	}

	if(restartFlowCounter == 0) {
		restartFlowCounter = NO_FLOW_BUFFERS;
		QElapsedTimer t;
		t.start();
		iio->lock();
//		m_m2k_context->stopMixedSignalAcquisition();
//		m_m2k_context->startMixedSignalAcquisition(active_sample_count);
		iio->unlock();
		qDebug(CAT_OSCILLOSCOPE)<<"Restarted flow @ " << QTime::currentTime().toString("hh:mm:ss") <<"restart took " << t.elapsed() << "ms";
	}
	restartFlowCounter--;
}

void Oscilloscope::autosetNextStep()
{
	static int prevmaxindex=0;
	// Only consider tone valid if it's index is higher than a preset value
	// This ensures that a good enough resolution bandwidth is achieved
	if(autosetSampleRateCnt > 1){
		prevmaxindex = autosetFFTIndex;
		setupAutosetFreqSweep();
	}
	else
	{
		autosetFinalStep();
	}
	toggle_blockchain_flow(true);
}

void Oscilloscope::autosetFinalStep() {
	double timebaseval = timeBase->minValue();
	double timeposval = 0;
	double voltspos = 0;
	double voltsperdiv = 1;
	double triggerlevel = 0;

	if(autoset_id[0]) {
		iio->disconnect(autoset_id[0]);
		autoset_id[0] = nullptr;
	}
	autosetRequested = false;
	if (ui->runSingleWidget->runButtonChecked()) {
		runStopToggled(false);
		runStopToggled(true);
	}

	// autoset frequency found
	if(autosetFFTIndex>1) {
		timebaseval = (1/autosetFrequency)/2;
		voltsperdiv = (max(abs(autosetMaxAmpl),
				   abs(autosetMinAmpl)))/5;
		triggerlevel = (autosetMaxAmpl+autosetMinAmpl)/2;
	}

	timeBase->setValue(timebaseval);
	timeBase->stepUp();
	timePosition->setValue(timeposval);
	voltsPosition->setValue(voltspos);
	if(voltsperdiv < 2e-2)
		voltsperdiv = 2e-2;
	voltsPerDiv->setValue(voltsperdiv);
	voltsPerDiv->stepUp();
	trigger_settings.setTriggerSource(autosetChannel);
	trigger_settings.setTriggerLevel(triggerlevel);
	trigger_settings.setTriggerEnable(true);
	trigger_settings.autoTriggerEnable();	
	onTimePositionChanged(timePosition->value());
}

void Oscilloscope::singleCaptureDone()
{
	if (!symmBufferMode->isEnhancedMemDepth()
			&& !plot_samples_sequentially) {
		Q_EMIT activateExportButton();
		if (ui->runSingleWidget->singleButtonChecked()){
			Q_EMIT startRunning(false);
		}
	}

	periodicFlowRestart();
	plot.repositionCursors();
	scaleHistogramPlot(true);

	if(autosetRequested)
	{
		if(autosetFFTSink->data().size() > autoset_fft_size)
		{
			bool found = autosetFindFrequency();
			if(found)
				autosetFindPeaks();
			autosetNextStep();
		}
	}
}

void Oscilloscope::scaleHistogramPlot(bool newData)
{
	if (hist_plot.isZoomed() && newData) {
		return;
	}

	for (int i = 0; i < nb_channels; i++) {
		double min = plot.axisInterval(QwtAxisId(QwtAxis::YLeft, i)).minValue();
		double max = plot.axisInterval(QwtAxisId(QwtAxis::YLeft, i)).maxValue();

		hist_plot.setYaxisSpan(i, min, max);
	}
	hist_plot.setAutoScaleX();
}

void Oscilloscope::onFilledScreen(bool full, unsigned int nb_samples)
{
	if (nb_samples == active_plot_sample_count) {
		d_shouldResetStreaming = full;
		resetStreamingFlag(full);
	}
}

void Oscilloscope::resetStreamingFlag(bool enable)
{
	bool started = isIioManagerStarted();
	if (started)
		iio->lock();

	try {
		m_m2k_analogin->getTrigger()->setAnalogStreamingFlag(false);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e)
		qDebug(CAT_OSCILLOSCOPE) << e.what();
	}
	cleanBuffersAllSinks();

	if (started)
		iio->unlock();

	try {
		if (enable && !d_displayOneBuffer) {
			m_m2k_analogin->getTrigger()->setAnalogStreamingFlag(true);
		}
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e)
		qDebug(CAT_OSCILLOSCOPE) << e.what();
	}

	/* Single capture done */
	if ((symmBufferMode->isEnhancedMemDepth() || plot_samples_sequentially)
			&& d_shouldResetStreaming) {
		Q_EMIT activateExportButton();
		if (ui->runSingleWidget->singleButtonChecked() && started){
			Q_EMIT startRunning(false);
		}
	}
	d_shouldResetStreaming = false;
}

void Oscilloscope::cleanBuffersAllSinks()
{
	this->qt_time_block->clean_buffers();

	if (mixed_sink) {
		mixed_sink->clean_buffers();
	}

	auto it = math_sinks.constBegin();
	while (it != math_sinks.constEnd()) {
		scope_sink_f::sptr math_sink = dynamic_pointer_cast<
				scope_sink_f>(it.value().second);
		math_sink->clean_buffers();
		++it;
	}
	qt_fft_block->clean_buffers();
}

void Oscilloscope::onIioDataRefillTimeout()
{
	if (trigger_settings.triggerMode() == TriggerSettings::AUTO)
		trigger_is_forced = true;
}

void Oscilloscope::onPlotNewData()
{
	// Flag the new received data as Triggered or Untriggered.
	bool triggerOn = trigger_settings.triggerIsArmed();

	if (triggerOn && !trigger_is_forced)
		new_data_is_triggered = true;
	else
		new_data_is_triggered = false;

	// Reset the Forced Trigger flag.
	trigger_is_forced = false;

	// Update trigger status
	if (new_data_is_triggered)
		triggerUpdater->setInput(CapturePlot::Triggered);
	else
		triggerUpdater->setInput(CapturePlot::Auto);

	updateBufferPreviewer();

	trigger_input = true; //used to read trigger status from Js
}

void Oscilloscope::onTriggerModeChanged(int mode)
{
	if (mode == 0) { // normal
		triggerUpdater->setIdleState(CapturePlot::Waiting);
		triggerUpdater->setInput(CapturePlot::Waiting);
	} else if (mode == 1) { // auto
		triggerUpdater->setIdleState(CapturePlot::Auto);
		triggerUpdater->setInput(CapturePlot::Auto);
	}
}

void Oscilloscope::updateBufferPreviewer()
{
	// Time interval within the plot canvas
	QwtInterval plotInterval = plot.axisInterval(QwtAxis::XBottom);

	// Time interval that represents the captured data
	QwtInterval dataInterval(0.0, 0.0);
	long long triggerSamples = trigger_settings.triggerDelay();
	long long totalSamples = last_set_sample_count;

	if (totalSamples > 0) {
		dataInterval.setMinValue(triggerSamples / getSampleRate());
		dataInterval.setMaxValue((triggerSamples + totalSamples)
			/ getSampleRate());
	}

	// Use the two intervals to determine the width and position of the
	// waveform and of the highlighted area
	QwtInterval fullInterval = plotInterval | dataInterval;
	double wPos = 1 - (fullInterval.maxValue() - dataInterval.minValue()) /
		fullInterval.width();
	double wWidth = dataInterval.width() / fullInterval.width();

	double hPos = 1 - (fullInterval.maxValue() - plotInterval.minValue()) /
		fullInterval.width();
	double hWidth = plotInterval.width() / fullInterval.width();

	// Determine the cursor position
	QwtInterval containerInterval = (totalSamples > 0) ? dataInterval :
		fullInterval;
	double containerWidth = (totalSamples > 0) ? wWidth : 1;
	double containerPos = (totalSamples > 0) ? wPos : 0;
	double cPosInContainer = 1 - (containerInterval.maxValue() - 0) /
		containerInterval.width();
	double cPos = cPosInContainer * containerWidth + containerPos;

	// Update the widget
	buffer_previewer->setWaveformWidth(wWidth);
	buffer_previewer->setWaveformPos(wPos);
	buffer_previewer->setHighlightWidth(hWidth);
	buffer_previewer->setHighlightPos(hPos);
	buffer_previewer->setCursorPos(cPos);
}

void Oscilloscope::on_btnSettings_clicked(bool checked)
{
	CustomPushButton *btn = nullptr;

	if (checked && !menuOrder.isEmpty()) {
		btn = menuOrder.back();
		menuOrder.pop_back();
	} else {
		btn = static_cast<CustomPushButton *>(
			ui->settings_group->checkedButton());
	}

	btn->setChecked(checked);
}

void Oscilloscope::updateGainMode()
{

	if (current_ch_widget >= nb_channels) {
		return;
	}

	double offset = plot.VertOffset(current_ch_widget);
	QwtInterval hw_input_itv(-2.5 + offset, 2.5 + offset);
	QwtInterval plot_vert_itv = plot.axisScaleDiv(
		QwtAxisId(QwtAxis::YLeft, current_ch_widget)).interval();

	auto chn = static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(current_ch_widget);
	// If max signal span that can be captured is smaller than the plot
	// screen try to increase the range (switch to low gain mode)
	if (plot_vert_itv.minValue() < hw_input_itv.minValue() ||
		plot_vert_itv.maxValue() > hw_input_itv.maxValue() ||
			std::abs(offset) > 5.0) {
		if (high_gain_modes[current_ch_widget]) {
			high_gain_modes[current_ch_widget] = false;
			bool running = ui->runSingleWidget->runButtonChecked();

			if (running)
				toggle_blockchain_flow(false);
			setGainMode(current_ch_widget, libm2k::analog::PLUS_MINUS_25V);
			if (running)
				toggle_blockchain_flow(true);


			auto adc_range_limits = m_m2k_analogin->getRangeLimits(libm2k::analog::PLUS_MINUS_25V);
			auto hyst_range_limits = m_m2k_analogin->getHysteresisRange(chn);

			trigger_settings.setTriggerLevelRange(current_ch_widget, adc_range_limits);
			trigger_settings.setTriggerHystRange(current_ch_widget, hyst_range_limits);
		}
	} else {
		if (!high_gain_modes[current_ch_widget]) {
			high_gain_modes[current_ch_widget] = true;
			bool running = ui->runSingleWidget->runButtonChecked();

			if (running)
				toggle_blockchain_flow(false);
			setGainMode(current_ch_widget, libm2k::analog::PLUS_MINUS_2_5V);
			if (running)
				toggle_blockchain_flow(true);

			auto adc_range_limits = m_m2k_analogin->getRangeLimits(libm2k::analog::PLUS_MINUS_25V);
			auto hyst_range_limits = m_m2k_analogin->getHysteresisRange(chn);

			trigger_settings.setTriggerLevelRange(current_ch_widget, adc_range_limits);
			trigger_settings.setTriggerHystRange(current_ch_widget, hyst_range_limits);
		}
	}

}

void Oscilloscope::setGainMode(uint chnIdx, libm2k::analog::M2K_RANGE gain_mode)
{
	if (ui->runSingleWidget->runButtonChecked()) {
		try {
			libm2k::analog::ANALOG_IN_CHANNEL chn = static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(chnIdx);
			runInHwThreadPool( m_m2k_analogin->setRange(chn, gain_mode) );

		} catch (libm2k::m2k_exception &e) {
			HANDLE_EXCEPTION(e)
			qDebug(CAT_OSCILLOSCOPE) << e.what();
		}
	}

	boost::shared_ptr<adc_sample_conv> block = dynamic_pointer_cast<adc_sample_conv>(adc_samp_conv_block);

	iio->freq_comp_filt[chnIdx][0]->set_high_gain(gain_mode);
	iio->freq_comp_filt[chnIdx][1]->set_high_gain(gain_mode);
	update_chn_settings_panel(chnIdx);
	runInHwThreadPool(trigger_settings.updateHwVoltLevels(chnIdx););
}

void Oscilloscope::setChannelHwOffset(uint chnIdx, double offset)
{
	channel_offset[chnIdx] = offset;
	if (ui->runSingleWidget->runButtonChecked()) {
		try {
			libm2k::analog::ANALOG_IN_CHANNEL chn = static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(chnIdx);
			runInHwThreadPool(m_m2k_analogin->setVerticalOffset(chn, offset); );

		} catch (libm2k::m2k_exception &e) {
			HANDLE_EXCEPTION(e)
			qDebug(CAT_OSCILLOSCOPE) << e.what();
		}
	}

}

void Oscilloscope::setAllSinksSampleCount(unsigned long sample_count)
{
	this->qt_time_block->set_nsamps(sample_count);
	this->qt_xy_block->set_nsamps(sample_count);
	this->qt_hist_block->set_nsamps(sample_count);

	if (mixed_sink) {
		mixed_sink->set_nsamps(sample_count);
	}

	auto it = math_sinks.constBegin();
	while (it != math_sinks.constEnd()) {
		scope_sink_f::sptr math_sink = dynamic_pointer_cast<
				scope_sink_f>(it.value().second);
		math_sink->set_nsamps(sample_count);
		++it;
	}
	this->qt_fft_block->set_nsamps(fft_plot_size);
}

void Oscilloscope::writeAllSettingsToHardware()
{
	// Sample Rate
	if (active_sample_rate != getSampleRate()) {
		setSampleRate(active_sample_rate);
	}

	// Offset and Gain
	if (m_m2k_analogin) {
		for (uint i = 0; i < nb_channels; i++) {
			try {
				runInHwThreadPool( {
				   libm2k::analog::M2K_RANGE mode = high_gain_modes[i] ?
				   libm2k::analog::PLUS_MINUS_2_5V : libm2k::analog::PLUS_MINUS_25V;
				   libm2k::analog::ANALOG_IN_CHANNEL chn = static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(i);
				   m_m2k_analogin->setRange(chn, mode);
				   m_m2k_analogin->setVerticalOffset(chn, channel_offset[i]);
								   } );
			} catch (libm2k::m2k_exception &e) {
				HANDLE_EXCEPTION(e)
				qDebug(CAT_OSCILLOSCOPE) << e.what();
			}
		}
		setSampleRate(active_sample_rate);
	}
	for(auto i=0;i<nb_channels;i++)
	{
		iio->freq_comp_filt[i][0]->set_sample_rate(active_sample_rate);
		iio->freq_comp_filt[i][1]->set_sample_rate(active_sample_rate);
	}

	// Writes all trigger settings to hardware
	runInHwThreadPool(trigger_settings.setAdcRunningState(true););
}

void Oscilloscope::on_xyPlotLineType_toggled(bool checked)
{
	if (checked) {
		xy_plot.setLineStyle(0, Qt::DotLine);
	} else {
		xy_plot.setLineStyle(0, Qt::SolidLine);
	}
	xy_plot.replot();
}


void adiscope::Oscilloscope::on_xyLineThickness_currentIndexChanged(int idx)
{
	 xy_plot.setLineWidth(0,idx);
}

void Oscilloscope::setup_xy_channels()
{
	int x = gsettings_ui->cmb_x_channel->currentIndex();
	int y = gsettings_ui->cmb_y_channel->currentIndex();
	QWidget *xsw = xy_plot.axisWidget(QwtAxis::XBottom);
	xsw->setStyleSheet(
		QString("color: %1").arg(plot.getLineColor(x).name()));
	QWidget *ysw = xy_plot.axisWidget(QwtAxis::YLeft);
	ysw->setStyleSheet(
		QString("color: %1").arg(plot.getLineColor(y).name()));

	// If XY visible, reconnect the data flow
	if(xy_is_visible)
		onXY_view_toggled(true);

	xy_plot.setHorizUnitsPerDiv(plot.VertUnitsPerDiv(x));
	xy_plot.setVertUnitsPerDiv(plot.VertUnitsPerDiv(y), QwtAxis::YLeft);

	updateXyPlotScales();
}

void Oscilloscope::on_btnAddMath_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void Oscilloscope::on_btnCursors_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void Oscilloscope::on_btnMeasure_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void Oscilloscope::on_btnTrigger_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void Oscilloscope::on_btnGeneralSettings_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
	if(checked)
		ui->btnSettings->setChecked(!checked);
}

void Oscilloscope::channelLineWidthChanged(int id)
{
	qreal width = 0.5 * (id + 1);

	if (width != plot.getLineWidthF(current_ch_widget)) {
		plot.setLineWidthF(current_ch_widget, width);
		plot.replot();
	}
}

void Oscilloscope::setSampleRate(double sample_rate)
{
	if (!m_m2k_analogin) {
		return;
	}
	runInHwThreadPool( {
	try {
		auto maxSampleRate = m_m2k_analogin->getMaximumSamplerate();
		if (m_filtering_enabled == false) {
			m_m2k_analogin->setSampleRate(maxSampleRate);
			m_m2k_analogin->setOversamplingRatio(maxSampleRate / sample_rate);
		} else {
			m_m2k_analogin->setSampleRate(sample_rate);
			m_m2k_analogin->setOversamplingRatio(1);
		}
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e)
		qDebug(CAT_OSCILLOSCOPE) << e.what();
	}
	});
}


double Oscilloscope::getSampleRate()
{
	if (!m_m2k_analogin) {
		return 0;
	}
	try {
		double sr = m_m2k_analogin->getSampleRate();
		if (m_filtering_enabled == false) {
			int osr = m_m2k_analogin->getOversamplingRatio();
			sr = (double)(sr / osr);
		}
		return sr;
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e);
		qDebug(CAT_OSCILLOSCOPE) << e.what();
		return 0;
	}
}
