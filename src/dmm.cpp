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

#include "dmm.hpp"
#include "gui/dynamicWidget.hpp"
#include "ui_dmm.h"
#include <config.h>
#include "utils.h"
#include "logging_categories.h"

#include <gnuradio/blocks/keep_one_in_n.h>
#include <gnuradio/blocks/moving_average.h>
#include <gnuradio/blocks/rms_ff.h>
#include <gnuradio/blocks/short_to_float.h>
#include <gnuradio/blocks/sub.h>
#include <gnuradio/blocks/skiphead.h>
#include <gnuradio/blocks/delay.h>
#include <gnuradio/filter/dc_blocker_ff.h>
#include <gnuradio/blocks/max_blk.h>
#include <gnuradio/blocks/min_blk.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/vector_to_stream.h>

#include <boost/make_shared.hpp>

#include <memory>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QThread>
#include <QJSEngine>

/* libm2k includes */
#include <libm2k/contextbuilder.hpp>
#include <scopyExceptionHandler.h>
#include <libm2k/m2kexceptions.hpp>

#include "dmm_api.hpp"

using namespace adiscope;
using namespace libm2k;
using namespace libm2k::context;

DMM::DMM(struct iio_context *ctx, Filter *filt, ToolMenuItem *toolMenuItem,
	 QJSEngine *engine, ToolLauncher *parent)
	: Tool(ctx, toolMenuItem, new DMM_API(this), "Voltmeter", parent),
	  ui(new Ui::DMM), signal(boost::make_shared<signal_sample>()),
	  manager(iio_manager::get_instance(ctx, filt->device_name(TOOL_DMM))),
	  m_m2k_context(m2kOpen(ctx, "")),
	m_m2k_analogin(m_m2k_context->getAnalogIn()),
	m_adc_nb_channels(m_m2k_analogin->getNbChannels()),
	interrupt_data_logging(false),
	data_logging(false),
	filename(""),
	use_timer(false),
	logging_refresh_rate(0),
	wheelEventGuard(nullptr),
	m_autoGainEnabled({true, true}),
	m_gainHistorySize(25)
{
	ui->setupUi(this);

	/* TODO: avoid hardcoding sample rate */
	sample_rate = 1e5;

	ui->sismograph_ch1->setColor(QColor("#ff7200"));
	ui->sismograph_ch2->setColor(QColor("#9013fe"));

	std::vector<adiscope::CustomScale *> scales {ui->scaleCh1, ui->scaleCh2};
	for (auto scale : scales) {
		scale->setOrientation(Qt::Horizontal);
		scale->setScalePosition(QwtThermo::LeadingScale);
		scale->setOriginMode(QwtThermo::OriginCustom);
	}

	data_logging_timer = new PositionSpinButton({
		{"s", 1},
		{"min", 60},
		{"h", 3600}
	}, tr("Timer"), 0, 3600,
	true, false, this);

	ui->horizontalLayout_2->addWidget(data_logging_timer);

	for (unsigned int i = 0; i < m_adc_nb_channels; i++)
	{
		m_min.push_back(Q_INFINITY);
		m_max.push_back(-Q_INFINITY);
	}

	connect(ui->run_button, SIGNAL(toggled(bool)),
			this, SLOT(toggleTimer(bool)));
	connect(ui->run_button, SIGNAL(toggled(bool)),
			runButton(), SLOT(setChecked(bool)));
	connect(runButton(), SIGNAL(toggled(bool)), ui->run_button,
			SLOT(setChecked(bool)));
	connect(ui->run_button, SIGNAL(toggled(bool)),
		this, SLOT(startDataLogging(bool)));

	connect(ui->btnDataLogging, SIGNAL(toggled(bool)),
		this, SLOT(toggleDataLogging(bool)));

	connect(ui->btnChooseFile, SIGNAL(clicked()),
		this, SLOT(chooseFile()));

	connect(ui->btn_overwrite, &QRadioButton::toggled, [&](bool en) {
		if(!ui->run_button->isChecked()) {
			toggleDataLogging(data_logging);
		}
	});

	connect(ui->btn_append, &QRadioButton::toggled, [&](bool en) {
		if(!ui->run_button->isChecked()) {
			toggleDataLogging(data_logging);
		}
	});

	connect(data_logging_timer, &PositionSpinButton::valueChanged, [&](double value) {
		if(value == 0)
			use_timer = false;
		else use_timer = true;
		logging_refresh_rate = value * 1000;
	});

	data_logging_timer->setValue(0);
	enableDataLogging(false);

	connect(ui->btn_ch1_ac, SIGNAL(toggled(bool)), this, SLOT(toggleAC()));
	connect(ui->btn_ch2_ac, SIGNAL(toggled(bool)), this, SLOT(toggleAC()));

	connect(ui->btn_ch1_dc, &QPushButton::toggled, [&](bool en) {
		setDynamicProperty(ui->labelCh1, "ac", !en);
	});
	connect(ui->btn_ch2_dc, &QPushButton::toggled, [&](bool en) {
		setDynamicProperty(ui->labelCh2, "ac", !en);
	});

	connect(ui->historySizeCh1, SIGNAL(currentIndexChanged(int)),
			this, SLOT(setHistorySizeCh1(int)));
	connect(ui->historySizeCh2, SIGNAL(currentIndexChanged(int)),
			this, SLOT(setHistorySizeCh2(int)));

        connect(ui->cbLineThicknessCh1, SIGNAL(currentIndexChanged(int)),
                this, SLOT(setLineThicknessCh1(int)));
        connect(ui->cbLineThicknessCh2, SIGNAL(currentIndexChanged(int)),
                this, SLOT(setLineThicknessCh2(int)));

	connect(ui->btnResetPeakHold, SIGNAL(clicked(bool)),
		SLOT(resetPeakHold(bool)));
	connect(ui->btnDisplayPeakHold, SIGNAL(toggled(bool)),
		SLOT(displayPeakHold(bool)));
	connect(ui->btnCollapseDataLog, SIGNAL(toggled(bool)),
		SLOT(collapseDataLog(bool)));
	connect(ui->btnCollapsePeakHold, SIGNAL(toggled(bool)),
		SLOT(collapsePeakHold(bool)));

	std::vector<QComboBox *> comboBoxes {ui->gainCh1ComboBox, ui->gainCh2ComboBox};

	for (auto cb : comboBoxes) {
		connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, &DMM::gainModeChanged);
		cb->setCurrentIndex(0);
	}

	setHistorySizeCh1(ui->historySizeCh1->currentIndex());
	setHistorySizeCh2(ui->historySizeCh2->currentIndex());

	setLineThicknessCh1(ui->cbLineThicknessCh1->currentIndex());
        setLineThicknessCh2(ui->cbLineThicknessCh2->currentIndex());

        /* Lock the flowgraph if we are already started */
	bool started = isIioManagerStarted();
	if (started)
		manager->lock();

	configureModes();

	connect(&*signal, SIGNAL(triggered(std::vector<float>)),
			this, SLOT(updateValuesList(std::vector<float>)));

	if (started)
		manager->unlock();

	api->setObjectName(QString::fromStdString(Filter::tool_name(
			TOOL_DMM)));
	api->load(*settings);
	api->js_register(engine);

	if(!wheelEventGuard)
		wheelEventGuard = new MouseWheelWidgetGuard(ui->widget_2);
	wheelEventGuard->installEventRecursively(ui->widget_2);
	readPreferences();

	ui->btnHelp->setUrl("https://wiki.analog.com/university/tools/m2k/scopy/voltmeter");

	for(unsigned int i=0;i < m_adc_nb_channels;i++)
	{
		m_gainHistory.push_back(boost::circular_buffer<libm2k::analog::M2K_RANGE>(m_gainHistorySize));
	}
}

