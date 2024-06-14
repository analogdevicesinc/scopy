#ifndef HARMONICCALIBRATION_H
#define HARMONICCALIBRATION_H

#include "scopy-admt_export.h"

#include <QBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QString>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QButtonGroup>

#include <admtplugin.h>
#include <iio.h>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include <menuheader.h>
#include <menusectionwidget.h>
#include <menucollapsesection.h>
#include <menucontrolbutton.h>
#include <verticalchannelmanager.h>
#include <measurementsettings.h>

namespace scopy {
class MenuControlButton;
class CollapsableMenuControlButton;

class HarmonicCalibration : public QWidget
{
	Q_OBJECT
public:
	HarmonicCalibration(PlotProxy *proxy, QWidget *parent = nullptr);
	~HarmonicCalibration();
	void init();
	void deinit();
	void startAddons();
	void stopAddons();
	bool running() const;
	void setRunning(bool newRunning);
public Q_SLOTS:
	void run(bool);
	void stop();
	void start();
	void restart();
	void showMeasurements(bool b);
	void createSnapshotChannel(SnapshotProvider::SnapshotRecipe rec);
	void deleteChannel(ChannelAddon *);
	MenuControlButton *addChannel(ChannelAddon *channelAddon, QWidget *parent);
	CollapsableMenuControlButton *addDevice(GRDeviceAddon *dev, QWidget *parent);
Q_SIGNALS:
	void runningChanged(bool);
private:
	bool m_running;
	ToolTemplate *tool;
	GearBtn *settingsButton;
	InfoBtn *infoButton;
	RunBtn *runButton;

	QPushButton *openLastMenuButton;
	PlotProxy *proxy;
	GRTimePlotAddon *plotAddon;
	GRTimePlotAddonSettings *plotAddonSettings;
	QButtonGroup *rightMenuButtonGroup;

	MenuControlButton *channelsButton;
	VerticalChannelManager *verticalChannelManager;
	QButtonGroup *channelGroup;
	MapStackedWidget *channelStack;
	MeasurementsPanel *measurePanel;
	MeasurementSettings *measureSettings;
	StatsPanel *statsPanel;

	void setupChannelsButtonHelper(MenuControlButton *channelsButton);
	void setupMeasureButtonHelper(MenuControlButton *measureButton);
	void setupChannelSnapshot(ChannelAddon *channelAddon);
	void setupChannelMeasurement(ChannelAddon *channelAddon);
	void setupChannelDelete(ChannelAddon *channelAddon);
	void setupChannelMenuControlButtonHelper(MenuControlButton *menuControlButton, ChannelAddon *channelAddon);
	void setupDeviceMenuControlButtonHelper(MenuControlButton *menuControlButton, GRDeviceAddon *channelAddon);

	int uuid = 0;
	const QString channelsMenuId = "channels";
	const QString measureMenuId = "measure";
	const QString statsMenuId = "stats";
	const QString verticalChannelManagerId = "vcm";
};
} // namespace scopy::admt
#endif // HARMONICCALIBRATION_H
