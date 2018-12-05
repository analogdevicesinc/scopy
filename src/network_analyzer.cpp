/*
 * Copyright 2016 Analog Devices, Inc.
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

#include "logging_categories.h"
#include "dynamicWidget.hpp"
#include "network_analyzer.hpp"
#include "signal_generator.hpp"
#include "spinbox_a.hpp"
#include "osc_adc.h"
#include "hardware_trigger.hpp"
#include "ui_network_analyzer.h"
#include "filemanager.h"
#include "cancel_dc_offset_block.h"

#include <gnuradio/analog/sig_source_c.h>
#include <gnuradio/analog/sig_source_waveform.h>
#include <gnuradio/blocks/complex_to_arg.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/float_to_short.h>
#include <gnuradio/blocks/moving_average_cc.h>
#include <gnuradio/blocks/multiply_cc.h>
#include <gnuradio/blocks/multiply_conjugate_cc.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/rotator_cc.h>
#include <gnuradio/blocks/skiphead.h>
#include <gnuradio/blocks/vector_sink_f.h>
#include <gnuradio/top_block.h>
#include <boost/make_shared.hpp>

#include <algorithm>

#include <QDebug>
#include <QThread>
#include <QFileDialog>
#include <QDateTime>
#include <QElapsedTimer>
#include <QSignalBlocker>

#include <iio.h>
#include <network_analyzer_api.hpp>

/* This should go away ASAP... */
#define DAC_BIT_COUNT   12
#define INTERP_BY_100_CORR 1.168 // correction value at an interpolation by 100
#define AMPLITUDE_VOLTS	5.0

using namespace adiscope;
using namespace gr;

