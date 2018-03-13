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
#include "customplotpositionbutton.h"

class QLabel;

namespace adiscope {
	class Oscilloscope_API;
	class PlotWidget;

	class OscilloscopePlot : public TimeDomainDisplayPlot
	{
		Q_OBJECT

	public:
		OscilloscopePlot(QWidget *parent, unsigned int xNumDivs = 10,
				 unsigned int yNumDiv = 10);
		~OscilloscopePlot();
	};

	struct cursorReadoutsText {
		QString t1;
		QString t2;
		QString tDelta;
		QString freq;
		QString v1;
		QString v2;
		QString vDelta;
	};

	class CapturePlot: public OscilloscopePlot
	{
		friend class Oscilloscope_API;

		Q_OBJECT

	public:
		enum TriggerState {
			Waiting,
			Triggered,
			Stop,
			Auto,
		};

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
		bool vertCursorsEnabled();
		bool horizCursorsEnabled();
		int selectedChannel();
		bool measurementsEnabled();
		struct cursorReadoutsText allCursorReadouts() const;

		void setOffsetWidgetVisible(int chnIdx, bool visible);
		void removeOffsetWidgets(int chnIdx);
		void removeLeftVertAxis(unsigned int axis);

		void measure();
		int activeMeasurementsCount(int chnIdx);
		QList<std::shared_ptr<MeasurementData>> measurements(int chnIdx);
		std::shared_ptr<MeasurementData> measurement(int id, int chnIdx);

		OscPlotZoomer* getZoomer();
		void setOffsetInterval(double minValue, double maxValue);
		double getMaxOffsetValue();
		double getMinOffsetValue();

		void bringCurveToFront(unsigned int curveIdx);

		void setTimeBaseZoomed(bool zoomed);

		void enableLabels(bool enabled);
		bool eventFilter(QObject *, QEvent *);
		bool setActiveVertAxis(unsigned int axisIdx, bool selected = true);
		void showYAxisWidget(unsigned int axisIdx, bool en);
		bool enableAxisLabels(bool enabled);

		void setDisplayScale(double value);

		void setTimeTriggerInterval(double min, double max);
		bool labelsEnabled();

	Q_SIGNALS:
		void timeTriggerValueChanged(double);
		void channelOffsetChanged(double);
		void measurementsAvailable();
		void cursorReadoutsChanged(struct cursorReadoutsText);
		void repositionTimeTrigger();

	public Q_SLOTS:
		void setTriggerAEnabled(bool en);
		void setTriggerBEnabled(bool en);
		void setVertCursorsEnabled(bool en);
		void setHorizCursorsEnabled(bool en);
		void setSelectedChannel(int id);
		void setMeasuremensEnabled(bool en);
		void setPeriodDetectLevel(int chnIdx, double lvl);
		void setPeriodDetectHyst(int chnIdx, double hyst);
		void setCursorReadoutsVisible(bool en);
		void setTimeBaseLabelValue(double timebase);
		void setBufferSizeLabelValue(int numSamples);
		void setSampleRatelabelValue(double sampleRate);
		void setTriggerState(int triggerState);
		void setCursorReadoutsTransparency(int value);
		void moveCursorReadouts(CustomPlotPositionButton::ReadoutsPosition position);

	protected:
		virtual void cleanUpJustBeforeChannelRemoval(int chnIdx);

	private:
		Measure* measureOfChannel(int chnIdx) const;
		void updateBufferSizeSampleRateLabel(int nsamples, double sr);

	private Q_SLOTS:
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
		void onTimeTriggerHandleGrabbed(bool);

		void onTriggerAHandleGrabbed(bool);
		void onTriggerBHandleGrabbed(bool);

	private:
		SymbolController *d_symbolCtrl;

		bool d_triggerAEnabled;
		bool d_triggerBEnabled;
		bool d_vertCursorsEnabled;
		bool d_horizCursorsEnabled;
		bool d_measurementsEnabled;
		bool d_labelsEnabled;

		int d_selected_channel;

		QWidget *d_topWidget;
		HorizHandlesArea *d_bottomHandlesArea;
		VertHandlesArea *d_leftHandlesArea;
		VertHandlesArea *d_rightHandlesArea;

		QLabel *d_timeBaseLabel;
		QLabel *d_sampleRateLabel;
		QLabel *d_triggerStateLabel;

		int d_bufferSizeLabelVal;
		double d_sampleRateLabelVal;

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
	        struct cursorReadoutsText d_cursorReadoutsText;
	        bool d_cursorReadoutsVisible;

	        QPen d_trigAactiveLinePen;
	        QPen d_trigAinactiveLinePen;
	        QPen d_trigBactiveLinePen;
	        QPen d_trigBinactiveLinePen;
		QPen d_timeTriggerInactiveLinePen;
		QPen d_timeTriggerActiveLinePen;

	        QList<Measure *> d_measureObjs;

		double value_v1, value_v2, value_h1, value_h2;
		double d_minOffsetValue, d_maxOffsetValue;
		double d_timeTriggerMinValue, d_timeTriggerMaxValue;
	};
}

#endif /* M2K_OSCILLOSCOPE_PLOT_H */
