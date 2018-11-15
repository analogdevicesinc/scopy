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

#include "logging_categories.h"
#include "dynamicWidget.hpp"
#include "network_analyzer.hpp"
#include "signal_generator.hpp"
#include "spinbox_a.hpp"
#include "osc_adc.h"
#include "hardware_trigger.hpp"
#include "ui_network_analyzer.h"
#include "filemanager.h"

#include <gnuradio/analog/sig_source.h>

#include <gnuradio/analog/sig_source_waveform.h>
#include <gnuradio/blocks/float_to_short.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/moving_average.h>
#include <gnuradio/blocks/multiply.h>
#include <gnuradio/blocks/multiply_conjugate_cc.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/rotator_cc.h>
#include <gnuradio/blocks/skiphead.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/top_block.h>
#include <boost/make_shared.hpp>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/vector_to_stream.h>
#include <gnuradio/fft/goertzel_fc.h>

#include "hw_dac.h"

#include <algorithm>

#include <QThread>
#include <QFileDialog>
#include <QDateTime>
#include <QSignalBlocker>
#include <QImageWriter>

#include <iio.h>
#include <network_analyzer_api.hpp>

#include <QElapsedTimer>

using namespace adiscope;
using namespace gr;

void NetworkAnalyzer::_configureDacFlowgraph()
{
	// Create the blocks that are used to generate sine waves
	top_block = make_top_block("Signal Generator");
	source_block = analog::sig_source_f::make(1, analog::GR_SIN_WAVE,
			1, 1, 1);
	f2s_block = blocks::float_to_short::make(1, 1);
	head_block = blocks::head::make(sizeof(short), 1);
	vector_block = blocks::vector_sink_s::make();

	// Connect the blocks for the sine wave generation
	top_block->connect(source_block, 0, f2s_block, 0);
	top_block->connect(f2s_block, 0, head_block, 0);
	top_block->connect(head_block, 0, vector_block, 0);
}

void NetworkAnalyzer::_configureAdcFlowgraph(size_t buffer_size)
{
	if (m_initFlowgraph) {


		capture_top_block = make_top_block("Network capture processing");

		// Get the available sample rates for the m2k-adc
		// Make sure the values are sorted in ascending order (1000,..,100e6)
		sampleRates = SignalGenerator::get_available_sample_rates(adc);
		qSort(sampleRates.begin(), sampleRates.end(), qLess<unsigned long>());

		auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);

		dc_cancel1 = gnuradio::get_initial_sptr(
					new cancel_dc_offset_block(1, false));
		dc_cancel2 = gnuradio::get_initial_sptr(
					new cancel_dc_offset_block(1, false));

		capture1 = gr::blocks::vector_source_s::make(std::vector<short>(), false, 1);
		capture2 = gr::blocks::vector_source_s::make(std::vector<short>(), false, 1);
		f11 = adiscope::frequency_compensation_filter::make(false);
		f12 = adiscope::frequency_compensation_filter::make(false);
		f21 = adiscope::frequency_compensation_filter::make(false);
		f22 = adiscope::frequency_compensation_filter::make(false);
		s2f1 = gr::blocks::short_to_float::make();
		s2f2 = gr::blocks::short_to_float::make();
		goertzel1 = gr::fft::goertzel_fc::make(1, 1, 1);
		goertzel2 = gr::fft::goertzel_fc::make(1, 1, 1);
		copy1 = gr::blocks::copy::make(sizeof(float));
		copy2 = gr::blocks::copy::make(sizeof(float));
		head1 = gr::blocks::head::make(sizeof(float), 1);
		head2 = gr::blocks::head::make(sizeof(float), 1);
		sink1 = gr::blocks::vector_sink_f::make();
		sink2 = gr::blocks::vector_sink_f::make();
		c2m1 = gr::blocks::complex_to_mag_squared::make();
		c2m2 = gr::blocks::complex_to_mag_squared::make();
		conj = gr::blocks::multiply_conjugate_cc::make();
		c2a = gr::blocks::complex_to_arg::make();
		signal = boost::make_shared<signal_sample>();
		adc_conv = boost::make_shared<adc_sample_conv>(2, m2k_adc);

		adc_conv->setCorrectionGain(0,
			m2k_adc->chnCorrectionGain(0));
		adc_conv->setCorrectionGain(1,
			m2k_adc->chnCorrectionGain(1));

		capture_top_block->connect(capture1, 0, f11, 0);
		capture_top_block->connect(capture2, 0, f21, 0);
		capture_top_block->connect(f11, 0, f12, 0);
		capture_top_block->connect(f21, 0, f22, 0);
		capture_top_block->connect(f12, 0, s2f1, 0);
		capture_top_block->connect(f22, 0, s2f2, 0);
		capture_top_block->connect(s2f1, 0, dc_cancel1, 0);
		capture_top_block->connect(s2f2, 0, dc_cancel2, 0);

		capture_top_block->connect(dc_cancel1, 0, goertzel1, 0);
		capture_top_block->connect(dc_cancel2, 0, goertzel2, 0);
		capture_top_block->connect(goertzel1, 0, c2m1, 0);
		capture_top_block->connect(c2m1, 0, signal, 0);
		capture_top_block->connect(goertzel2, 0, c2m2, 0);
		capture_top_block->connect(c2m2, 0, signal, 1);

		capture_top_block->connect(goertzel1, 0, conj, 0);
		capture_top_block->connect(goertzel2, 0, conj, 1);
		capture_top_block->connect(conj, 0, c2a, 0);
		capture_top_block->connect(c2a, 0, signal, 2);

		mag1 = 0.0, mag2 = 0.0, phase = 0.0;
		connect(&*signal, &signal_sample::triggered,
		[&](const std::vector<float> values) {
			mag1 = values[0];
			mag2 = values[1];
			phase = values[2];
			captureDone = true;
		});

		capture_top_block->connect(dc_cancel1, 0, adc_conv, 0);
		capture_top_block->connect(dc_cancel2, 0, adc_conv, 1);
		capture_top_block->connect(adc_conv, 0, sink1, 0);
		capture_top_block->connect(adc_conv, 1, sink2, 0);
	}

	// Build the flowgraph only once
	m_initFlowgraph = false;
}

