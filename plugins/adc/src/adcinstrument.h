#ifndef ADCINSTRUMENT_H
#define ADCINSTRUMENT_H

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
class ADCInstrument : public QWidget
{
	Q_OBJECT
public:
	ADCInstrument(PlotProxy *proxy, QWidget *parent = nullptr);
	~ADCInstrument();

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

	MenuControlButton *getTimeBtn() const;
	MenuControlButton *getXyBtn() const;
	MenuControlButton *getFftBtn() const;

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
	QButtonGroup *plotGroup;

	RunBtn *runBtn;
	SingleShotBtn *singleBtn;
	MenuControlButton *channelsBtn;
	VerticalChannelManager *m_vcm;

	MenuControlButton *timeBtn;
	MenuControlButton *xyBtn;
	MenuControlButton *fftBtn;

	void setupToolLayout();
	void setupRunSingleButtonHelper();

	void setupChannelsButtonHelper(MenuControlButton *channelsBtn);

	bool m_running;
	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
	void setupTimeButtonHelper(MenuControlButton *time);
	void setupXyButtonHelper(MenuControlButton *xy);
	void setupFFTButtonHelper(MenuControlButton *fft);
};
} // namespace adc
} // namespace scopy

#endif // ADCINSTRUMENT_H
