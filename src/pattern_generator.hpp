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

#ifndef PATTERN_GENERATOR_H
#define PATTERN_GENERATOR_H

#include <QWidget>
#include <QPushButton>
#include <QJsonArray>
#include <QIntValidator>
#include <QtQml/QJSEngine>
#include <QtUiTools/QUiLoader>
#include <vector>
#include <string>

#include "src/pulseview/pv/devices/binarybuffer.hpp"
#include "filter.hpp"

#include "apiObject.hpp"
#include "pg_patterns.hpp"
#include "pg_channel_manager.hpp"
#include "pg_buffer_manager.hpp"

// Generated UI
#include "ui_pattern_generator.h"
#include "ui_pg_settings.h"

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
	struct iio_buffer;
}

namespace pv {
class MainWindow;
class DeviceManager;
namespace toolbars {
class MainBar;
}
}

namespace sigrok {
class Context;
}

namespace Ui {
class PatternGenerator;
class PGSettings;
/*    class BinaryCounterPatternUI;
    class UARTPatternUI;
    class LFSRPatternUI;
    class ClockPatternUI;
    class GenericJSPatternUI;*/
}

class QJSEngine;

namespace adiscope {
class PatternGenerator_API;


class PatternGenerator : public QWidget
{
	friend class PatternGenerator_API;

	Q_OBJECT

public:
	explicit PatternGenerator(struct iio_context *ctx, Filter *filt,
			QPushButton *runButton, QJSEngine *engine,
			QWidget *parent = 0, bool offline_mode = 0);
	~PatternGenerator();


private Q_SLOTS:

	void generatePattern();
//  void onChannelEnabledChanged();
//  void onChannelSelectedChanged();
	void startStop(bool start);
	void singleRun();
	void singleRunStop();
	void toggleRightMenu();
//  void update_ui();
//  void on_sampleRateCombo_activated(const QString &arg1);
//  void on_generatePattern_clicked();
//  void on_clearButton_clicked();
//  void on_generateUI_clicked();

//  void on_save_PB_clicked();
//  void on_load_PB_clicked();
//  void createRightPatternWidget(PatternUI* patternui);

//  void rightMenuFinished(bool opened);
	void on_btnHideInactive_clicked();
	void on_btnGroupWithSelected_clicked();
	void on_btnExtendChannelManager_clicked();

private:

	// UI
	Ui::PatternGenerator *ui;
	Ui::PGSettings *pgSettings;
	QButtonGroup *settings_group;
	QPushButton *menuRunButton;


	typedef enum rightMenuState_t {
		CLOSED,
		OPENED_PG,
		OPENED_CG
	} rightMenuState;



	// QWidget *current;
	PatternUI *currentUI;
	uint16_t channel_group;
	bool offline_mode;

	PatternGenerator_API *pg_api;

	PatternGeneratorChannelGroup *selected_channel_group;
	PatternGeneratorChannelManager chm;
	PatternGeneratorChannelManagerUI *chmui;
	PatternGeneratorBufferManager *bufman;
	PatternGeneratorBufferManagerUi *bufui;

	// Buffer

	short *buffer;
	bool buffer_created;
	uint32_t start_sample;
	uint32_t last_sample;
	uint32_t number_of_samples;
	uint32_t buffersize;

	// Device parameters

	uint16_t channel_enable_mask;
	uint32_t sample_rate;
	int no_channels;

	// IIO

	struct iio_context *ctx;
	struct iio_device *dev;
	struct iio_device *channel_manager_dev;
	struct iio_buffer *txbuf;


	// PV and Sigrok


	/* std::shared_ptr<sigrok::Context> context;
	 std::shared_ptr<pv::devices::BinaryBuffer> pattern_generator_ptr;
	 std::shared_ptr<sigrok::InputFormat> binary_format;
	 std::map<std::string, Glib::VariantBase> options;*/
	pv::MainWindow *main_win;


	bool startPatternGeneration(bool cyclic);
	void stopPatternGeneration();
	void dataChanged();

	void createBinaryBuffer();
	void toggleRightMenu(QPushButton *btn);

	std::vector<PatternUI *> patterns;
	static QStringList digital_trigger_conditions;
	static QStringList possibleSampleRates;
	static const char *channelNames[];
};

class PatternGenerator_API : public ApiObject
{
	Q_OBJECT

public:
	explicit PatternGenerator_API(PatternGenerator *pg) :
		ApiObject(TOOL_PATTERN_GENERATOR), pg(pg) {}
	~PatternGenerator_API() {}

private:
	PatternGenerator *pg;
};

} /* namespace adiscope */


#endif // LOGIC_ANALYZER_H

