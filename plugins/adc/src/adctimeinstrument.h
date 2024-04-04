#ifndef ADCTIMEINSTRUMENT_H
#define ADCTIMEINSTRUMENT_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

#include <gui/tooltemplate.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/verticalchannelmanager.h>
#include <gui/cursorcontroller.h>
#include <gui/widgets/measurementpanel.h>
#include <gui/widgets/measurementsettings.h>

#include "toolcomponent.h"

namespace scopy {
namespace adc {
class ADCTimeInstrument : public QWidget
{
	Q_OBJECT
public:
	ADCTimeInstrument(PlotProxy *proxy, QWidget *parent = nullptr);
	~ADCTimeInstrument();

	bool running() const;
	void setRunning(bool newRunning);

	ToolTemplate *getToolTemplate();
	MapStackedWidget *getRightStack();

	int uuid = 0;
	const QString channelsMenuId = "channels";
	const QString measureMenuId = "measure";
	const QString statsMenuId = "stats";
	const QString verticalChannelManagerId = "vcm";
	const QString settingsMenuId = "settings";

	VerticalChannelManager *vcm() const;

public Q_SLOTS:
	void run(bool);
	void stop();
	void start();
	void restart();
	void addDevice(CollapsableMenuControlButton *b, ToolComponent *dev);
	void addChannel(MenuControlButton *btn, ToolComponent *ch, CompositeWidget *c);


Q_SIGNALS:
	void setSingleShot(bool);
	void requestStop();
	void runningChanged(bool);

private:
	void init();
	void deinit();

	ToolTemplate *tool;
	PlotProxy *proxy;

	QPushButton *openLastMenuBtn;
	MapStackedWidget *rightStack;
	QButtonGroup *rightMenuBtnGrp;
	QButtonGroup *channelGroup;

	RunBtn *runBtn;
	SingleShotBtn *singleBtn;
	MenuControlButton *channelsBtn;
	VerticalChannelManager *m_vcm;

	void setupToolLayout();
	void setupRunSingleButtonHelper();

	void setupChannelsButtonHelper(MenuControlButton *channelsBtn);

	bool m_running;
	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
};
} // namespace adc
} // namespace scopy

#endif // ADCTIMEINSTRUMENT_H
