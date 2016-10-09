/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef M2K_OSCILLOSCOPE_PLOT_H
#define M2K_OSCILLOSCOPE_PLOT_H

#include "TimeDomainDisplayPlot.h"
#include "symbol_controller.h"
#include "handles_area.hpp"
#include "plot_line_handle.h"
#include "cursor_readouts.h"
#include "measure.h"

namespace adiscope {
	class PlotWidget;

	class OscilloscopePlot : public TimeDomainDisplayPlot
	{
		Q_OBJECT

	public:
		OscilloscopePlot(QWidget *parent, unsigned int xNumDivs = 10,
				 unsigned int yNumDiv = 10);
		~OscilloscopePlot();
	};

	class CapturePlot: public OscilloscopePlot
	{
		Q_OBJECT

	public:
		CapturePlot(QWidget *parent, unsigned int xNumDivs = 10,
			    unsigned int yNumDivs = 10);
		~CapturePlot();

		HorizBar *levelTriggerA();
		HorizBar *levelTriggerB();

		QWidget *topArea();
		QWidget *bottomHandlesArea();
		QWidget *leftHandlesArea();
		QWidget *rightHandlesArea();

		bool triggerAEnabled();
		bool triggerBEnabled();
		bool measurementCursorsEnabled();
		bool vertCursorsEnabled();
		bool horizCursorsEnabled();
		int selectedChannel();
		bool measurementsEnabled();
		int channelToMeasure();

		void setOffsetWidgetVisible(int chnIdx, bool visible);
		void removeOffsetWidgets(int chnIdx);

		void measure(int chnIdx);
		double measuredPeriod();
		double measuredFreq();
		double measuredMin();
		double measuredMax();
		double measuredPkToPk();
		double measuredMean();
		double measuredRms();
		double measuredRmsAC();
		double measuredLow();
		double measuredMiddle();
		double measuredHigh();
		double measuredAmplitude();
		double measuredPosOvershoot();
		double measuredNegOvershoot();
		double measuredRiseTime();
		double measuredFallTime();
		double measuredPosWidth();
		double measuredNegWidth();
		double measuredPosDuty();
		double measuredNegDuty();

	signals:
		void timeTriggerValueChanged(double);
		void channelOffsetChanged(double);
		void measurementsAvailable();

	public slots:
		void setTriggerAEnabled(bool en);
		void setTriggerBEnabled(bool en);
		void setMeasurementCursorsEnabled(bool en);
		void setVertCursorsEnabled(bool en);
		void setHorizCursorsEnabled(bool en);
		void setSelectedChannel(int id);
		void setMeasuremensEnabled(bool en);
		void setChannelToMeasure(int chnIdx);

	private slots:
		void onChannelAdded(int);
		void onNewDataReceived();


		void onHbar1PixelPosChanged(int);
		void onHbar2PixelPosChanged(int);
		void onVbar1PixelPosChanged(int);
		void onVbar2PixelPosChanged(int);

		void onTimeCursor1Moved(double);
		void onTimeCursor2Moved(double);
		void onVoltageCursor1Moved(double);
		void onVoltageCursor2Moved(double);

		void onTimeTriggerHandlePosChanged(int);

		void onTriggerAHandleGrabbed(bool);
		void onTriggerBHandleGrabbed(bool);

	private:
		SymbolController *d_symbolCtrl;

		bool d_triggerAEnabled;
		bool d_triggerBEnabled;
		bool d_measurementEnabled;
		bool d_vertCursorsEnabled;
		bool d_horizCursorsEnabled;
		bool d_measurementsEnabled;

		int d_selected_channel;

		QWidget *d_topWidget;
		HorizHandlesArea *d_bottomHandlesArea;
		VertHandlesArea *d_leftHandlesArea;
		VertHandlesArea *d_rightHandlesArea;

		QList<HorizBar*> d_offsetBars;
		QList<RoundedHandleV*> d_offsetHandles;

		PlotLineHandleV *d_vCursorHandle1;
		PlotLineHandleV *d_vCursorHandle2;
		PlotLineHandleH *d_hCursorHandle1;
		PlotLineHandleH *d_hCursorHandle2;

		VertBar *d_vBar1;
		VertBar *d_vBar2;
		HorizBar *d_hBar1;
		HorizBar *d_hBar2;

		VertBar *d_timeTriggerBar;
		HorizBar *d_levelTriggerABar;
		HorizBar *d_levelTriggerBBar;
		FreePlotLineHandleH *d_timeTriggerHandle;
		FreePlotLineHandleV *d_levelTriggerAHandle;
		FreePlotLineHandleV *d_levelTriggerBHandle;

		CursorReadouts *d_cursorReadouts;
		MetricPrefixFormatter d_cursorMetricFormatter;
	        TimePrefixFormatter d_cursorTimeFormatter;

	        QPen d_trigAactiveLinePen;
	        QPen d_trigAinactiveLinePen;
	        QPen d_trigBactiveLinePen;
	        QPen d_trigBinactiveLinePen;

	        Measure d_measure;
	        int d_chnToMeasure;
	};
}

#endif /* M2K_OSCILLOSCOPE_PLOT_H */