NetworkAnalyzer::NetworkAnalyzer(struct iio_context *ctx, Filter *filt,
				 std::shared_ptr<GenericAdc>& adc_dev,
				 QPushButton *runButton, QJSEngine *engine,
				 ToolLauncher *parent) :
	Tool(ctx, runButton, new NetworkAnalyzer_API(this), "Network Analyzer", parent),
	ui(new Ui::NetworkAnalyzer),
	adc_dev(adc_dev),
	d_cursorsEnabled(false),
	stop(true), amp1(nullptr), amp2(nullptr),
	wheelEventGuard(nullptr), wasChecked(false)
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


	connect(ui->run_button, SIGNAL(toggled(bool)),
		runButton, SLOT(setChecked(bool)));
	connect(runButton, SIGNAL(toggled(bool)),
		ui->run_button, SLOT(setChecked(bool)));
	connect(ui->run_button, SIGNAL(toggled(bool)),
		this, SLOT(startStop(bool)));

	connect(ui->single_button, &QPushButton::toggled,
		this, &NetworkAnalyzer::startStop);

	connect(this, &NetworkAnalyzer::sweepDone,
	[=]() {
		if (ui->run_button->isChecked()) {
			startStop(true);
			return;
		}

		ui->single_button->setChecked(false);

		dynamic_cast<CustomPushButton *>(this->runButton())->setChecked(false);
	});


	ui->rightMenu->setMaximumWidth(0);

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
	m_dBgraph.setXTitle("Frequency (Hz)");
	m_dBgraph.setYTitle("Magnitude(dB)");
	m_dBgraph.setXMin(1000.000000);
	m_dBgraph.setXMax(50000.000000);
	m_dBgraph.setYMin(-90.000000);
	m_dBgraph.setYMax(10.000000);
	m_dBgraph.useLogFreq(true);

	m_phaseGraph.setColor(QColor(144,19,254));
	m_phaseGraph.setYTitle("Phase (°)");
	m_phaseGraph.setYUnit("°");
	m_phaseGraph.setXMin(1000.000000);
	m_phaseGraph.setXMax(50000.000000);
	m_phaseGraph.setYMin(-180.000000);
	m_phaseGraph.setYMax(180.000000);
	m_phaseGraph.useLogFreq(true);

	samplesCount = new ScaleSpinButton({
		{"samples",1e0},
	}, "Samples count", 10, 1000, false, false, this);
	samplesCount->setValue(1000);

	start_freq = new ScaleSpinButton({
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	},"Min Freq", 1e0, 5e7, false, false, this,
	{1, 2.5, 5, 7.5});
	start_freq->setValue(1000);

	stop_freq = new ScaleSpinButton({
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	},"Max Freq", 1e0, 5e7, false, false, this,
	{1, 2.5, 5, 7.5});
	stop_freq->setValue(50000);

	start_freq->enableNumberSeriesRebuild(false);
	stop_freq->enableNumberSeriesRebuild(false);

	span_freq = new ScaleSpinButton({
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	}, "Span", 1e0, 5e7, false, false, this,
	{1, 2.5, 5, 7.5});
	ui->spanFreqLayout->addWidget(span_freq);

	center_freq = new ScaleSpinButton({
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	}, "Center", 1e0, 5e7, false, false, this,
	{1, 2.5, 5, 7.5});
	ui->centerFreqLayout->addWidget(center_freq);

	center_freq->enableNumberSeriesRebuild(false);
	span_freq->enableNumberSeriesRebuild(false);

	amplitude = new ScaleSpinButton({
		{"μVolts",1e-6},
		{"mVolts",1e-3},
		{"Volts",1e0}
	},"Amplitude", 1e-6, 1e1, false, false, this);
	amplitude->setValue(1);

	offset = new PositionSpinButton({
		{"μVolts",1e-6},
		{"mVolts",1e-3},
		{"Volts",1e0}
	},"Offset", -5, 5, false, false, this);

	offset->setValue(0);

	magMax = new PositionSpinButton({
		{"dB",1e0}
	}, "Max. Magnitude", -120, 120, false, false, this);
	magMax->setValue(10);

	magMin = new PositionSpinButton({
		{"dB",1e0}
	}, "Min. Magnitude", -120, 120, false, false, this);
	magMin->setValue(-90);

	phaseMax = new PositionSpinButton({
		{"°",1e0}
	}, "Max. Phase", -360, 360, false, false, this);
	phaseMax->setValue(180);

	phaseMin = new PositionSpinButton({
		{"°",1e0}
	}, "Min. Phase", -360, 360, false, false, this);
	phaseMin->setValue(-180);


	ui->samplesCountLayout->addWidget(samplesCount);
	ui->minFreqLayout->addWidget(start_freq);
	ui->maxFreqLayout->addWidget(stop_freq);
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

	connect(start_freq, &ScaleSpinButton::valueChanged,
		this, &NetworkAnalyzer::onStartStopFrequencyChanged);
	connect(stop_freq, &ScaleSpinButton::valueChanged,
		this, &NetworkAnalyzer::onStartStopFrequencyChanged);
	connect(center_freq, &ScaleSpinButton::valueChanged,
		this, &NetworkAnalyzer::onCenterSpanFrequencyChanged);
	connect(span_freq, &ScaleSpinButton::valueChanged,
		this, &NetworkAnalyzer::onCenterSpanFrequencyChanged);

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

	ui->gridLayout_plots->addWidget(&m_dBgraph,0,0,1,1);
	ui->gridLayout_plots->addWidget(&m_phaseGraph,1,0,1,1);
	ui->gridLayout_plots->addWidget(d_bottomHandlesArea,2,0,1,1);

	d_hCursorHandle1 = new PlotLineHandleH(
		QPixmap(":/icons/h_cursor_handle.svg"),
		d_bottomHandlesArea);
	d_hCursorHandle2 = new PlotLineHandleH(
		QPixmap(":/icons/h_cursor_handle.svg"),
		d_bottomHandlesArea);

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

	stop_freq->setMaxValue((double) max_samplerate / 3.0 - 1.0);
	center_freq->setMinValue(2);
	center_freq->setMaxValue((double) max_samplerate / 3.0 - 2.0);
	span_freq->setMinValue(1);
	span_freq->setMaxValue((double) max_samplerate / 3.0 - 1.0);

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
		ui->transLabel->setText("Transparency " + QString::number(value) + "%");
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

		QString fileNameHint = "Scopy-" + api->objectName() + "-" + date + ".png";

		QString fileName = QFileDialog::getSaveFileName(this,
				   tr("Save to"), fileNameHint,
				   tr({"(*.png);;"}));
		painter.end();
		img.invertPixels(QImage::InvertRgb);
		img.save(fileName, 0, -1);
	});

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

	// Create the blocks that are used to generate sine waves
	top_block = make_top_block("Signal Generator");
	source_block = analog::sig_source_f::make(1, analog::GR_SIN_WAVE,
			1, 1, 1);

	// DAC_RAW = (-Vout * 2^11) / 5V
	// Multiplying with 16 because the HDL considers the DAC data as 16 bit
	// instead of 12 bit(data is shifted to the left).
	f2s_block = blocks::float_to_short::make(1,
			-1 * (1 << (DAC_BIT_COUNT - 1)) /
			AMPLITUDE_VOLTS * 16 / INTERP_BY_100_CORR);
	head_block = blocks::head::make(sizeof(short), 1);
	vector_block = blocks::vector_sink_s::make();

	// Connect the blocks for the sine wave generation
	top_block->connect(source_block, 0, f2s_block, 0);
	top_block->connect(f2s_block, 0, head_block, 0);
	top_block->connect(head_block, 0, vector_block, 0);

	// Get the available sample rates for the m2k-adc
	// Make sure the values are sorted in ascending order (1000,..,100e6)
	sampleRates = SignalGenerator::get_available_sample_rates(adc);
	qSort(sampleRates.begin(), sampleRates.end(), qLess<unsigned long>());
	fixedRate = sampleRates[0];
}

