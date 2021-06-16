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

#include "measure.hpp"

#include <qwt_plot_zoneitem.h>

#include <functional>
#include <scopy/gui/custom_plot_position_button.hpp>
#include <scopy/gui/graticule.hpp>
#include <scopy/gui/TimeDomainDisplayPlot.h>

class QLabel;

namespace scopy {
namespace gui {

class Oscilloscope_API;
class PlotWidget;

class OscilloscopePlot : public TimeDomainDisplayPlot
{
	Q_OBJECT

public:
	OscilloscopePlot(QWidget* parent, bool isdBgraph = false, unsigned int xNumDivs = 10,
			 unsigned int yNumDiv = 10);
	~OscilloscopePlot();
};

class CapturePlot : public OscilloscopePlot
{
	friend class Oscilloscope_API;
	friend class LogicAnalyzer_API;
	friend class Channel_API;

	Q_OBJECT

public:
	enum TriggerState
	{
		Waiting,
		Triggered,
		Stop,
		Auto,
	};

public:
	CapturePlot(QWidget* parent, bool isdBgraph = false, unsigned int xNumDivs = 10, unsigned int yNumDivs = 10);
	~CapturePlot();

	void replot();

	HorizBar* levelTriggerA();
	HorizBar* levelTriggerB();

	QWidget* topArea();
	QWidget* topHandlesArea();

	void setBonusWidthForHistogram(int width);

	bool triggerAEnabled();
	bool triggerBEnabled();
	int selectedChannel();
	bool measurementsEnabled();

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
	bool eventFilter(QObject*, QEvent*);
	void setActiveVertAxis(unsigned int axisIdx, bool selected = true);
	void showYAxisWidget(unsigned int axisIdx, bool en);
	void enableAxisLabels(bool enabled);

	void setDisplayScale(double value);

	void setTimeTriggerInterval(double min, double max);
	bool labelsEnabled();

	void setGraticuleEnabled(bool enabled);
	void setGatingEnabled(bool enabled);

	void computeMeasurementsForChannel(unsigned int chnIdx, unsigned int sampleRate);

	void setConversionFunction(const std::function<double(unsigned int, double, bool)>& fp);

	void enableXaxisLabels();
	void enableTimeTrigger(bool enable);
	QString getChannelName(int chIdx) const;
	void setChannelName(const QString& name, int chIdx);

	QString formatXValue(double value, int precision) const;
	QString formatYValue(double value, int precision) const;

	CursorReadouts* getCursorReadouts() const;

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
	void setMaxBufferSizeErrorLabel(bool reached, const QString& customWarning = "");

	void showEvent(QShowEvent* event);
	void printWithNoBackground(const QString& toolName = "", bool editScaleDraw = true);

	int getAnalogChannels() const;

	/* digital channels */
	void onDigitalChannelAdded(int chnIdx);
	void setChannelSelectable(int chnIdx, bool selectable);
	void removeDigitalPlotCurve(QwtPlotCurve* curve);
	void setOffsetHandleVisible(int chIdx, bool visible);

	/* channel group */
	void addToGroup(int currentGroup, int toAdd);
	void beginGroupSelection();
	bool endGroupSelection(bool moveAnnotationCurvesLast = false); // TODO: toggle group selection
	QVector<int> getGroupOfChannel(int chnIdx);
	QVector<QVector<int>> getAllGroups();
	void removeFromGroup(int chnIdx, int removedChnIdx, bool& didGroupVanish);
	void positionInGroupChanged(int chnIdx, int from, int to);
	void setGroups(const QVector<QVector<int>>& groups);

protected:
	virtual void cleanUpJustBeforeChannelRemoval(int chnIdx);

private:
	Measure* measureOfChannel(int chnIdx) const;
	void updateBufferSizeSampleRateLabel(int nsamples, double sr);
	void updateHandleAreaPadding(bool);
	void updateGateMargins();

	void pushBackNewOffsetWidgets(RoundedHandleV* chOffsetHdl, HorizBar* chOffsetBar);

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

	bool m_triggerAEnabled;
	bool m_triggerBEnabled;
	bool m_measurementsEnabled;
	bool m_labelsEnabled;

	QWidget* m_topWidget;
	GateHandlesArea* m_topGateHandlesArea;
	int m_bonusWidth;

	QLabel* m_timeBaseLabel;
	QLabel* m_sampleRateLabel;
	QLabel* m_triggerStateLabel;
	QLabel* m_maxBufferError;

	int m_bufferSizeLabelVal;
	double m_sampleRateLabelVal;

	QList<HorizBar*> m_offsetBars;
	QList<RoundedHandleV*> m_offsetHandles;

	// Channel grouping
	QVector<QList<RoundedHandleV*>> m_groupHandles;
	bool m_startedGrouping;
	QVector<QwtPlotZoneItem*> m_groupMarkers;

	PlotGateHandle* m_hGatingHandle1;
	PlotGateHandle* m_hGatingHandle2;

	VertBar* m_gateBar1;
	VertBar* m_gateBar2;

	VertBar* m_timeTriggerBar;
	HorizBar* m_levelTriggerABar;
	HorizBar* m_levelTriggerBBar;
	FreePlotLineHandleH* m_timeTriggerHandle;
	FreePlotLineHandleV* m_levelTriggerAHandle;
	FreePlotLineHandleV* m_levelTriggerBHandle;

	MetricPrefixFormatter m_cursorMetricFormatter;
	TimePrefixFormatter m_cursorTimeFormatter;

	QPen m_trigAactiveLinePen;
	QPen m_trigAinactiveLinePen;
	QPen m_trigBactiveLinePen;
	QPen m_trigBinactiveLinePen;
	QPen m_timeTriggerInactiveLinePen;
	QPen m_timeTriggerActiveLinePen;

	QList<Measure*> m_measureObjs;

	double m_valueV1, m_valueV2, m_valueH1, m_valueH2;
	double m_valueGateLeft, m_valueGateRight;
	double m_minOffsetValue, m_maxOffsetValue;
	double m_timeTriggerMinValue, m_timeTriggerMaxValue;

	bool m_displayGraticule;
	Graticule* m_graticule;

	QwtPlotShapeItem *m_leftGate, *m_rightGate;
	QRectF m_leftGateRect, m_rightGateRect;
	bool m_gatingEnabled;

	QPair<double, double> m_xAxisInterval;
	int m_currentHandleInitPx;
};
} // namespace gui
} // namespace scopy

#endif /* M2K_OSCILLOSCOPE_PLOT_H */