NetworkAnalyzer::NetworkAnalyzer(struct iio_context *ctx, Filter *filt,
				 std::shared_ptr<GenericAdc>& adc_dev,
				 QList<std::shared_ptr<GenericDac>> dacs,
				 ToolMenuItem *toolMenuItem, QJSEngine *engine,
				 ToolLauncher *parent) :
	Tool(ctx, toolMenuItem, new NetworkAnalyzer_API(this), "Network Analyzer", parent),
	ui(new Ui::NetworkAnalyzer),
	adc_dev(adc_dev),
	d_cursorsEnabled(false),
	m_stop(true), amp1(nullptr), amp2(nullptr),
	wheelEventGuard(nullptr), wasChecked(false),
	dacs(dacs), justStarted(false),
	iterationsThreadCanceled(false), iterationsThreadReady(false),
	iterationsThread(nullptr), autoAdjustGain(true),
	filterDc(false), m_initFlowgraph(true), m_hasReference(false),
	m_importDataLoaded(false)
{
	iio = iio_manager::get_instance(ctx,
					filt->device_name(TOOL_NETWORK_ANALYZER, 2));

	adc = filt->find_device(ctx, TOOL_NETWORK_ANALYZER, 2);

	dac_channels.push_back(filt->find_channel(ctx, TOOL_NETWORK_ANALYZER, 0, true));
	dac_channels.push_back(filt->find_channel(ctx, TOOL_NETWORK_ANALYZER, 1, true));

	for (const auto& channel : dac_channels) {
		if (!channel) {
			throw std::runtime_error("Unable to find channels in filter file");
		}
	}

	/* FIXME: TODO: Move this into a HW class / lib M2k */
	struct iio_device *fabric = iio_context_find_device(ctx, "m2k-fabric");

	if (fabric) {
		this->amp1 = iio_device_find_channel(fabric, "voltage0", true);
		this->amp2 = iio_device_find_channel(fabric, "voltage1", true);

		if (amp1 && amp2) {
			iio_channel_attr_write_bool(amp1, "powerdown", true);
			iio_channel_attr_write_bool(amp2, "powerdown", true);
		}
	}

	ui->setupUi(this);

	bufferPreviewer = new NetworkAnalyzerBufferViewer();
	bufferPreviewer->setVisible(false);

	ui->statusLabel->setText(tr("Stopped"));

	connect(ui->bufferPreviewSwitch, &QCheckBox::toggled,
		this, &NetworkAnalyzer::toggleBufferPreview);

	connect(ui->prevBtn, &QPushButton::pressed,
		bufferPreviewer, &NetworkAnalyzerBufferViewer::btnPreviousClicked);
	connect(ui->nextBtn, &QPushButton::pressed,
		bufferPreviewer, &NetworkAnalyzerBufferViewer::btnNextClicked);
	connect(ui->viewInOscBtn, &QPushButton::pressed,
		bufferPreviewer, &NetworkAnalyzerBufferViewer::sendBufferToOscilloscope);

	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		[=](bool checked){
		auto btn = dynamic_cast<CustomPushButton *>(runButton());
		btn->setChecked(checked);
	});
	connect(runButton(), &QPushButton::toggled,
		ui->runSingleWidget, &RunSingleWidget::toggle);
	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		this, &NetworkAnalyzer::startStop);

	connect(this, &NetworkAnalyzer::sweepDone,
	[=]() {
		if (ui->runSingleWidget->runButtonChecked()) {
			thd = QtConcurrent::run(this, &NetworkAnalyzer::goertzel);
			return;
		}

		dynamic_cast<CustomPushButton *>(this->runButton())->setChecked(false);
	});


	startStopRange = new StartStopRangeWidget(1.0, 25e06);
	ui->sweepRangeLayout->addWidget(startStopRange);
	ui->rightMenu->setMaximumWidth(0);

	connect(startStopRange, &StartStopRangeWidget::rangeChanged,
		[=](double start, double stop){
		// Update plot settings
		m_dBgraph.setXMin(start);
		m_dBgraph.setXMax(stop);
		m_phaseGraph.setXMin(start);
		m_phaseGraph.setXMax(stop);

		computeIterations();
		updateNumSamples(true);
	});


	std::vector<unsigned long> rates;
	rates.resize(dac_channels.size());
	std::transform(dac_channels.begin(), dac_channels.end(),
		       rates.begin(),
	[](struct iio_channel *ch) {
		return SignalGenerator::get_max_sample_rate(
			       iio_channel_get_device(ch));
	});

	unsigned long max_samplerate = *std::max_element(rates.begin(), rates.end());

	m_dBgraph.setColor(QColor(255,114,0));
	m_dBgraph.setXTitle(tr("Frequency (Hz)"));
	m_dBgraph.setYTitle(tr("Magnitude(dB)"));
	m_dBgraph.setXMin(1000.000000);
	m_dBgraph.setXMax(50000.000000);
	m_dBgraph.setYMin(-80.000000);
	m_dBgraph.setYMax(20.000000);
	m_dBgraph.useLogFreq(true);

	m_phaseGraph.setColor(QColor(144,19,254));
	m_phaseGraph.setYTitle(tr("Phase (°)"));
	m_phaseGraph.setYUnit("°");
	m_phaseGraph.setXMin(1000.000000);
	m_phaseGraph.setXMax(50000.000000);
	m_phaseGraph.setYMin(-180.000000);
	m_phaseGraph.setYMax(180.000000);
	m_phaseGraph.useLogFreq(true);

	samplesCount = new ScaleSpinButton({
		{"samples",1e0},
	}, tr("Samples count"), 10, 1000, false, false, this);
	samplesCount->setValue(1000);

	amplitude = new ScaleSpinButton({
		{"μVolts",1e-6},
		{"mVolts",1e-3},
		{"Volts",1e0}
	},tr("Amplitude"), 1e-6, 1e1, false, false, this);
	amplitude->setValue(1);

	offset = new PositionSpinButton({
		{"μVolts",1e-6},
		{"mVolts",1e-3},
		{"Volts",1e0}
	},tr("Offset"), -5, 5, false, false, this);

	offset->setValue(0);

	magMax = new PositionSpinButton({
		{"dB",1e0}
	}, tr("Max. Magnitude"), -120, 120, false, false, this);
	magMax->setValue(20);

	magMin = new PositionSpinButton({
		{"dB",1e0}
	}, tr("Min. Magnitude"), -120, 120, false, false, this);
	magMin->setValue(-80);

	phaseMax = new PositionSpinButton({
		{"°",1e0}
	}, tr("Max. Phase"), -360, 360, false, false, this);
	phaseMax->setValue(180);

	phaseMin = new PositionSpinButton({
		{"°",1e0}
	}, tr("Min. Phase"), -360, 360, false, false, this);
	phaseMin->setValue(-180);

	pushDelay = new PositionSpinButton({
		{"ms",1e0},
		{"s",1e3}
	}, tr("Settling time"), 0, 2000, false, false, this);
	pushDelay->setValue(0);
	pushDelay->setStep(10);
	pushDelay->setToolTip(tr("Before Buffer"));

	captureDelay = new PositionSpinButton({
		{"ms",1e0},
		{"s",1e3}
	}, tr("Settling time"), 0, 2000, false, false, this);
	captureDelay->setValue(0);
	captureDelay->setStep(10);
	captureDelay->setToolTip(tr("After Buffer"));

	ui->pushDelayLayout->addWidget(pushDelay);
	ui->captureDelayLayout->addWidget(captureDelay);
	startStopRange->insertWidgetIntoLayout(samplesCount, 2, 1);
	ui->amplitudeLayout->addWidget(amplitude);
	ui->offsetLayout->addWidget(offset);
	ui->magMaxLayout->addWidget(magMax);
	ui->magMinLayout->addWidget(magMin);
	ui->phaseMaxLayout->addWidget(phaseMax);
	ui->phaseMinLayout->addWidget(phaseMin);

	setMinimumDistanceBetween(magMin, magMax, 1);
	setMinimumDistanceBetween(phaseMin, phaseMax, 1);

	connect(magMax, &PositionSpinButton::valueChanged,
		ui->xygraph, &NyquistGraph::setMax);
	connect(magMax, &PositionSpinButton::valueChanged,
		ui->nicholsgraph, &dBgraph::setYMax);
	connect(magMin, &PositionSpinButton::valueChanged,
		ui->xygraph, &NyquistGraph::setMin);
	connect(magMin, &PositionSpinButton::valueChanged,
		ui->nicholsgraph, &dBgraph::setYMin);
	connect(phaseMax, &PositionSpinButton::valueChanged,
		ui->nicholsgraph, &dBgraph::setXMax);
	connect(phaseMin, &PositionSpinButton::valueChanged,
		ui->nicholsgraph, &dBgraph::setXMin);

	connect(magMin, SIGNAL(valueChanged(double)),
		&m_dBgraph, SLOT(setYMin(double)));
	connect(magMax, SIGNAL(valueChanged(double)),
		&m_dBgraph, SLOT(setYMax(double)));
	connect(ui->btnIsLog, SIGNAL(toggled(bool)),
		&m_dBgraph, SLOT(useLogFreq(bool)));

	connect(phaseMin, SIGNAL(valueChanged(double)),
		&m_phaseGraph, SLOT(setYMin(double)));
	connect(phaseMax, SIGNAL(valueChanged(double)),
		&m_phaseGraph, SLOT(setYMax(double)));
	connect(ui->btnIsLog, SIGNAL(toggled(bool)),
		&m_phaseGraph, SLOT(useLogFreq(bool)));
	connect(ui->btnIsLog, &CustomSwitch::toggled, [=](bool value) {
		Q_UNUSED(value);
		computeIterations();
	});

	ui->nicholsgraph->setPlotBarEnabled(false);

	connect(phaseMin, &PositionSpinButton::valueChanged,
		this, &NetworkAnalyzer::onMinMaxPhaseChanged);
	connect(phaseMax, &PositionSpinButton::valueChanged,
		this, &NetworkAnalyzer::onMinMaxPhaseChanged);


	connect(ui->cbLineThickness,SIGNAL(currentIndexChanged(int)),&m_dBgraph,
		SLOT(setThickness(int)));
	connect(ui->cbLineThickness,SIGNAL(currentIndexChanged(int)),&m_phaseGraph,
		SLOT(setThickness(int)));
	connect(ui->cbLineThickness,SIGNAL(currentIndexChanged(int)),ui->nicholsgraph,
		SLOT(setThickness(int)));
	connect(ui->cbLineThickness,SIGNAL(currentIndexChanged(int)),ui->xygraph,
		SLOT(setThickness(int)));

	d_bottomHandlesArea = new HorizHandlesArea(this);
	d_bottomHandlesArea->setMinimumHeight(50);

	ui->gridLayout_plots->addWidget(bufferPreviewer, 0, 0, 1, 1);
	ui->gridLayout_plots->addWidget(ui->statusWidget, 1, 0, 1, 1);
	ui->gridLayout_plots->addWidget(&m_dBgraph, 2, 0, 1, 1);
	ui->gridLayout_plots->addWidget(&m_phaseGraph, 3, 0, 1, 1);
	ui->gridLayout_plots->addWidget(d_bottomHandlesArea, 4, 0, 1, 1);

	ui->currentFrequencyLabel->setVisible(false);
	ui->currentSampleLabel->setVisible(false);

	d_frequencyHandle = new FreePlotLineHandleH(
				QPixmap(":/icons/time_trigger_handle.svg"),
				QPixmap(":/icons/time_trigger_left.svg"),
				QPixmap(":/icons/time_trigger_right.svg"),
				d_bottomHandlesArea);
	d_frequencyHandle->setPen(QPen(QColor(74, 100, 255), 2, Qt::SolidLine));
	d_frequencyHandle->setVisible(true);

	d_hCursorHandle1 = new PlotLineHandleH(
		QPixmap(":/icons/h_cursor_handle.svg"),
		d_bottomHandlesArea);
	d_hCursorHandle2 = new PlotLineHandleH(
		QPixmap(":/icons/h_cursor_handle.svg"),
		d_bottomHandlesArea);

	ui->nicholsgraph->enableFrequencyBar(false);

	connect(d_frequencyHandle, &FreePlotLineHandleH::positionChanged,
		&m_dBgraph, &dBgraph::onFrequencyCursorPositionChanged);
	connect(d_frequencyHandle, &FreePlotLineHandleH::positionChanged,
		&m_phaseGraph, &dBgraph::onFrequencyCursorPositionChanged);
	connect(&m_dBgraph, &dBgraph::frequencyBarPositionChanged,
		this, &NetworkAnalyzer::onFrequencyBarMoved);
	connect(&m_phaseGraph, &dBgraph::frequencyBarPositionChanged,
		this, &NetworkAnalyzer::onFrequencyBarMoved);

	connect(bufferPreviewer, &NetworkAnalyzerBufferViewer::moveHandleAt,
		&m_dBgraph, &dBgraph::onFrequencyBarMoved);
	connect(bufferPreviewer, &NetworkAnalyzerBufferViewer::moveHandleAt,
		&m_phaseGraph, &dBgraph::onFrequencyBarMoved);
	connect(bufferPreviewer, &NetworkAnalyzerBufferViewer::indexChanged,
		[=](int value) {
		ui->currentFrequencyLabel->setVisible(true);
		ui->currentSampleLabel->setVisible(true);
		ui->currentSampleLabel->setText(QString(tr("Sample: ") + QString::number(1 + value)
							+ " / " + QString::number(m_dBgraph.getNumSamples()) + " "));
		MetricPrefixFormatter d_cursorTimeFormatter;
		d_cursorTimeFormatter.setTwoDecimalMode(false);
		QString text = d_cursorTimeFormatter.format(iterations[value].frequency, "Hz", 3);
		ui->currentFrequencyLabel->setText(QString(tr("Current Frequency: ") + text));

		if (value < iterationStats.size()) {
			double dcVoltage = iterationStats[value].dcVoltage;
			text = d_cursorTimeFormatter.format(dcVoltage, "V", 2);
			ui->dcLabel->setText(tr("DC Voltage: ") + text);

			if (iterationStats[value].hasError) {
				ui->errorLabel->setText(tr("Response channel voltage < 50mV!"));
			} else {
				ui->errorLabel->setText("");
			}
			auto gain = iterationStats[value].gain;
			QString gainText = !gain ? tr("Low") : tr("High");
			ui->gainLabel->setText(tr("Gain Mode: ") + gainText);
		}
	});


	connect(&m_dBgraph, &dBgraph::frequencySelected,
		bufferPreviewer, &NetworkAnalyzerBufferViewer::selectBuffers);

	QPen cursorsLinePen = QPen(QColor(155,155,155),1,Qt::DashLine);
	d_hCursorHandle1->setPen(cursorsLinePen);
	d_hCursorHandle2->setPen(cursorsLinePen);
	d_hCursorHandle1->setVisible(false);
	d_hCursorHandle2->setVisible(false);

	connect(&m_dBgraph,SIGNAL(VBar1PixelPosChanged(int)),
		SLOT(onVbar1PixelPosChanged(int)));
	connect(&m_dBgraph,SIGNAL(VBar2PixelPosChanged(int)),
		SLOT(onVbar2PixelPosChanged(int)));

	connect(d_hCursorHandle1, SIGNAL(positionChanged(int)),&m_dBgraph,
		SLOT(onCursor1PositionChanged(int)));
	connect(d_hCursorHandle2, SIGNAL(positionChanged(int)),&m_dBgraph,
		SLOT(onCursor2PositionChanged(int)));
	connect(d_hCursorHandle1, SIGNAL(positionChanged(int)),&m_phaseGraph,
		SLOT(onCursor1PositionChanged(int)));
	connect(d_hCursorHandle2, SIGNAL(positionChanged(int)),&m_phaseGraph,
		SLOT(onCursor2PositionChanged(int)));

	startStopRange->setStopValue((double) max_samplerate / 3.0 - 1.0);

	connect(samplesCount, SIGNAL(valueChanged(double)),
		this, SLOT(updateNumSamples()));
	connect(ui->boxCursors,SIGNAL(toggled(bool)),
		SLOT(toggleCursors(bool)));

	connect(ui->cmb_graphs,SIGNAL(currentIndexChanged(int)),
		SLOT(onGraphIndexChanged(int)));

	readPreferences();

	api->setObjectName(QString::fromStdString(Filter::tool_name(
				   TOOL_NETWORK_ANALYZER)));

	ui->xygraph->enableZooming(ui->btnZoomIn, ui->btnZoomOut);

	api->load(*settings);
	api->js_register(engine);

	connect((m_dBgraph.getAxisWidget(QwtPlot::xTop)), SIGNAL(scaleDivChanged()),
		&m_phaseGraph, SLOT(scaleDivChanged()));
	connect((m_phaseGraph.getAxisWidget(QwtPlot::xTop)), SIGNAL(scaleDivChanged()),
		&m_dBgraph, SLOT(scaleDivChanged()));

	connect(&m_dBgraph,SIGNAL(resetZoom()),&m_phaseGraph,SLOT(onResetZoom()));
	connect(&m_phaseGraph,SIGNAL(resetZoom()),&m_dBgraph,SLOT(onResetZoom()));


	connect(ui->rightMenu, &MenuAnim::finished, this,
		&NetworkAnalyzer::rightMenuFinished);

	connect(ui->btnSettings, &CustomPushButton::toggled, [=](bool checked) {
		triggerRightMenuToggle(ui->btnSettings, checked);
	});
	connect(ui->btnGeneralSettings, &CustomPushButton::toggled, [=](bool checked) {
		triggerRightMenuToggle(ui->btnGeneralSettings, checked);
	});
	connect(ui->btnCursors, &CustomPushButton::toggled, [=](bool checked) {
		triggerRightMenuToggle(ui->btnCursors, checked);
	});

	ui->btnSettings->setProperty("id",QVariant(-1));
	ui->btnGeneralSettings->setProperty("id",QVariant(-2));
	ui->btnCursors->setProperty("id",QVariant(-3));


	connect(ui->horizontalSlider, &QSlider::valueChanged, [=](int value) {
		ui->transLabel->setText(tr("Transparency ") + QString::number(value) + "%");
		m_dBgraph.setCursorReadoutsTransparency(value);
		m_phaseGraph.setCursorReadoutsTransparency(value);
	});

	connect(ui->posSelect, &CustomPlotPositionButton::positionChanged,
	[=](CustomPlotPositionButton::ReadoutsPosition position) {
		m_dBgraph.moveCursorReadouts(position);
		m_phaseGraph.moveCursorReadouts(position);
	});

	if (!wheelEventGuard) {
		wheelEventGuard = new MouseWheelWidgetGuard(ui->mainWidget);
	}

	wheelEventGuard->installEventRecursively(ui->mainWidget);

	connect(ui->btnPrint, &QPushButton::clicked, [=]() {
		QWidget *widget = ui->stackedWidget->currentWidget();
		QImage img(widget->width(), widget->height(), QImage::Format_ARGB32);
		QPainter painter(&img);
		img.fill(Qt::black);
		widget->render(&painter);
		QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");

		QString fileNameHint = "Scopy-" + api->objectName() + "-" + date;

		const QList<QByteArray> imageFormats =
		    QImageWriter::supportedImageFormats();

		QStringList filter;
		if ( imageFormats.size() > 0 ) {
		    for ( int i = 0; i < imageFormats.size(); i++ ) {
			filter += (imageFormats[i].toUpper() + " "
				+ tr("Image") + " (*." +  imageFormats[i] + ")");
		    }
		}

		QString selectedFilter = filter[0];
			QString fileName = QFileDialog::getSaveFileName(this,
		    tr("Save to"), fileNameHint, filter.join(";;"),
		    &selectedFilter, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

		if (fileName.split(".").size() <= 1) {
		    // file name w/o extension. Let's append it
		    QString ext = selectedFilter.split(".")[1].split(")")[0];
		    fileName += "." + ext;
		}

		painter.end();
		img.invertPixels(QImage::InvertRgb);
		img.save(fileName, 0, -1);
	});

	ui->deltaBtn->setDisabled(ui->btnIsLog->isChecked());

	connect(ui->deltaBtn, &QPushButton::toggled,
		&m_dBgraph, &dBgraph::useDeltaLabel);
	connect(ui->deltaBtn, &QPushButton::toggled,
		&m_phaseGraph, &dBgraph::useDeltaLabel);
	connect(ui->btnIsLog, &QPushButton::toggled, [=](bool checked) {
		ui->deltaBtn->setDisabled(checked);

		if (checked) {
			wasChecked = ui->deltaBtn->isChecked();
			ui->deltaBtn->setChecked(false);
		} else {
			ui->deltaBtn->setChecked(wasChecked);
		}
	});
	connect(ui->dcFilterBtn, &QPushButton::toggled, [=](bool checked){
		if (checked != filterDc) {
			filterDc = checked;
			dc_cancel1->set_enabled(filterDc);
			dc_cancel2->set_enabled(filterDc);
		}
	});

	connect(ui->responseGainCmb, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[=](int value) {
		autoAdjustGain = (value == 0);
	});

	connect(ui->snapshotBtn, &QCheckBox::clicked, [=](){
		m_hasReference = true;
		bool didAddDbgraph = m_dBgraph.addReferenceWaveformFromPlot();
		bool didAddPhasegraph = m_phaseGraph.addReferenceWaveformFromPlot();

		if (didAddDbgraph || didAddPhasegraph) {
			ui->removeReferenceBtn->setEnabled(true);
		}
	});

	connect(ui->removeReferenceBtn, &QPushButton::clicked, [=]() {
		m_dBgraph.removeReferenceWaveform();
		m_phaseGraph.removeReferenceWaveform();
		ui->removeReferenceBtn->setDisabled(true);
	});

	connect(ui->importBtn, &QPushButton::clicked, [=](){
		QString fileName = QFileDialog::getOpenFileName(this,
		    tr("Import"), "", tr("Comma-separated values files (*.csv);;"
					       "Tab-delimited values files (*.txt)"),
		    nullptr, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

		FileManager fm("Network Analyzer");

		try {
			fm.open(fileName, FileManager::IMPORT);

			ui->importFileLineEdit->setText(fileName);
			ui->importFileLineEdit->setToolTip(fileName);

			m_importDataLoaded = true;
			m_importData = fm.read();

			QVector<double> frequency, magnitude, phase;

			for (size_t i = 0; i < m_importData.size(); ++i) {
				frequency.push_back(m_importData[i][0]);
				qDebug() << frequency.back();
				magnitude.push_back(m_importData[i][1]);
				phase.push_back(m_importData[i][2]);
			}

			m_dBgraph.addReferenceWaveform(frequency, magnitude);
			m_phaseGraph.addReferenceWaveform(frequency, phase);

			m_hasReference = true;
			ui->removeReferenceBtn->setEnabled(true);
		} catch (FileManagerException &e) {
			ui->importFileLineEdit->setText(e.what());
		}

	});

	_configureDacFlowgraph();
	_configureAdcFlowgraph();
}

NetworkAnalyzer::~NetworkAnalyzer()
{
	disconnect(prefPanel,&Preferences::notify,this,
		   &NetworkAnalyzer::readPreferences);
	startStop(false);
	ui->runSingleWidget->toggle(false);

	if (saveOnExit) {
		api->save(*settings);
	}

	top_block->disconnect_all();

	if (iterationsThread) {
		if (iterationsThread->joinable()) {
			iterationsThreadCanceled = true;
			iterationsThread->join();
		}

		delete iterationsThread;
		iterationsThread = nullptr;
	}

	delete api;

	delete ui;
}

void NetworkAnalyzer::setOscilloscope(Oscilloscope *osc)
{
	bufferPreviewer->setOscilloscope(osc);
}

bool NetworkAnalyzer::isIterationsThreadReady()
{
	return iterationsThreadReady;
}

bool NetworkAnalyzer::isIterationsThreadCanceled()
{
	return iterationsThreadCanceled;
}

void NetworkAnalyzer::onMinMaxPhaseChanged(double value) {

	if (QObject::sender() == phaseMin) {
		double phaseMaxValue = phaseMax->value();
		if (qAbs(phaseMaxValue - value) > 360) {
			phaseMax->setValue(phaseMaxValue - ((int)qAbs(phaseMaxValue - value) % 360));
		}
	} else {
		double phaseMinValue = phaseMin->value();
		if (qAbs(value - phaseMinValue) > 360) {
			phaseMin->setValue(phaseMinValue + ((int)qAbs(value - phaseMinValue) % 360));
		}
	}

	auto interval = getPhaseInterval();
	m_phaseGraph.setYAxisInterval(interval.first, interval.second, 360);
}

void NetworkAnalyzer::computeIterations()
{
	if (iterationsThread) {
		if (iterationsThread->joinable()) {
			iterationsThreadCanceled = true;
			iterationsThread->join();
			delete iterationsThread;
			iterationsThread = nullptr;
		} else {
			delete iterationsThread;
			iterationsThread = nullptr;
		}
	}

	// at this point no other thread is using iterationsThreadReady
	// it is safe to modify without using a lock
	iterationsThreadCanceled = false;
	iterationsThreadReady = false;
	iterationsThread = new boost::thread(boost::bind(&NetworkAnalyzer::computeFrequencyArray, this));
}

void NetworkAnalyzer::setMinimumDistanceBetween(SpinBoxA *min, SpinBoxA *max,
		double distance)
{

	connect(max, &SpinBoxA::valueChanged, [=](double value) {
		min->setMaxValue(value - distance);
//		min->setValue(min->value());
	});
	connect(min, &SpinBoxA::valueChanged, [=](double value) {
		max->setMinValue(value + distance);
//		max->setValue(max->value());
	});
}

void NetworkAnalyzer::triggerRightMenuToggle(CustomPushButton *btn,
		bool checked)
{
	if (ui->rightMenu->animInProgress()) {
		menuButtonActions.enqueue(
			QPair<CustomPushButton *, bool>(btn, checked));
	} else {
		toggleRightMenu(btn, checked);
	}
}

void NetworkAnalyzer::toggleRightMenu(CustomPushButton *btn, bool checked)
{
	int id = btn->property("id").toInt();

	if (checked) {
		ui->stackedWidget_2->setCurrentIndex(-id-1);
	}

	ui->rightMenu->toggleMenu(checked);
}

void NetworkAnalyzer::rightMenuFinished(bool opened)
{
	Q_UNUSED(opened)

	while (menuButtonActions.size()) {
		auto pair = menuButtonActions.dequeue();
		toggleRightMenu(pair.first, pair.second);
	}
}

void NetworkAnalyzer::showEvent(QShowEvent *event)
{
	d_bottomHandlesArea->setLeftPadding(m_dBgraph.axisWidget(QwtAxisId(
			QwtPlot::yLeft, 0))->width()
					    + ui->gridLayout_plots->margin()
					    + ui->widgetPlotContainer->layout()->margin() + 1);
	int rightPadding = 0;
	rightPadding = rightPadding + m_dBgraph.width()
		       - m_dBgraph.axisWidget(QwtPlot::yLeft)->width() - m_dBgraph.canvas()->width()
		       - ui->widgetPlotContainer->layout()->margin() ;
	d_bottomHandlesArea->setRightPadding(rightPadding);
	d_hCursorHandle1->setPosition(d_hCursorHandle1->pos().x());
	d_hCursorHandle2->setPosition(d_hCursorHandle2->pos().x());
	Tool::showEvent(event);
}

void NetworkAnalyzer::on_btnExport_clicked()
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
		FileManager fm("Network Analyzer");

		fm.open(fileName, FileManager::EXPORT);

		fm.setAdditionalInformation(ui->btnRefChn->isChecked() ?
					    "Reference channel: 1" : "Reference channel: 2");

		fm.save(m_dBgraph.getXAxisData(), "Frequency(Hz)");
		fm.save(m_dBgraph.getYAxisData(), "Magnitude(dB)");
		fm.save(m_phaseGraph.getYAxisData(), "Phase(°)");

		fm.performWrite();
	}
}