NetworkAnalyzer::~NetworkAnalyzer()
{
	disconnect(prefPanel,&Preferences::notify,this,
		   &NetworkAnalyzer::readPreferences);
	ui->run_button->setChecked(false);

	if (saveOnExit) {
		api->save(*settings);
	}

	top_block->disconnect_all();

	delete api;

	delete ui;
}

void NetworkAnalyzer::onStartStopFrequencyChanged(double value)
{
	if (QObject::sender() == start_freq) {
		qDebug() << " start freq changed to: " << value;
	} else {
		qDebug() << " stop freq changed to: " << value;
	}

	double start = start_freq->value();
	double stop = stop_freq->value();

	start_freq->setMaxValue(stop - 1);
	stop_freq->setMinValue(start + 1);

	double span = stop - start;
	double center = start + (span / 2);

	span_freq->silentSetValue(span);
	center_freq->silentSetValue(center);

	// Update plot settings
	m_dBgraph.setXMin(start);
	m_dBgraph.setXMax(stop);
	m_phaseGraph.setXMin(start);
	m_phaseGraph.setXMax(stop);
	updateNumSamples();
}

void NetworkAnalyzer::onCenterSpanFrequencyChanged(double value)
{
	double span = span_freq->value();
	double center = center_freq->value();
	double start = center - (span / 2);
	double stop = center + (span / 2);

	if (QObject::sender() == center_freq) {
		// Center value was changed by the user, so we
		// check if the span value is valid, if not we
		// adjust it
		if (start < 0) {
			start = 1;
			span = (center - start) * 2;
			stop = center + (span / 2);
		} else if (stop > stop_freq->maxValue()) {
			stop = stop_freq->maxValue();
			span = (stop - center) * 2;
			start = center - (span / 2);
		}
		span_freq->silentSetValue(span);
	} else {
		// Span value was changed by the user, so we
		// check if the center value is valid, if not we
		// adjust it
		if (start < 0) {
			start = 1;
			center = start + (span / 2);
			stop = center + (span / 2);
		} else if (stop > stop_freq->maxValue()) {
			stop = stop_freq->maxValue();
			center = stop - (span / 2);
			start = center - (span / 2);
		}
		center_freq->silentSetValue(center);
	}

	start_freq->silentSetMaxValue(stop - 1);
	stop_freq->silentSetMinValue(start + 1);

	start_freq->silentSetValue(start);
	stop_freq->silentSetValue(stop);

	// Update plot settings
	m_dBgraph.setXMin(start);
	m_dBgraph.setXMax(stop);
	m_phaseGraph.setXMin(start);
	m_phaseGraph.setXMax(stop);
	updateNumSamples();
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
	auto export_dialog(new QFileDialog(this));
	export_dialog->setWindowModality(Qt::WindowModal);
	export_dialog->setFileMode(QFileDialog::AnyFile);
	export_dialog->setAcceptMode(QFileDialog::AcceptSave);
	export_dialog->setNameFilters({"Comma-separated values files (*.csv)",
				       "Tab-delimited values files (*.txt)"});

	if (export_dialog->exec()) {
		FileManager fm("Network Analyzer");

		fm.open(export_dialog->selectedFiles().at(0), FileManager::EXPORT);

		fm.setAdditionalInformation(ui->btnRefChn->isChecked() ?
					    "Reference channel: 1" : "Reference channel: 2");

		fm.save(m_dBgraph.getXAxisData(), "Frequency(Hz)");
		fm.save(m_dBgraph.getYAxisData(), "Magnitude(dB)");
		fm.save(m_phaseGraph.getYAxisData(), "Phase(°)");

		fm.performWrite();
	}
}

