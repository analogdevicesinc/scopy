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

#ifndef DMM_HPP
#define DMM_HPP

#include "apiObject.hpp"
#include "filter.hpp"
#include "iio_manager.hpp"
#include "scroll_filter.hpp"
#include "signal_sample.hpp"
#include "spinbox_a.hpp"
#include "tool.hpp"

#include <QPushButton>
#include <QWidget>

#include <atomic>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <thread>

namespace Ui {
class DMM;
}

class QJSEngine;

namespace adiscope {
class DMM_API;
class GenericAdc;

class DMM : public Tool
{
	friend class DMM_API;
	friend class ToolLauncher_API;

	Q_OBJECT

public:
	explicit DMM(struct iio_context* ctx, Filter* filt, std::shared_ptr<GenericAdc> adc, ToolMenuItem* toolMenuItem,
		     QJSEngine* engine, ToolLauncher* parent);
	~DMM();

private:
	Ui::DMM* ui;
	boost::shared_ptr<iio_manager> manager;
	iio_manager::port_id id_ch1, id_ch2;
	std::shared_ptr<GenericAdc> adc;
	boost::shared_ptr<signal_sample> signal;
	unsigned long sample_rate;

	std::atomic<bool> interrupt_data_logging;
	std::atomic<bool> data_logging;
	QString filename;
	std::thread data_logging_thread;
	bool use_timer;
	unsigned long logging_refresh_rate;
	PositionSpinButton* data_logging_timer;

	boost::mutex data_mutex;
	boost::condition_variable data_cond;
	MouseWheelWidgetGuard* wheelEventGuard;

	std::vector<double> m_min, m_max;

	void disconnectAll();
	gr::basic_block_sptr configureGraph(gr::basic_block_sptr s2f, bool is_low_ac, bool is_high_ac);
	void configureModes();
	int numSamplesFromIdx(int idx);
	void writeAllSettingsToHardware();
	void checkPeakValues(int, double);
	bool isIioManagerStarted() const;

public Q_SLOTS:
	void toggleTimer(bool start);
	void run() override;
	void stop() override;

private Q_SLOTS:
	void setHistorySizeCh1(int idx);
	void setHistorySizeCh2(int idx);

	void updateValuesList(std::vector<float> values);

	void toggleAC();

	void enableDataLogging(bool);

	void toggleDataLogging(bool);

	void startDataLogging(bool);

	void dataLoggingThread();

	void chooseFile();

	void resetPeakHold(bool);

	void displayPeakHold(bool);

	void collapsePeakHold(bool);
	void collapseDataLog(bool);

Q_SIGNALS:
	void showTool();
};
} // namespace adiscope

#endif /* DMM_HPP */