void NetworkAnalyzer::updateNumSamples(bool force)
{
	unsigned int num_samples;

	if (force) {
		num_samples = iterations.size();
	} else {
		num_samples = (unsigned int) samplesCount->value();
	}

	m_dBgraph.setNumSamples(num_samples);
	m_phaseGraph.setNumSamples(num_samples);
	ui->xygraph->setNumSamples(num_samples);
	ui->nicholsgraph->setNumSamples(num_samples);
	bufferPreviewer->setNumBuffers(num_samples);

	if (QObject::sender() == samplesCount) {
		computeIterations();
	}
}

void NetworkAnalyzer::updateGainMode()
{
	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);

	int responseGainCmbIndex = ui->responseGainCmb->currentIndex();
	int responseChannel = ui->btnRefChn->isChecked() ? 1 : 0;
	int referenceChannel = 1 - responseChannel;

	if (m2k_adc) {
		double sweep_ampl = amplitude->value();
		QPair<double, double> range = m2k_adc->inputRange(
						      M2kAdc::HIGH_GAIN_MODE);
		double threshold = range.second; // - range.first
		M2kAdc::GainMode gain_mode;

		if ((sweep_ampl / 2.0) + offset->value() > threshold
				|| -(sweep_ampl / 2.0) + offset->value() < -threshold) {
			gain_mode = M2kAdc::LOW_GAIN_MODE;
		} else {
			gain_mode = M2kAdc::HIGH_GAIN_MODE;
		}

		for (int chn = 0; chn < static_cast<int>(m2k_adc->numAdcChannels()); chn++) {
			if (chn == referenceChannel) {
				m2k_adc->setChnHwGainMode(chn, gain_mode);
			} else if (chn == responseChannel) {
				if (!autoAdjustGain) {
					auto gain = responseGainCmbIndex == 1 ? M2kAdc::LOW_GAIN_MODE
									      : M2kAdc::HIGH_GAIN_MODE;
					m2k_adc->setChnHwGainMode(chn, gain);
				} else {
					m2k_adc->setChnHwGainMode(chn, gain_mode);
				}
			}
		}
	}
}

