#include "BasicPlot.h"
#include <QwtPlotOpenGLCanvas>
#include <QDebug>

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
	qDebug(CAT_FREE_RUNNING_PLOT)<<QString::number(id)<<"Created freerunning plot";
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
	qDebug(CAT_FREE_RUNNING_PLOT)<<QString::number(id)<<"Starting freerunning plot - framerate - " << replotFrameRate;
	pfps.clearHistory();
	ifps.clearHistory();
	started = true;
}

bool BasicPlot::isStarted() {
	return started;
}
void BasicPlot::stop() {
	qDebug(CAT_FREE_RUNNING_PLOT)<<QString::number(id)<<"FreeRunningPlot - Stopping freerunning plot - will force one replot";
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
}
void BasicPlot::showFpsLabel() {
	fpsLabel.show();
}

double BasicPlot::getRefreshRate() {
	return replotFrameRate;

}
void BasicPlot::replotNow() {
	auto replotDuration = fpsTimer.nsecsElapsed() / 1e+06;	;
	fpsTimer.start();

	QwtPlot::replot();

	auto replotFrameTime = fpsTimer.nsecsElapsed() / 1e+06;
	fpsTimer.restart();

	auto iFps = 1000.0/(double)(replotDuration + replotFrameTime);

	auto instrumentFpsAvg = ifps.pushValueReturnAverage(iFps);
	auto replotFrameTimeAvg = pfps.pushValueReturnAverage(replotFrameTime);

	qDebug(CAT_FREE_RUNNING_PLOT)	<< QString::number(id) << " - FreeRunningPlot - drawing plot - "
				<< instrumentFpsAvg <<" fps - "
#ifdef IMMEDIATE_PAINT
				<< replotFrameTime	<< "ms frame time - "
				<< replotFrameTimeAvg << "ms avg frame time"
#endif
				;


	fpsTxt.setText(QString::number(instrumentFpsAvg) + " fps    "
#ifdef IMMEDIATE_PAINT
					+ QString::number(replotFrameTimeAvg) + "ms replotFrameTime"
#endif
					);
	fpsLabel.setText(fpsTxt);

}


void BasicPlot::replot() {
#ifdef IMMEDIATE_PAINT
		if(!replotTimer.isActive()) {
			qDebug(CAT_FREE_RUNNING_PLOT)<<QString::number(id)<<"FreeRunningPlot - freerunning replot - schedule next frame";
			replotTimer.setSingleShot(true);
			replotTimer.start(1000.0/replotFrameRate);
		}
		else
		{
			qDebug(CAT_FREE_RUNNING_PLOT)<<QString::number(id)<<"FreeRunningPlot - freerunning replot - already scheduled";
		}
#else
	replotNow();
#endif
}

} // adiscope
