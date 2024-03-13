#include "timegrplot.h"
#include <pluginbase/preferences.h>
#include <QtConcurrent>
#include "adcinstrument.h"

using namespace scopy;
using namespace scopy::grutil;

Q_LOGGING_CATEGORY(CAT_GRTIMEPLOT, "GRTimePlot");


TimeGRPlotRecipe::TimeGRPlotRecipe(GRTopBlock *top, QObject *parent, bool main) : PlotRecipe(parent,main), m_top(top) {}

TimeGRPlotRecipe::~TimeGRPlotRecipe()
{

}

void TimeGRPlotRecipe::init()
{
	Preferences *p = Preferences::GetInstance();
	auto widget = new QTabWidget();
	m_lay = new QVBoxLayout(widget);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);

	m_plotWidget = new PlotWidget(widget);
	m_plotWidget->xAxis()->setInterval(0, 1);
	m_plotWidget->leftHandlesArea()->setVisible(true);
	m_plotWidget->rightHandlesArea()->setVisible(true);
	m_plotWidget->bottomHandlesArea()->setVisible(true);
	m_plotWidget->xAxis()->setVisible(true);
	//	m_plotWidget->topHandlesArea()->setVisible(true);

	m_info = new TimePlotInfo(m_plotWidget, widget);
	m_plotWidget->addPlotInfoSlot(m_info);

	       //	m_lay->addWidget(m_plotWidget);
	m_plotTimer = new QTimer(this);
	m_plotTimer->setSingleShot(true);
	connect(m_plotTimer, &QTimer::timeout, this, &TimeGRPlotRecipe::replot);
	connect(p, SIGNAL(preferenceChanged(QString, QVariant)), this, SLOT(handlePreferences(QString, QVariant)));

	fw = new QFutureWatcher<void>(this);
	futureWatcherConn = connect(
		fw, &QFutureWatcher<void>::finished, this,
		[=]() {
			drawPlot();
			if(m_refreshTimerRunning)
				m_plotTimer->start();
		},
		Qt::QueuedConnection);
}

void TimeGRPlotRecipe::deinit()
{

}

void TimeGRPlotRecipe::drawPlot()
{
	//	qInfo(CAT_GRTIMEPLOT)<<"Draw plot";
	if(!time_sink)
		return;
	setRawSamplesPtr();
	drawTags();
	plot()->replot();
	if(time_sink->finishedAcquisition())
		m_instrument->stop();
}

void TimeGRPlotRecipe::replot()
{
	if(!time_sink)
		return;
#ifdef GUI_THREAD_SAMPLING
	time_sink->updateData();
	drawPlot();
	if(m_refreshTimerRunning)
		m_plotTimer->start();
#else
	refillFuture = QtConcurrent::run([=]() {
		//		qInfo(CAT_GRTIMEPLOT)<<"UpdateData";
		std::unique_lock lock(refillMutex);

		time_sink->updateData();
	});
	fw->setFuture(refillFuture);
#endif
}

void TimeGRPlotRecipe::stopPlotRefresh()
{

}

void GRTimePlotAddon::stopPlotRefresh()
{
	qInfo(CAT_GRTIMEPLOT) << "Stopped plotting";
	m_refreshTimerRunning = false;
#ifdef GUI_THREAD_SAMPLING
#else
	refillFuture.cancel();
//	disconnect(futureWatcherConn);
#endif
	m_plotTimer->stop();
}

void GRTimePlotAddon::startPlotRefresh()
{
	qInfo(CAT_GRTIMEPLOT) << "Start plotting";
	updateFrameRate();
	m_refreshTimerRunning = true;

#ifdef GUI_THREAD_SAMPLING
#else

#endif
	drawPlot();
	m_plotTimer->start();
}

PlotWidget *TimeGRPlotRecipe::plot() { return m_plotWidget; }