unsigned long NetworkAnalyzer::_getBestSampleRate(double frequency, const struct iio_device *dev)
{
	QVector<unsigned long> values = SignalGenerator::get_available_sample_rates(dev);

	qSort(values.begin(), values.end(), qLess<unsigned long>());

	for (const auto &rate : values) {
		if (rate == values[0]) {
			continue;
		}

		size_t bufferSize = _getSamplesCount(frequency, rate, true);

		if (bufferSize) {
			return rate;
		}
	}

	for (const auto &rate : values) {
		if (rate == values[0]) {
			continue;
		}

		size_t bufferSize = _getSamplesCount(frequency, rate);

		if (bufferSize) {
			return rate;
		}
	}

	return 0;
}

size_t NetworkAnalyzer::_getSamplesCount(double frequency, unsigned long rate, bool perfect)
{
	size_t minBufferSize = SignalGenerator::min_buffer_size;
	size_t maxBufferSize = 128 * 1024;

	double ratio = static_cast<double>(rate) / frequency;

	if (ratio < 10.0 && rate < 75e06) {
		return 0;
	}

	if (ratio < 2.5) {
		return 0;
	}

	double fract = 0.0;
	ratio = SignalGenerator::get_best_ratio(ratio, static_cast<double>(maxBufferSize / 4), &fract);

	if (perfect && fract != 0.0) {
		return 0;
	}

	size_t size = ratio;

	/* The buffer size must be a multiple of 4 */
	while (size & 0x3) {
		size <<= 1;
	}

	/* The buffer size shouldn't be too small */
	while (size < minBufferSize) {
		size <<= 1;
	}

	if (size > maxBufferSize) {
		return 0;
	}

	return size;
}