void DMM::readPreferences()
{
	ui->instrumentNotes->setVisible(prefPanel->getInstrumentNotesActive());
}

void DMM::gainModeChanged(int idx)
{
	const QObject *who = QObject::sender();
	const int channelIdx = who == ui->gainCh1ComboBox ? 0 : (who == ui->gainCh2ComboBox ? 1 : -1);

	if (channelIdx == -1 || !who) {
		return;
	}

	QLabel *gainLabel = channelIdx == 0 ? ui->currentGainCh1Label : ui->currentGainCh2Label;

	const bool started = isIioManagerStarted();
	if (started) {
		manager->lock();
	}

	// rebuild the flowgraph
	// there might be data on in the gnuradio flowgraph that
	// was captured using one gain mode and plotted and converted
	// with another gain mode. This will ensure this situation doesn't
	// happen
	manager->disconnect(id_ch1);
	manager->disconnect(id_ch2);

	configureModes();

	m_autoGainEnabled[channelIdx] = false;
	if (idx == 0) { // auto
		m_autoGainEnabled[channelIdx] = true;
	} else if (idx == 1) { // low
		gainLabel->setText("±25V");
		m_m2k_analogin->setRange(static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(channelIdx),
					 libm2k::analog::PLUS_MINUS_25V);
	} else if (idx == 2) { // high
		gainLabel->setText("±2.5V");
		m_m2k_analogin->setRange(static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(channelIdx),
					 libm2k::analog::PLUS_MINUS_2_5V);
	}

	if (started) {
		manager->start(id_ch1);
		manager->start(id_ch2);
		manager->unlock();
	}
}

