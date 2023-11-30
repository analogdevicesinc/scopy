#ifndef GRTIMEPLOTADDON_H
#define GRTIMEPLOTADDON_H

#include "plotinfo.h"
#include "scopy-gr-util_export.h"
#include "time_sink_f.h"
#include "timechanneladdon.h"
#include "tooladdon.h"

#include <QFuture>
#include <QGridLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <gui/oscilloscope_plot.hpp>
#include <plotwidget.h>

namespace scopy {
class TimePlotHDivInfo;
class TimePlotSamplingInfo;
namespace grutil {
class GRTopBlock;
class GRTimeChannelAddon;
class GRTimePlotAddonSettings;
class ChannelAddon;

class SCOPY_GR_UTIL_EXPORT PlotAddon
{
public:
	virtual PlotWidget *plot() = 0;
	virtual void replot() = 0;
	virtual double sampleRate() = 0;
};

class SCOPY_GR_UTIL_EXPORT GRTimePlotAddon : public QObject,
					     public ToolAddon,
					     public ChannelConfigAware,
					     public PlotAddon,
					     public GRTopAddon
{
	Q_OBJECT
public:
	GRTimePlotAddon(QString name, GRTopBlock *top, QObject *parent = nullptr);
	virtual ~GRTimePlotAddon();

	QString getName() override;
	QWidget *getWidget() override;
	PlotWidget *plot() override;
	double sampleRate() override;
	int xMode();

	QList<GRTimeChannelAddon *> getGrChannels() const;

Q_SIGNALS:
	void requestRebuild();
	void requestStop();
	void newData();
	void xAxisUpdated();

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;
	void preFlowStart() override;
	void postFlowStart() override;
	void preFlowStop() override;
	void postFlowStop() override;

	void onChannelAdded(ChannelAddon *t) override;
	void onChannelRemoved(ChannelAddon *t) override;

	void replot() override;
	void connectSignalPaths();
	void tearDownSignalPaths();
	void onNewData();
	void updateBufferPreviewer();

	void setRollingMode(bool b);
	void setDrawPlotTags(bool b);
	void setSampleRate(double);
	void setBufferSize(uint32_t size);
	void setPlotSize(uint32_t size);
	void handlePreferences(QString, QVariant);
	void setSingleShot(bool);
	void setFrameRate(double);
	void setXMode(int mode);

private Q_SLOTS:
	void stopPlotRefresh();
	void startPlotRefresh();
	void drawPlot();

private:
	QTimer *m_plotTimer;
	GRTopBlock *m_top;
	QString name;
	QWidget *widget;

	PlotWidget *m_plotWidget;
	PlotWidget *m_fftPlotWidget;
	PlotWidget *m_xyPlotWidget;

	TimePlotInfo *m_info;
	time_sink_f::sptr time_sink;
	QList<GRTimeChannelAddon *> grChannels;
	QVBoxLayout *m_lay;
	void setupBufferPreviewer();

	QFuture<void> refillFuture;
	QFutureWatcher<void> *fw;
	QMetaObject::Connection futureWatcherConn;
	std::mutex refillMutex;

	PlotSamplingInfo m_currentSamplingInfo;
	bool m_started;
	bool m_rollingMode;
	bool m_singleShot;
	bool m_showPlotTags;
	bool m_refreshTimerRunning;
	int m_xmode;

	QMap<QString, int> time_channel_map;

	void setRawSamplesPtr();
	void updateXAxis();
	void updateFrameRate();
	void drawTags();
};
} // namespace grutil
} // namespace scopy

#endif // GRTIMEPLOTADDON_H
