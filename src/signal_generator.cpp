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
#include "signal_generator.hpp"
#include "spinbox_a.hpp"
#include "ui_signal_generator.h"
#include "channel_widget.hpp"

#include <cmath>

#include <QBrush>
#include <QFileDialog>
#include <QFileInfo>
#include <QPalette>
#include <QSharedPointer>
#include <QElapsedTimer>

#include <gnuradio/analog/sig_source.h>
#include <gnuradio/analog/sig_source_waveform.h>
#include <gnuradio/analog/noise_source.h>
#include <gnuradio/analog/noise_type.h>
#include <gnuradio/analog/rail_ff.h>
#include <gnuradio/blocks/delay.h>
#include <gnuradio/blocks/file_source.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/wavfile_source.h>
#include <gnuradio/blocks/float_to_short.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/int_to_float.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/blocks/repeat.h>
#include <gnuradio/blocks/keep_one_in_n.h>
#include <gnuradio/blocks/nop.h>
#include <gnuradio/blocks/copy.h>
#include <gnuradio/blocks/skiphead.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/null_source.h>
#include <iio/device_sink.h>
#include <scopy/math.h>
#include <scopy/trapezoidal.h>

#ifdef MATLAB_SUPPORT_SIGGEN
#include <matio.h>
#endif

#include <iio.h>

#include "signal_generator_api.hpp"

/* libm2k includes */
#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2kexceptions.hpp>

#define NB_POINTS	32768
#define DAC_BIT_COUNT   12
#define INTERP_BY_100_CORR 1.168 // correction value at an interpolation by 100

#define AMPLITUDE_VOLTS	5.0
#define MULTIPLY_CT	4
#define FREQUENCY_CT	40

using namespace adiscope;
using namespace gr;
using namespace libm2k::context;
using namespace libm2k::analog;

enum {
	DATA_IIO_PTR,
	DATA_SIGNAL_STRUCT,
	DATA_TIME_BLOCK,
};

Q_DECLARE_METATYPE(QSharedPointer<signal_generator_data>);

bool SignalGenerator::riffCompare(riff_header_t& ptr,const char *id2)
{
	const char riff[]="RIFF";

	for (uint8_t i=0; i<4; i++)
		if (ptr.riff[i]!=riff[i]) {
			return false;
		}

	for (uint8_t i=0; i<4; i++)
		if (ptr.id[i]!=id2[i]) {
			return false;
		}

	return true;
}

bool SignalGenerator::chunkCompare(chunk_header_t& ptr,const char *id2)
{
	for (uint8_t i=0; i<4; i++)
		if (ptr.id[i]!=id2[i]) {
			return false;
		}

	return true;
}

