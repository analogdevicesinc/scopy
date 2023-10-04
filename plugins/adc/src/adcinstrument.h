#ifndef ADCINSTRUMENT_H
#define ADCINSTRUMENT_H

#include <QWidget>
#include <adcplugin.h>
#include <cursorcontroller.h>
#include "gui/tooltemplate.h"
#include <gui/widgets/toolbuttons.h>
#include <QPushButton>

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
Q_SIGNALS:
	void runningChanged(bool);

private:
	bool m_running;
	RunBtn *runBtn;
	SingleShotBtn *singleBtn;
	ToolTemplate *tool;
	PlotProxy* proxy;
	QPushButton *openLastMenuBtn;

	MenuControlButton *channelsBtn;

	MeasurementsPanel* measure_panel;
	StatsPanel* stats_panel;

	GRTimePlotAddon* plotAddon;
	GRTimePlotAddonSettings* plotAddonSettings;

	MapStackedWidget *channelStack;
	QButtonGroup* rightMenuBtnGrp;
	QButtonGroup *channelGroup;

	CursorController *cursorController;

	void setupCursorButtonHelper(MenuControlButton *cursor);
	void setupMeasureButtonHelper(MenuControlButton *measure);
	void setupChannelsButtonHelper(MenuControlButton *channelsBtn);
	void setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, GRDeviceAddon *dev);
	void setupChannelMenuControlButtonHelper(MenuControlButton *btn, GRTimeChannelAddon *ch);
	void initCursors();

	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

	int uuid = 0;
	const QString channelsMenuId = "channels";
	const QString measureMenuId = "measure";
	const QString statsMenuId = "stats";
	const QString verticalChannelManagerId = "vcm";
};
}
#endif // ADCINSTRUMENT_H