void DMM::disconnectAll()
{
	bool started = isIioManagerStarted();
	if (started)
		manager->lock();

	manager->disconnect(id_ch1);
	manager->disconnect(id_ch2);

	if (started)
		manager->unlock();
}

DMM::~DMM()
{
	disconnect(prefPanel, &Preferences::notify, this, &DMM::readPreferences);
	ui->run_button->setChecked(false);
	disconnectAll();

	if (saveOnExit) {
		api->save(*settings);
	}
	delete api;

	delete ui;
}

void DMM::updateValuesList(std::vector<float> values)
{
	if(!use_timer)
		boost::unique_lock<boost::mutex> lock(data_mutex);

	const double volts_ch1 = m_m2k_analogin->convertRawToVolts(0, static_cast<int>(values[0]));
	const double volts_ch2 = m_m2k_analogin->convertRawToVolts(1, static_cast<int>(values[1]));

	ui->lcdCh1->display(volts_ch1);
	ui->lcdCh2->display(volts_ch2);

	ui->scaleCh1->setValue(volts_ch1);
	ui->scaleCh2->setValue(volts_ch2);

	ui->sismograph_ch1->plot(volts_ch1);
	ui->sismograph_ch2->plot(volts_ch2);

	checkPeakValues(0, volts_ch1);
	checkPeakValues(1, volts_ch2);

	checkAndUpdateGainMode({m_m2k_analogin->convertRawToVolts(0, static_cast<int>(values[2])),
			       m_m2k_analogin->convertRawToVolts(0, static_cast<int>(values[3])),
			       m_m2k_analogin->convertRawToVolts(1, static_cast<int>(values[4])),
			       m_m2k_analogin->convertRawToVolts(1, static_cast<int>(values[5]))});

	if(!use_timer)
		data_cond.notify_all();
}

void DMM::checkPeakValues(int ch, double peak)
{
	if(peak < m_min[ch])
	{
		m_min[ch] = peak;
		if(ch == 0) ui->minCh1->display(m_min[ch]);
		if(ch == 1) ui->minCh2->display(m_min[ch]);
	}
	if(peak > m_max[ch])
	{
		m_max[ch] = peak;
		if(ch == 0) ui->maxCh1->display(m_max[ch]);
		if(ch == 1) ui->maxCh2->display(m_max[ch]);
	}
}

bool DMM::isIioManagerStarted() const
{
	return manager->started() && ui->run_button->isChecked();
}

libm2k::analog::M2K_RANGE DMM::suggestRange(double volt_max, double volt_min)
{

	double hi = 0.0;
	double lo = 0.0;
	std::tie(lo, hi) = m_m2k_analogin->getRangeLimits(libm2k::analog::PLUS_MINUS_2_5V);

	// hist interval 10% from range of High gain ~ 250 mV
	const double percent = 0.1; // 10%
	const double half_hist_interval = (hi * percent) / 2.0;

	const double hi_hi = hi + half_hist_interval;
	const double hi_lo = hi - half_hist_interval;
	const double lo_hi = lo + half_hist_interval;
	const double lo_lo = lo - half_hist_interval;

	libm2k::analog::M2K_RANGE gain = libm2k::analog::PLUS_MINUS_25V;

	if (volt_max > 0.0) {
		if (volt_max >= hi_hi) {
			gain = libm2k::analog::PLUS_MINUS_25V;
		} else if (volt_max <= hi_lo) {
			gain = libm2k::analog::PLUS_MINUS_2_5V;
		}

		if (volt_min <= 0.0) {
			if (volt_min >= lo_hi ) {
				gain = libm2k::analog::PLUS_MINUS_2_5V;
			} else if (volt_min <= lo_lo) {
				gain = libm2k::analog::PLUS_MINUS_25V;
			}
		}
	}
	return gain;
}