SignalGenerator::SignalGenerator(struct iio_context *_ctx, Filter *filt,
				 ToolMenuItem *toolMenuItem, QJSEngine *engine, ToolLauncher *parent) :
	Tool(_ctx, toolMenuItem, new SignalGenerator_API(this), "Signal Generator",
	     parent),
	ui(new Ui::SignalGenerator),
	time_block_data(new adiscope::time_block_data),
	m_m2k_context(m2kOpen(ctx, "")),
	m_m2k_analogout(m_m2k_context->getAnalogOut()),
	nr_of_periods(2),
	currentChannel(0), sample_rate(0),
	settings_group(new QButtonGroup(this)),nb_points(NB_POINTS),
	channels_group(new QButtonGroup(this)),
	m_maxNbOfSamples(4 * 1024 * 1024)
{
	zoomT1=0;
	zoomT2=1;
	ui->setupUi(this);
	ui->run_button->enableSingleButton(false);
	this->setAttribute(Qt::WA_DeleteOnClose, true);

	this->plot = new OscilloscopePlot(this);

	QVector<struct iio_channel *> iio_channels;

	for (int i = 0; i < m_m2k_analogout->getNbChannels(); i++) {

		unsigned long dev_sample_rate = m_m2k_analogout->getMaximumSamplerate(i);

		if (dev_sample_rate > sample_rate) {
			sample_rate = dev_sample_rate;
		}
	}

	max_sample_rate = sample_rate;

	/* Create waveform control widgets */
	phase = new PhaseSpinButton({
	{tr("deg"),1},
	{tr("π rad"),180},
	{tr("ns"),1e-9},
	{tr("μs"),1e-6},
	{tr("ms"),1e-3},
	{tr("s"),1e0}
	}, tr("Phase"), 0, 360, true, true, this);

	amplitude = new ScaleSpinButton({
	{tr("μVolts p-p"),1e-6},
	{tr("mVolts p-p"),1e-3},
	{tr("Volts p-p"),1e0}
	}, tr("Amplitude"), 0.000001, 10, true, true, this);

	offset = new PositionSpinButton({
	{tr("μVolts"),1e-6},
	{tr("mVolts"),1e-3},
	{tr("Volts"),1e0}
	}, tr("Offset"), -5, 5, true, true, this);

	frequency = new ScaleSpinButton({
	{tr("mHz"),1e-3},
	{tr("Hz"),1e0},
	{tr("kHz"),1e3},
	{tr("MHz"),1e6}
	},tr("Frequency"), 0.001, 0.0, true, false, this);

	/* Create stairstep waveform control widgets*/
	stepsUp = new PositionSpinButton({
		{"steps",1e0},
		}, tr("Rising"),1,1024,true,false,this);

	stepsDown = new PositionSpinButton({
		{"steps",1e0},
		}, tr("Falling"),1,1024,true,false,this);

	stairPhase = new PositionSpinButton({
		{"samples",1e0}
	}, tr("Phase"), 0, 1024, true, false, this);
	stairPhase->setFineModeAvailable(false);



	/* Create trapezoidal waveform control widgets */
	riseTime = new ScaleSpinButton({
	{tr("ns"),1e-9},
	{tr("μs"),1e-6},
	{tr("ms"),1e-3},
	{tr("s"),1e0}
	},tr("Rise Time"), 0, 10, true, false, this);

	fallTime = new ScaleSpinButton({
	{tr("ns"),1e-9},
	{tr("μs"),1e-6},
	{tr("ms"),1e-3},
	{tr("s"),1e0}
	},tr("Fall Time"), 0, 10, true, false, this);

	holdHighTime = new ScaleSpinButton({
	{tr("ns"),1e-9},
	{tr("μs"),1e-6},
	{tr("ms"),1e-3},
	{tr("s"),1e0}
	},tr("High Time"), 0, 10, true, false, this);

	holdLowTime = new ScaleSpinButton({
	{tr("ns"),1e-9},
	{tr("μs"),1e-6},
	{tr("ms"),1e-3},
	{tr("s"),1e0}
	},tr("Low Time"), 0, 10, true, false, this);

	/* Create file control widgets */

	filePhase = new PositionSpinButton({
		{"samples",1e0}
	}, tr("Phase"), 0.0, 360.0, true, false, this);
	filePhase->setFineModeAvailable(false);

	fileOffset = new PositionSpinButton({
	{tr("μVolts"),1e-6},
	{tr("mVolts"),1e-3},
	{tr("Volts"),1e0}
	}, tr("Offset"), -5, 5, true, true, this);

	fileSampleRate = new ScaleSpinButton({
		{"mHz",1e-3},
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	},tr("SampleRate"), 0.001, 0.0, true, false, this);
	fileSampleRate->setIntegerDivider(75000000);

	fileAmplitude = new ScaleSpinButton({
	{tr("μVolts"),1e-6},
	{tr("mVolts"),1e-3},
	{tr("Volts"),1e0}
	}, tr("Amplitude"), 0.000001, 10, true, true, this);

	mathSampleRate =  new ScaleSpinButton({
		{"mHz",1e-3},
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	},tr("SampleRate"), 0.001, 75000000.0, true, false, this);

	mathSampleRate->setIntegerDivider(75000000);

	mathRecordLength = new ScaleSpinButton({
		{"ns",1e-9},
		{"μs",1e-6},
		{"ms",1e-3},
		{"s",1}
		}, tr("Record Length"), 1e-9, 100.0, true, false, this, {1,2,5});

	noiseAmplitude = new ScaleSpinButton({
	{tr("μVolts"),1e-6},
	{tr("mVolts"),1e-3},
	{tr("Volts"),1e0}
	}, tr("Amplitude"), 0.000001, 10, true, true, this);

	constantValue = new PositionSpinButton({
	{tr("mVolts"),1e-3},
	{tr("Volts"),1e0}
	}, tr("Value"), -5, 5, true, true, this);

	dutycycle = new PositionSpinButton({
		{"%",1e0}
	}, tr("Duty Cycle"), -5, 100, true, false, this);

	ui->waveformGrid->addWidget(amplitude, 0, 0, 1, 1);
	ui->waveformGrid->addWidget(offset, 0, 1, 1, 1);
	ui->waveformGrid->addWidget(frequency, 1, 0, 1, 1);
	ui->waveformGrid->addWidget(phase, 1, 1, 1, 1);
	ui->waveformGrid->addWidget(dutycycle, 2, 1, 1, 1);

	ui->gridLayout_2->addWidget(riseTime, 0, 0, 1, 1);
	ui->gridLayout_2->addWidget(holdHighTime, 0, 1, 1, 1);
	ui->gridLayout_2->addWidget(fallTime, 1, 0, 1, 1);
	ui->gridLayout_2->addWidget(holdLowTime, 1, 1, 1, 1);

	ui->waveformGrid->addWidget(stepsUp,2,0,1,1);
	ui->waveformGrid->addWidget(stepsDown,2,1,1,1);
	ui->waveformGrid->addWidget(stairPhase,1,1,1,1);

	ui->waveformGrid_2->addWidget(fileAmplitude, 0, 0, 1, 1);
	ui->waveformGrid_2->addWidget(fileOffset, 0, 1, 1, 1);
	ui->waveformGrid_2->addWidget(fileSampleRate, 1, 0, 1, 1);
	ui->waveformGrid_2->addWidget(filePhase, 1, 1, 1, 1);

	ui->verticalLayout_3->insertWidget(0, mathRecordLength);
	ui->verticalLayout_3->insertWidget(1, mathSampleRate);

	ui->horizontalLayout_5->insertWidget(1, noiseAmplitude);
	ui->gridLayout->addWidget(constantValue,0,0,1,1);

	/* Max amplitude by default */
	amplitude->setValue(amplitude->maxValue());

	/* Set max frequency according to max sample rate */
	fileSampleRate->setMinValue(0.1);
	frequency->setMaxValue(max_frequency);
	fileSampleRate->setMaxValue(sample_rate);

	mathSampleRate->setMinValue(1);
	mathSampleRate->setMaxValue(sample_rate);
	mathSampleRate->setValue(1000000);

	/* (lowest freq * 100 * 1000) frequency by default */
	frequency->setValue(frequency->minValue() * 1000 * 1000.0);
	fileSampleRate->setValue(fileSampleRate->minValue() * 100 * 1000.0);
	fileAmplitude->setValue(1);
	fileOffset->setValue(0);
	filePhase->setValue(0);

	fileAmplitude->setDisabled(true);
	filePhase->setDisabled(true);
	fileOffset->setDisabled(true);
	fileSampleRate->setDisabled(true);

	fallTime->setMinValue(0.00000001);
	riseTime->setMinValue(0.00000001);
	holdHighTime->setMinValue(0.00000001);
	holdLowTime->setMinValue(0.00000001);

	stepsUp->setMinValue(1);
	stepsDown->setMinValue(1);
	stairPhase->setMinValue(0);

	fallTime->setValue(0.25);
	riseTime->setValue(0.25);
	holdHighTime->setValue(0.25);
	holdLowTime->setValue(0.25);

	stepsUp->setValue(5);
	stepsDown->setValue(5);
	stepsUp->setVisible(false);
	stepsDown->setVisible(false);
	stairPhase->setValue(0);
	stairPhase->setVisible(false);

	dutycycle->setValue(50);
	dutycycle->setVisible(false);
	ui->wtrapezparams->setVisible(false);
	mathRecordLength->setValue(mathRecordLength->minValue() * 100 * 1000.0);

	ui->cbNoiseType->setCurrentIndex(0);
	noiseAmplitude->setMinValue(1e-06);
	noiseAmplitude->setValue(noiseAmplitude->minValue());
	ui->btnNoiseCollapse->setVisible(true);
	ui->cbNoiseType->setItemData(SG_NO_NOISE,0);
	ui->cbNoiseType->setItemData(SG_UNIFORM_NOISE, analog::GR_UNIFORM);
	ui->cbNoiseType->setItemData(SG_GAUSSIAN_NOISE, analog::GR_GAUSSIAN);
	ui->cbNoiseType->setItemData(SG_LAPLACIAN_NOISE, analog::GR_LAPLACIAN);
	ui->cbNoiseType->setItemData(SG_IMPULSE_NOISE, analog::GR_IMPULSE);

	connect(ui->btnNoiseCollapse,&QPushButton::clicked,
		[=](bool check) {
			ui->wNoise->setVisible(!check);
		});

	ui->cbLineThickness->setCurrentIndex(1);

	unsigned int nb_channels = m_m2k_analogout->getNbChannels();
	for (unsigned int i = 0; i < nb_channels; i++) {
		auto ptr = QSharedPointer<signal_generator_data>(
		                   new signal_generator_data);
		ptr->amplitude = amplitude->value();
		ptr->offset = offset->value();
		ptr->frequency = frequency->value();
		ptr->constant = constantValue->value();
		ptr->phase = phase->value();
		ptr->indexValue = phase->indexValue();
		ptr->holdh = holdHighTime->value();
		ptr->holdl = holdLowTime->value();
		ptr->rise = riseTime->value();
		ptr->fall = fallTime->value();
		ptr->steps_up = stepsUp->value();
		ptr->steps_down = stepsDown->value();
		ptr->stairphase = stairPhase->value();
		ptr->dutycycle = dutycycle->value();
		ptr->waveform = SG_SIN_WAVE;
		ptr->math_record_length = mathRecordLength->value();
		ptr->math_sr = mathSampleRate->value();
		ptr->noiseType = (gr::analog::noise_type_t)0;
		ptr->noiseAmplitude=noiseAmplitude->value();
		ptr->file_sr = fileSampleRate->value();
		ptr->file_amplitude = fileAmplitude->value();
		ptr->file_offset = fileOffset->value();
		ptr->file_phase = filePhase->value();
		ptr->file_type=FORMAT_NO_FILE;
		ptr->file_nr_of_channels=0;
		ptr->file_channel=0;

		ptr->type = SIGNAL_TYPE_CONSTANT;
		ptr->id = i;

		ChannelWidget *cw = new ChannelWidget(i, false, false, QColor());
		cw->setShortName(QString("CH %1").arg(i + 1));
		cw->nameButton()->setText(cw->shortName());

		std::string ch_name = m_m2k_analogout->getChannelName(i);

		if (ch_name == "") {
			ch_name += "Channel ";
			ch_name += std::to_string(i + 1);
		}

		cw->setFullName(ch_name.c_str());

		cw->setProperty("signal_generator_data",
		                QVariant::fromValue(ptr));
		cw->setProperty("channel",
				qVariantFromValue(i));
		channels.append(cw);

		ui->channelsList->addWidget(cw);

		connect(cw, SIGNAL(enabled(bool)),
		        SLOT(channelWidgetEnabled(bool)));

		connect(cw, SIGNAL(menuToggled(bool)),
		        SLOT(channelWidgetMenuToggled(bool)));

		channels_group->addButton(cw->nameButton());
		settings_group->addButton(cw->menuButton());

		connect(cw->nameButton(), &QAbstractButton::toggled,
			cw->menuButton(), &QAbstractButton::setChecked);
		connect(cw->menuButton(), &QAbstractButton::toggled,
			cw->nameButton(), &QAbstractButton::setChecked);

		if (i == 0) {
			cw->menuButton()->setChecked(true);
		}
	}

	time_block_data->nb_channels = nb_channels;
	time_block_data->time_block = scope_sink_f::make(
					      nb_points, sample_rate,
					      "Signal Generator", nb_channels,
					      static_cast<QWidget *>(plot));

	/* Attach all curves by default */
	plot->registerSink(time_block_data->time_block->name(),
	                   nb_channels, nb_points);

	for(auto i = 0; i < nb_channels; i++) {
		plot->Curve(i)->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
		plot->Curve(i)->setPaintAttribute(QwtPlotCurve::FilterPointsAggressive, true);
	}


	/* This must be done after attaching the curves; otherwise
	 * plot->getLineColor(i) returns black. */
	for (unsigned int i = 0; i < nb_channels; i++) {
		channels[i]->setColor(plot->getLineColor(i));
	}

	plot->disableLegend();
	plot->setPaletteColor(QColor("black"));

	plot->setVertUnitsPerDiv(AMPLITUDE_VOLTS * 2.0 / 10.0);

	plot->setHorizUnitsPerDiv((double) nb_points /
	                          ((double) sample_rate * 10.0));
	plot->setHorizOffset((double) nb_points /
	                     ((double) sample_rate * 2.0));
	plot->zoomBaseUpdate();
	ui->plot->insertWidget(0,plot, 0, 0);

	connect(ui->btnAppearanceCollapse, SIGNAL(toggled(bool)),ui->wAppearance, SLOT(setVisible(bool)));
	ui->wAppearance->hide();



	fileManager = new FileManager("Signal Generator");

	api->setObjectName(QString::fromStdString(Filter::tool_name(
	                           TOOL_SIGNAL_GENERATOR)));
	api->load(*settings);
	api->js_register(engine);

	renameConfigPanel();
	connect(ui->rightMenu, SIGNAL(finished(bool)), this,
	        SLOT(rightMenuFinished(bool)));

	connect(constantValue, SIGNAL(valueChanged(double)),
	        SLOT(constantValueChanged(double)));

	connect(amplitude, SIGNAL(valueChanged(double)),
	        this, SLOT(amplitudeChanged(double)));

	connect(noiseAmplitude,SIGNAL(valueChanged(double)),
		this, SLOT(noiseAmplitudeChanged(double)));
	connect(fileAmplitude, SIGNAL(valueChanged(double)),
	        this, SLOT(fileAmplitudeChanged(double)));
	connect(fileOffset, SIGNAL(valueChanged(double)),
	        this, SLOT(fileOffsetChanged(double)));
	connect(filePhase, SIGNAL(valueChanged(double)),
	        this, SLOT(filePhaseChanged(double)));
	connect(ui->fileChannel, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(fileChannelChanged(int)));
	connect(fileSampleRate, SIGNAL(valueChanged(double)),
	        this, SLOT(fileSampleRateChanged(double)));
	connect(offset, SIGNAL(valueChanged(double)),
	        this, SLOT(offsetChanged(double)));
	connect(frequency, SIGNAL(valueChanged(double)),
	        this, SLOT(frequencyChanged(double)));
	connect(phase, SIGNAL(valueChanged(double)),
	        this, SLOT(phaseChanged(double)));

	connect(dutycycle, SIGNAL(valueChanged(double)),
		this, SLOT(dutyChanged(double)));


	connect(fallTime, SIGNAL(valueChanged(double)),
		this, SLOT(fallChanged(double)));
	connect(holdHighTime, SIGNAL(valueChanged(double)),
		this, SLOT(holdHighChanged(double)));
	connect(holdLowTime, SIGNAL(valueChanged(double)),
		this, SLOT(holdLowChanged(double)));
	connect(riseTime, SIGNAL(valueChanged(double)),
		this, SLOT(riseChanged(double)));

	connect(stepsUp,SIGNAL(valueChanged(double)),
		this,SLOT(stepsUpChanged(double)));

	connect(stepsDown,SIGNAL(valueChanged(double)),
		this,SLOT(stepsDownChanged(double)));

	connect(stairPhase,SIGNAL(valueChanged(double)),
		this,SLOT(stairPhaseChanged(double)));

	connect(mathRecordLength, SIGNAL(valueChanged(double)),
			this, SLOT(mathRecordLengthChanged(double)));
	connect(mathSampleRate, SIGNAL(valueChanged(double)),
			this, SLOT(mathSampleRateChanged(double)));

	connect(ui->type, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(waveformTypeChanged(int)));
	connect(ui->cbNoiseType, SIGNAL(currentIndexChanged(int)),
		this, SLOT(noiseTypeChanged(int)));

        connect(ui->cbLineThickness, SIGNAL(currentIndexChanged(int)),
                this, SLOT(lineThicknessChanged(int)));

	connect(ui->tabWidget, SIGNAL(currentChanged(int)),
	        this, SLOT(tabChanged(int)));

	connect(ui->load_file, SIGNAL(pressed()), this, SLOT(loadFile()));
	connect(ui->mathWidget, SIGNAL(functionValid(const QString&)),
	        this, SLOT(setFunction(const QString&)));

	connect(ui->run_button, SIGNAL(toggled(bool)), runButton(),
	        SLOT(setChecked(bool)));
	connect(runButton(), SIGNAL(toggled(bool)), ui->run_button,
		SLOT(toggle(bool)));
	connect(runButton(), SIGNAL(toggled(bool)),
		this, SLOT(startStop(bool)));

	connect(ui->refreshBtn, SIGNAL(clicked()),
		this, SLOT(loadFileCurrentChannelData()));

	time_block_data->time_block->set_update_time(0.001);

	plot->addZoomer(0);
	resetZoom();

	auto ptr = getCurrentData();
	phase->setFrequency(ptr->frequency);

	readPreferences();

	// Reduce the extent of the yLeft axis because it is not needed
	plot->axisWidget(QwtPlot::yLeft)->scaleDraw()->setMinimumExtent(65);
}

