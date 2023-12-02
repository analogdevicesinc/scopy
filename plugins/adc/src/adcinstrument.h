#ifndef ADCINSTRUMENT_H
#define ADCINSTRUMENT_H

#include "gui/tooltemplate.h"
#include "measurementsettings.h"
#include "verticalchannelmanager.h"

#include <QPushButton>
#include <QWidget>

#include <adcplugin.h>
#include <cursorcontroller.h>
#include <gui/widgets/toolbuttons.h>

namespace scopy {
class MenuControlButton;
class CollapsableMenuControlButton;

class AdcInstrument : public QWidget
{
	Q_OBJECT
public:
	AdcInstrument(PlotProxy *proxy, QWidget *parent = nullptr);
	~AdcInstrument();
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
	MenuControlButton *addChannel(ChannelAddon *ch, QWidget *parent);
	void deleteChannel(ChannelAddon *);
	CollapsableMenuControlButton *addDevice(GRDeviceAddon *dev, QWidget *parent);
Q_SIGNALS:
	void runningChanged(bool);

private:
	bool m_running;
	RunBtn *runBtn;
	SingleShotBtn *singleBtn;
	ToolTemplate *tool;
	PlotProxy *proxy;
	QPushButton *openLastMenuBtn;

	MenuControlButton *channelsBtn;

	MeasurementsPanel *measure_panel;
	MeasurementSettings *measureSettings;
	StatsPanel *stats_panel;

	GRTimePlotAddon *plotAddon;
	GRTimePlotAddonSettings *plotAddonSettings;
	VerticalChannelManager *vcm;

	MapStackedWidget *channelStack;
	QButtonGroup *rightMenuBtnGrp;
	QButtonGroup *channelGroup;

	CursorController *cursorController;
	CursorController *fftcursorController;

	void setupTimeButtonHelper(MenuControlButton *time);
	void setupXyButtonHelper(MenuControlButton *xy);
	void setupFFTButtonHelper(MenuControlButton *fft);
	void setupCursorButtonHelper(MenuControlButton *cursor);
	void setupMeasureButtonHelper(MenuControlButton *measure);
	void setupChannelsButtonHelper(MenuControlButton *channelsBtn);
	void setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, GRDeviceAddon *dev);
	void setupChannelMenuControlButtonHelper(MenuControlButton *btn, ChannelAddon *ch);
	void initCursors();
	void setupChannelDelete(ChannelAddon *ch);

	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

	int uuid = 0;
	const QString channelsMenuId = "channels";
	const QString measureMenuId = "measure";
	const QString statsMenuId = "stats";
	const QString verticalChannelManagerId = "vcm";
	void setupChannelMeasurement(ChannelAddon *ch);
	void setupChannelSnapshot(ChannelAddon *ch);
};
} // namespace scopy
#endif // ADCINSTRUMENT_H
