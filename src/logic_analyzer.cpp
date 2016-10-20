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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <iio.h>

/* GNU Radio includes */
#include <gnuradio/blocks/file_descriptor_sink.h>

/* Qt includes */
#include <QDebug>

/* Local includes */
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "streams_to_short.h"
#include "logic_analyzer.hpp"
#include "spinbox_a.hpp"

/* Sigrok includes */
#include "libsigrokcxx/libsigrokcxx.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

/* Generated UI */
#include "ui_logic_analyzer.h"

/* Boost includes */
#include <boost/thread.hpp>

#if _UNIX
	#include <unistd.h>
#endif
#if _WIN32
	#include <windows.h>
	#include <io.h>
#endif
#define DATA_PIPE "/tmp/myfifo"

using namespace std;
using namespace adiscope;
using namespace pv;
using namespace pv::toolbars;
using namespace pv::widgets;
using sigrok::Context;
using namespace Glibmm;

LogicAnalyzer::LogicAnalyzer(struct iio_context *ctx, Filter *filt,
			QPushButton *runBtn, QWidget *parent) :
	QWidget(parent),
	dev_name(filt->device_name(TOOL_LOGIC_ANALYZER)),
	ctx(ctx),
	itemsize(sizeof(uint16_t)),
	dev(iio_context_find_device(ctx, dev_name.c_str())),
	menuOpened(false),
	fd(-1),
	settings_group(new QButtonGroup(this)),
	menuRunButton(runBtn),
	ui(new Ui::LogicAnalyzer)
{
	ui->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose, true);

	this->settings_group->setExclusive(true);
	this->no_channels = get_no_channels(dev);

	/* sigrok and sigrokdecode initialisation */
	std::shared_ptr<sigrok::Context> context;
	std::string open_file, open_file_format;
	context = sigrok::Context::create();

	/* Initialise libsigrokdecode */
	if (srd_init(nullptr) != SRD_OK) {
		qDebug() << "ERROR: libsigrokdecode init failed.";
	}
	/* Load the protocol decoders */
	srd_decoder_load_all();

	pv::DeviceManager device_manager(context);
	pv::MainWindow* w = new pv::MainWindow(device_manager, open_file,
						open_file_format, parent);

	/* Gnuradio Blocks */
	manager = iio_manager::get_instance(ctx, dev_name);
	sink_streams_to_short = adiscope::streams_to_short::make(itemsize,
								no_channels);
	ids = new iio_manager::port_id[no_channels];

	/* setup view */
	main_win = w;
	ui->verticalLayout_8->removeWidget(ui->centralWidget);
	ui->verticalLayout_8->insertWidget(1, static_cast<QWidget*>(main_win));

	/* setup toolbar */
	/*
	pv::toolbars::MainBar* main_bar = main_win->main_bar_;
	QPushButton *btnDecoder = new QPushButton();
	btnDecoder->setIcon(QIcon::fromTheme("add-decoder", QIcon(":/icons/add-decoder.svg")));
	btnDecoder->setMenu(main_win->menu_decoder_add());
	ui->gridLayout->addWidget(btnDecoder);
	ui->gridLayout->addWidget(static_cast<QWidget *>(main_bar));
	*/

	ui->rightWidget->setMaximumWidth(0);

	/* General settings */
	settings_group->addButton(ui->btnSettings);
	int settings_panel = ui->stackedWidget->indexOf(ui->generalSettings);
	ui->btnSettings->setProperty("id", QVariant(-settings_panel));

	// Controls for scale/division and position
	timeBase = new ScaleSpinButton({
					       {"ns", 1E-9},
					       {"μs", 1E-6},
					       {"ms", 1E-3},
					       {"s", 1E0}
				       }, "Time Base", 100e-9, 100e-6);
	timePosition = new PositionSpinButton({
						      {"ns", 1E-9},
						      {"μs", 1E-6},
						      {"ms", 1E-3},
						      {"s", 1E0}
					      }, "Position",
					      -timeBase->maxValue() * 5,
					      timeBase->maxValue() * 5);
	QVBoxLayout *vLayout = new QVBoxLayout();
	vLayout->insertWidget(1, timeBase, 0, Qt::AlignLeft);
	vLayout->insertWidget(2, timePosition, 0, Qt::AlignLeft);
	vLayout->insertSpacerItem(-1, new QSpacerItem(0, 0,
						QSizePolicy::Minimum,
						QSizePolicy::Expanding));
	ui->generalSettings->setLayout(vLayout);

	int ret = mkfifo(DATA_PIPE, 0666);

	connect(ui->btnRunStop, SIGNAL(toggled(bool)),
			this, SLOT(startStop(bool)));
	connect(runBtn, SIGNAL(toggled(bool)), ui->btnRunStop,
			SLOT(setChecked(bool)));
	connect(ui->btnRunStop, SIGNAL(toggled(bool)), runBtn,
			SLOT(setChecked(bool)));
	connect(ui->btnSettings, SIGNAL(pressed()),
			this, SLOT(toggleRightMenu()));
	connect(ui->rightWidget, SIGNAL(finished(bool)),
			this, SLOT(rightMenuFinished(bool)));
}