SignalGenerator::~SignalGenerator()
{
	disconnect(prefPanel, &Preferences::notify, this, &SignalGenerator::readPreferences);
	ui->run_button->toggle(false);	
	setDynamicProperty(runButton(), "disabled", false);
	if (saveOnExit) {
		api->save(*settings);
	}
	delete api;

	delete fileManager;

	delete plot;
	delete ui;
	delete time_block_data;
}

void SignalGenerator::settingsLoaded()
{
	updatePreview();
}

void SignalGenerator::readPreferences()
{
	double prefered_periods_nr = prefPanel->getSig_gen_periods_nr();
	if (nr_of_periods != prefered_periods_nr) {
		nr_of_periods = prefered_periods_nr;
		resetZoom();
	}
	ui->instrumentNotes->setVisible(prefPanel->getInstrumentNotesActive());
}

void SignalGenerator::resetZoom()
{

	disconnect(plot->getZoomer(),SIGNAL(zoomed(QRectF)),this,SLOT(rescale()));
	bool disable_zoom=false;

	for (auto it = channels.begin(); it != channels.end(); ++it) {
		auto ptr=getData(*it);

		if (ptr->type==SIGNAL_TYPE_BUFFER) {
			disable_zoom=true;
			break;
		}
	}

	if (!disable_zoom) {
		connect(plot->getZoomer(),SIGNAL(zoomed(QRectF)),this,SLOT(rescale()));
	}

	double period = 0.0;
	unsigned int slowSignalId = 0;

	for (auto it = channels.begin(); it != channels.end(); ++it) {
		if ((*it)->enableButton()->isChecked()) {
			auto ptr = getData((*it));

			switch (ptr->type) {
			case SIGNAL_TYPE_CONSTANT:
				break;

			case SIGNAL_TYPE_BUFFER:
				if (ptr->file_type) {
					double length = 1;
					if (ptr->file_nr_of_samples.size() > 0) {
						length=(1/ptr->file_sr)*ptr->file_nr_of_samples[ptr->file_channel];
					}
					if (period<length) {
						period=length;
						slowSignalId = ptr->id;
					}
				}

				break;

			case SIGNAL_TYPE_WAVEFORM:
				if (period< 1.0 / (ptr->frequency)) {
					period=(1.0/ptr->frequency);
					slowSignalId = ptr->id;
				}

				break;

			case SIGNAL_TYPE_MATH:
				if (period< (ptr->math_record_length)) {
					period=(ptr->math_record_length);
					slowSignalId = ptr->id;
				}

				break;

			default:
				break;

			}
		}
	}

	period*= nr_of_periods;

	if (period==0.0) { // prevent empty graph
		period=0.1;
	}

	plot->setVertUnitsPerDiv(1);
	plot->setVertOffset(0);
	plot->setHorizUnitsPerDiv(period/10);
	plot->setHorizOffset(period/2);
	plot->zoomBaseUpdate();
	rescale();
	plot->DetachCurve(slowSignalId);
	plot->AttachCurve(slowSignalId);
}

void SignalGenerator::rescale()
{

	auto hOffset=plot->HorizOffset();
	auto hUnitsPerDiv=plot->HorizUnitsPerDiv();
	auto xDivisions=10;


	zoomT1=hOffset-hUnitsPerDiv*(xDivisions/2);
	zoomT2=hOffset+hUnitsPerDiv*(xDivisions/2);

	zoomT1OnScreen=hOffset-hUnitsPerDiv*(xDivisions/2);
	zoomT2OnScreen=hOffset+hUnitsPerDiv*(xDivisions/2);

	auto deltaT= zoomT2-zoomT1;

	double max_sample_rate=750000000;
	nb_points=NB_POINTS;
	sample_rate=(double)nb_points/deltaT;

	if (sample_rate>max_sample_rate) {
		nb_points=nb_points / (long)(sample_rate/max_sample_rate);
		sample_rate=max_sample_rate;

	}

	long startSample=sample_rate*zoomT1;
	plot->setDataStartingPoint(startSample);
	plot->setSampleRate(sample_rate,1,"Hz");
	if (nb_points < 16) {
		nb_points = 16;
	}
	time_block_data->time_block->set_nsamps(nb_points);
	time_block_data->time_block->set_samp_rate(sample_rate);
	updatePreview();
	plot->replot();
}

void SignalGenerator::constantValueChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->constant != (float) value) {
		ptr->constant = (float) value;
		resetZoom();
	}
}

void SignalGenerator::amplitudeChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->amplitude != value) {
		ptr->amplitude = value;
		resetZoom();
	}
}

void SignalGenerator::fileAmplitudeChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->file_amplitude != value) {
		ptr->file_amplitude = value;
		resetZoom();
	}
}

void SignalGenerator::fileOffsetChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->file_offset!= value) {
		ptr->file_offset= value;
		resetZoom();
	}
}

void SignalGenerator::filePhaseChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->file_phase!= value) {
		ptr->file_phase= value;
		resetZoom();
	}
}

void SignalGenerator::offsetChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->offset != (float) value) {
		ptr->offset = (float) value;
		resetZoom();
	}
}

void SignalGenerator::fileChannelChanged(int value)
{
	auto ptr = getCurrentData();

	if (ptr->file_channel != (int) value) {
		ptr->file_channel = (int) value;
		loadFileChannelData(currentChannel);
		this->ui->label_size->setText(QString::number(
						      ptr->file_nr_of_samples[ptr->file_channel]) +
					      tr(" samples"));

		resetZoom();
	}
}
void SignalGenerator::fileSampleRateChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->file_sr != value) {
		ptr->file_sr = value;
		resetZoom();
	}
}


void SignalGenerator::frequencyChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->frequency != value) {
		phase->setFrequency(value);
		if(phase->inSeconds()) {
			ptr->phase = phase->value();
			ptr->indexValue = phase->indexValue();
		}
		ptr->frequency = value;
		resetZoom();
	}
}

void SignalGenerator::mathRecordLengthChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->math_record_length != value) {
		ptr->math_record_length = value;
		resetZoom();
	}
}


void SignalGenerator::mathSampleRateChanged(double value)
{
	auto ptr = getCurrentData();
	if (ptr->math_sr != value) {
		ptr->math_sr = value;
		resetZoom();
	}
}

