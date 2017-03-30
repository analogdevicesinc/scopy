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
#include <QVector>
#include <vector>
#include <string>

#include "src/pulseview/pv/devices/binarybuffer.hpp"
#include "pulseview/pv/widgets/colourbutton.hpp"
#include "filter.hpp"

#include "apiObject.hpp"
#include "pg_patterns.hpp"
#include "pg_channel_manager.hpp"
#include "pg_buffer_manager.hpp"

// Generated UI
#include "ui_pattern_generator.h"
#include "ui_pg_settings.h"
#include "ui_pg_cg_settings.h"


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
namespace view {
class TracePalette;
}
namespace widgets {
class ColourButton;
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
	static const int channelGroupLabelMaxLength = 13;
	explicit PatternGenerator(struct iio_context *ctx, Filter *filt,
				  QPushButton *runButton, QJSEngine *engine, DIOManager* diom,
				  QWidget *parent = 0, bool offline_mode_ = 0);
	~PatternGenerator();
	void updateCGSettings();



private Q_SLOTS:

	void generatePattern();
	void startStop(bool start);
	void singleRun();
	void singleRunStop();
	void toggleRightMenu();
	void updatePGettings();
	void on_btnHideInactive_clicked();
	void on_btnGroupWithSelected_clicked();
	void colorChanged(QColor);

private:

	// UI
	static const char *channelNames[];
	Ui::PatternGenerator *ui;
	Ui::PGSettings *pgSettings;
	Ui::PGCGSettings *cgSettings;

	QButtonGroup *settings_group;
	QPushButton *menuRunButton;

	pv::widgets::ColourButton *colour_button_edge,
		*colour_button_high, *colour_button_BG,
		*colour_button_low;

	typedef enum rightMenuState_t {
		CLOSED,
		OPENED_PG,
		OPENED_CG
	} rightMenuState;

	PatternUI *currentUI;
	bool offline_mode;

	PatternGenerator_API *pg_api;

	PatternGeneratorChannelGroup *selected_channel_group;
	PatternGeneratorChannelManager chm;
	PatternGeneratorChannelManagerUI *chmui;
	PatternGeneratorBufferManager *bufman;
	PatternGeneratorBufferManagerUi *bufui;

	bool buffer_created;
	int no_channels;
	pv::MainWindow *main_win;

	// IIO

	struct iio_context *ctx;
	struct iio_device *dev;
	struct iio_device *channel_manager_dev;
	struct iio_buffer *txbuf;
	DIOManager* diom;

	bool startPatternGeneration(bool cyclic);
	void stopPatternGeneration();
	void toggleRightMenu(QPushButton *btn);

	std::vector<PatternUI *> patterns;
	static QStringList digital_trigger_conditions;
	static QStringList possibleSampleRates;

	QString toString();
	QJsonValue chmToJson();

	void fromString(QString);
	void jsonToChm(QJsonObject obj);
	void deleteSettingsWidget();
	void createSettingsWidget();
	void showColorSettings(bool check);

private Q_SLOTS:
	void patternChanged(int index);
	void configureAutoSet();
	void changeName(QString name);
	void changeChannelThickness(QString);
	void pushButtonLeft();
	void pushButtonRight();
	void updateSampleRate();
	void updateBufferSize();
	void resetPGToDefault();
};

class PatternGenerator_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QString chm READ chm WRITE setChm SCRIPTABLE false);

public:
	explicit PatternGenerator_API(PatternGenerator *pg) :
		ApiObject(TOOL_PATTERN_GENERATOR), pg(pg) {}
	~PatternGenerator_API() {}

	QString chm() const;
	void setChm(QString);

private:
	PatternGenerator *pg;
};

} /* namespace adiscope */

//Q_DECLARE_METATYPE(adiscope::PGChannelGroupStructure)

#endif // LOGIC_ANALYZER_H

