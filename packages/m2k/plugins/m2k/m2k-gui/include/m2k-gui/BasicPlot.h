/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef BASICPLOT_H
#define BASICPLOT_H

/*Qwt includes*/
#include "scopy-m2k-gui_export.h"
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
#include "movingaverage.h"

//#define IMMEDIATE_PAINT

namespace scopy {

class SCOPY_M2K_GUI_EXPORT BasicPlot : public QwtPlot
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