void NetworkAnalyzer::computeFrequencyArray()
{
	QVector<double> ret;
	iterations.clear();

	unsigned int steps = (unsigned int) samplesCount->value();
	double min_freq = start_freq->value();
	double max_freq = stop_freq->value();
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
		double frequency;

		if (is_log) {
			frequency = pow(10.0,
					log10_min_freq + (double) i * step);
		} else {
			frequency = min_freq + (double) i * step;
		}

		ret.push_back(frequency);
	}

	QVector<double> adjFreq;
	adjFreq.resize(ret.size());

	auto sampleRates = SignalGenerator::get_available_sample_rates(
				   iio_channel_get_device(dac_channels[0]));
	qSort(sampleRates.begin(), sampleRates.end(), qLess<unsigned int>());

	uint32_t lastRate = sampleRates[0];

	for (int i = 0; i < ret.size(); ++i) {
		double freq = ret[i];

		double next = (i != ret.size() - 1) ? ret[i + 1] : sampleRates.back();
		double prev = (i > 0) ? ret[i - 1] : ret[i];

		next -= ((next - freq) * 0.5);
		prev += ((freq - prev) * 0.5);

		bool stop = false;
		adjFreq[i] = ret[i];

		for (int j = 1; j < 1024 && !stop; ++j) {
			double integral;
			double dummy;
			double fract = modf(1.0/(double)j,&dummy);

			for (auto rate : sampleRates) {
				if (rate < lastRate) {
					continue;
				}

				modf(rate / freq, &integral);

				if (integral < 2.5) {
					continue;
				}

				if (integral < 14 && lastRate < sampleRates.back()) {
					continue;
				}

				double newFrequency = rate / (integral + fract);

				if (newFrequency > prev && newFrequency < next && (integral * j > 2)) {
					stop = true;
					adjFreq[i] = newFrequency;
					size_t bufferSize = integral * j;

					while (bufferSize & 0x3) {
						bufferSize <<= 1;
					}

					while (bufferSize < 1280) {
						bufferSize <<= 1;
					}

					iterations.push_back(networkIteration(newFrequency, rate, bufferSize));
					lastRate = rate;
					break;
				}
			}

		}
	}

	// Needs to be invoked on the main thread
	QMetaObject::invokeMethod(this,
				  "updateNumSamples",
				  Qt::QueuedConnection,
				  Q_ARG(bool, true));
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
}