void SignalGenerator::noiseTypeChanged(int index)
{
	auto ptr = getCurrentData();

	gr::analog::noise_type_t value = qvariant_cast<gr::analog::noise_type_t>(ui->cbNoiseType->itemData(index));
	if (ptr->noiseType != value) {
		ptr->noiseType= value;
		resetZoom();
	}
}

void SignalGenerator::noiseAmplitudeChanged(double value)
{

	auto ptr = getCurrentData();

	if (ptr->noiseAmplitude != value) {
		ptr->noiseAmplitude = value;
		resetZoom();
	}
}

void SignalGenerator::lineThicknessChanged(int index)
{
        auto ptr = getCurrentData();
        int lineThickness = (int)(ptr->lineThickness / 0.5) - 1;
        if (lineThickness != index) {
                ptr->lineThickness = 0.5 * (index + 1);
                plot->setLineWidth(ptr->id, ptr->lineThickness);
                plot->replot();
        }
}

void SignalGenerator::dutyChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->dutycycle != value) {
		ptr->dutycycle= value;
		resetZoom();
	}
}


void SignalGenerator::trapezoidalComputeFrequency()
{
	auto ptr = getCurrentData();
	double value = static_cast<double>((round(1/(ptr->rise+ptr->fall+ptr->holdh+ptr->holdl))*1000)/1000);
	frequency->setValue(value);
}

void SignalGenerator::riseChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->rise != value) {
		ptr->rise= value;
		trapezoidalComputeFrequency();
	}
}

void SignalGenerator::fallChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->fall != value) {
		ptr->fall= value;
		trapezoidalComputeFrequency();
	}
}

void SignalGenerator::holdHighChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->holdh != value) {
		ptr->holdh= value;
		trapezoidalComputeFrequency();
	}
}

void SignalGenerator::holdLowChanged(double value)
{
	auto ptr = getCurrentData();

	if (ptr->holdl != value) {
		ptr->holdl= value;
		trapezoidalComputeFrequency();
	}
}

void SignalGenerator::stepsUpChanged(double value)
{
	auto ptr = getCurrentData();
	if(ptr->steps_up != (int)value) {
		ptr->steps_up =(int) value;
		stairPhase->setMaxValue(ptr->steps_up + ptr->steps_down);
		resetZoom();
	}
}

void SignalGenerator::stepsDownChanged(double value)
{
	auto ptr = getCurrentData();

	if(ptr->steps_down != (int)value) {
		ptr->steps_down =(int) value;
		stairPhase->setMaxValue(ptr->steps_up + ptr->steps_down);
		resetZoom();
	}
}
void SignalGenerator::stairPhaseChanged(double value)
{
	auto ptr = getCurrentData();

	if(ptr->stairphase != (int) value) {
		ptr->stairphase = (int) value;
		resetZoom();
	}
}

void SignalGenerator::phaseChanged(double value)
{
	auto ptr = getCurrentData();

	ptr->indexValue = phase->indexValue();
	if (ptr->phase != value) {
		ptr->phase = value;
		resetZoom();
	}
}

void SignalGenerator::phaseIndexChanged()
{
	auto ptr = getCurrentData();

	ptr->indexValue = phase->indexValue();
}

void SignalGenerator::waveformUpdateUi(int val)
{
	frequency->setEnabled(val!=SG_TRA_WAVE);
	ui->wtrapezparams->setVisible(val==SG_TRA_WAVE);
	stepsUp->setVisible(val==SG_STAIR_WAVE);
	stepsDown->setVisible(val==SG_STAIR_WAVE);
	stairPhase->setVisible(val==SG_STAIR_WAVE);
	phase->setVisible(val!=SG_STAIR_WAVE);
	dutycycle->setVisible(val==SG_SQR_WAVE);
	if(val==SG_TRA_WAVE) {
		trapezoidalComputeFrequency();
	}

}

void SignalGenerator::waveformTypeChanged(int val)
{
	enum sg_waveform types[] = {
		SG_SIN_WAVE,
		SG_SQR_WAVE,
		SG_TRI_WAVE,
		SG_TRA_WAVE,
		SG_SAW_WAVE,
		SG_INV_SAW_WAVE,
		SG_STAIR_WAVE,
	};

	auto ptr = getCurrentData();

	if (ptr->waveform != types[val]) {
		ptr->waveform = types[val];
		// only show trapezoid parameters for the apropriate waveform
		waveformUpdateUi(types[val]);
		resetZoom();
	}

}

QSharedPointer<signal_generator_data> SignalGenerator::getCurrentData()
{
	return getData(channels[currentChannel]);
}

QSharedPointer<signal_generator_data> SignalGenerator::getData(QWidget *obj)
{
	auto var = obj->property("signal_generator_data");
	return var.value<QSharedPointer<signal_generator_data>>();
}

void SignalGenerator::resizeTabWidget(int index)
{
	for(int i=0;i<ui->tabWidget->count();i++)
		if(i!=index)
		    ui->tabWidget->widget(i)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	ui->tabWidget->widget(index)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	ui->tabWidget->widget(index)->resize(ui->tabWidget->widget(index)->minimumSizeHint());
	ui->tabWidget->widget(index)->adjustSize();
}

void SignalGenerator::tabChanged(int index)
{
	auto ptr = getCurrentData();

	if (ptr->type != (enum SIGNAL_TYPE) index) {
		ptr->type = (enum SIGNAL_TYPE) index;
		if(ptr->type == SIGNAL_TYPE_BUFFER)
		{
			loadFileCurrentChannelData();
		}
		resizeTabWidget(index);
		resetZoom();
	}
}

void SignalGenerator::updatePreview()
{
	static const float MAX_PREVIEW_RANGE = (float)(SHRT_MAX);
	static const float MIN_PREVIEW_RANGE = (float)(SHRT_MIN);

	const int nb_points_correction = 16; // generate slightly more points to avoid incomplete scope_sink_f buffer
	gr::top_block_sptr top = make_top_block("Signal Generator Update");
	unsigned int i = 0;
	bool enabled = false;

	time_block_data->time_block->reset();
	for (auto it = channels.begin(); it != channels.end(); ++it) {
		basic_block_sptr source;

		if ((*it)->enableButton()->isChecked()) {

			source = getSource((*it), sample_rate, top, true);
			enabled = true;
		} else {
			source = blocks::nop::make(sizeof(float));
		}

		auto head = blocks::head::make(sizeof(float), nb_points + nb_points_correction);
		auto clamp = analog::rail_ff::make(MIN_PREVIEW_RANGE, MAX_PREVIEW_RANGE); // prevent plotting infinite values
		top->connect(source, 0, clamp, 0);
		top->connect(clamp, 0, head ,0);
		top->connect(head, 0, time_block_data->time_block, i++);
	}

	QElapsedTimer timer;
	timer.start();

	top->run();
	qDebug(CAT_SIGNAL_GENERATOR) << "The slow operation took" << timer.elapsed() << "milliseconds";
	top->disconnect_all();

	if (ui->run_button->runButtonChecked()) {
		if (enabled) {
			stop();
			start();
		} else {
			ui->run_button->toggle(false);
		}
	}
}

enum sg_file_format SignalGenerator::getFileFormat(QString filePath)
{
	if (filePath.isEmpty()) {
		return FORMAT_NO_FILE;
	}

	QFile f;
	f.setFileName(filePath);

	if (!f.open(QIODevice::ReadOnly)) {
		return FORMAT_NO_FILE;
	}

	f.close();

	if (filePath.endsWith(".wav")) {
		return FORMAT_WAVE;
	}

	if (filePath.endsWith(".csv")||filePath.endsWith(".txt")) {
		return FORMAT_CSV;
	}

	if (filePath.endsWith(".mat")) {
		return FORMAT_MAT;
	}

	return FORMAT_BIN_FLOAT;
}