void NetworkAnalyzer::computeFrequencyArray()
{
	boost::unique_lock<boost::mutex> lock(iterationsReadyMutex);

	iterations.clear();

	unsigned int steps = (unsigned int) samplesCount->value();
	double min_freq = startStopRange->getStartValue();
	double max_freq = startStopRange->getStopValue();
	double log10_min_freq = log10(min_freq);
	double log10_max_freq = log10(max_freq);
	double step;

	bool is_log = ui->btnIsLog->isChecked();

	if (is_log) {
		step = (log10_max_freq - log10_min_freq) / (double)(steps - 1);
	} else {
		step = (max_freq - min_freq) / (double)(steps - 1);
	}

	for (unsigned int i = 0; i < steps; ++i) {

		if (iterationsThreadCanceled) {
			return;
		}

		double frequency = 0.0;

		if (is_log) {
			frequency = pow(10.0,
					log10_min_freq + (double) i * step);
		} else {
			frequency = min_freq + (double) i * step;
		}

		unsigned long rate = _getBestSampleRate(frequency, iio_channel_get_device(dac_channels[0]));
		size_t bufferSize = _getSamplesCount(frequency, rate);

		iterations.push_back(networkIteration(frequency, rate, bufferSize));
	}

	// Needs to be invoked on the main thread
	QMetaObject::invokeMethod(this,
				  "updateNumSamples",
				  Qt::QueuedConnection,
				  Q_ARG(bool, true));

	iterationsThreadReady = true;
	lock.unlock();
	iterationsReadyCv.notify_one();
}