void NetworkAnalyzer::updateGainMode()
{
	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);

	if (m2k_adc) {
		double sweep_ampl = amplitude->value();
		QPair<double, double> range = m2k_adc->inputRange(
						      M2kAdc::HIGH_GAIN_MODE);
		double threshold = range.second - range.first;
		M2kAdc::GainMode gain_mode;

		if (sweep_ampl > threshold) {
			gain_mode = M2kAdc::LOW_GAIN_MODE;
		} else {
			gain_mode = M2kAdc::HIGH_GAIN_MODE;
		}

		for (int chn = 0; chn < m2k_adc->numAdcChannels(); chn++) {
			m2k_adc->setChnHwGainMode(chn, gain_mode);
		}
	}
}

void NetworkAnalyzer::run()
{
	// Enable the available dac channels
	for (auto& channel : dac_channels) {
		iio_channel_enable(channel);
	}

	// Adjust the gain of the ADC channels based on sweep settings
	updateGainMode();

	// Compute the frequency for each iteration
	computeFrequencyArray();

	fixedRate = sampleRates[0];

	for (unsigned int i = 0; !stop && i < iterations.size(); ++i) {


		unsigned long rate = iterations[i].rate;
		size_t samples_count = iterations[i].bufferSize;
		double frequency = iterations[i].frequency;

		double amplitudeValue = amplitude->value();
		double offsetValue = offset->value();


		// Create and push the generated sine waves on
		// the devices
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

			iio_device_attr_write_bool(dev, "dma_sync", false);
		}

		size_t buffer_size = 0;
		size_t adc_rate = 0;

		// Compute capture params;
		computeCaptureParams(frequency, buffer_size, adc_rate);

		uint32_t maxSR = 100e6;
		adc_dev->setSampleRate(maxSR);
		iio_device_attr_write_double(adc, "oversampling_ratio", maxSR/adc_rate);

		if (buffer_size == 0) {
			qDebug(CAT_NETWORK_ANALYZER) << "buffer size 0";
			return;
		}

		// Lock the flowgraph if we are already started
		bool started = iio->started();

		if (started) {
			iio->lock();
		}

		auto dc1 = gnuradio::get_initial_sptr(
					new cancel_dc_offset_block(buffer_size, true));
		auto dc2 = gnuradio::get_initial_sptr(
					new cancel_dc_offset_block(buffer_size, true));

		auto f2c1 = blocks::float_to_complex::make();
		auto f2c2 = blocks::float_to_complex::make();
		auto id1 = iio->connect(dc1, 0, 0, true,
					buffer_size);
		auto id2 = iio->connect(dc2, 1, 0, true,
					buffer_size);
		iio->connect(dc1, 0, f2c1, 0);
		iio->connect(dc2, 0, f2c2, 0);

		auto null = blocks::null_source::make(sizeof(float));
		iio->connect(null, 0, f2c1, 1);
		iio->connect(null, 0, f2c2, 1);

		auto cosine = analog::sig_source_c::make(
				      (unsigned int) adc_rate,
				      gr::analog::GR_COS_WAVE, -frequency, 1.0);

		auto mult1 = blocks::multiply_cc::make();
		iio->connect(f2c1, 0, mult1, 0);
		iio->connect(cosine, 0, mult1, 1);

		auto mult2 = blocks::multiply_cc::make();
		iio->connect(f2c2, 0, mult2, 0);
		iio->connect(cosine, 0, mult2, 1);

		auto signal = boost::make_shared<signal_sample>();
		auto conj = blocks::multiply_conjugate_cc::make();

		auto avg1 = blocks::moving_average_cc::make(buffer_size,
				2.0 / buffer_size, buffer_size);
		auto skiphead3 = blocks::skiphead::make(sizeof(gr_complex),
							buffer_size - 1);
		auto c2m1 = blocks::complex_to_mag_squared::make();

		iio->connect(mult1, 0, avg1, 0);
		iio->connect(avg1, 0, skiphead3, 0);
		iio->connect(skiphead3, 0, c2m1, 0);
		iio->connect(skiphead3, 0, conj, 0);
		iio->connect(c2m1, 0, signal, 0);

		auto avg2 = blocks::moving_average_cc::make(buffer_size,
				2.0 / buffer_size, buffer_size);
		auto skiphead4 = blocks::skiphead::make(sizeof(gr_complex),
							buffer_size - 1);
		auto c2m2 = blocks::complex_to_mag_squared::make();

		iio->connect(mult2, 0, avg2, 0);
		iio->connect(avg2, 0, skiphead4, 0);
		iio->connect(skiphead4, 0, c2m2, 0);
		iio->connect(skiphead4, 0, conj, 1);
		iio->connect(c2m2, 0, signal, 1);

		auto c2a = blocks::complex_to_arg::make();
		iio->connect(conj, 0, c2a, 0);
		iio->connect(c2a, 0, signal, 2);

		bool got_it = false;
		float mag1 = 0.0f, mag2 = 0.0f, phase = 0.0f;

		connect(&*signal, &signal_sample::triggered,
		[&](const std::vector<float> values) {
			mag1 = values[0];
			mag2 = values[1];
			phase = values[2];
			got_it = true;
		});

		QElapsedTimer t;
		t.start();

		iio->start(id1);
		iio->start(id2);


		if (started) {
			iio->unlock();
		}

		// Wait for the signal_sample sink block to capture the data
		do {
			QCoreApplication::processEvents();
			QThread::msleep(1);

			if (!(ui->run_button->isChecked() ||
			      ui->single_button->isChecked())) {
				break;
			}
		} while (!got_it);

		iio->stop(id1);
		iio->stop(id2);

		std::cout << "For freq: " << frequency << " buffer_size: " << buffer_size
			  << " Rate: " << adc_rate << " Took: " << t.elapsed() / 1000.0
			  << " s" << std::endl;

		started = iio->started();

		if (started) {
			iio->lock();
		}

		iio->disconnect(id1);
		iio->disconnect(id2);

		if (started) {
			iio->unlock();
		}

		// Clear the iio_buffers that were created
		for (auto& buffer : buffers) {
			iio_buffer_destroy(buffer);
		}

		// Process was cancelled
		if (!got_it) {
			return;
		}


		// Plot the data captured for this iteration
		QMetaObject::invokeMethod(this,
					  "plot",
					  Qt::QueuedConnection,
					  Q_ARG(double, frequency),
					  Q_ARG(double, mag1),
					  Q_ARG(double, mag2),
					  Q_ARG(double, phase));

	}

	Q_EMIT sweepDone();
}