LogicAnalyzer::~LogicAnalyzer()
{
	if(manager)
		this->startStop(false);
	delete[] ids;
	#ifdef _WIN32
		system("pause");
	#else
		unlink(DATA_PIPE);
	#endif
	delete ui;
	/* Destroy libsigrokdecode */
	srd_exit();
}

void LogicAnalyzer::startStop(bool start)
{
	if (start)
	{
		std::thread thr1 = std::thread(
			&LogicAnalyzer::create_fifo, this);
		main_win->run_stop();

		if(thr1.joinable()) {
			thr1.join();
		}

		this->sink_fd_block = gr::blocks::file_descriptor_sink::make(itemsize, fd);

		/* connect the manager to the sink block: streams to short */
		for (int i = 0; i < no_channels; i++) {
			ids[i] = manager->connect(sink_streams_to_short, i, i, false);
		}

		/* connect the streams to short block to the file descriptor block */
		manager->connect(sink_streams_to_short, 0, sink_fd_block, 0);

		for ( int i = 0; i < no_channels; i++) {
			manager->start(ids[i]);
		}
		ui->btnRunStop->setText("Stop");
	}
	else
	{
		if (manager->started()){
			main_win->run_stop();
			manager->stop_all();
			manager->wait();
			manager->disconnect_all();
		}
		ui->btnRunStop->setText("Run");
	}
}

unsigned int LogicAnalyzer::get_no_channels(struct iio_device *dev)
{
	unsigned int nb = 0;

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_output(chn) &&
		iio_channel_is_scan_element(chn))
		nb++;
	}
	return nb;
}

void LogicAnalyzer::create_fifo()
{
	#ifdef _WIN32
		LPTSTR name = TEXT( "\\\\.\\pipe\\myfifo");
		HANDLE pipe = CreateNamedPipe(
			name, // name of the pipe
			PIPE_ACCESS_OUTBOUND, // 1-way pipe -- send only
			PIPE_TYPE_BYTE, // send data as a byte stream
			1, // only allow 1 instance of this pipe
			0, // no outbound buffer
			0, // no inbound buffer
			0, // use default wait time
			NULL // use default security attributes
		);

		fd = _open_osfhandle((long)pipe, O_WRONLY);

		if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) {
			printf("Failed to create outbound pipe instance.");
			system("pause");
		}

		BOOL result = ConnectNamedPipe(pipe, NULL);
		if (!result) {
			printf("Failed to make connection on named pipe.");
			CloseHandle(pipe); // close the pipe
			system("pause");
		}
	#else
		fd = open(DATA_PIPE, O_WRONLY);
	#endif
}

void LogicAnalyzer::toggleRightMenu(QPushButton *btn)
{
	int id = btn->property("id").toInt();
	bool btn_old_state = btn->isChecked();
	bool open = !menuOpened;

	active_settings_btn = btn;
	settings_group->setExclusive(!btn_old_state);

	if (open)
		settings_panel_update(id);

	ui->rightWidget->toggleMenu(open);
}

void LogicAnalyzer::settings_panel_update(int id)
{
	if (id >= 0)
		ui->stackedWidget->setCurrentIndex(0);
	else
		ui->stackedWidget->setCurrentIndex(-id);
}

void LogicAnalyzer::toggleRightMenu()
{
	toggleRightMenu(static_cast<QPushButton *>(QObject::sender()));
}

void LogicAnalyzer::rightMenuFinished(bool opened)
{
	menuOpened = opened;

	if (!opened && active_settings_btn && active_settings_btn->isChecked()) {
		int id = active_settings_btn->property("id").toInt();
		settings_panel_update(id);
		ui->rightWidget->toggleMenu(true);
	}
}
