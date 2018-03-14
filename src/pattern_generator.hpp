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
#include <QVector>
#include <vector>
#include <string>
#include <QTimer>

#include "apiObject.hpp"
#include "pg_patterns.hpp"
#include "pg_channel_manager.hpp"
#include "pg_buffer_manager.hpp"
#include "tool.hpp"


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
class QPushButton;

namespace adiscope {
class PatternGenerator_API;
class PatternGeneratorChannelGroup_API;
class PatternGeneratorChannel_API;
class Filter;

const uint32_t PGMaxSampleRate = 100000000;

class PatternGenerator : public Tool
{
	friend class PatternGenerator_API;
	friend class PatternGeneratorChannelGroup_API;
	friend class PatternGeneratorChannel_API;
	friend class ToolLauncher_API;

	Q_OBJECT

public:
	static const int channelGroupLabelMaxLength = 13;
	explicit PatternGenerator(struct iio_context *ctx, Filter *filt,
	                          QPushButton *runButton, QJSEngine *engine, DIOManager *diom,
	                          ToolLauncher *parent, bool offline_mode_ = 0);
	~PatternGenerator();
	void updateCGSettings();
	PatternUI *getCurrentPatternUI();
	void enableBufferUpdates(bool enabled);
	void settingsLoaded();
	bool suppressCGSettingsUpdate;

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

	pv::widgets::ColourButton *colour_button_edge,
	*colour_button_high, *colour_button_BG,
	*colour_button_low;

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
	//Q_PROPERTY(runState pgStatus READ pgStatus WRITE setPGStatus)

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
	struct iio_device *channel_manager_dev;
	struct iio_buffer *txbuf;
	DIOManager *diom;

	bool startPatternGeneration(bool cyclic);
	void stopPatternGeneration();
	void toggleRightMenu(QPushButton *btn);

	std::vector<PatternUI *> patterns;
	static QStringList digital_trigger_conditions;
	static QStringList possibleSampleRates;

	//QList<PatternGeneratorChannelGroup_API*> getChgApi();
	//void setChgApi();

	QString toString();
	QJsonValue chmToJson();

	void fromString(QString);
	void jsonToChm(QJsonObject obj);
	void deleteSettingsWidget();
	void createSettingsWidget();
	void showColorSettings(bool check);
	void enableCgSettings(bool en);

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
};

class PatternGeneratorChannel_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QString label READ label WRITE setLabel);
	Q_PROPERTY(int id READ id WRITE setId);
	Q_PROPERTY(int mask READ mask);
	Q_PROPERTY(bool outputMode READ outputMode WRITE setOutputMode)
	Q_PROPERTY(QList<int> color READ color WRITE setColor);
	Q_PROPERTY(double thickness READ thickness WRITE setThickness);
	PatternGeneratorChannel *ch;
	PatternGenerator *pg;
	PatternGeneratorChannelManagerUI *chmui;
	PatternGeneratorChannelManager *chm;


public:
	PatternGeneratorChannel_API(PatternGeneratorChannel *_ch,
				    PatternGeneratorChannelManagerUI *_chmui)
		: ch(_ch),chmui(_chmui),chm(_chmui->chm),pg(_chmui->pg) {}

	QList<int> color();
	void setColor(QList<int>);

	QString label()
	{
		return QString::fromStdString(ch->get_label());
	}
	void setLabel(QString _str)
	{
		ch->set_label(_str.toStdString());
	}
	int id()
	{
		return ch->get_id();
	}
	void setId(int val)
	{
		//	ch->set_id(val);
	}
	int mask()
	{
		return ch->get_mask();
	}
	bool outputMode()
	{
		return ch->getOutputMode();
	}
	void setOutputMode(bool val)
	{
		ch->setOutputMode(val);
		pg->diom->setOutputMode(id(), val);
	}

	void setThickness(double val)
	{
		ch->setCh_thickness(val);
	}
	double thickness()
	{
		return ch->getCh_thickness();
	}
};

class PatternGeneratorChannelGroup_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QString label READ label WRITE setLabel);
	Q_PROPERTY(bool grouped READ grouped WRITE setGrouped);
	Q_PROPERTY(bool enabled READ enabled WRITE setEnabled);
	Q_PROPERTY(bool collapsed READ collapsed WRITE setCollapsed);
	Q_PROPERTY(QString pattern READ pattern WRITE setPattern);

	Q_PROPERTY(QList<int> channel_list READ channels WRITE setChannels);
	Q_PROPERTY(double thickness READ thickness WRITE setThickness);
	Q_PROPERTY(QList<int> color READ color WRITE setColor);

	PatternGeneratorChannelGroup *chg;
	QList<int> channel_list_;

	PatternGeneratorChannelManagerUI *chmui;
	PatternGeneratorChannelManager *chm;

public:
	PatternGeneratorChannelGroup_API(PatternGeneratorChannelGroup *_chg,
	                                 PatternGeneratorChannelManagerUI *chmui) : chg(_chg),chmui(chmui),
		chm(chmui->chm) {}

	QString label()
	{
		return QString::fromStdString(chg->get_label());
	}
	void setLabel(QString _str)
	{
		chg->set_label(_str.toStdString());
	}

	QList<int> channels();
	void setChannels(QList<int>);

	QList<int> color();
	void setColor(QList<int>);


	bool grouped()
	{
		return chg->is_grouped();
	}
	void setGrouped(bool _val)
	{
		chg->group(_val);
	}

	QString pattern();
	void setPattern(QString);

	bool enabled()
	{
		return chg->is_enabled();
	}
	void setEnabled(bool _val)
	{
		return chg->enable(_val);
	}

	bool collapsed()
	{
		return chg->isCollapsed();
	}
	void setCollapsed(bool _val)
	{
		chg->collapse(_val);
	}
	void setThickness(double val)
	{
		chg->setCh_thickness(val,false);
	}
	double thickness()
	{
		return chg->getCh_thickness();
	}

};

class PatternGenerator_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(int channel_size READ channel_size WRITE set_channel_size SCRIPTABLE
	           false);
	Q_PROPERTY(QVariantList channel_list READ getChannels);

	Q_PROPERTY(int channel_groups_size READ channel_groups_size WRITE
	           set_channel_groups_size SCRIPTABLE false);
	Q_PROPERTY(QVariantList channel_groups_list READ getChannelGroups);

	Q_PROPERTY(bool running READ running WRITE run STORED false);
	Q_PROPERTY(bool single READ single WRITE run_single STORED false);
	Q_PROPERTY(bool inactive_hidden READ inactiveHidden WRITE setInactiveHidden);

public:
	explicit PatternGenerator_API(PatternGenerator *pg) :
		ApiObject(), pg(pg) {}
	~PatternGenerator_API() {}


	int channel_groups_size();
	void set_channel_groups_size(int val);
	QVariantList getChannelGroups();

	int channel_size();
	void set_channel_size(int val);
	QVariantList getChannels();

	bool running() const;
	void run(bool en);
	bool single() const;
	void run_single(bool en);
	bool inactiveHidden();
	void setInactiveHidden(bool);

private:
	void refreshApi();
	PatternGenerator *pg;
	QList<PatternGeneratorChannelGroup_API *> pg_cga;
	QList<PatternGeneratorChannel_API *> pg_cha;
};

} /* namespace adiscope */


#endif // LOGIC_ANALYZER_H