void NetworkAnalyzer::setFilterParameters()
{

	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);

	f11->set_enable(iio->freq_comp_filt[0][0]->get_enable());
	f12->set_enable(iio->freq_comp_filt[0][1]->get_enable());
	f21->set_enable(iio->freq_comp_filt[1][0]->get_enable());
	f22->set_enable(iio->freq_comp_filt[1][1]->get_enable());

	f11->set_TC(iio->freq_comp_filt[0][0]->get_TC());
	f12->set_TC(iio->freq_comp_filt[0][1]->get_TC());
	f21->set_TC(iio->freq_comp_filt[1][0]->get_TC());
	f22->set_TC(iio->freq_comp_filt[1][1]->get_TC());

	f11->set_filter_gain(iio->freq_comp_filt[0][0]->get_filter_gain());
	f12->set_filter_gain(iio->freq_comp_filt[0][1]->get_filter_gain());
	f21->set_filter_gain(iio->freq_comp_filt[1][0]->get_filter_gain());
	f22->set_filter_gain(iio->freq_comp_filt[1][1]->get_filter_gain());

	f11->set_sample_rate(m2k_adc->sampleRate());
	f12->set_sample_rate(m2k_adc->sampleRate());
	f21->set_sample_rate(m2k_adc->sampleRate());
	f22->set_sample_rate(m2k_adc->sampleRate());

	f11->set_high_gain(m2k_adc->chnHwGainMode(0));
	f12->set_high_gain(m2k_adc->chnHwGainMode(0));
	f21->set_high_gain(m2k_adc->chnHwGainMode(1));
	f22->set_high_gain(m2k_adc->chnHwGainMode(1));
}

void NetworkAnalyzer::goertzel()
{
	// Network Analyzer run method using the Goertzel Algorithm (single bin DFT)

	// Enable the available dac channels
	for (auto& channel : dac_channels) {
		iio_channel_enable(channel);
	}

	// Adjust the gain of the ADC channels based on sweep settings
	updateGainMode();
	setFilterParameters();

	// Wait for the iterations thread to finish
	boost::unique_lock<boost::mutex> lock(iterationsReadyMutex);
	iterationsReadyCv.wait(lock, boost::bind(&NetworkAnalyzer::isIterationsThreadReady, this));
	if (iterationsThread) {
		iterationsThread->join();
		delete iterationsThread;
		iterationsThread = nullptr;
	}

	justStarted = true;

	for (int i = 0; !m_stop && i < iterations.size(); ++i) {

		// Get current sweep settings
		unsigned long rate = iterations[i].rate;
		size_t samples_count = iterations[i].bufferSize;
		double frequency = iterations[i].frequency;

		double amplitudeValue = amplitude->value();
		double offsetValue = offset->value();

		// Create and push the generated sine waves to the DACs
		QVector<struct iio_buffer *> buffers;

		for (const auto& channel : dac_channels) {
			const struct iio_device *dev = iio_channel_get_device(channel);
			iio_device_attr_write_bool(dev, "dma_sync", true);
			struct iio_buffer *buf_dac = generateSinWave(dev,
						     frequency, amplitudeValue, offsetValue,
						     rate, samples_count);
			buffers.push_back(buf_dac);

			if (!buf_dac) {
				qCritical() << "Unable to create DAC buffer";
				break;
			}
		}

		// Sleep before DACs start
		QThread::msleep(pushDelay->value());
		for (const auto& channel : dac_channels) {
			const struct iio_device *dev = iio_channel_get_device(channel);
			iio_device_attr_write_bool(dev, "dma_sync", false);
		}


		size_t buffer_size = 0;
		size_t adc_rate = 0;

		// Compute capture params for the ADC
		computeCaptureParams(frequency, buffer_size, adc_rate);

		iio_device_attr_write_double(adc, "oversampling_ratio", 1);
		adc_dev->setSampleRate(adc_rate);

		auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);
		double corr_gain = 1.0;
		double hw_gain = 1.0;

		corr_gain = m2k_adc->chnCorrectionGain(1);
		hw_gain = m2k_adc->gainAt(m2k_adc->chnHwGainMode(1));

		if (buffer_size == 0) {
			qDebug(CAT_NETWORK_ANALYZER) << "buffer size 0";
			return;
		}

		dc_cancel1->set_buffer_size(buffer_size);
		dc_cancel2->set_buffer_size(buffer_size);

		goertzel1->set_freq(frequency);
		goertzel2->set_freq(frequency);
		goertzel1->set_len(buffer_size);
		goertzel2->set_len(buffer_size);
		goertzel1->set_rate(adc_rate);
		goertzel2->set_rate(adc_rate);


		setFilterParameters();
		// TODO: Use libm2k here

		std::vector<struct iio_channel *> adc_channels;

		unsigned int nb_channels = iio_device_get_channels_count(adc);
		for (unsigned int i = 0; i < nb_channels; i++) {
			iio_channel_disable(iio_device_get_channel(adc, i));
		}
		for (unsigned int i = 0; i < nb_channels; i++) {
			struct iio_channel *chn =
					iio_device_get_channel(adc, i);
			iio_channel_enable(chn);
			adc_channels.push_back(chn);
		}


		adc_buffer = iio_device_create_buffer(adc, buffer_size, false);

		size_t ret = iio_buffer_refill(adc_buffer);
		if (m_stop) {
			iio_buffer_destroy(adc_buffer);
			adc_buffer = nullptr;
			for (auto& buffer : buffers) {
				iio_buffer_destroy(buffer);
			}
			return;
		}

		std::vector<std::vector<short>> data;

		for (int i = 0; i < 2; ++i) {
			std::vector<short> d;
			for (int j = 0; j < buffer_size; ++j) {
				d.push_back(j);
			}
			data.push_back(d);
		}

		for (auto chn : adc_channels) {
			ptrdiff_t p_inc = iio_buffer_step(adc_buffer);
			uintptr_t p_dat;
			uintptr_t p_end = (uintptr_t)iio_buffer_end(adc_buffer);
			unsigned int i;
			for (i = 0, p_dat = (uintptr_t)iio_buffer_first(adc_buffer, adc_channels[0]);
					p_dat < p_end; p_dat += p_inc, i++)
			{
				for (unsigned int ch = 0; ch < data.size(); ch++) {
					data[ch][i] = ((int16_t*)p_dat)[ch];
				}
			}
		}

		capture1->rewind();
		capture1->set_data(data[0]);
		capture2->rewind();
		capture2->set_data(data[1]);

		{
			boost::unique_lock<boost::mutex> lock(bufferMutex);
			sink1->reset();
			sink2->reset();
		}

		captureDone = false;

		QElapsedTimer t;
		t.start();
		// Sleep before ADC capture
		QThread::msleep(captureDelay->value());

		capture_top_block->run();

		float dcOffset = 0.0;
		dcOffset = dc_cancel2->get_dc_offset();
		dcOffset = adc_sample_conv::convSampleToVolts(dcOffset,
							corr_gain, 1, 0, hw_gain);


		// Clear the iio_buffers that were created
		for (auto& buffer : buffers) {
			iio_buffer_destroy(buffer);
		}

		iio_buffer_destroy(adc_buffer);
		adc_buffer = nullptr;

		// Process was cancelled
		if (m_stop) {
			return;
		}

		QMetaObject::invokeMethod(this,
					  "_saveChannelBuffers",
					  Qt::QueuedConnection,
					  Q_ARG(double, frequency),
					  Q_ARG(double, adc_rate),
					  Q_ARG(std::vector<float>, sink1->data()),
					  Q_ARG(std::vector<float>, sink2->data()));

		// Plot the data captured for this iteration
		QMetaObject::invokeMethod(this,
					  "plot",
					  Qt::QueuedConnection,
					  Q_ARG(double, frequency),
					  Q_ARG(double, mag1),
					  Q_ARG(double, mag2),
					  Q_ARG(double, phase),
					  Q_ARG(float, dcOffset));




	}

	Q_EMIT sweepDone();
}

