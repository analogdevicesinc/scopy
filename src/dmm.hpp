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

#include <QPushButton>
#include <QWidget>
#include <atomic>

#include "apiObject.hpp"
#include "filter.hpp"
#include "iio_manager.hpp"
#include "signal_sample.hpp"
#include "tool.hpp"
#include "scroll_filter.hpp"
#include <thread>
#include "spinbox_a.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

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
		explicit DMM(struct iio_context *ctx, Filter *filt,
				std::shared_ptr<GenericAdc> adc,
				QPushButton *runButton, QJSEngine *engine,
				ToolLauncher *parent);
		~DMM();

	private:
		Ui::DMM *ui;
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
		PositionSpinButton *data_logging_timer;

		boost::mutex data_mutex;
		boost::condition_variable data_cond;
		MouseWheelWidgetGuard *wheelEventGuard;

		std::vector<double> m_min, m_max;

		void disconnectAll();
		gr::basic_block_sptr configureGraph(gr::basic_block_sptr s2f,
				bool is_low_ac, bool is_high_ac);
		void configureModes();
		int numSamplesFromIdx(int idx);
		void writeAllSettingsToHardware();
		void checkPeakValues(int, double);

	public Q_SLOTS:
		void toggleTimer(bool start);

	private Q_SLOTS:
		void setHistorySizeCh1(int idx);
		void setHistorySizeCh2(int idx);

		void updateValuesList(std::vector<float> values);

		void toggleAC();

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

	class DMM_API : public ApiObject
	{
		friend class ToolLauncher_API;

		Q_OBJECT

		Q_PROPERTY(bool mode_ac_high_ch1
				READ get_mode_ac_high_ch1 WRITE set_mode_ac_high_ch1);
		Q_PROPERTY(bool mode_ac_low_ch1
				READ get_mode_ac_low_ch1 WRITE set_mode_ac_low_ch1);
		Q_PROPERTY(bool mode_ac_high_ch2
				READ get_mode_ac_high_ch2 WRITE set_mode_ac_high_ch2);
		Q_PROPERTY(bool mode_ac_low_ch2
				READ get_mode_ac_low_ch2 WRITE set_mode_ac_low_ch2);
		Q_PROPERTY(bool running READ running WRITE run STORED false);

		Q_PROPERTY(bool histogram_ch1
				READ get_histogram_ch1
				WRITE set_histogram_ch1);
		Q_PROPERTY(bool histogram_ch2
				READ get_histogram_ch2
				WRITE set_histogram_ch2);

		Q_PROPERTY(int history_ch1_size_idx
				READ get_history_ch1_size_idx
				WRITE set_history_ch1_size_idx);
		Q_PROPERTY(int history_ch2_size_idx
				READ get_history_ch2_size_idx
				WRITE set_history_ch2_size_idx);

		Q_PROPERTY(double value_ch1 READ read_ch1);
		Q_PROPERTY(double value_ch2 READ read_ch2);
		Q_PROPERTY(bool data_logging_en READ getDataLoggingEn
			   WRITE setDataLoggingEn)
		Q_PROPERTY(double data_logging_timer READ getDataLoggingTimer
			   WRITE setDataLoggingTimer)
		Q_PROPERTY(bool data_logging_append READ getDataLoggingAppend
			   WRITE setDataLoggingAppend)
		Q_PROPERTY(bool peak_hold_en READ getPeakHoldEn
			  WRITE setPeakHoldEn)

	public:
		bool get_mode_ac_high_ch1() const;
		bool get_mode_ac_low_ch1() const;
		bool get_mode_ac_high_ch2() const;
		bool get_mode_ac_low_ch2() const;

		void set_mode_ac_high_ch1(bool en);
		void set_mode_ac_low_ch1(bool en);
		void set_mode_ac_high_ch2(bool en);
		void set_mode_ac_low_ch2(bool en);


		bool get_histogram_ch1() const;
		bool get_histogram_ch2() const;
		void set_histogram_ch1(bool en);
		void set_histogram_ch2(bool en);

		int get_history_ch1_size_idx() const;
		int get_history_ch2_size_idx() const;
		void set_history_ch1_size_idx(int idx);
		void set_history_ch2_size_idx(int idx);

		double read_ch1() const;
		double read_ch2() const;

		bool running() const;
		void run(bool en);

		bool getDataLoggingEn() const;
		void setDataLoggingEn(bool);

		double getDataLoggingTimer() const;
		void setDataLoggingTimer(double);

		bool getDataLoggingAppend() const;
		void setDataLoggingAppend(bool);

		bool getPeakHoldEn() const;
		void setPeakHoldEn(bool);

		Q_INVOKABLE void show();

		explicit DMM_API(DMM *dmm) : ApiObject(), dmm(dmm) {}
		~DMM_API() {}

	private:
		DMM *dmm;
	};
}

#endif /* DMM_HPP */
