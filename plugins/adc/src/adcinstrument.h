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
#include <adcplugin.h>


namespace scopy {
namespace adc {

class ADCInstrument : public QWidget
{
	Q_OBJECT
public:
	ADCInstrument(PlotProxy *proxy, ToolMenuEntry *tme, QWidget *parent = nullptr);
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

	QPushButton *sync() const;

public Q_SLOTS:
	void run(bool);
	void stop();
	void start();
	void restart();
	void addDevice(CollapsableMenuControlButton *b, ToolComponent *dev);
	void addChannel(MenuControlButton *btn, ToolComponent *ch, CompositeWidget *c);

Q_SIGNALS:
	void setSingleShot(bool);
	void runningChanged(bool);
	void requestNewInstrument(ADCInstrumentType t);
	void requestDeleteInstrument();

private:
	void init();
	void deinit();

	ToolTemplate *tool;
	ToolMenuEntry *m_tme;
	PlotProxy *proxy;

	QPushButton *openLastMenuBtn;
	MapStackedWidget *rightStack;
	QButtonGroup *rightMenuBtnGrp;
	QButtonGroup *channelGroup;

	AddBtn *addBtn;
	RemoveBtn *removeBtn;
	RunBtn *runBtn;
	SingleShotBtn *singleBtn;
	QPushButton *m_sync;
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
static int instrumentIdx = 0;

#endif // ADCINSTRUMENT_H
