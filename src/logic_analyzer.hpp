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

#ifndef LOGIC_ANALYZER_H
#define LOGIC_ANALYZER_H

#include <vector>

/* Qt includes */
#include <QWidget>
#include <QPushButton>

/* GNU Radio includes */
#include <gnuradio/blocks/file_descriptor_sink.h>

/* Local includes */
#include "iio_manager.hpp"
#include "filter.hpp"
#include "streams_to_short.h"
#include "pulseview/pv/widgets/sweeptimingwidget.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/mainwindow.hpp"

using namespace pv;
using namespace pv::toolbars;
using namespace pv::widgets;
using sigrok::Context;

namespace Glibmm {
}

namespace Ui {
	class LogicAnalyzer;
}

namespace adiscope {
	class LogicAnalyzer : public QWidget
	{
		Q_OBJECT

	public:
		explicit LogicAnalyzer(struct iio_context *ctx, Filter* filt,
				QPushButton *runButton, QWidget *parent = 0);
		~LogicAnalyzer();

	private Q_SLOTS:
		void startStop(bool start);
		void toggleRightMenu();

	private:
		Ui::LogicAnalyzer *ui;
		QButtonGroup *settings_group;
		QPushButton *menuRunButton;

		boost::shared_ptr<iio_manager> manager;
		iio_manager::port_id* ids;
		struct iio_context *ctx;
		struct iio_device *dev;
		unsigned int no_channels;
		unsigned int itemsize;
		int fd;
		adiscope::streams_to_short::sptr sink_streams_to_short;
		gr::blocks::file_descriptor_sink::sptr sink_fd_block;
		pv::MainWindow* main_win;

		void disconnectAll();
		static unsigned int get_no_channels(struct iio_device *dev);
		void create_fifo();

		void toggleRightMenu(QPushButton *btn);
		bool menuOpened;
	};
}

#endif // LOGIC_ANALYZER_H
