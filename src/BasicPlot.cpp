#include "BasicPlot.h"

#include <QwtPlotOpenGLCanvas>
#include <QDebug>

#include <logging_categories.h>
#include <tool_launcher.hpp>

namespace adiscope {

static int staticPlotId = 0;

#define replotFrameDuration 1000.0/replotFrameRate

BasicPlot::BasicPlot(QWidget* parent) : QwtPlot(parent), started(false), replotFrameRate(60)
{
	connect(&replotTimer,SIGNAL(timeout()),this,SLOT(replotNow()));
	QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas *>( canvas() );
	plotCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore );
#ifdef IMMEDIATE_PAINT
	plotCanvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);
#endif
	qDebug(CAT_PLOT)<<QString::number(id)<<"Created plot";
	id = staticPlotId;
	staticPlotId++; // for debug
	pfps.setCapacity(fpsHistoryCount);
	ifps.setCapacity(fpsHistoryCount);
	fpsLabel.attach(this);
	QFont font;
	font.setBold( true );
	fpsTxt.setFont( font );
	fpsTxt.setRenderFlags( Qt::AlignRight | Qt::AlignTop );
	fpsTxt.setColor(QColor("red"));
	fpsLabelTimer.start();
	replotNow();
}

void BasicPlot::startStop(bool en) {
	if(en) {
		start();
	} else  {
		stop();
	}
}

void BasicPlot::start() {
	if(debug) qDebug(CAT_PLOT)<<QString::number(id)<<"BasicPlot - Starting plot - framerate - " << replotFrameRate;
	pfps.clearHistory();
	ifps.clearHistory();
	ims.clearHistory();
	pms.clearHistory();

	started = true;
}

bool BasicPlot::isStarted() {
	return started;
}
void BasicPlot::stop() {
	if(debug) qDebug(CAT_PLOT)<<QString::number(id)<<"BasicPlot - Stopping plot - will force one replot";
	started = false;
	replotTimer.stop();
	replotNow();
}
void BasicPlot::setRefreshRate(double hz) {
	replotFrameRate = hz;
	if(replotTimer.isActive())	{
		replotTimer.start(replotFrameRate);
	}
}

void BasicPlot::setVisibleFpsLabel(bool vis) {
	fpsLabel.setVisible(vis);
}

void BasicPlot::hideFpsLabel() {
	fpsLabel.hide();
	debug = false;
}
void BasicPlot::showFpsLabel() {
	fpsLabel.show();
	debug = true;
}

double BasicPlot::getRefreshRate() {
	return replotFrameRate;

}
void BasicPlot::replotNow() {
	auto instrumentCycle = fpsTimer.nsecsElapsed() / 1e+06;	;
	fpsTimer.start();

	QwtPlot::replot();

	auto replotDuration = fpsTimer.nsecsElapsed() / 1e+06;
	fpsTimer.restart();

	// -------- COMPUTE TIMINGS --------
	instrumentCycle = instrumentCycle + replotDuration;
//	auto instrumentFps = 1000.0/(double)(instrumentCycle);

	auto instrumentCycleAvg = ims.pushValueReturnAverage(instrumentCycle);
//	auto instrumentFpsAvg = ifps.pushValueReturnAverage(instrumentFps);

#ifdef IMMEDIATE_PAINT
	auto replotTheoreticalFpsAvg = pfps.pushValueReturnAverage(1000.0/replotDuration);
	auto replotDurationAvg = pms.pushValueReturnAverage(replotDuration);
#endif

	if(debug) qDebug(CAT_PLOT)	<< QString::number(id) << " - BasicPlot - drawing plot - "
				<< 1000.0 / instrumentCycleAvg <<" fps - "
				<< instrumentCycleAvg << " ms"
#ifdef IMMEDIATE_PAINT
				<< replotTheoreticalFpsAvg << " fps "
				<< replotDurationAvg << "ms avg frame time"
#endif
				;

	if(fpsLabelTimer.elapsed() > fpsLabelRefreshTime) {
		QString rendering = (useOpenGlCanvas) ? "OpenGl rendering" : "Software rendering";
		fpsTxt.setText(rendering + "\n" +
			       "instrument: " + QString::number(1000.0 / instrumentCycleAvg) + " fps / " + QString::number(instrumentCycleAvg) + " ms" + "\n"
#ifdef IMMEDIATE_PAINT
					+ "plot: " + QString::number(replotTheoreticalFpsAvg) + "fps / " + QString::number(replotDurationAvg) + " ms"
#endif
					);
		fpsLabel.setText(fpsTxt);
		fpsLabelTimer.restart();
	}
}


void BasicPlot::replot() {
#ifdef IMMEDIATE_PAINT
		if(!replotTimer.isActive()) {
			//qDebug(CAT_PLOT)<<QString::number(id)<<"FreeRunningPlot - freerunning replot - schedule next frame";
			replotTimer.setSingleShot(true);
			replotTimer.start(1000.0/replotFrameRate);
		}
		else
		{
			if(debug) qDebug(CAT_PLOT)<<QString::number(id)<<"BasicPlot - replot - already scheduled";
		}
#else
	replotNow();
#endif
}

} // adiscope
