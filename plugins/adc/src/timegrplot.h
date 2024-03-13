#ifndef TIMEGRPLOT_H
#define TIMEGRPLOT_H

#include <scopy-adcplugin_export.h>
#include <QWidget>
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include "gr-util/time_sink_f.h"
#include <gr-util/grtopblock.h>

#include <gui/plotwidget.h>
#include <gui/plotaxis.h>
#include <plotinfo.h>
#include "plotrecipe.h"

namespace scopy {
using namespace grutil;

class SCOPY_ADCPLUGIN_EXPORT TimeGRPlotRecipe : public PlotRecipe {
public:
	TimeGRPlotRecipe(GRTopBlock *top, QObject *parent = nullptr, bool main = false);
	~TimeGRPlotRecipe();

	virtual void init() override;
	virtual void deinit() override;

	PlotWidget *plot();

public Q_SLOTS:
	void replot();

private Q_SLOTS:
	void stopPlotRefresh();
	void startPlotRefresh();
	void drawPlot();

private:
	QTimer *m_plotTimer;
	GRTopBlock *m_top;

	PlotWidget *m_plotWidget;

	TimePlotInfo *m_info;
	time_sink_f::sptr time_sink;


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

	QMap<QString, int> time_channel_map;

	void setRawSamplesPtr();
	void updateXAxis();
	void updateFrameRate();
	void drawTags();

	QVBoxLayout *m_lay;
};
}

#endif // TIMEGRPLOT_H