bool SignalGenerator::loadParametersFromFile(
        QSharedPointer<signal_generator_data> ptr,QString filePath)
{
	ptr->file_message="";
	ptr->file=filePath;
	ptr->file_type=getFileFormat(ptr->file);
	auto info = QFileInfo(ptr->file);
	ptr->file_channel_names.clear();
	ptr->file_nr_of_channels=0;
	ptr->file_nr_of_samples.clear();
	ptr->file_channel=0;

	if (ptr->file_type==FORMAT_BIN_FLOAT) {
		ptr->file_nr_of_samples.push_back(info.size() / sizeof(float));
		ptr->file_nr_of_channels=1;
		ptr->file_message="Binary floats file";
	}

	if (ptr->file_type==FORMAT_WAVE) {
		// read samples per second
		bool ok=true;
		try {
			auto fs = blocks::wavfile_source::make(ptr->file.toLocal8Bit(),true);
		} catch(std::runtime_error& e)
		{
			ok=false;
			ptr->file_message=QString::fromLocal8Bit(e.what());
			ptr->file_nr_of_samples.push_back(0);
			ptr->file_type=FORMAT_NO_FILE;
		}
		if(!ok)
			return false;

		riff_header_t riff;
		chunk_header_t chunk;
		uint32_t nr_of_samples;
		QFile f;
		QByteArray val;

		f.setFileName(ptr->file);
		f.open(QIODevice::ReadOnly);
		f.read(riff.data,12);

		if (!riffCompare(riff,"WAVE")) {
			return false;
		}

		while (!f.atEnd()) {
			f.read(chunk.data,8);

			if (chunkCompare(chunk,"fmt ")) {
				f.read(ptr->file_wav_hdr.header_data,sizeof(ptr->file_wav_hdr.header_data));
				ptr->file_sr=ptr->file_wav_hdr.SamplesPerSec;
				ptr->file_nr_of_channels=ptr->file_wav_hdr.noChan;
				ptr->file_channel=0;
				continue;
			}

			if (chunkCompare(chunk,"data")) {
				auto bytesPerSample = (ptr->file_wav_hdr.bitsPerSample/8);
				nr_of_samples = riff.size/(bytesPerSample);
				continue;
			}

			f.seek(f.pos()+chunk.size);
		}

		for (auto i=0; i<ptr->file_nr_of_channels; i++) {
			ptr->file_channel_names.push_back("Channel " + QString::number(i));
			ptr->file_nr_of_samples.push_back(nr_of_samples);
		}

		ptr->file_message="WAV";
	}

	if (ptr->file_type==FORMAT_CSV) {

		try {
			fileManager->open(ptr->file, FileManager::IMPORT);
		} catch(FileManagerException &e) {
			ptr->file_message=QString::fromLocal8Bit(e.what());
			ptr->file_nr_of_samples.push_back(0);
			ptr->file_type=FORMAT_NO_FILE;
			return false;
		}

		ptr->file_data.clear();
		ptr->file_nr_of_channels = fileManager->getNrOfChannels();

		if(fileManager->getSampleRate())
			ptr->file_sr = fileManager->getSampleRate();

		ptr->file_channel=0; // autoselect channel 0
		for (auto i=0; i<ptr->file_nr_of_channels; i++) {
			ptr->file_channel_names.push_back("Column " + QString::number(i));
			ptr->file_nr_of_samples.push_back(fileManager->getNrOfSamples());
		}

		ptr->file_message="CSV";
	}

#ifdef MATLAB_SUPPORT_SIGGEN
	if (ptr->file_type==FORMAT_MAT) {
		mat_t *matfp;
		matvar_t *matvar;
		matfp = Mat_Open(filePath.toStdString().c_str(),MAT_ACC_RDONLY);

		if (NULL == matfp) {
			qDebug(CAT_SIGNAL_GENERATOR)<<"Error opening MAT file "<<filePath;
			ptr->file_nr_of_samples.push_back(0);
			ptr->file_message = "MAT file could not be parsed";
			return false;
		}

		while ((matvar = Mat_VarReadNextInfo(matfp)) != NULL) {

			/* must be a vector */
			if (!(matvar->rank !=2 || (matvar->dims[0] > 1 && matvar->dims[1] > 1)
			      || matvar->class_type != MAT_C_DOUBLE)) {
				Mat_VarReadDataAll(matfp, matvar);

				if (!matvar->isComplex) {
					qDebug(CAT_SIGNAL_GENERATOR)<<"Complex buffers not supported";
					ptr->file_message="Complex buffers not supported";
					ptr->file_channel_names.push_back(QString(matvar->name));
					ptr->file_nr_of_samples.push_back(*matvar->dims);
					ptr->file_nr_of_channels++;
				}
			}

			Mat_VarFree(matvar);
			matvar = NULL;
		}

		Mat_Close(matfp);
	}
#endif

	if (ptr->file_nr_of_channels==0) {
		ptr->file_message+="File not loaded due to errors";
		ptr->file_nr_of_samples.push_back(0);
		ptr->file_type=FORMAT_NO_FILE;
        return false;
	}

	ui->fileChannel->setEnabled(ptr->file_nr_of_channels > 1);

	for(auto i = 0; i < ptr->file_nr_of_samples.size(); i++)
	{
		if(ptr->file_nr_of_samples[i] > m_maxNbOfSamples)
		{
			ptr->file_nr_of_samples[i] = m_maxNbOfSamples;
			ptr->file_message = "File truncated. Max 4MS";
		}
	}

	ptr->file_amplitude=1.0;
	ptr->file_offset=0;
	ptr->file_phase=0;

	return true;
}

void SignalGenerator::reloadFileFromPath()
{
	auto ptr = getCurrentData();
	if (!ptr->file.isEmpty()) {
		loadFileFromPath(ptr->file);
	}
}

void SignalGenerator::loadFileFromPath(QString filename){
    auto ptr = getCurrentData();

    ptr->file = filename;
    ui->label_path->setText(ptr->file);
    Util::setWidgetNrOfChars(ui->label_path,10,30);
    bool loaded = loadParametersFromFile(ptr,ptr->file);

    fileAmplitude->setEnabled(loaded);
    fileSampleRate->setEnabled(loaded);
    filePhase->setEnabled(loaded);
    fileOffset->setEnabled(loaded);
    if (!loaded) {
        ptr->file_type=FORMAT_NO_FILE;
        resetZoom();
        return;
    }
    fileOffset->setValue(ptr->file_offset);
    filePhase->setValue(ptr->file_phase);
    filePhase->setMaxValue(ptr->file_nr_of_samples[ptr->file_channel]);
    fileSampleRate->setValue(ptr->file_sr);
    ui->fileChannel->blockSignals(true);
    ui->fileChannel->clear();
    ui->label_format->setText(ptr->file_message);

    if (ptr->file_channel_names.isEmpty()) {
        for (auto i=0; i<ptr->file_nr_of_channels; i++) {
            ui->fileChannel->addItem(QString::number(i));
        }
    } else {
        ui->fileChannel->addItems(ptr->file_channel_names);
    }

    this->ui->label_size->setText(QString::number(
                                          ptr->file_nr_of_samples[ptr->file_channel]) +
                                  tr(" samples"));
    ui->fileChannel->setEnabled(ptr->file_nr_of_channels>1);
    ui->fileChannel->setCurrentIndex(ptr->file_channel);
    ui->fileChannel->blockSignals(false);
    loadFileChannelData(currentChannel);
    updateRightMenuForChn(currentChannel);
}

