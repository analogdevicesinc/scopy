/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
#include "graticule.h"

#include <functional>

#include <qwt_plot_zoneitem.h>

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
		friend class Channel_API;

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

		void replot();

		HorizBar *levelTriggerA();
		HorizBar *levelTriggerB();

		QWidget *topArea();
		QWidget *topHandlesArea();
		QWidget *bottomHandlesArea();
		QWidget *leftHandlesArea();
		QWidget *rightHandlesArea();

		void setBonusWidthForHistogram(int width);

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
		void setActiveVertAxis(unsigned int axisIdx, bool selected = true);
		void showYAxisWidget(unsigned int axisIdx, bool en);
		void enableAxisLabels(bool enabled);

		void setDisplayScale(double value);

		void setTimeTriggerInterval(double min, double max);
		bool labelsEnabled();
		void trackModeEnabled(bool enabled);
		void repositionCursors();

		void setGraticuleEnabled(bool enabled);
		void setGatingEnabled(bool enabled);

		void computeMeasurementsForChannel(unsigned int chnIdx, unsigned int sampleRate);

		void setConversionFunction(const std::function<double(unsigned int, double, bool)> &fp);

		void enableXaxisLabels();
		void enableTimeTrigger(bool enable);
		QString getChannelName(int chIdx) const;
		void setChannelName(const QString &name, int chIdx);

	Q_SIGNALS:
		void timeTriggerValueChanged(double);
		void channelOffsetChanged(unsigned int, double);
		void measurementsAvailable();
		void cursorReadoutsChanged(struct cursorReadoutsText);
		void canvasSizeChanged();
		void leftGateChanged(double);
		void rightGateChanged(double);
		void channelSelected(int, bool);

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
		void setHorizCursorsLocked(bool value);
		void setVertCursorsLocked(bool value);
		void showEvent(QShowEvent *event);
		void printWithNoBackground(const QString& toolName = "", bool editScaleDraw = true);
		void onDigitalChannelAdded(int chnIdx);
		void setChannelSelectable(int chnIdx, bool selectable);
		void removeDigitalPlotCurve(QwtPlotCurve *curve);
		void setOffsetHandleVisible(int chIdx, bool visible);
		void addToGroup(int currentGroup, int toAdd);
		void beginGroupSelection();
		bool endGroupSelection(bool moveAnnotationCurvesLast = false);    // TODO: toggle group selection
		QVector<int> getGroupOfChannel(int chnIdx);
		QVector<QVector<int>> getAllGroups();
		void removeFromGroup(int chnIdx, int removedChnIdx, bool &didGroupVanish);
		void positionInGroupChanged(int chnIdx, int from, int to);
		void setGroups(const QVector<QVector<int>> &groups);

	protected:
		virtual void cleanUpJustBeforeChannelRemoval(int chnIdx);

	private:
		Measure* measureOfChannel(int chnIdx) const;
		void updateBufferSizeSampleRateLabel(int nsamples, double sr);
		void updateHandleAreaPadding(bool);
		double getHorizontalCursorIntersection(double time);
		void displayIntersection();
		void updateGateMargins();

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

		void onGateBar1PixelPosChanged(int);
		void onGateBar2PixelPosChanged(int);

		void onGateBar1Moved(double);
		void onGateBar2Moved(double);

		void onTimeTriggerHandlePosChanged(int);
		void onTimeTriggerHandleGrabbed(bool);

		void onTriggerAHandleGrabbed(bool);
		void onTriggerBHandleGrabbed(bool);

		void handleInGroupChangedPosition(int position);
	private:
		std::function<double(unsigned int, double, bool)> m_conversion_function;
		SymbolController *d_symbolCtrl;

		bool d_triggerAEnabled;
		bool d_triggerBEnabled;
		bool d_vertCursorsEnabled;
		bool d_horizCursorsEnabled;
		bool d_measurementsEnabled;
		bool d_labelsEnabled;

		int d_selected_channel;

		QWidget *d_topWidget;
		GateHandlesArea *d_topHandlesArea;
		HorizHandlesArea *d_bottomHandlesArea;
		VertHandlesArea *d_leftHandlesArea;
		VertHandlesArea *d_rightHandlesArea;
		int d_bonusWidth;

		QLabel *d_timeBaseLabel;
		QLabel *d_sampleRateLabel;
		QLabel *d_triggerStateLabel;

		int d_bufferSizeLabelVal;
		double d_sampleRateLabelVal;

		QList<HorizBar*> d_offsetBars;
		QList<RoundedHandleV*> d_offsetHandles;

		// Channel grouping
		QVector<QList<RoundedHandleV*>> d_groupHandles;
		bool d_startedGrouping;
		QVector<QwtPlotZoneItem *> d_groupMarkers;

		PlotLineHandleV *d_vCursorHandle1;
		PlotLineHandleV *d_vCursorHandle2;
		PlotLineHandleH *d_hCursorHandle1;
		PlotLineHandleH *d_hCursorHandle2;

		PlotGateHandle *d_hGatingHandle1;
		PlotGateHandle *d_hGatingHandle2;

		VertBar *d_vBar1;
		VertBar *d_vBar2;
		HorizBar *d_hBar1;
		HorizBar *d_hBar2;

		VertBar *d_gateBar1;
		VertBar *d_gateBar2;

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
		double value_gateLeft, value_gateRight;
		double d_minOffsetValue, d_maxOffsetValue;
		double d_timeTriggerMinValue, d_timeTriggerMaxValue;

		bool displayGraticule;
		Graticule *graticule;

		bool d_trackMode;
		QwtPlotMarker *markerIntersection1;
		QwtPlotMarker *markerIntersection2;
		bool horizCursorsLocked;
		bool vertCursorsLocked;
		int pixelPosHandleHoriz1;
		int pixelPosHandleHoriz2;
		int pixelPosHandleVert1;
		int pixelPosHandleVert2;

		QwtPlotShapeItem *leftGate, *rightGate;
		QRectF leftGateRect, rightGateRect;
		bool d_gatingEnabled;

		QPair<double, double> d_xAxisInterval;
		int d_currentHandleInitPx;
	};
}

#endif /* M2K_OSCILLOSCOPE_PLOT_H */
