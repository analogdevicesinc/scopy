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
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "streams_to_short.h"
#include "logic_analyzer.hpp"
#include "spinbox_a.hpp"

/* Sigrok includes */
#include "libsigrokcxx/libsigrokcxx.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

/* Generated UI */
#include "ui_logic_analyzer.h"
#include "ui_logic_channel_settings.h"

/* Boost includes */
#include <boost/thread.hpp>

using namespace std;
using namespace adiscope;
using namespace pv;
using namespace pv::toolbars;
using namespace pv::widgets;
using sigrok::Context;
using sigrok::ConfigKey;
using namespace Glibmm;

LogicAnalyzer::LogicAnalyzer(struct iio_context *ctx,
                             Filter *filt,
                             QPushButton *runBtn,
                             QWidget *parent,
                             unsigned int sample_rate) :
	QWidget(parent),
	dev_name(filt->device_name(TOOL_LOGIC_ANALYZER)),
	ctx(ctx),
	itemsize(sizeof(uint16_t)),
	dev(iio_context_find_device(ctx, dev_name.c_str())),
	menuOpened(false),
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
	QString path = QCoreApplication::applicationDirPath() + "/decoders";
	if (srd_init(path.toStdString().c_str()) != SRD_OK) {
		qDebug() << "ERROR: libsigrokdecode init failed.";
	}

	/* Load the protocol decoders */
	srd_decoder_load_all();

	pv::DeviceManager device_manager(context);
	pv::MainWindow *w = new pv::MainWindow(device_manager, filt, open_file,
	                                       open_file_format, parent);

	options["numchannels"] = Glib::Variant<gint32>(
	                                 g_variant_new_int32(no_channels),true);
	options["samplerate"] = Glib::Variant<guint64>(
	                                g_variant_new_uint64(sample_rate),true);


	for (unsigned int j = 0; j < iio_device_get_channels_count(dev); j++) {
		struct iio_channel *chn = iio_device_get_channel(dev, j);

		if (!iio_channel_is_output(chn) &&
		    iio_channel_is_scan_element(chn)) {
			iio_channel_enable(chn);
		}
	}

	iio_device_attr_write_longlong(dev, "sampling_frequency", sample_rate);

	logic_analyzer_ptr = std::make_shared<pv::devices::BinaryStream>(
	                             device_manager.context(), dev, sample_rate / 100,
	                             w->get_format_from_string("binary"),
	                             options);

	/* setup view */
	main_win = w;
	main_win->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->centralWidgetLayout->addWidget(static_cast<QWidget * >(main_win));
	main_win->select_device(logic_analyzer_ptr);

	ui->rightWidget->setMaximumWidth(0);
	static_cast<QWidget *>(main_win->main_bar_)->setVisible(false);

	/* General settings */
	settings_group->addButton(ui->btnSettings);
	int settings_panel = ui->stackedWidget->indexOf(ui->generalSettings);
	ui->btnSettings->setProperty("id", QVariant(-settings_panel));

	/* Channel settings */
	settings_group->addButton(ui->btnChSettings);
	int ch_settings_panel = ui->stackedWidget->indexOf(ui->colorSettings);
	ui->btnChSettings->setProperty("id", QVariant(-ch_settings_panel));

	// Controls for scale/division and position
	timeBase = new ScaleSpinButton({
		{"ps", 1E-12},
		{"ns", 1E-9},
		{"μs", 1E-6},
		{"ms", 1E-3},
		{"s", 1E0}
	}, "Time Base", 100e-12, 1e0);
	timePosition = new PositionSpinButton({
		{"ns", 1E-9},
		{"μs", 1E-6},
		{"ms", 1E-3},
		{"s", 1E0}
	}, "Position",
	-timeBase->maxValue() * 5,
	timeBase->maxValue() * 5);
	QVBoxLayout *vLayout = new QVBoxLayout(ui->generalSettings);
	vLayout->insertWidget(1, timeBase, 0, Qt::AlignLeft);
	vLayout->insertWidget(2, timePosition, 0, Qt::AlignLeft);
	vLayout->insertSpacerItem(-1, new QSpacerItem(0, 0,
	                          QSizePolicy::Minimum,
	                          QSizePolicy::Expanding));
	ui->generalSettings->setLayout(vLayout);

	connect(ui->btnRunStop, SIGNAL(toggled(bool)),
	        this, SLOT(startStop(bool)));
	connect(runBtn, SIGNAL(toggled(bool)), ui->btnRunStop,
	        SLOT(setChecked(bool)));
	connect(ui->btnRunStop, SIGNAL(toggled(bool)), runBtn,
	        SLOT(setChecked(bool)));
	connect(ui->btnSettings, SIGNAL(pressed()),
	        this, SLOT(toggleRightMenu()));
	connect(ui->btnChSettings, SIGNAL(pressed()),
	        this, SLOT(toggleRightMenu()));
	connect(ui->rightWidget, SIGNAL(finished(bool)),
	        this, SLOT(rightMenuFinished(bool)));
	connect(ui->btnShowHideMenu, SIGNAL(clicked(bool)),
	        this, SLOT(toggleLeftMenu(bool)));
	connect(timeBase, SIGNAL(valueChanged(double)),
	        main_win->view_, SLOT(set_timebase(double)));

	timeBase->setValue(1e-3);
	timeBase->valueChanged(timeBase->value());

	chm_ui = new LogicAnalyzerChannelManagerUI(0, main_win, &chm, ui->colorSettings,
	                this);
	ui->leftLayout->addWidget(chm_ui);
	chm_ui->update_ui();
	chm_ui->setVisible(true);
	lachannelsettings = new Ui::LChannelSettings;

	connect(ui->btnGroupChannels, SIGNAL(pressed()),
	        chm_ui, SLOT(on_groupSplit_clicked()));
	connect(ui->btnShowChannels, SIGNAL(clicked(bool)),
	        chm_ui, SLOT(on_hideInactive_clicked(bool)));
	connect(ui->btnShowChannels, SIGNAL(clicked(bool)),
	        this, SLOT(on_btnShowChannelsClicked(bool)));
}