void SignalGenerator::loadFile()
{
	QString fileName = QFileDialog::getOpenFileName(this,
	    tr("Open File"), "", tr("Comma-separated values files (*.csv);;"
				    "Tab-delimited values files (*.txt);;"
				    "Waveform Audio File Format (*.wav);;"
				    "Matlab files (*.mat)"),
	    nullptr, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	if(fileName.isEmpty()) { // user hit cancel
		return;
	}
    loadFileFromPath(fileName);
    updateRightMenuForChn(currentChannel);
    resetZoom();
}

void SignalGenerator::start()
{
	m_running = true;

	/* Avoid from being started twice */
	if (buffers.size() > 0) {
		return;
	}

	m_m2k_analogout->cancelBuffer();

	for (auto it = channels.begin(); it != channels.end(); ++it) {
		if (!(*it)->enableButton()->isChecked()) {
			m_m2k_analogout->enableChannel((*it)->id(), false);
			continue;
		}

		m_m2k_analogout->enableChannel((*it)->id(), true);
		auto chn_data = getData(*it);

		if (chn_data->file_type==FORMAT_NO_FILE && chn_data->type==SIGNAL_TYPE_BUFFER) {
			continue;
		}
	}

	unsigned long final_rate;
	unsigned long oversampling;

	for (int i = 0; i < m_m2k_analogout->getNbChannels(); i++) {
		buffers.push_back({});
		if (!m_m2k_analogout->isChannelEnabled(i)) {
			continue;
		}

		top_block = gr::make_top_block("Signal Generator");
		double best_rate = get_best_sample_rate(i);

		/* Do not generate anything if samplerate can't be determined */
		if (best_rate <= 0) {
			continue;
		}
		size_t samples_count = get_samples_count(i, best_rate);

		calc_sampling_params(i, best_rate, final_rate,
				     oversampling);

		QWidget* w = channels[i];
		auto source = getSource(w, best_rate, top_block);
		auto head = blocks::head::make(sizeof(float), samples_count);
		auto vector = blocks::vector_sink_f::make();
		auto clamp = analog::rail_ff::make(-AMPLITUDE_VOLTS, AMPLITUDE_VOLTS);

		top_block->connect(source, 0, clamp, 0);
		top_block->connect(clamp,0, head,0);
		top_block->connect(head, 0, vector, 0);
		top_block->run();

		const std::vector<float>& f_samples = vector->data();
		std::vector<double> samples(f_samples.begin(), f_samples.end());

		buffers.at(i) = samples;

		m_m2k_analogout->setOversamplingRatio(i, oversampling);
		m_m2k_analogout->setSampleRate(i, final_rate);
	}

	qDebug(CAT_SIGNAL_GENERATOR) << "Pushed cyclic buffer";

	m_m2k_analogout->setCyclic(true);
	m_m2k_analogout->push(buffers);
}


void SignalGenerator::run()
{
	start();
}

void SignalGenerator::stop()
{
	try {
		buffers.clear();
		m_running = false;
		m_m2k_analogout->stop();
	} catch (libm2k::m2k_exception &e) {
		qDebug(CAT_SIGNAL_GENERATOR) << e.what();
	}
}

void SignalGenerator::startStop(bool pressed)
{
	if (pressed) {
		start();
	} else {
		stop();
	}

	setDynamicProperty(ui->run_button, "running", pressed);
}

void SignalGenerator::setFunction(const QString& function)
{
	auto ptr = getCurrentData();

	if (ptr->function != function) {
		ptr->function = function;
		resetZoom();
	}
}

//std::vector<float> stairdata;

basic_block_sptr SignalGenerator::getSignalSource(gr::top_block_sptr top,
		double samp_rate, struct signal_generator_data& data,
                double phase_correction)
{
	analog::gr_waveform_t waveform;
	double phase;
	double amplitude;
	double rise=0.5,fall=0.5;
	double holdh=0.0,holdl=0.0;
	float offset;
	int rising_steps=1;
	int falling_steps=1;
	int stairphase=0;

	amplitude = data.amplitude / 2.0;
	offset = data.offset;
	phase = data.phase + phase_correction;

	if (data.waveform == SG_TRI_WAVE) {
		phase = std::fmod(phase + 90.0, 360.0);
	} else if (data.waveform == SG_SQR_WAVE) {
		phase = std::fmod(phase + 180.0, 360.0);
	} else if (phase<0) {
		phase=phase+360.0;
	}

	switch(data.waveform)
	{
	case SG_SQR_WAVE:
		rise=fall=0;
		holdh=(data.dutycycle/100.0);
		holdl=1.0-(data.dutycycle/100.0);
		break;
	case SG_TRI_WAVE:
		rise=fall=1;
		holdh=0;
		holdl=0;
		break;
	case SG_SAW_WAVE:
		fall=holdh=holdl=0;
		rise=1;
		break;
	case SG_INV_SAW_WAVE:
		rise=holdh=holdl=0;
		fall=1;
		break;
	case SG_TRA_WAVE:
		rise=data.rise;
		fall=data.fall;
		holdl=data.holdl;
		holdh=data.holdh;
		break;
	case SG_STAIR_WAVE:
		rising_steps = data.steps_up;
		falling_steps = data.steps_down;
		stairphase = data.stairphase;
		break;
	default:
		break;
	}

	basic_block_sptr src = nullptr;
	if(data.waveform==SG_SIN_WAVE)
		src = analog::sig_source_f::make(samp_rate, analog::GR_SIN_WAVE,
			data.frequency, amplitude, offset, phase*0.01745329);
	else if(data.waveform==SG_STAIR_WAVE) {
		data.stairdata = get_stairstep(rising_steps, falling_steps,
					  amplitude, offset, stairphase);
		src = blocks::vector_source_f::make(data.stairdata, true);

	}
	else
		src = scopy::trapezoidal::make(samp_rate, data.frequency, amplitude,
					       rise, holdh, fall, holdl, offset,
					       phase*0.01745329);

	return src;

}

void SignalGenerator::loadFileCurrentChannelData()
{
	reloadFileFromPath();
	updateRightMenuForChn(currentChannel);
	resetZoom();

}

void SignalGenerator::loadFileChannelData(int chIdx)
{
	auto ptr = getData(channels[chIdx]);

	if (ptr->type!=SIGNAL_TYPE_BUFFER) {
		qDebug(CAT_SIGNAL_GENERATOR)<<"loadFileChannelData called without having SIGNAL_TYPE_BUFFER";
		return;
	}

	ptr->file_data.clear();
	if(ptr->file_type == FORMAT_WAVE || ptr->file_type == FORMAT_MAT) // let GR flow load data
		return;
	try {
		fileManager->open(ptr->file, FileManager::IMPORT);

		if (ptr->file_type==FORMAT_CSV) {

			if (ptr->file_channel >= ptr->file_nr_of_channels) {
				return;
			}

			for (auto x : fileManager->read(ptr->file_channel)) {
				ptr->file_data.push_back(x);
			}
		}

#ifdef MATLAB_SUPPORT_SIGGEN
		if (ptr->file_type==FORMAT_MAT) {
			mat_t *matfp;
			matvar_t *matvar;

			matfp = Mat_Open(ptr->file.toStdString().c_str(),MAT_ACC_RDONLY);

			if (NULL == matfp) {
				qDebug(CAT_SIGNAL_GENERATOR)<<"Error opening MAT file "<<ptr->file;
				return;
			}

			matvar=Mat_VarRead(matfp,
					   ptr->file_channel_names[ptr->file_channel].toStdString().c_str());
			const double *xData = static_cast<const double *>(matvar->data) ;

			for (auto i=0; i<ptr->file_nr_of_samples[ptr->file_channel]; ++i) {
				ptr->file_data.push_back(xData[i]);
			}

			Mat_Close(matfp);
			return;
		}
#endif

	} catch(FileManagerException &e) {
		ptr->file_message=QString::fromLocal8Bit(e.what());
		ptr->file_nr_of_samples.push_back(0);
		ptr->file_type=FORMAT_NO_FILE;
	}
}

gr::basic_block_sptr SignalGenerator::getNoise(QWidget *obj, gr::top_block_sptr top)
{
	auto ptr = getData(obj);
	auto noiseaAmpl = ptr->noiseAmplitude/2;
	if((int)ptr->noiseType != 0)
	{
		long noiseSeed=(rand());
		double noiseDivider=1;
		switch(ptr->noiseType)
		{
		case analog::GR_IMPULSE:
			noiseaAmpl=ptr -> noiseAmplitude;
			noiseDivider=15;
			break;
		case analog::GR_GAUSSIAN:
			noiseDivider=7;
			break;
		case analog::GR_UNIFORM:
			noiseDivider=2;
			break;
		case analog::GR_LAPLACIAN:
			noiseDivider=14;
			break;
		default:
			noiseDivider=1;
			break;
		}

		auto noise = analog::noise_source_f::make((analog::noise_type_t)ptr->noiseType,ptr->noiseAmplitude/noiseDivider,noiseSeed);
		auto rail = analog::rail_ff::make(-noiseaAmpl, noiseaAmpl);
		top->connect(noise,0,rail,0);
		return rail;
	}
	else
	{
		return blocks::null_source::make(sizeof(float));
	}
}

gr::basic_block_sptr SignalGenerator::displayResampler(double samp_rate,
						       double freq,
						       gr::top_block_sptr top,
						       gr::basic_block_sptr generated_wave,
						       gr::basic_block_sptr noiseSrc,
						       gr::basic_block_sptr noiseAdd
						       )
{
	auto ratio = samp_rate/freq;
	long m,n;
	bool ok=false;
	for(auto precision = 128;precision > 8 ;precision>>=1)
	{
		reduceFraction(ratio,&m,&n,precision);
		if(m!=0 && n!=0)
		{
			ok=true;
			break;
		}
	}
	ok = true;
	if(!ok)
	{
		return blocks::nop::make(sizeof(float));
	}


	auto interp= blocks::repeat::make(sizeof(float),m);
	auto decim=blocks::keep_one_in_n::make(sizeof(float),n);

	top->connect(noiseSrc,0,noiseAdd,0);
	top->connect(generated_wave,0,noiseAdd,1);
	top->connect(noiseAdd,0,interp,0);
	top->connect(interp,0,decim,0);
	return decim;
}

gr::basic_block_sptr SignalGenerator::getSource(QWidget *obj,
		double samp_rate, gr::top_block_sptr top, bool preview)
{
	auto ptr = getData(obj);
	enum SIGNAL_TYPE type = ptr->type;
	double phase=0.0;

	auto noiseSrc = getNoise(obj, top);
	auto noiseAdd = blocks::add_ff::make();
	gr::basic_block_sptr generated_wave;

	switch (type) {
	case SIGNAL_TYPE_CONSTANT:
		generated_wave = analog::sig_source_f::make(samp_rate,
							analog::GR_CONST_WAVE, 0, 0,
							ptr->constant);
		break;

	case SIGNAL_TYPE_WAVEFORM:
		if (preview) {
			int full_periods=(int)((double)zoomT1OnScreen*ptr->frequency);
			double phase_in_time = zoomT1OnScreen - full_periods/ptr->frequency;
			phase = (phase_in_time*ptr->frequency) * 360.0;

			generated_wave = getSignalSource(top, samp_rate, *ptr, phase);

			if(ptr->waveform!=SG_STAIR_WAVE)
				break;

			// Only for STAIR wave
			return displayResampler(sample_rate, (ptr->frequency * (ptr->steps_up + ptr->steps_down)),
						       top, generated_wave, noiseSrc, noiseAdd);


		}
		else
		{
			generated_wave = getSignalSource(top, samp_rate, *ptr, phase);
		}
		break;

	case SIGNAL_TYPE_BUFFER:
        if (!ptr->file.isNull() && ptr->file_type != FORMAT_NO_FILE) {
			auto str = ptr->file.toStdString();
			boost::shared_ptr<basic_block> fs;

			auto null = blocks::null_sink::make(sizeof(float));
			auto buffer=blocks::head::make(sizeof(float),(4*1024*1024));

			switch (ptr->file_type) {
			case FORMAT_BIN_FLOAT:
				fs = blocks::file_source::make(sizeof(float), str.c_str(), true);
				top->connect(fs,0,buffer,0);
				break;

			case FORMAT_WAVE:
				try {
					fs = blocks::wavfile_source::make(str.c_str(),true);
				} catch(std::runtime_error& e)
				{
					ptr->file_message=QString::fromLocal8Bit(e.what());
					fs=blocks::null_source::make(sizeof(float));
				}
				for (auto i=0; i<ptr->file_nr_of_channels; i++) {
					if (i==ptr->file_channel) {
						top->connect(fs,i,buffer,0);
					} else {
						top->connect(fs,i,null,0);
					}
				}
				break;

			case FORMAT_CSV:
			case FORMAT_MAT:
				fs = blocks::vector_source_f::make(ptr->file_data,true);
				top->connect(fs,0,buffer,0);
				break;

			default:
				fs=blocks::null_source::make(sizeof(float));
				noiseSrc=blocks::null_source::make(sizeof(float));
				top->connect(fs,0,buffer,0);
				break;
			}

			auto mult = blocks::multiply_const_ff::make(ptr->file_amplitude);
			top->connect(buffer,0,mult,0);
			auto add = blocks::add_const_ff::make(ptr->file_offset);
			top->connect(mult,0,add,0);
			auto phase_skip = blocks::skiphead::make(sizeof(float),ptr->file_phase);
			top->connect(add,0,phase_skip,0);

			if (preview) {
				auto resamp = displayResampler(sample_rate, (ptr->file_sr),
							       top, phase_skip, noiseSrc, noiseAdd);
				double buffer_freq = 1;
				if (ptr->file_nr_of_samples.size() > 0) {
					buffer_freq = ptr->file_sr/(double)
				                   ptr->file_nr_of_samples[ptr->file_channel];
				}
				int full_periods=(int)((double)zoomT1OnScreen * buffer_freq);
				double phase_in_time = zoomT1OnScreen - (full_periods/buffer_freq);
				unsigned long samples_to_skip = phase_in_time * samp_rate;
				auto skip = blocks::skiphead::make(sizeof(float),samples_to_skip);
				top->connect(resamp,0,skip,0);
				// return before readding the noise.
				return skip;
			} else {
				generated_wave = phase_skip;
			}
		}
		else {
			generated_wave = blocks::nop::make(sizeof(float));
		}
		break;

	case SIGNAL_TYPE_MATH:
		if (!ptr->function.isEmpty()) {
			auto str = ptr->function.toStdString();
			double math_record_freq = (1.0/ptr->math_record_length);

			if (preview) {

				int full_periods=(int)((double)zoomT1OnScreen * math_record_freq);
				double phase_in_time = zoomT1OnScreen - full_periods/math_record_freq;
				phase = (phase_in_time*math_record_freq) * 360.0;
				uint64_t to_skip = samp_rate * phase / (math_record_freq * 360.0);
				auto skip_head = blocks::skiphead::make(sizeof(float),(uint64_t)to_skip);

				if(ptr->math_sr < samp_rate)
				{
					auto src = scopy::iio_math_gen::make(ptr->math_sr, str, (uint64_t)ptr->math_sr * ptr->math_record_length);
					auto resamp = displayResampler(samp_rate, ptr->math_sr, top, src, noiseSrc, noiseAdd);
					top->connect(resamp, 0, skip_head, 0);
					return skip_head;
				}
				else
				{
					auto src = scopy::iio_math_gen::make(samp_rate, str, (uint64_t)samp_rate * ptr->math_record_length);
					top->connect(src, 0, skip_head, 0);
					generated_wave = skip_head;
				}
			} else {
				generated_wave = scopy::iio_math_gen::make(samp_rate, str, (uint64_t)samp_rate * ptr->math_record_length);
			}
			break;
		}

	default:
		generated_wave = blocks::nop::make(sizeof(float));
		break;
	}

	top->connect(noiseSrc,0,noiseAdd,0);
	top->connect(generated_wave,0,noiseAdd,1);
	return noiseAdd;
}

void adiscope::SignalGenerator::channelWidgetEnabled(bool en)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());
	int id = cw->id();

	m_m2k_analogout->enableChannel(id, en);

	if (en) {
		plot->AttachCurve(id);
	} else {
		plot->DetachCurve(id);
	}

	resetZoom();
	plot->replot();

	bool enable_run = en;

	if (!en) {
		for (auto it = channels.begin();
		     !enable_run && it != channels.end(); ++it) {
			enable_run = (*it)->enableButton()->isChecked();
		}
	}


	ui->run_button->setEnabled(enable_run);
	run_button->setEnabled(enable_run);
	setDynamicProperty(run_button, "disabled", !enable_run);
}

