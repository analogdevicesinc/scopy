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
#include "gui/measure.h"
#include "gui/customplotpositionbutton.h"
#include "graticule.h"

#include <functional>

#include <qwt_plot_zoneitem.h>

#include <logicanalyzer/genericlogicplotcurve.h>

class QLabel;

namespace adiscope {
	class Oscilloscope_API;
	class PlotWidget;

	class OscilloscopePlot : public TimeDomainDisplayPlot
	{
		Q_OBJECT

	public:
		OscilloscopePlot(QWidget *parent, bool isdBgraph = false, unsigned int xNumDivs = 10,
				 unsigned int yNumDiv = 10, PrefixFormatter* pfXaxis = nullptr, PrefixFormatter* pfYaxis = nullptr);
		~OscilloscopePlot();
	};

	class CapturePlot: public OscilloscopePlot
	{
		friend class Oscilloscope_API;
		friend class LogicAnalyzer_API;
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
		CapturePlot(QWidget *parent, bool isdBgraph = false, unsigned int xNumDivs = 10, unsigned int yNumDivs = 10,
			    PrefixFormatter* pfXaxis = nullptr, PrefixFormatter* pfYaxis = nullptr);
		~CapturePlot();

		void replot();

		HorizBar *levelTriggerA();
		HorizBar *levelTriggerB();

		QWidget *topArea();
		QWidget *topHandlesArea();

		void setBonusWidthForHistogram(int width);

		bool triggerAEnabled();
		bool triggerBEnabled();
		int selectedChannel();
		bool measurementsEnabled();

		void setOffsetWidgetVisible(int chnIdx, bool visible);
		void removeOffsetWidgets(int chnIdx);
		void removeLeftVertAxis(unsigned int axis);

		QList<Measure *>* getMeasurements();
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

		void setGraticuleEnabled(bool enabled);
		void setGatingEnabled(bool enabled);

		void computeMeasurementsForChannel(unsigned int chnIdx, unsigned int sampleRate);

		void setConversionFunction(const std::function<double(unsigned int, double, bool)> &fp);

		void enableXaxisLabels();
		void enableTimeTrigger(bool enable);
		QString getChannelName(int chIdx) const;
		void setChannelName(const QString &name, int chIdx);

		QString formatXValue(double value, int precision) const;
		QString formatYValue(double value, int precision) const;

		CursorReadouts * getCursorReadouts() const;

	Q_SIGNALS:
		void timeTriggerValueChanged(double);
		void channelOffsetChanged(unsigned int, double);
		void measurementsAvailable();
		void canvasSizeChanged();
		void leftGateChanged(double);
		void rightGateChanged(double);
		void channelSelected(int, bool);

	public Q_SLOTS:
		void setTriggerAEnabled(bool en);
		void setTriggerBEnabled(bool en);
		void setSelectedChannel(int id);
		void setMeasuremensEnabled(bool en);
		void setPeriodDetectLevel(int chnIdx, double lvl);
		void setPeriodDetectHyst(int chnIdx, double hyst);
		void setTimeBaseLabelValue(double timebase);
		void setBufferSizeLabelValue(int numSamples);
		void setSampleRatelabelValue(double sampleRate);
		void setTriggerState(int triggerState);
		void setMaxBufferSizeErrorLabel(bool reached, const QString &customWarning = "");

		void showEvent(QShowEvent *event);
		void printWithNoBackground(const QString& toolName = "", bool editScaleDraw = true);

		int getAnalogChannels() const;

		/* digital channels */
		void onDigitalChannelAdded(int chnIdx);
		void setChannelSelectable(int chnIdx, bool selectable);
		void removeDigitalPlotCurve(QwtPlotCurve *curve);
		void setOffsetHandleVisible(int chIdx, bool visible);

		/* channel group */
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
		void updateGateMargins();

	private Q_SLOTS:
		void onChannelAdded(int);
		void onNewDataReceived();

		void onGateBar1PixelPosChanged(int);
		void onGateBar2PixelPosChanged(int);

		void onGateBar1Moved(double);
		void onGateBar2Moved(double);

		void onTimeTriggerHandlePosChanged(int);
		void onTimeTriggerHandleGrabbed(bool);

		void onTriggerAHandleGrabbed(bool);
		void onTriggerBHandleGrabbed(bool);

		void handleInGroupChangedPosition(int position);

		void onHCursor1Moved(double);
		void onHCursor2Moved(double);
		void onVCursor1Moved(double);
		void onVCursor2Moved(double);

	private:
		std::function<double(unsigned int, double, bool)> m_conversion_function;

		bool d_triggerAEnabled;
		bool d_triggerBEnabled;
		bool d_measurementsEnabled;
		bool d_labelsEnabled;

		QWidget *d_topWidget;
		GateHandlesArea *d_topGateHandlesArea;
		int d_bonusWidth;

		QLabel *d_timeBaseLabel;
		QLabel *d_sampleRateLabel;
		QLabel *d_triggerStateLabel;
		QLabel *d_maxBufferError;

		int d_bufferSizeLabelVal;
		double d_sampleRateLabelVal;

		QList<HorizBar*> d_offsetBars;
		QList<RoundedHandleV*> d_offsetHandles;

		// Channel grouping
		QVector<QList<RoundedHandleV*>> d_groupHandles;
		bool d_startedGrouping;
		QVector<QwtPlotZoneItem *> d_groupMarkers;

		PlotGateHandle *d_hGatingHandle1;
		PlotGateHandle *d_hGatingHandle2;

		VertBar *d_gateBar1;
		VertBar *d_gateBar2;

		VertBar *d_timeTriggerBar;
		HorizBar *d_levelTriggerABar;
		HorizBar *d_levelTriggerBBar;
		FreePlotLineHandleH *d_timeTriggerHandle;
		FreePlotLineHandleV *d_levelTriggerAHandle;
		FreePlotLineHandleV *d_levelTriggerBHandle;

		MetricPrefixFormatter d_cursorMetricFormatter;
		TimePrefixFormatter d_cursorTimeFormatter;

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

		QwtPlotShapeItem *leftGate, *rightGate;
		QRectF leftGateRect, rightGateRect;
		bool d_gatingEnabled;

		QPair<double, double> d_xAxisInterval;
		int d_currentHandleInitPx;
		void pushBackNewOffsetWidgets(RoundedHandleV *chOffsetHdl, HorizBar *chOffsetBar);

		QVector<GenericLogicPlotCurve *> plot_logic_curves;
	};
}

#endif /* M2K_OSCILLOSCOPE_PLOT_H */