LogicAnalyzer::~LogicAnalyzer()
{
	delete ui;
	/* Destroy libsigrokdecode */
	srd_exit();
}

void LogicAnalyzer::startStop(bool start)
{
	main_win->run_stop();

	if (start) {
		ui->btnRunStop->setText("Stop");
	} else {
		ui->btnRunStop->setText("Run");
	}
}

unsigned int LogicAnalyzer::get_no_channels(struct iio_device *dev)
{
	unsigned int nb = 0;

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_output(chn) &&
		    iio_channel_is_scan_element(chn)) {
			nb++;
		}
	}

	return nb;
}

void LogicAnalyzer::clearLayout(QLayout *layout)
{
	for (int i = 0 ; i < layout->children().size();) {
		delete layout->takeAt(i);
	}

	delete layout;
}

void LogicAnalyzer::toggleRightMenu(QPushButton *btn)
{
	int id = btn->property("id").toInt();
	bool btn_old_state = btn->isChecked();
	bool open; //= !menuOpened;

	if (active_settings_btn != btn) {
		open = !menuOpened;
	} else {
		open = true;
	}

	if (!open) {
		settings_group->setExclusive(false);
		ui->btnChSettings->setChecked(false);
		ui->btnSettings->setChecked(false);
		settings_group->setExclusive(true);
	}

	if (menuOpened != open) {
		ui->rightWidget->toggleMenu(open);
	}

	menuOpened = open;
	active_settings_btn = btn;

	if (open) {
		settings_panel_update(id);
		chm_ui->showHighlight(true);
	} else {
		chm_ui->showHighlight(false);
	}
}

void LogicAnalyzer::settings_panel_update(int id)
{
	if (id < 0) {
		ui->stackedWidget->setCurrentIndex(-id);
	} else {
		ui->stackedWidget->setCurrentIndex(id);
//		clearLayout(ui->colorSettings->layout());
//		lachannelsettings->setupUi(ui->colorSettings);
//		LogicAnalyzerChannelGroup* chg_ui = chm_ui->chm->getHighlightedChannelGroup();
//		lachannelsettings->channelName->setText(QString::fromStdString(chg_ui->get_label()));
//		connect(lachannelsettings->channelName, SIGNAL(textChanged(const QString&)),
//			chm_ui, SLOT(changeChannelName(const QString&)));
//		ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->colorSettings));
	}
}

void LogicAnalyzer::toggleRightMenu()
{
	toggleRightMenu(static_cast<QPushButton *>(QObject::sender()));
}

void LogicAnalyzer::toggleLeftMenu(bool val)
{
	if (val) {
		ui->btnGroupChannels->hide();
		ui->btnShowChannels->hide();
		ui->btnShowHideMenu->setText(">");
		chm_ui->collapse(true);
	} else {
		ui->btnGroupChannels->show();
		ui->btnShowChannels->show();
		ui->btnShowHideMenu->setText("<");
		chm_ui->collapse(false);
	}
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

void LogicAnalyzer::leftMenuFinished(bool closed)
{
	if (ui->btnShowHideMenu->isChecked() && !closed) {
		ui->btnGroupChannels->hide();
		ui->btnShowChannels->hide();
		ui->btnShowHideMenu->setText(">");
		chm_ui->collapse(true);
	} else {
		ui->btnGroupChannels->show();
		ui->btnShowChannels->show();
		ui->btnShowHideMenu->setText("<");
		chm_ui->collapse(false);
	}
}

void LogicAnalyzer::on_btnShowChannelsClicked(bool check)
{
	if (check) {
		ui->btnShowChannels->setText("Show all");
	} else {
		ui->btnShowChannels->setText("Hide inactive");
	}
}
