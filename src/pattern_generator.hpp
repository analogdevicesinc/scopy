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

#include "apiObject.hpp"
#include "pg_buffer_manager.hpp"
#include "pg_channel_manager.hpp"
#include "pg_patterns.hpp"
#include "scroll_filter.hpp"
#include "tool.hpp"

#include <QTimer>
#include <QVector>
#include <QWidget>

#include <string>
#include <vector>

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
} // namespace pv

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
} // namespace Ui

class QJSEngine;
class QPushButton;

namespace adiscope {
class PatternGenerator_API;
class PatternGeneratorChannelGroup_API;
class PatternGeneratorChannel_API;
class Filter;

const uint32_t PGMaxSampleRate = 100000000;

class PatternGenerator : public Tool {
	friend class PatternGenerator_API;
	friend class PatternGeneratorChannelGroup_API;
	friend class PatternGeneratorChannel_API;
	friend class ToolLauncher_API;

	Q_OBJECT

public:
	static const int channelGroupLabelMaxLength = 13;
	explicit PatternGenerator(struct iio_context *ctx, Filter *filt,
				  ToolMenuItem *toolMenuItem, QJSEngine *engine,
				  DIOManager *diom, ToolLauncher *parent,
				  bool offline_mode_ = 0);
	~PatternGenerator();
	void updateCGSettings();
	PatternUI *getCurrentPatternUI();
	void enableBufferUpdates(bool enabled);
	void settingsLoaded();
	bool suppressCGSettingsUpdate;

	bool getUseDecoders() const;
	void setUseDecoders(bool use_decoders);

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
	void checkEnabledChannels();

private:
	// UI
	static const char *channelNames[];
	Ui::PatternGenerator *ui;
	Ui::PGSettings *pgSettings;
	Ui::PGCGSettings *cgSettings;
	MouseWheelWidgetGuard *wheelEventGuard;

	QButtonGroup *settings_group;

	pv::widgets::ColourButton *colour_button_edge, *colour_button_high,
		*colour_button_BG, *colour_button_low;

	typedef enum rightMenuState_t {
		CLOSED,
		OPENED_PG,
		OPENED_CG
	} rightMenuState;

	typedef enum runState_t {
		STOPPED,
		CONFIG,
		RUNNING,
		WAITING,
	} runState;

	runState _pgStatus;
	static QStringList strStatus;
	QTimer *singleRunTimer;
	// Q_PROPERTY(runState pgStatus READ pgStatus WRITE setPGStatus)

	void setPGStatus(runState _val);
	runState pgStatus();

	PatternUI *currentUI;
	bool offline_mode;

	PatternGeneratorChannelGroup *selected_channel_group;
	PatternGeneratorChannelManager chm;
	PatternGeneratorChannelManagerUI *chmui;
	PatternGeneratorBufferManager *bufman;
	PatternGeneratorBufferManagerUi *bufui;

	bool buffer_created;
	int no_channels;
	pv::MainWindow *main_win;

	// IIO

	struct iio_device *dev;
	struct iio_buffer *txbuf;
	DIOManager *diom;

	bool startPatternGeneration(bool cyclic);
	void stopPatternGeneration();
	void toggleRightMenu(QPushButton *btn);

	std::vector<PatternUI *> patterns;
	static QStringList digital_trigger_conditions;
	static QStringList possibleSampleRates;

	// QList<PatternGeneratorChannelGroup_API*> getChgApi();
	// void setChgApi();

	QString toString();
	QJsonValue chmToJson();

	void fromString(QString);
	void jsonToChm(QJsonObject obj);
	void deleteSettingsWidget();
	void createSettingsWidget();
	void showColorSettings(bool check);
	void enableCgSettings(bool en);

	bool m_use_decoders;

private Q_SLOTS:

	void reloadBufferInDevice();
	void outputModeChanged(int index);
	void patternChanged(int index);
	void configureAutoSet();
	void changeName(QString name);
	void changeChannelThickness(QString);
	void pushButtonLeft();
	void pushButtonRight();
	void updateSampleRate();
	void updateBufferSize();
	void resetPGToDefault();

public Q_SLOTS:
	void run() override;
	void stop() override;

Q_SIGNALS:
	void showTool();
};
} /* namespace adiscope */

#endif // LOGIC_ANALYZER_H