void DMM::checkAndUpdateGainMode(const std::vector<double> &volts)
{
	if (volts.size() < m_adc_nb_channels * 2) {
		return;
	}

	std::vector<QLabel *> gainLabels {ui->currentGainCh1Label, ui->currentGainCh2Label};
	std::vector<QLabel *> errorLabels {ui->ch1ErrorLabel, ui->ch2ErrorLabel};


	libm2k::analog::M2K_RANGE gain[m_adc_nb_channels];
	libm2k::analog::M2K_RANGE currentChannelGain[m_adc_nb_channels];
	bool recreateFlowGraph = false;

	for (unsigned int i = 0; i < m_adc_nb_channels; ++i) {
		gain[i]=libm2k::analog::PLUS_MINUS_25V;
		currentChannelGain[i] = m_m2k_analogin->getRange(static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(i));
		//gain[i] = currentChannelGain[i];//libm2k::analog::PLUS_MINUS_25V;

		// add all gains to circular buffer
		auto suggested_range = suggestRange (volts[m_adc_nb_channels * i], volts[(m_adc_nb_channels * i) + 1]);
		m_gainHistory[i].push_back(suggested_range);


		// define m2k_range sum computation operation as lambda
		auto range_sum = [](double a, libm2k::analog::M2K_RANGE b) {
					 return (a) + static_cast<double>(b);
				     };



		// compute average of circular buffer - moving average
		double sum = std::accumulate(m_gainHistory[i].begin(),m_gainHistory[i].end(), 0.0, range_sum);
		double avg = sum / m_gainHistory[i].size();
		// only change to high gain if ALL values in circular buffer == HIGH_GAIN
		// this means as soon as we find a value out of HIGH range we switch immediately - but wait m_gainHistorySize to change back to HIGH
		if(avg == libm2k::analog::PLUS_MINUS_2_5V) {
			gain[i] = libm2k::analog::PLUS_MINUS_2_5V;
		}

		if(gain[i] != currentChannelGain[i] ) {
			if(m_autoGainEnabled[i]) {
				errorLabels[i]->setText("");
				m_m2k_analogin->setRange(static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(i), gain[i]);
				gainLabels[i]->setText(gain[i] == libm2k::analog::PLUS_MINUS_25V ? "±25V" : "±2.5V");
				recreateFlowGraph = true;

			} else {
				if(currentChannelGain[i] == libm2k::analog::PLUS_MINUS_2_5V) { // only show out of range for +/- 2.5
					errorLabels[i]->setText("Out of range");
				} else	{
					errorLabels[i]->setText("");
				}
			}
		} else {
			errorLabels[i]->setText("");
		}


	}

	if(recreateFlowGraph) {
		const bool started = isIioManagerStarted();
		if (started) {
			manager->lock();
		}

		// rebuild the flowgraph
		// there might be data on in the gnuradio flowgraph that
		// was captured using one gain mode and plotted and converted
		// with another gain mode. This will ensure this situation doesn't
		// happen
		manager->disconnect(id_ch1);
		manager->disconnect(id_ch2);

		configureModes();

		if (started) {
			manager->start(id_ch1);
			manager->start(id_ch2);
			manager->unlock();
		}
	}
}


void DMM::collapseDataLog(bool checked)
{
	if(checked)
		ui->dataLogWidget->hide();
	else
		ui->dataLogWidget->show();
}

void DMM::collapsePeakHold(bool checked)
{
	if(checked)
		ui->gridLayout_4Widget->hide();
	else
		ui->gridLayout_4Widget->show();
}

