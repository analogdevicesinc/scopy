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
#include <QTimer>
#include <QWidget>

#include "apiObject.hpp"
#include "filter.hpp"
#include "iio_manager.hpp"
#include "peek_sample.hpp"
#include "tool.hpp"

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
		QTimer timer;
		boost::shared_ptr<iio_manager> manager;
		boost::shared_ptr<peek_sample<float>>
			peek_block_ch1, peek_block_ch2;
		iio_manager::port_id id_ch1, id_ch2;
		std::shared_ptr<GenericAdc> adc;
		bool mode_ac_ch1, mode_ac_ch2;

		void disconnectAll();
		void configureMode(bool is_ac);
		iio_manager::port_id configureMode(bool is_ac, unsigned int ch);
		int numSamplesFromIdx(int idx);
		void writeAllSettingsToHardware();

	public Q_SLOTS:
		void updateValuesList();

		void toggleTimer(bool start);
		void toggleAC1(bool enable);
		void toggleAC2(bool enable);

	private Q_SLOTS:
		void setHistorySizeCh1(int idx);
		void setHistorySizeCh2(int idx);
	};

	class DMM_API : public ApiObject
	{
		friend class ToolLauncher_API;

		Q_OBJECT

		Q_PROPERTY(bool mode_ac_ch1
				READ get_mode_ac_ch1 WRITE set_mode_ac_ch1);
		Q_PROPERTY(bool mode_ac_ch2
				READ get_mode_ac_ch2 WRITE set_mode_ac_ch2);
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

	public:
		bool get_mode_ac_ch1() const { return dmm->mode_ac_ch1; }
		bool get_mode_ac_ch2() const { return dmm->mode_ac_ch2; }
		void set_mode_ac_ch1(bool en) { dmm->toggleAC1(en); }
		void set_mode_ac_ch2(bool en) { dmm->toggleAC2(en); }

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

		explicit DMM_API(DMM *dmm) : ApiObject(), dmm(dmm) {}
		~DMM_API() {}

	private:
		DMM *dmm;
	};
}

#endif /* DMM_HPP */