void NetworkAnalyzer::computeCaptureParams(double frequency,
		size_t& buffer_size, size_t& adc_rate)
{
	adc_rate = fixedRate;
	size_t nrOfPeriods = 2;

	for (const auto& rate : sampleRates) {

		double ratio = rate / frequency;
		buffer_size = ratio * nrOfPeriods;

		if (rate < fixedRate) {
			continue;
		}

		if (ratio < 2.5) {
			continue;
		}

		if (ratio < 14 && fixedRate < sampleRates.back()) {
			continue;
		}

		while (buffer_size & 0x3) {
			buffer_size <<= 1;
		}

		while (buffer_size < 1024) {
			buffer_size <<= 1;
		}

		adc_rate = rate;
		fixedRate = rate;
		break;
	}
}

void NetworkAnalyzer::plot(double frequency, double mag1, double mag2,
			   double phase)
{
	double mag;

	if (ui->btnRefChn->isChecked()) {
		phase = -phase;
		mag = 10.0 * log10(mag2) - 10.0 * log10(mag1);
	} else {
		mag = 10.0 * log10(mag1) - 10.0 * log10(mag2);
	}

	double phase_deg = phase * 180.0 / M_PI;
	double adjusted_phase_deg = phase_deg;

//	if (phase_deg > phaseMax->value()) {
//		adjusted_phase_deg = (int)phase_deg - 360;
//	} else if (phase_deg < phaseMin->value()) {
//		adjusted_phase_deg = (int)phase_deg + 360;
//	}

	m_dBgraph.plot(frequency, mag);
	m_phaseGraph.plot(frequency, adjusted_phase_deg);
	ui->xygraph->plot(frequency, mag);
	ui->nicholsgraph->plot(phase_deg, mag);
}