void NetworkAnalyzer::onFrequencyBarMoved(int pos)
{
	d_frequencyHandle->setPositionSilenty(pos);
}

void NetworkAnalyzer::toggleBufferPreview(bool toggle)
{
	bufferPreviewer->setVisible(toggle);

	ui->viewInOscBtn->setEnabled(toggle);
	ui->prevBtn->setEnabled(toggle);
	ui->nextBtn->setEnabled(toggle);
}

void NetworkAnalyzer::_saveChannelBuffers(double frequency, double sample_rate, std::vector<float> data1, std::vector<float> data2)
{
	boost::unique_lock<boost::mutex> lock(bufferMutex);

	Buffer buffer1 = Buffer(frequency, sample_rate, data1.size(), data1);
	Buffer buffer2 = Buffer(frequency, sample_rate, data2.size(), data2);

	bufferPreviewer->pushBuffers(QPair<Buffer, Buffer>(buffer1, buffer2));
}

void NetworkAnalyzer::computeCaptureParams(double frequency,
		size_t& buffer_size, size_t& adc_rate)
{
	size_t nrOfPeriods = 2;

	for (const auto& rate : sampleRates) {

		if (rate == sampleRates[0]) {
			continue;
		}

		double ratio = rate / frequency;
		buffer_size = ratio * nrOfPeriods;

		if (ratio < 2.5) {
			continue;
		}

		if (ratio < 10 && rate < sampleRates.back()) {
			continue;
		}

		while (buffer_size < 160) {
			buffer_size <<= 1;
		}

		buffer_size += (buffer_size & 1);

		adc_rate = rate;
		break;
	}
}

QPair<double, double> NetworkAnalyzer::getPhaseInterval()
{
	double maxValue = phaseMax->value();

	double lo = -360;

	double intervalMin = 0;
	double intervalMax = 0;

	intervalMax = maxValue;
	intervalMin = intervalMax - 360;

	if (intervalMin < lo) {
		intervalMin = lo;
		intervalMax = lo + 360;
	}

	return QPair<double, double>(intervalMin, intervalMax);
}

void NetworkAnalyzer::plot(double frequency, double mag1, double mag2,
			   double phase, float dcVoltage)
{
	double mag;
	static double magBonus = 0;
	static int currentSample = 0;

	if (ui->btnRefChn->isChecked()) {
		phase = -phase;

		mag = 10.0 * log10(mag2) - 10.0 * log10(mag1);
	} else {

		mag = 10.0 * log10(mag1) - 10.0 * log10(mag2);
	}

	double phase_deg = phase * 180.0 / M_PI;
	double adjusted_phase_deg = phase_deg;

	auto interval = getPhaseInterval();

	if (phase_deg > interval.second) {
		adjusted_phase_deg = (int)phase_deg - 360;
	} else if (phase_deg < interval.first) {
		adjusted_phase_deg = (int)phase_deg + 360;
	}

	static int index = 0;
	if (justStarted) {
		currentSample = 0;
		justStarted = false;
		ui->currentFrequencyLabel->setVisible(true);
		ui->currentSampleLabel->setVisible(true);
		index = 0;
		magBonus = autoUpdateGainMode(mag, magBonus, dcVoltage);
	}

	ui->currentSampleLabel->setText(QString(tr("Sample: ") + QString::number(1 + currentSample++ )
						+ " / " + QString::number(m_dBgraph.getNumSamples()) + " "));

	MetricPrefixFormatter d_cursorTimeFormatter;
	d_cursorTimeFormatter.setTwoDecimalMode(false);
	QString text = d_cursorTimeFormatter.format(frequency, "Hz", 3);
	ui->currentFrequencyLabel->setText(QString(tr("Current Frequency: ") + text));

	d_cursorTimeFormatter.setTwoDecimalMode(true);
	text = d_cursorTimeFormatter.format(dcVoltage, "V", 2);
	ui->dcLabel->setText(tr("DC Voltage: ") + text);

	bool hasError = _checkMagForOverrange(mag + magBonus);

	m_dBgraph.plot(frequency, mag + magBonus);
	m_phaseGraph.plot(frequency, adjusted_phase_deg);
	ui->xygraph->plot(phase_deg, mag + magBonus);
	ui->nicholsgraph->plot(phase_deg, mag + magBonus);

	d_frequencyHandle->triggerMove();

	int responseChanel = ui->btnRefChn->isChecked() ? 1 : 0;
	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);
	QString gain = !m2k_adc->chnHwGainMode(responseChanel) ? tr("Low") : tr("High");
	ui->gainLabel->setText(tr("Gain Mode: ") + gain);

	if (iterationStats.size() < samplesCount->value()) {
		iterationStats.push_back(NetworkIterationStats(dcVoltage, m2k_adc->chnHwGainMode(responseChanel), hasError));
	} else {
		iterationStats[index++] = NetworkIterationStats(dcVoltage, m2k_adc->chnHwGainMode(responseChanel), hasError);
		if (index == iterationStats.size()) {
			index = 0;
		}
	}

	magBonus = autoUpdateGainMode(mag, magBonus, dcVoltage);
}

bool NetworkAnalyzer::_checkMagForOverrange(double magnitude)
{
	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);
	double corr_gain = 1.0;
	double hw_gain = 1.0;

	corr_gain = m2k_adc->chnCorrectionGain(1);
	hw_gain = m2k_adc->gainAt(m2k_adc->chnHwGainMode(1));

	double vlsb = adc_sample_conv::convSampleToVolts(1,
							 corr_gain, 1, 0, hw_gain);

	double magnitudeThreshold = 20 * std::log10(3 * vlsb / amplitude->value());

	if (magnitude < magnitudeThreshold) {
		ui->errorLabel->setText(tr("Response channel voltage < 50mV!"));
		m_dBgraph.parametersOverrange(true);
		m_phaseGraph.parametersOverrange(true);
		return true;
	} else {
		ui->errorLabel->setText("");
		m_dBgraph.parametersOverrange(false);
		m_phaseGraph.parametersOverrange(false);
	}

	return false;
}