void DMM::displayPeakHold(bool checked)
{
	if(!checked) {
		Util::retainWidgetSizeWhenHidden(ui->peakCh1Widget);
		Util::retainWidgetSizeWhenHidden(ui->peakCh2Widget);
		ui->peakCh1Widget->hide();
		ui->peakCh2Widget->hide();
	}
	else  {
		ui->peakCh1Widget->show();
		ui->peakCh2Widget->show();
	}
}

void DMM::resetPeakHold(bool clicked)
{
	for(unsigned int ch = 0; ch < m_adc_nb_channels; ch++) {
		m_min[ch] = Q_INFINITY;
		m_max[ch] = -Q_INFINITY;
		if(ch == 0) {
			ui->maxCh1->display(0);
			ui->minCh1->display(0);
		}
		if(ch == 1) {
			ui->maxCh2->display(0);
			ui->minCh2->display(0);
		}
	}
}
void DMM::toggleTimer(bool start)
{
	enableDataLogging(start);
	if (start) {
		manager->set_kernel_buffer_count(4);
		writeAllSettingsToHardware();
		manager->start(id_ch1);
		manager->start(id_ch2);

		ui->scaleCh1->start();
		ui->scaleCh2->start();
	} else {
		ui->scaleCh1->stop();
		ui->scaleCh2->stop();

		manager->stop(id_ch1);
		manager->stop(id_ch2);
		manager->set_kernel_buffer_count();
	}

	setDynamicProperty(ui->run_button, "running", start);
	ui->run_button->setText(start ? tr("Stop") : tr("Run"));
	m_running = start;
}


gr::basic_block_sptr DMM::configureGraph(gr::basic_block_sptr s2f,
		bool is_ac)
{
	/* 10 fps refresh rate for the plot */
	auto keep_one = gr::blocks::keep_one_in_n::make(sizeof(float),
			sample_rate / 10.0);

	auto blocker_val = 4000;
	auto blocker = gr::filter::dc_blocker_ff::make(blocker_val, true);

	manager->connect(s2f, 0, blocker, 0);

	if (is_ac) {
		/* TODO: figure out best value for the RMS parameter */
		auto rms = gr::blocks::rms_ff::make(0.0001);
		manager->connect(blocker, 0, rms, 0);
		manager->connect(rms, 0, keep_one, 0);
	} else {
		auto sub = gr::blocks::sub_ff::make();
		manager->connect(s2f, 0, sub, 0);
		manager->connect(blocker, 0, sub, 1);
		auto moving = gr::blocks::moving_average_ff::make(4000,1.0/4000);
		manager->connect(sub, 0, moving,0 );
		manager->connect(moving, 0, keep_one, 0);
	}

	return keep_one;
}

void DMM::configureModes()
{
	auto s2f1 = gr::blocks::short_to_float::make();
	auto s2f2 = gr::blocks::short_to_float::make();

	auto max1 = gr::blocks::max_ff::make(sample_rate / 10);
	auto max2 = gr::blocks::max_ff::make(sample_rate / 10);
	auto min1 = gr::blocks::min_ff::make(sample_rate / 10);
	auto min2 = gr::blocks::min_ff::make(sample_rate / 10);
	auto stv1 = gr::blocks::stream_to_vector::make(sizeof(float), sample_rate / 10);
	auto stv2 = gr::blocks::stream_to_vector::make(sizeof(float), sample_rate / 10);

	const bool is_ac_ch1 = ui->btn_ch1_ac->isChecked();
	const bool is_ac_ch2 = ui->btn_ch2_ac->isChecked();

	id_ch1 = manager->connect(s2f1, 0, 0, false, sample_rate / 10);
	id_ch2 = manager->connect(s2f2, 1, 0, false, sample_rate / 10);

	manager->connect(s2f1, 0, stv1, 0);
	manager->connect(s2f2, 0, stv2, 0);
	manager->connect(stv1, 0, max1, 0);
	manager->connect(stv2, 0, max2, 0);
	manager->connect(stv1, 0, min1, 0);
	manager->connect(stv2, 0, min2, 0);

	auto block1 = configureGraph(s2f1, is_ac_ch1);
	auto block2 = configureGraph(s2f2, is_ac_ch2);

	manager->connect(block1, 0, signal, 0);
	manager->connect(block2, 0, signal, 1);

	manager->connect(max1, 0, signal, 2);
	manager->connect(min1, 0, signal, 3);
	manager->connect(max2, 0, signal, 4);
	manager->connect(min2, 0, signal, 5);
}