void adiscope::SignalGenerator::triggerRightMenuToggle(int chIdx, bool checked)
{
	// Queue the action, if right menu animation is in progress. This way
	// the action will be remembered and performed right after the animation
	// finishes
	if (ui->rightMenu->animInProgress()) {
		menuButtonActions.enqueue(
		        QPair<int, bool>(chIdx, checked));
	} else {
		phaseIndexChanged();
		currentChannel = chIdx;
		updateAndToggleMenu(chIdx, checked);
	}
}

void adiscope::SignalGenerator::channelWidgetMenuToggled(bool checked)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());

	plot->setActiveVertAxis(cw->id());

	triggerRightMenuToggle(cw->id(), checked);
}

void adiscope::SignalGenerator::renameConfigPanel()
{
	/*ui->config_panel->setTitle(QString("Configuration for %1").arg(
					   channels[currentChannel]->fullName()));*/
	QString stylesheet = "QTabBar::tab:selected {\
			color: white;\
			margin-top: 0px;\
			border-bottom: 2px solid "
		      + channels[currentChannel]->color().name() + "}";
	ui->tabWidget->setStyleSheet(stylesheet);
}

int SignalGenerator::sg_waveform_to_idx(enum sg_waveform wave)
{
	switch (wave) {
	case SG_SIN_WAVE:
	default:
		return 0;

	case SG_SQR_WAVE:
		return 1;

	case SG_TRI_WAVE:
		return 2;

	case SG_TRA_WAVE:
		return 3;

	case SG_SAW_WAVE:
		return 4;

	case SG_INV_SAW_WAVE:
		return 5;

	case SG_STAIR_WAVE:
		return 6;
	}
}

void SignalGenerator::updateRightMenuForChn(int chIdx)
{
	auto ptr = getData(channels[chIdx]);

	constantValue->setValue(ptr->constant);
	frequency->setValue(ptr->frequency);
	phase->blockSignals(true);
	phase->setComboboxIndex(ptr->indexValue);
	phase->blockSignals(false);
	if (ptr->indexValue < 2) {
		phase->setInSeconds(false);
		phase->setValue(ptr->phase);
	} else {
		phase->setInSeconds(true);
		phase->setValue(phase->changeValueFromDegreesToSeconds(ptr->phase));
	}

	fileAmplitude->setEnabled(ptr->file_type != FORMAT_NO_FILE);
	fileSampleRate->setEnabled(ptr->file_type != FORMAT_NO_FILE);
	filePhase->setEnabled(ptr->file_type != FORMAT_NO_FILE);
	fileOffset->setEnabled(ptr->file_type != FORMAT_NO_FILE);

	ui->fileChannel->blockSignals(true);
	ui->fileChannel->clear();

	if (ptr->file_channel_names.isEmpty()) {
		for (auto i=0; i<ptr->file_nr_of_channels; i++) {
			ui->fileChannel->addItem(QString::number(i));
		}
	} else {
		ui->fileChannel->addItems(ptr->file_channel_names);
	}

	ui->fileChannel->setEnabled(ptr->file_nr_of_channels>1);
	ui->fileChannel->setCurrentIndex(ptr->file_channel);
	ui->fileChannel->blockSignals(false);

	offset->setValue(ptr->offset);
	amplitude->setValue(ptr->amplitude);
	dutycycle->setValue(ptr->dutycycle);

	noiseAmplitude->setValue(ptr->noiseAmplitude);
	auto noiseIndex = ui->cbNoiseType->findData(ptr->noiseType);
	ui->cbNoiseType->setCurrentIndex(noiseIndex);

	int lineThicknessIndex = (int)(ptr->lineThickness / 0.5) - 1;
	ui->cbLineThickness->setCurrentIndex(lineThicknessIndex);

	fallTime->setValue(ptr->fall);
	riseTime->setValue(ptr->rise);
	holdHighTime->setValue(ptr->holdh);
	holdLowTime->setValue(ptr->holdl);

	stepsUp->setValue(ptr->steps_up);
	stepsDown->setValue(ptr->steps_down);

	ui->label_path->setText(ptr->file);
	ui->label_format->setText(ptr->file_message);
	if(!ptr->file_nr_of_samples.empty()) {
        ui->label_size->setText(QString::number(ptr->file_nr_of_samples[ptr->file_channel]) +
                                tr(" samples"));
	} else {
		ui->label_size->setText("");
	}

	ui->mathWidget->setFunction(ptr->function);
	mathRecordLength->setValue(ptr->math_record_length);
	fileSampleRate->setValue(ptr->file_sr);
	fileOffset->setValue(ptr->file_offset);
	filePhase->setValue(ptr->file_phase);
	fileAmplitude->setValue(ptr->file_amplitude);

	ui->type->setCurrentIndex(sg_waveform_to_idx(ptr->waveform));
	waveformUpdateUi(ptr->waveform);
	renameConfigPanel();
	ui->tabWidget->setCurrentIndex((int) ptr->type);
	resizeTabWidget((int)ptr->type);
}

void SignalGenerator::updateAndToggleMenu(int chIdx, bool open)
{
	if (open) {
		updateRightMenuForChn(chIdx);
	}

	ui->rightMenu->toggleMenu(open);
}

void adiscope::SignalGenerator::rightMenuFinished(bool opened)
{
	Q_UNUSED(opened)

	// At the end of each animation, check if there are other button check
	// actions that might have happened while animating and execute all
	// these queued actions
	while (menuButtonActions.size()) {
		auto pair = menuButtonActions.dequeue();
		int chIdx = pair.first;
		bool open = pair.second;

		currentChannel = chIdx;
		updateAndToggleMenu(chIdx, open);
	}
}

