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

	const QString channelsMenuId = "channels";
	const QString measureMenuId = "measure";
	const QString statsMenuId = "stats";
	const QString verticalChannelManagerId = "vcm";
	const QString settingsMenuId = "settings";

public Q_SLOTS:
	void run(bool);
	void stop();
	void start();
	void restart();

Q_SIGNALS:
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
	VerticalChannelManager *vcm;

	void setupToolLayout();
	void setupRunSingleButtonHelper();

	void setupChannelsButtonHelper(MenuControlButton *channelsBtn);
	/*void setupCursorButtonHelper(MenuControlButton *cursor);
	void setupMeasureButtonHelper(MenuControlButton *measure);*/

	bool m_running;
	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
};
} // namespace adc
} // namespace scopy

#endif // ADCTIMEINSTRUMENT_H