void NetworkAnalyzer::startStop(bool pressed)
{

	QPushButton *btn = dynamic_cast<QPushButton *>(QObject::sender());

	if (btn) {
		setDynamicProperty(btn, "running", pressed);

		bool runToSingle = (btn == ui->single_button) & ui->run_button->isChecked();
		bool singleToRun = (btn == ui->run_button) & ui->single_button->isChecked();

		if (runToSingle) {
			{
				const QSignalBlocker blocker(ui->run_button);
				ui->run_button->setChecked(false);
			}
			setDynamicProperty(ui->run_button, "running", false);
			return;
		} else if (singleToRun) {
			{
				const QSignalBlocker blocker(ui->single_button);
				ui->single_button->setChecked(false);
			}
			setDynamicProperty(ui->single_button, "running", false);
			return;
		}
	}

	stop = !pressed;

	if (amp1 && amp2) {
		/* FIXME: TODO: Move this into a HW class / lib M2k */
		iio_channel_attr_write_bool(amp1, "powerdown", !pressed);
		iio_channel_attr_write_bool(amp2, "powerdown", !pressed);
	}

	ui->btnRefChn->setEnabled(!pressed);
	ui->btnIsLog->setEnabled(!pressed);
	stop_freq->setEnabled(!pressed);
	start_freq->setEnabled(!pressed);
	samplesCount->setEnabled(!pressed);
	amplitude->setEnabled(!pressed);
	offset->setEnabled(!pressed);
	center_freq->setEnabled(!pressed);
	span_freq->setEnabled(!pressed);

	if (pressed) {
		if (btn) {
			m_dBgraph.reset();
			m_phaseGraph.reset();
			ui->xygraph->reset();
			ui->nicholsgraph->reset();
			updateNumSamples();
			configHwForNetworkAnalyzing();
		}

		thd = QtConcurrent::run(this, &NetworkAnalyzer::run);
	} else {
		btn->setEnabled(false);
		btn->setText("Stopping");
		QCoreApplication::processEvents();
		thd.waitForFinished();
		btn->setEnabled(true);
		m_dBgraph.sweepDone();
		m_phaseGraph.sweepDone();
	}

	if (btn) {
		setDynamicProperty(btn, "running", pressed);
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


	iio_device_attr_write_longlong(dev, "oversampling_ratio", 1);

	// Make sure to clear everything left from the last
	// sine generation iteration
	vector_block->reset();
	head_block->reset();

	// Setup params for sine wave generation and run
	source_block->set_sampling_freq(rate);
	source_block->set_frequency(frequency);
	source_block->set_amplitude(amplitude / 2.0);
	source_block->set_offset(offset);
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
		}
	}

	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc_dev);
//	if (m2k_adc) {
//		iio_device_attr_write_longlong(m2k_adc->iio_adc_dev(),
//			"oversampling_ratio", 1);
//	}
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
}

void NetworkAnalyzer::onGraphIndexChanged(int index)
{
	ui->stackedWidget->setCurrentIndex(index);
}
