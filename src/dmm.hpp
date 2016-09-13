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

#include "filter.hpp"
#include "iio_manager.hpp"
#include "peek_sample.hpp"

namespace Ui {
	class DMM;
}

namespace adiscope {
	class DMM : public QWidget
	{
		Q_OBJECT

	public:
		explicit DMM(struct iio_context *ctx, Filter *filt,
				QPushButton *runButton, float gain_ch1,
				float gain_ch2, QWidget *parent = 0);
		~DMM();

	private:
		Ui::DMM *ui;
		QTimer timer;
		boost::shared_ptr<iio_manager> manager;
		boost::shared_ptr<peek_sample<float>>
			peek_block_ch1, peek_block_ch2;
		iio_manager::port_id id_ch1, id_ch2;
		bool mode_ac_ch1, mode_ac_ch2;
		float gain_ch1, gain_ch2;

		void disconnectAll();
		void configureMode(bool is_ac);
		iio_manager::port_id configureMode(bool is_ac, unsigned int ch);
		int numSamplesFromIdx(int idx);

	public slots:
		void updateValuesList();

		void toggleTimer(bool start);
		void toggleAC1(bool enable);
		void toggleAC2(bool enable);

	private slots:
		void setHistorySizeCh1(int idx);
		void setHistorySizeCh2(int idx);
	};
}

#endif /* DMM_HPP */