double NetworkAnalyzer::autoUpdateGainMode(double magnitude, double magnitudeGain, float dcVoltage)
{
	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);

	// compute Vout knowing that magnitude = 20 * log(Vout / Vin);
	double Vin = amplitude->value();
	double Vout = pow(10.0, (magnitude + magnitudeGain) / 20.0) * Vin;

	QPair<double, double> range = m2k_adc->inputRange(
					      M2kAdc::HIGH_GAIN_MODE);
	double hi = range.second;
	double lo = range.first;

	double hi_hi = hi + 0.1;
	double hi_lo = hi - 0.1;
	double lo_hi = lo + 0.1;
	double lo_lo = lo - 0.1;

	M2kAdc::GainMode gain;

	auto insideDeltaZone = [&](double voltage) {
		return (voltage > hi_lo && voltage < hi_hi)
				|| (voltage < lo_hi && voltage > lo_lo);
	};

	bool noAdjustAllowedUp = insideDeltaZone(Vout / 2.0 + dcVoltage);
	bool noAdjustAllowedDown = insideDeltaZone(-Vout / 2.0 + dcVoltage);

	if (Vout / 2.0 + dcVoltage >= hi_lo || -Vout / 2.0 + dcVoltage <= lo_hi) {
		if (noAdjustAllowedDown && noAdjustAllowedUp) {
			return magnitudeGain;
		}
		gain = M2kAdc::LOW_GAIN_MODE;
	}
	if (Vout / 2.0 + dcVoltage < hi_hi && -Vout / 2.0 + dcVoltage > lo_lo) {
		if (noAdjustAllowedDown || noAdjustAllowedUp) {
			return magnitudeGain;
		}
		gain = M2kAdc::HIGH_GAIN_MODE;
	}

	int responseChannel = ui->btnRefChn->isChecked() ? 1 : 0;

	double value = m2k_adc->gainAt(M2kAdc::LOW_GAIN_MODE) / m2k_adc->gainAt(M2kAdc::HIGH_GAIN_MODE);
	double magnitudeGainBetweenGainModes = 10.0 * log10(1.0 / value) - 10.0 * log10(1.0 * value);


	if (m2k_adc->chnHwGainMode(responseChannel) != gain) {
		if (autoAdjustGain) {
			m2k_adc->setChnHwGainMode(responseChannel, gain);
		} else {
			int selectedResponseGain = ui->responseGainCmb->currentIndex();
			gain = (selectedResponseGain == 1 ? M2kAdc::LOW_GAIN_MODE
							  : M2kAdc::HIGH_GAIN_MODE);
		}

		if (gain == M2kAdc::LOW_GAIN_MODE) {
			if (gain != m2k_adc->chnHwGainMode(1 - responseChannel)) {
				return magnitudeGainBetweenGainModes;
			} else {
				return 0.0;
			}
		} else {
			if (gain != m2k_adc->chnHwGainMode(1 - responseChannel)) {
				return -magnitudeGainBetweenGainModes;
			} else {
				return 0.0;
			}
		}
	}

	return magnitudeGain;
}


void NetworkAnalyzer::run()
{
	startStop(true);
}
void NetworkAnalyzer::stop()
{
	startStop(false);
}

void NetworkAnalyzer::startStop(bool pressed)
{
	m_stop = !pressed;

	if (amp1 && amp2) {
		/* FIXME: TODO: Move this into a HW class / lib M2k */
		iio_channel_attr_write_bool(amp1, "powerdown", !pressed);
		iio_channel_attr_write_bool(amp2, "powerdown", !pressed);
	}

	if (m_running == pressed) {
		return;
	}

	m_running = pressed;

	bool shouldClear = false;
	if (QObject::sender() == ui->runSingleWidget) {
		shouldClear = true;
	}

	ui->btnRefChn->setEnabled(!pressed);
	ui->btnIsLog->setEnabled(!pressed);
	samplesCount->setEnabled(!pressed);
	amplitude->setEnabled(!pressed);
	offset->setEnabled(!pressed);
	startStopRange->setEnabled(!pressed);
	ui->dcFilterBtn->setEnabled(!pressed);
	ui->responseGainCmb->setEnabled(!pressed);
	pushDelay->setEnabled(!pressed);
	captureDelay->setEnabled(!pressed);

	if (pressed) {
		if (shouldClear) {
			m_dBgraph.reset();
			m_phaseGraph.reset();
			ui->xygraph->reset();
			ui->nicholsgraph->reset();
			updateNumSamples(true);
		}
		iterationStats.clear();
		bufferPreviewer->clear();
		configHwForNetworkAnalyzing();
		m_stop = false;
		thd = QtConcurrent::run(this, &NetworkAnalyzer::goertzel);
		ui->statusLabel->setText(tr("Running"));
	} else {
		ui->statusLabel->setText(tr("Stopping..."));
		QCoreApplication::processEvents();
		m_stop = true;
		if (adc_buffer) {
			iio_buffer_cancel(adc_buffer);
		}
		thd.waitForFinished();
		m_dBgraph.sweepDone();
		m_phaseGraph.sweepDone();
		ui->statusLabel->setText(tr("Stopped"));
	}
}

struct iio_buffer *NetworkAnalyzer::generateSinWave(
	const struct iio_device *dev, double frequency,
	double amplitude, double offset,
	unsigned long rate, size_t samples_count)
{
	/* Create the IIO buffer */
	struct iio_buffer *buf = iio_device_create_buffer(
					 dev, samples_count, true);

	if (!buf) {
		return buf;
	}

	double vlsb = 1;
	double corr = 1;
	for (auto dac : dacs) {
		if (dac->iio_dac_dev() == dev) {
			vlsb = dac->vlsb();
			auto m2k_dac = std::dynamic_pointer_cast<M2kDac>
				       (dac);
			if (m2k_dac) {
				corr = m2k_dac->compTable(rate);
			}
			break;
		}
	}

	float volts_to_raw_coef = (-1 * (1 / vlsb) * 16) / corr;

	iio_device_attr_write_longlong(dev, "oversampling_ratio", 1);

	// Make sure to clear everything left from the last
	// sine generation iteration
	vector_block->reset();
	head_block->reset();
	f2s_block->set_scale(volts_to_raw_coef);

	// Setup params for sine wave generation and run
	source_block->set_sampling_freq(rate);
	source_block->set_frequency(frequency);
	source_block->set_amplitude(amplitude / 2.0);
	source_block->set_offset(offset);
	source_block->set_phase(0);
	head_block->set_length(samples_count);
	top_block->run();

	const std::vector<short>& samples = vector_block->data();
	const short *data = samples.data();

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (iio_channel_is_enabled(chn)) {
			iio_channel_write(chn, buf, data,
					  samples_count * sizeof(short));
		}
	}

	iio_device_attr_write_longlong(dev, "sampling_frequency", rate);

	iio_buffer_push(buf);

	return buf;
}

void NetworkAnalyzer::configHwForNetworkAnalyzing()
{
	auto trigger = adc_dev->getTrigger();

	if (trigger) {
		for (uint i = 0; i < trigger->numChannels(); i++) {
			trigger->setTriggerMode(i, HardwareTrigger::ALWAYS);
			trigger->setDelay(0);
		}
	}

	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);
	if (m2k_adc) {
		iio_device_attr_write_longlong(m2k_adc->iio_adc_dev(),
			"oversampling_ratio", 1);
		for (int i = 0; i < m2k_adc->numAdcChannels(); ++i) {
			m2k_adc->setChnHwOffset(i, 0);
		}
	}
}

void NetworkAnalyzer::onVbar1PixelPosChanged(int pos)
{
	d_hCursorHandle1->setPositionSilenty(pos);
}

void NetworkAnalyzer::onVbar2PixelPosChanged(int pos)
{
	d_hCursorHandle2->setPositionSilenty(pos);
}

void NetworkAnalyzer::toggleCursors(bool en)
{
	if (!en) {
		ui->btnCursors->setChecked(en);
	}

	if (d_cursorsEnabled != en) {
		d_cursorsEnabled = en;
		m_dBgraph.toggleCursors(en);
		m_phaseGraph.toggleCursors(en);
		d_hCursorHandle1->setVisible(en);
		d_hCursorHandle2->setVisible(en);
		ui->btnCursors->setEnabled(en);
	}

}

void NetworkAnalyzer::readPreferences()
{
	m_dBgraph.setShowZero(prefPanel->getNa_show_zero());
	m_phaseGraph.setShowZero(prefPanel->getNa_show_zero());
//	autoAdjustGain = prefPanel->getNaGainUpdateEnabled();
}

void NetworkAnalyzer::onGraphIndexChanged(int index)
{
	ui->stackedWidget->setCurrentIndex(index);
}
