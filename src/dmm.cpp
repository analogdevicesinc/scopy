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
#include "dynamicWidget.hpp"
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
	wheelEventGuard(nullptr)
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
}

void DMM::readPreferences()
{
	ui->instrumentNotes->setVisible(prefPanel->getInstrumentNotesActive());
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

	double volts_ch1 = m_m2k_analogin->convertRawToVolts(0, (int)values[0]);
	double volts_ch2 = m_m2k_analogin->convertRawToVolts(1, (int)values[1]);

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

bool DMM::isIioManagerStarted() const
{
	return manager->started() && ui->run_button->isChecked();
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

	bool is_ac_ch1 = ui->btn_ch1_ac->isChecked();
	bool is_ac_ch2 = ui->btn_ch2_ac->isChecked();

	id_ch1 = manager->connect(s2f1, 0, 0, false, sample_rate / 10);
	id_ch2 = manager->connect(s2f2, 1, 0, false, sample_rate / 10);

	auto block1 = configureGraph(s2f1, is_ac_ch1);
	auto block2 = configureGraph(s2f2, is_ac_ch2);

	manager->connect(block1, 0, signal, 0);
	manager->connect(block2, 0, signal, 1);
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
				m_m2k_analogin->setRange(chn, libm2k::analog::PLUS_MINUS_25V);
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
