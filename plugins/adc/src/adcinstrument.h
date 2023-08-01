#ifndef ADCINSTRUMENT_H
#define ADCINSTRUMENT_H

#include <QWidget>
#include <adcplugin.h>
#include "gui/tooltemplate.h"
#include <gui/widgets/toolbuttons.h>
#include <QPushButton>

namespace scopy {
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
Q_SIGNALS:
	void runningChanged(bool);

private:
	bool m_running;
	RunBtn *runBtn;
	SingleShotBtn *singleBtn;
	ToolTemplate *tool;
	PlotProxy* proxy;
	QPushButton *openLastMenuBtn;

	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
};
}
#endif // ADCINSTRUMENT_H