void DMM::chooseFile()
{
	QString selectedFilter;

	filename = QFileDialog::getSaveFileName(this,
	    tr("Export"), "", tr("Comma-separated values files (*.csv);;All Files(*)"),
	    &selectedFilter, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	ui->filename->setText(filename);

	if(!ui->run_button->isChecked()) {
		toggleDataLogging(data_logging);
	}

	if (ui->run_button->isChecked() && data_logging) {
		toggleDataLogging(false);
		toggleDataLogging(true);
	}

}

void DMM::enableDataLogging(bool en)
{
	ui->gridLayout_3Widget->setEnabled(en);
	if (!en) {
		setDynamicProperty(ui->filename, "invalid", false);
	}
}

void DMM::toggleDataLogging(bool en)
{
	data_logging = en;
	if (en) {
		enableDataLogging(en);
		setDynamicProperty(ui->filename, "invalid", false);
	} else if (!ui->run_button->isChecked()) {
		enableDataLogging(en);
	}

	/* If DMM is already running, check all the parameters before
	 * starting the data logging */
	if(filename.isEmpty() && ui->filename->isEnabled()) {
		ui->filename->setText(tr("No file selected"));
		setDynamicProperty(ui->filename, "invalid", true);
		if(ui->run_button->isChecked()) {
			ui->btnDataLogging->setChecked(false);
		}
		return;
	}

	if(ui->run_button->isChecked()) {
		ui->btn_overwrite->setEnabled(false);
		ui->btn_append->setEnabled(false);
	}

	if(en && ui->run_button->isChecked()) {
		QFile file(filename);

		if(ui->btn_overwrite->isChecked() || file.size() == 0) {
			if( !file.open(QIODevice::WriteOnly)) {
				ui->lblFileStatus->setText(tr("File is open in another program"));
				setDynamicProperty(ui->filename, "invalid", true);
				if(ui->run_button->isChecked()) {
					ui->btnDataLogging->setChecked(false);
				}
				return;
			} else {
				ui->lblFileStatus->setText(tr("Choose a file"));
				setDynamicProperty(ui->filename, "invalid", false);
			}
			QTextStream out(&file);

			/* Write the header */
			out << ";Generated by Scopy-" << QString(SCOPY_VERSION_GIT) << "\n" <<
			       ";Started on " <<  QDateTime::currentDateTime().toString() << "\n";
			out << "Timestamp,Channel_0_DC_RMS,Channel_0_AC_RMS,Channel_1_DC_RMS,Channel_1_AC_RMS\n";

			file.close();
		}
	}
	else {
		ui->btn_overwrite->setEnabled(true);
		ui->btn_append->setEnabled(true);
	}

	/* If running, start the thread */
	if(ui->run_button->isChecked() && en) {
		if(data_logging_thread.joinable())
			return;

		interrupt_data_logging = false;
		data_logging_thread = std::thread(&DMM::dataLoggingThread, this);
	}
	else if(!en) {
		if(!use_timer)
			data_cond.notify_all();
		interrupt_data_logging = true;
		if(data_logging_thread.joinable()) {
			data_logging_thread.detach();
		}
	}
}

void DMM::startDataLogging(bool start)
{
	if(!data_logging)
		return;

	toggleDataLogging(data_logging);
	if(start) {
		if(filename == "")
			return;
		if(data_logging_thread.joinable())
			return;

		interrupt_data_logging = false;
		ui->btn_overwrite->setEnabled(false);
		ui->btn_append->setEnabled(false);
		data_logging_thread = std::thread(&DMM::dataLoggingThread, this);

	}
	else {
		if(!use_timer)
			data_cond.notify_all();
		interrupt_data_logging = true;
		if(data_logging_thread.joinable()) {
			data_logging_thread.detach();
		}
		ui->btn_overwrite->setEnabled(true);
		ui->btn_append->setEnabled(true);
	}
}

void DMM::dataLoggingThread()
{
	QString separator = ",";
	QFile file(filename);

	QTextStream out(&file);

	while(!interrupt_data_logging) {
		if (!file.isOpen()) {
			if (!file.open(QIODevice::Append)) {
				ui->lblFileStatus->setText(tr("File is open in another program"));
				setDynamicProperty(ui->filename, "invalid", true);
				if(ui->run_button->isChecked()) {
					ui->btnDataLogging->setChecked(false);
				}
				return;
			} else {
				ui->lblFileStatus->setText(tr("Choose a file"));
			}
		}
		bool is_ac_ch1 = ui->btn_ch1_ac->isChecked();
		bool is_ac_ch2 = ui->btn_ch2_ac->isChecked();

		QString ch1_dc_rms="-", ch2_dc_rms="-", ch1_ac_rms="-", ch2_ac_rms="-";

		out << QDateTime::currentDateTime().time().toString() << separator;

		if(!use_timer) {
			boost::unique_lock<boost::mutex> lock(data_mutex);
			data_cond.wait(lock);
		}


		if(!is_ac_ch1) {
			ch1_dc_rms = QString::number(ui->lcdCh1->value());
		}
		else {
			ch1_ac_rms = QString::number(ui->lcdCh1->value());
		}


		if(!is_ac_ch2) {
			ch2_dc_rms = QString::number(ui->lcdCh2->value());
		}
		else {
			ch2_ac_rms = QString::number(ui->lcdCh2->value());
		}

		/* Write the values to file */
		out <<  ch1_dc_rms << separator <<
			ch1_ac_rms << separator <<
			ch2_dc_rms << separator <<
			ch2_ac_rms << "\n";

		if (file.isOpen()) {
			file.close();
		}
		if (use_timer) {
			QThread::msleep(logging_refresh_rate);
		}
	}
	if (file.isOpen()) {
		file.close();
	}
}

void DMM::toggleAC()
{
	bool started = isIioManagerStarted();
	if (started)
		manager->lock();

	manager->disconnect(id_ch1);
	manager->disconnect(id_ch2);

	configureModes();

	if (started) {
		writeAllSettingsToHardware();
		manager->start(id_ch1);
		manager->start(id_ch2);
		manager->unlock();
	}
}

int DMM::numSamplesFromIdx(int idx)
{
	switch(idx) {
	case 0:
		return 10;
	case 1:
		return 100;
	case 2:
		return 600;
	default:
		throw std::runtime_error("Invalid IDX");
	}
}

void DMM::setHistorySizeCh1(int idx)
{
	int num_samples = numSamplesFromIdx(idx);

	ui->sismograph_ch1->setNumSamples(num_samples);
}

void DMM::setHistorySizeCh2(int idx)
{
	int num_samples = numSamplesFromIdx(idx);

	ui->sismograph_ch2->setNumSamples(num_samples);
}

void DMM::setLineThicknessCh1(int idx)
{
        float thickness = 0.5 * (idx + 1);

        ui->cbLineThicknessCh1->setCurrentIndex(idx);
        ui->sismograph_ch1->setLineWidth(thickness);
        ui->sismograph_ch1->replot();
}

void DMM::setLineThicknessCh2(int idx)
{
        float thickness = 0.5 * (idx + 1);

        ui->cbLineThicknessCh2->setCurrentIndex(idx);
        ui->sismograph_ch2->setLineWidth(thickness);
        ui->sismograph_ch2->replot();
}

void DMM::writeAllSettingsToHardware()
{
	if (m_m2k_analogin) {
		try {
			m_m2k_analogin->setSampleRate(sample_rate);
			m_m2k_analogin->setOversamplingRatio(1);
			auto trigger = m_m2k_analogin->getTrigger();
			for (unsigned int i = 0; i < m_adc_nb_channels; i++) {
				auto chn = static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(i);
				m_m2k_analogin->setVerticalOffset(chn, 0.0);
				if (trigger) {
					trigger->setAnalogMode(i, libm2k::ALWAYS);
				}
			}
		} catch (libm2k::m2k_exception &e) {
			HANDLE_EXCEPTION(e);
			qDebug(CAT_VOLTMETER) << "Can't write to hardware: " << e.what();
		}
	}
}

void DMM::run()
{
	//ui->run_button->setChecked(true);
	toggleTimer(true);
}

void DMM::stop()
{
	//ui->run_button->setChecked(false);
	toggleTimer(false);
}
