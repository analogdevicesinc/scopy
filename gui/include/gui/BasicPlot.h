#ifndef BASICPLOT_H
#define BASICPLOT_H

/*Qwt includes*/
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_textlabel.h>
#include <qwt_scale_engine.h>
#include <qwt_text.h>

/*Qt includes*/
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

/*Own includes */
#include <movingaverage.h>

//#define IMMEDIATE_PAINT
#include "scopy-gui_export.h"

namespace scopy {

class SCOPY_GUI_EXPORT BasicPlot : public QwtPlot
{
	Q_OBJECT
public:
	BasicPlot(QWidget *parent);

public Q_SLOTS:
	void start();
	void startStop(bool en = true);
	void stop();
	bool isStarted();

	void setRefreshRate(double hz);
	double getRefreshRate();

	void replotNow();
	void setVisibleFpsLabel(bool vis);
	void hideFpsLabel();
	void showFpsLabel();
	void replot() override;

protected:
	QTimer replotTimer;

private:
	static int staticPlotId;
	bool started;
	bool debug;
	bool useOpenGlCanvas;
	double replotFrameRate;
	int id = 0;
	MovingAverage pfps, pms;
	MovingAverage ifps, ims;
	const int fpsHistoryCount = 30;
	const int fpsLabelRefreshTime = 500;
	QwtPlotTextLabel fpsLabel;
	QwtText fpsTxt;
	QElapsedTimer fpsTimer;
	QElapsedTimer fpsLabelTimer;
};

} // namespace scopy

#endif // BASICPLOT_H
