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

#include "dmm.hpp"
#include "dynamicWidget.hpp"
#include "ui_dmm.h"
#include <config.h>
#include "osc_adc.h"
#include "hardware_trigger.hpp"

#include <gnuradio/blocks/keep_one_in_n.h>
#include <gnuradio/blocks/moving_average_ff.h>
#include <gnuradio/blocks/rms_ff.h>
#include <gnuradio/blocks/short_to_float.h>
#include <gnuradio/blocks/sub_ff.h>
#include <gnuradio/filter/dc_blocker_ff.h>

#include <boost/make_shared.hpp>

#include <memory>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QThread>
#include <QJSEngine>

using namespace adiscope;

DMM::DMM(struct iio_context *ctx, Filter *filt, std::shared_ptr<GenericAdc> adc,
		QPushButton *runButton, QJSEngine *engine, ToolLauncher *parent)
	: Tool(ctx, runButton, new DMM_API(this), "Voltmeter", parent),
	ui(new Ui::DMM), signal(boost::make_shared<signal_sample>()),
	manager(iio_manager::get_instance(ctx, filt->device_name(TOOL_DMM))),
	adc(adc),
	data_logging(false),
	interrupt_data_logging(false),
	filename(""),
	use_timer(false),
	logging_refresh_rate(0)
{
	ui->setupUi(this);

	/* TODO: avoid hardcoding sample rate */
	sample_rate = 1e6;

	ui->sismograph_ch1->setColor(QColor("#ff7200"));
	ui->sismograph_ch2->setColor(QColor("#9013fe"));

	data_logging_timer = new PositionSpinButton({
		{"s", 1},
		{"min", 60},
		{"h", 3600}
	}, "", 0, 3600,
	true, false, this);

	ui->gridLayout_3->addWidget(data_logging_timer, 9, 0);

	for(int i = 0; i < adc->numAdcChannels(); i++)
	{
		m_min.push_back(0);
		m_max.push_back(0);
	}

	connect(ui->run_button, SIGNAL(toggled(bool)),
			this, SLOT(toggleTimer(bool)));
	connect(ui->run_button, SIGNAL(toggled(bool)),
			runButton, SLOT(setChecked(bool)));
	connect(runButton, SIGNAL(toggled(bool)), ui->run_button,
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

	connect(ui->btn_ch1_ac, SIGNAL(toggled(bool)), this, SLOT(toggleAC()));
	connect(ui->btn_ch2_ac, SIGNAL(toggled(bool)), this, SLOT(toggleAC()));

	connect(ui->btn_ch1_ac2, SIGNAL(toggled(bool)), this, SLOT(toggleAC()));
	connect(ui->btn_ch2_ac2, SIGNAL(toggled(bool)), this, SLOT(toggleAC()));

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

	connect(ui->btnResetPeakHold, SIGNAL(clicked(bool)),
		SLOT(resetPeakHold(bool)));
	connect(ui->btnDisplayPeakHold, SIGNAL(toggled(bool)),
		SLOT(displayPeakHold(bool)));
	connect(ui->btnCollapseDataLog, SIGNAL(toggled(bool)),
		SLOT(collapseDataLog(bool)));
	connect(ui->btnCollapsePeakHold, SIGNAL(toggled(bool)),
		SLOT(collapsePeakHold(bool)));

	setHistorySizeCh1(ui->historySizeCh1->currentIndex());
	setHistorySizeCh2(ui->historySizeCh2->currentIndex());

	/* Lock the flowgraph if we are already started */
	bool started = manager->started();
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
}

void DMM::disconnectAll()
{
	bool started = manager->started();
	if (started)
		manager->lock();

	manager->disconnect(id_ch1);
	manager->disconnect(id_ch2);

	if (started)
		manager->unlock();
}

DMM::~DMM()
{
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

	double volts_ch1 = adc->convSampleToVolts(0, (double) values[0]);
	double volts_ch2 = adc->convSampleToVolts(1, (double) values[1]);

	ui->lcdCh1->display(volts_ch1);
	ui->lcdCh2->display(volts_ch2);

	ui->scaleCh1->setValue(volts_ch1);
	ui->scaleCh2->setValue(volts_ch2);

	ui->sismograph_ch1->plot(volts_ch1);
	ui->sismograph_ch2->plot(volts_ch2);

	checkPeakValues(0, volts_ch1);
	checkPeakValues(1, volts_ch2);

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

void DMM::collapseDataLog(bool checked)
{
	if(checked)
		ui->gridLayout_3Widget->hide();
	else
		ui->gridLayout_3Widget->show();
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
	for(int ch = 0; ch < adc->numAdcChannels(); ch++) {
		m_min[ch] = 0;
		m_max[ch] = 0;
		if(ch == 0) {
			ui->maxCh1->display(m_max[ch]);
			ui->minCh1->display(m_min[ch]);
		}
		if(ch == 1) {
			ui->maxCh2->display(m_max[ch]);
			ui->minCh2->display(m_min[ch]);
		}
	}
}
void DMM::toggleTimer(bool start)
{
	if (start) {
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
	}

	setDynamicProperty(ui->run_button, "running", start);
}

gr::basic_block_sptr DMM::configureGraph(gr::basic_block_sptr s2f,
		bool is_low_ac, bool is_high_ac)
{
	/* 10 fps refresh rate for the plot */
	auto keep_one = gr::blocks::keep_one_in_n::make(sizeof(float),
			is_high_ac ? (sample_rate / 10.0) : 1000.0);

	/* TODO: figure out best value for the blocker parameter */
	auto blocker = gr::filter::dc_blocker_ff::make(1000, true);

	manager->connect(s2f, 0, blocker, 0);

	if (is_low_ac || is_high_ac) {
		/* TODO: figure out best value for the RMS parameter */
		auto rms = gr::blocks::rms_ff::make(0.0001);
		manager->connect(blocker, 0, rms, 0);

		manager->connect(rms, 0, keep_one, 0);
	} else {
		auto sub = gr::blocks::sub_ff::make();

		manager->connect(s2f, 0, sub, 0);
		manager->connect(blocker, 0, sub, 1);
		manager->connect(sub, 0, keep_one, 0);
	}

	return keep_one;
}

void DMM::configureModes()
{
	auto s2f1 = gr::blocks::short_to_float::make();
	auto s2f2 = gr::blocks::short_to_float::make();

	bool is_low_ac_ch1 = ui->btn_ch1_ac->isChecked();
	bool is_low_ac_ch2 = ui->btn_ch2_ac->isChecked();
	bool is_high_ac_ch1 = ui->btn_ch1_ac2->isChecked();
	bool is_high_ac_ch2 = ui->btn_ch2_ac2->isChecked();

	if (is_high_ac_ch1) {
		id_ch1 = manager->connect(s2f1, 0, 0, false, sample_rate / 10);
	} else {
		/* Low-frequency AC: decimate data rate */
		auto keep_one1 = gr::blocks::keep_one_in_n::make(
				sizeof(short), sample_rate / 1e4);
		id_ch1 = manager->connect(keep_one1, 0, 0, false,
				sample_rate / 10);

		manager->connect(keep_one1, 0, s2f1, 0);
	}

	if (is_high_ac_ch2) {
		id_ch2 = manager->connect(s2f2, 1, 0, false, sample_rate / 10);
	} else {
		/* Low-frequency AC: decimate data rate */
		auto keep_one2 = gr::blocks::keep_one_in_n::make(
				sizeof(short), sample_rate / 1e4);
		id_ch2 = manager->connect(keep_one2, 1, 0, false,
				sample_rate / 10);

		manager->connect(keep_one2, 0, s2f2, 0);
	}

	auto block1 = configureGraph(s2f1, is_low_ac_ch1, is_high_ac_ch1);
	auto block2 = configureGraph(s2f2, is_low_ac_ch2, is_high_ac_ch2);

	writeAllSettingsToHardware();

	manager->connect(block1, 0, signal, 0);
	manager->connect(block2, 0, signal, 1);
}

void DMM::chooseFile()
{
	QString selectedFilter;
	filename = QFileDialog::getSaveFileName(this,
		tr("Scopy DMM data logging"), "",
		tr("Comma-separated values files (*.csv);;All Files(*)"),
		&selectedFilter);
	ui->filename->setText(filename);
	if(!ui->run_button->isChecked()) {
		toggleDataLogging(data_logging);
	}
}

void DMM::toggleDataLogging(bool en)
{
	data_logging = en;
	setDynamicProperty(ui->filename, "valid", true);

	/* If DMM is already running, check all the parameters before
	 * starting the data logging */
	if(filename.isEmpty()) {
		ui->filename->setText("No file selected");
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
				ui->lblFileStatus->setText("File is open in another program");
				setDynamicProperty(ui->lblFileStatus, "invalid", true);
				if(ui->run_button->isChecked()) {
					ui->btnDataLogging->setChecked(false);
				}
				return;
			} else {
				ui->lblFileStatus->setText("Choose a file");
				setDynamicProperty(ui->lblFileStatus, "invalid", false);
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
				ui->lblFileStatus->setText("File is open in another program");
				setDynamicProperty(ui->lblFileStatus, "invalid", true);
				if(ui->run_button->isChecked()) {
					ui->btnDataLogging->setChecked(false);
				}
				return;
			} else {
				ui->lblFileStatus->setText("Choose a file");
			}
		}
		bool is_low_ac_ch1 = ui->btn_ch1_ac->isChecked();
		bool is_low_ac_ch2 = ui->btn_ch2_ac->isChecked();
		bool is_high_ac_ch1 = ui->btn_ch1_ac2->isChecked();
		bool is_high_ac_ch2 = ui->btn_ch2_ac2->isChecked();
		QString ch1_dc_rms="-", ch2_dc_rms="-", ch1_ac_rms="-", ch2_ac_rms="-";

		out << QDateTime::currentDateTime().time().toString() << separator;

		if(!use_timer) {
			boost::unique_lock<boost::mutex> lock(data_mutex);
			data_cond.wait(lock);
		}


		if(!is_low_ac_ch1 && !is_high_ac_ch1) {
			ch1_dc_rms = QString::number(ui->lcdCh1->value());
		}
		else {
			ch1_ac_rms = QString::number(ui->lcdCh1->value());
		}


		if(!is_low_ac_ch2 && !is_high_ac_ch2) {
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
	bool started = manager->started();
	if (started)
		manager->lock();

	manager->disconnect(id_ch1);
	manager->disconnect(id_ch2);

	configureModes();

	if (started) {
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

void DMM::writeAllSettingsToHardware()
{
	adc->setSampleRate(sample_rate);

	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc);
	if (m2k_adc) {
		for (uint i = 0; i < adc->numAdcChannels(); i++) {
			m2k_adc->setChnHwOffset(i, 0.0);
			m2k_adc->setChnHwGainMode(i, M2kAdc::LOW_GAIN_MODE);
		}

		iio_device_attr_write_longlong(adc->iio_adc_dev(),
			"oversampling_ratio", 1);
	}

	auto trigger = adc->getTrigger();
	if (trigger) {
		for (uint i = 0; i < trigger->numChannels(); i++)
			trigger->setTriggerMode(i, HardwareTrigger::ALWAYS);
	}
}

bool DMM_API::get_mode_ac_high_ch1() const
{
	return dmm->ui->btn_ch1_ac2->isChecked();
}

bool DMM_API::get_mode_ac_low_ch1() const
{
	return dmm->ui->btn_ch1_ac->isChecked();
}

bool DMM_API::get_mode_ac_high_ch2() const
{
	return dmm->ui->btn_ch2_ac2->isChecked();
}

bool DMM_API::get_mode_ac_low_ch2() const
{
	return dmm->ui->btn_ch2_ac->isChecked();
}

void DMM_API::set_mode_ac_high_ch1(bool en)
{
	/* Set DC mode as default */
	if (!get_mode_ac_low_ch1() && !en) {
		dmm->ui->btn_ch1_dc->setChecked(!en);
	}
	dmm->ui->btn_ch1_ac2->setChecked(en);
}

void DMM_API::set_mode_ac_low_ch1(bool en)
{
	/* Set DC mode as default */
	if (!get_mode_ac_high_ch1() && !en) {
		dmm->ui->btn_ch1_dc->setChecked(!en);
	}
	dmm->ui->btn_ch1_ac->setChecked(en);
}

void DMM_API::set_mode_ac_high_ch2(bool en)
{
	/* Set DC mode as default */
	if (!get_mode_ac_low_ch2() && !en) {
		dmm->ui->btn_ch2_dc->setChecked(!en);
	}
	dmm->ui->btn_ch2_ac2->setChecked(en);
}

void DMM_API::set_mode_ac_low_ch2(bool en)
{
	/* Set DC mode as default */
	if (!get_mode_ac_high_ch2() && !en) {
		dmm->ui->btn_ch2_dc->setChecked(!en);
	}
	dmm->ui->btn_ch2_ac->setChecked(en);
}

bool DMM_API::running() const
{
	return dmm->ui->run_button->isChecked();
}

void DMM_API::run(bool en)
{
	dmm->ui->run_button->setChecked(en);
}

double DMM_API::read_ch1() const
{
	return dmm->ui->lcdCh1->value();
}

double DMM_API::read_ch2() const
{
	return dmm->ui->lcdCh2->value();
}

bool DMM_API::get_histogram_ch1() const
{
	return dmm->ui->histogramCh1->isChecked();
}

void DMM_API::set_histogram_ch1(bool en)
{
	dmm->ui->histogramCh1->setChecked(en);
}

void DMM_API::set_histogram_ch2(bool en)
{
	dmm->ui->histogramCh2->setChecked(en);
}

bool DMM_API::get_histogram_ch2() const
{
	return dmm->ui->histogramCh2->isChecked();
}

int DMM_API::get_history_ch1_size_idx() const
{
	return dmm->ui->historySizeCh1->currentIndex();
}

int DMM_API::get_history_ch2_size_idx() const
{
	return dmm->ui->historySizeCh2->currentIndex();
}

void DMM_API::set_history_ch1_size_idx(int index)
{
	dmm->ui->historySizeCh1->setCurrentIndex(index);
}

void DMM_API::set_history_ch2_size_idx(int index)
{
	dmm->ui->historySizeCh2->setCurrentIndex(index);
}

bool DMM_API::getDataLoggingEn() const
{
	return dmm->ui->btnDataLogging->isChecked();
}

void DMM_API::setDataLoggingEn(bool en)
{
	dmm->ui->btnDataLogging->setChecked(en);
}

double DMM_API::getDataLoggingTimer() const
{
	return dmm->data_logging_timer->value();
}

void DMM_API::setDataLoggingTimer(double val)
{
	dmm->data_logging_timer->setValue(val);
}

bool DMM_API::getPeakHoldEn() const
{
	return dmm->ui->btnDisplayPeakHold->isChecked();
}

void DMM_API::setPeakHoldEn(bool val)
{
	dmm->ui->btnDisplayPeakHold->setChecked(val);
}

bool DMM_API::getDataLoggingAppend() const
{
	return dmm->ui->btn_append->isChecked();
}

void DMM_API::setDataLoggingAppend(bool val)
{
	dmm->ui->btn_append->setChecked(val);
	dmm->ui->btn_overwrite->setChecked(!val);
}