bool SignalGenerator::use_oversampling(unsigned int chnIdx)
{
	if (!m_m2k_analogout->isChannelEnabled(chnIdx)) {
		return false;
	}

	QWidget *w = channels.at(chnIdx);
	auto ptr = getData(w);

	switch (ptr->type) {
	case SIGNAL_TYPE_WAVEFORM:

		/* We only want oversampling for square waveforms */
		if (ptr->waveform == SG_SQR_WAVE || ptr->waveform == SG_STAIR_WAVE) {
			return true;
		}

		break;

	case SIGNAL_TYPE_BUFFER:
		return true;
	case SIGNAL_TYPE_MATH:
		return true;

	default:
		break;
	}

	return false;
}


double SignalGenerator::get_best_sample_rate(unsigned int chnIdx)
{
	std::vector<double> values = m_m2k_analogout->getAvailableSampleRates(chnIdx);

	std::sort(values.begin(), values.end(), std::greater<double>());

	/* When using oversampling, we actually want to generate the
	 * signal with the lowest sample rate possible. */

	if (sample_rate_forced(chnIdx)) {
		return get_forced_sample_rate(chnIdx);
	}

	if (use_oversampling(chnIdx)) {
		qSort(values.begin(), values.end(), qLess<double>());
	}

	/* Return the best sample rate that we can create a buffer for */
	for (unsigned long rate : values) {
		size_t buf_size = get_samples_count(chnIdx, rate, true);

		if (buf_size) {
			return rate;
		}

		qDebug(CAT_SIGNAL_GENERATOR) << QString("Rate %1 not ideal").arg(rate);
	}

	/* If we can't find a perfect sample rate, use the highest one */
	if (use_oversampling(chnIdx)) {
		qSort(values.begin(), values.end(), qGreater<double>());
	}


	for (unsigned long rate : values) {
		size_t buf_size = get_samples_count(chnIdx, rate);

		if (buf_size) {
			return rate;
		}

		qDebug(CAT_SIGNAL_GENERATOR) << QString("Rate %1 not possible").arg(rate);
	}

	return -1;
}

bool SignalGenerator::sample_rate_forced(unsigned int chnIdx)
{
	if (!m_m2k_analogout->isChannelEnabled(chnIdx)) {
		return false;
	}

	QWidget *w = channels.at(chnIdx);
	auto ptr = getData(w);

	if (ptr->file_type && ptr->file_sr && ptr->type==SIGNAL_TYPE_BUFFER) {
		return true;
	}
	if (ptr->type == SIGNAL_TYPE_WAVEFORM && ptr->waveform == SG_STAIR_WAVE)
		return true;

	if(ptr->type==SIGNAL_TYPE_MATH)
		return true;
	return false;
}

double SignalGenerator::get_forced_sample_rate(unsigned int chnIdx)
{
	if (sample_rate_forced(chnIdx)) {
		QWidget *w = channels.at(chnIdx);
		auto ptr = getData(w);

		if (ptr->file_type && ptr->file_sr && ptr->type==SIGNAL_TYPE_BUFFER) {
			return ptr->file_sr;
		}

		if (ptr->type == SIGNAL_TYPE_WAVEFORM && ptr->waveform == SG_STAIR_WAVE)
			return (ptr->frequency * (ptr->steps_up + ptr->steps_down));

		if(ptr->type==SIGNAL_TYPE_MATH)	{
			return ptr->math_sr;
		}
	}
	return false;
}

void SignalGenerator::calc_sampling_params(unsigned int chnIdx,
		double rate, unsigned long& out_sample_rate,
                unsigned long& out_oversampling_ratio)
{
	if (use_oversampling(chnIdx)) {
		/* We assume that the rate requested here will always be a
		 * divider of the max sample rate */
		out_oversampling_ratio = max_sample_rate / rate;
		out_sample_rate = max_sample_rate;

		qDebug(CAT_SIGNAL_GENERATOR) << QString("Using oversampling with a ratio of %1")
		         .arg(out_oversampling_ratio);
	} else {
		out_sample_rate = rate;
		out_oversampling_ratio = 1;
	}
}

void SignalGenerator::reduceFraction(double input, long *numerator, long *denominator, long precision)
{
    double integral = std::floor(input);
    double frac = input - integral;
    long gcd_ = gcd(round(frac * precision), precision);

    *denominator = precision / gcd_;
    *numerator = (round(frac * precision) / gcd_) + ((long)integral*(*denominator));
}

size_t SignalGenerator::gcd(size_t a, size_t b)
{
	for (;;) {
		if (!a) {
			return b;
		}

		b %= a;

		if (!b) {
			return a;
		}

		a %= b;
	}
}

size_t SignalGenerator::lcm(size_t a, size_t b)
{
	size_t temp = gcd(a, b);

	return temp ? (a / temp * b) : 0;
}

double SignalGenerator::get_best_ratio(double ratio, double max, double *fract)
{
	double max_it = max / ratio;
	double best_ratio = ratio;
	double best_fract = 1.0;

	for (double i = 1.0; i < max_it; i += 1.0) {
		double integral, new_ratio = i * ratio;
		double new_fract = modf(new_ratio, &integral);

		if (new_fract < best_fract) {
			best_fract = new_fract;
			best_ratio = new_ratio;
		}

		if (new_fract == 0.0) {
			break;
		}
	}

	qDebug(CAT_SIGNAL_GENERATOR) << QString("Input ratio %1, ratio: %2 (fract left %3)")
	         .arg(ratio).arg(best_ratio).arg(best_fract);

	if (fract) {
		*fract = best_fract;
	}

	return best_ratio;
}

std::vector<float> SignalGenerator::get_stairstep(int rise, int fall,
		float amplitude, float offset, int phase)
{

	std::vector<float> aux,buff,rising_buff,falling_buff,phased,final_buff;
	rising_buff.clear();
	falling_buff.clear();
	aux.clear();
	buff.clear();
	phased.clear();
	for(float i=-amplitude;i<=amplitude;i+=amplitude/(float)rise*2.0){
		rising_buff.push_back(i);
	}
	for(float i=amplitude;i>=-amplitude;i-=amplitude/(float)fall*2.0){
		falling_buff.push_back(i);
	}
	for(int i =0;i<(rise+fall);i++){
		if(i<rise)
			aux.push_back((rising_buff[i]) + offset);

		else
			aux.push_back((falling_buff[i-rise]) + offset);

	}
	for(int i=0;i<phase;i++){
		phased.push_back(aux[i]);
	}
	for(int i=phase;i<rise+fall+phase;i++){
		if(i<rise+fall)
			buff.push_back(aux[i]);
		else
			buff.push_back(phased[i-(rise+fall)]);

	}

	for(int i=0;i<(rise+fall)*MULTIPLY_CT;i++)
		for(int k=0;k<rise+fall;k++)
			final_buff.push_back(buff[k]);

	return final_buff;

}

size_t SignalGenerator::get_samples_count(unsigned int chnIdx,
		double rate, bool perfect)
{
	size_t size = 1;
	size_t max_buffer_size=min_buffer_size<<1;
	max_buffer_size = m_maxNbOfSamples;
	QWidget *w;
	QSharedPointer<signal_generator_data>  ptr;

	if (!m_m2k_analogout->isChannelEnabled(chnIdx)) {
		goto out_cleanup;
	}

	w = channels.at(chnIdx);
	ptr = getData(w);
	double ratio, fract;

	switch (ptr->type) {
	case SIGNAL_TYPE_WAVEFORM:
		if(ptr->waveform == SG_STAIR_WAVE)
		{
			return (ptr->steps_up+ptr->steps_down)*MULTIPLY_CT;
		}

		ratio = (double) rate / ptr->frequency;

		// for less than max sample rates, generate at least 10 samples per period
		if (ratio < 10.0 && rate < max_sample_rate)
			return 0;
		if (ratio < 2.0) {
			return 0;        /* rate too low */
		}

		/* The ratio must be even for square waveforms */
		if (perfect && (ptr->type == SIGNAL_TYPE_WAVEFORM)
				&& (ptr->waveform == SG_SQR_WAVE)
				&& (fmod(ratio, 2.0) != 0.0)) {
			return 0;
		}


		ratio = get_best_ratio(ratio,
				       (double)(max_buffer_size / 4), &fract);

		if (perfect && fract != 0.0) {
			return 0;
		}

		size = lcm(size, (size_t) ratio);
		break;
	case SIGNAL_TYPE_MATH:
		size = (size_t)ptr->math_sr * ptr->math_record_length;
		break;

	case SIGNAL_TYPE_BUFFER:
		if (!ptr->file_type) {
			return 0;
		}

		if (perfect && rate!=ptr->file_sr) {
			return 0;
		}

		ratio = rate/ptr->file_sr;
		if (ptr->file_nr_of_samples.size() > 0) {
			size=(ptr->file_nr_of_samples[ptr->file_channel] * ratio);
		}
		break;

	case SIGNAL_TYPE_CONSTANT:
	default:
		break;
	}

out_cleanup:
	/* The buffer size must be a multiple of 4 */
	if(ptr->type == SIGNAL_TYPE_BUFFER)
	{
		size = size + size % 0x04;
	}
	else
		{
		while (size & 0x3) {
			size <<= 1;
		}
	}

	/* The buffer size shouldn't be too small */
	while (size < min_buffer_size) {
		size <<= 1;
	}

	if (size > max_buffer_size) {
		return 0;
	}

	return size;
}


