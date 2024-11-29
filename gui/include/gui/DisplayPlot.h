/* -*- c++ -*- */
/*
 * Copyright 2008-2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
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

#ifndef M2K_DISPLAY_PLOT_H
#define M2K_DISPLAY_PLOT_H

#include "cursor_readouts.h"
#include "extendingplotzoomer.h"
#include "handles_area.hpp"
#include "osc_adjuster.hpp"
#include "plot_line_handle.h"
#include "plot_utils.hpp"
#include "plotpickerwrapper.h"
#include "printableplot.h"
#include "qwt_utils.h"
#include "scopy-gui_export.h"
#include "symbol_controller.h"

#include <QWidget>
#include <qwt_legend.h>
#include <qwt_painter.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_scaleitem.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>

#include <cmath>
#include <cstdio>
#include <stdint.h>
#include <vector>

typedef QList<QColor> QColorList;
Q_DECLARE_METATYPE(QColorList)

#if QWT_VERSION < 0x060100
#include <qwt_legend_item.h>
#else /* QWT_VERSION < 0x060100 */
#include <qwt_legend_data.h>
#include <qwt_legend_label.h>
#include <plotmagnifier.hpp>
#endif /* QWT_VERSION < 0x060100 */

namespace scopy {

class PlotAxisConfiguration;

class SCOPY_GUI_EXPORT ScaleDivisions : public QObject
{
	Q_OBJECT

public:
	ScaleDivisions()
		: m_divLowerLimit(1E-9)
		, m_divUpperLimit(1E9)
		, m_power(0.0)
		, m_magnitude_step(10.0)
		, m_templateSteps({1, 2, 5})
		, m_currentStep(m_templateSteps.begin())
	{
		updateDivision();
	}

	ScaleDivisions(const std::vector<double> &steps)
		: m_divLowerLimit(1E-9)
		, m_divUpperLimit(1E9)
		, m_power(0.0)
		, m_magnitude_step(10.0)
		, m_templateSteps(steps)
		, m_currentStep(m_templateSteps.begin())
	{
		updateDivision();
	}

	~ScaleDivisions() {}

	void setDivisioList(const std::vector<double> &newSteps)
	{
		m_templateSteps = newSteps;
		m_currentStep = m_templateSteps.begin();
		updateDivision();
	}

	double divisionValue() { return m_currentDiv; }

	void setMagnitudeStep(double newStep) { m_magnitude_step = newStep; }

	double magnitudeStep() { return m_magnitude_step; }

	void setLowerLimit(double limit) { m_divLowerLimit = limit; }

	double lowerLimit() { return m_divLowerLimit; }

	void setUpperLimit(double limit) { m_divUpperLimit = limit; }

	double upperLimit() { return m_divUpperLimit; }

Q_SIGNALS:
	void divisionChanged(double division);

public Q_SLOTS:
	void moveToNextDivision()
	{
		std::vector<double>::iterator templateStep;
		int power = m_power;

		if(m_currentStep < m_templateSteps.end() - 1) {
			templateStep = m_currentStep + 1;
		} else {
			templateStep = m_templateSteps.begin();
			power++;
		}
		if(division(templateStep, power) <= m_divUpperLimit) {
			m_currentStep = templateStep;
			m_power = power;
			updateDivision();
		}
	}

	void moveToPreviousDivision()
	{
		std::vector<double>::iterator templateStep;
		int power = m_power;

		if(m_currentStep > m_templateSteps.begin()) {
			templateStep = m_currentStep - 1;
		} else {
			templateStep = m_templateSteps.end() - 1;
			power--;
		}
		if(division(templateStep, power) >= m_divLowerLimit) {
			m_currentStep = templateStep;
			m_power = power;
			updateDivision();
		}
	}

private:
	double m_currentDiv;
	double m_divLowerLimit;
	double m_divUpperLimit;
	int m_power;
	double m_magnitude_step;
	std::vector<double> m_templateSteps;
	std::vector<double>::iterator m_currentStep;

	void updateDivision()
	{
		m_currentDiv = (*m_currentStep) * pow(m_magnitude_step, m_power);
		Q_EMIT divisionChanged(m_currentDiv);
	}

	double division(std::vector<double>::iterator step, int power)
	{
		return ((*step) * pow(m_magnitude_step, power));
	}
};

struct cursorReadoutsText
{
	QString t1;
	QString t2;
	QString tDelta;
	QString freq;
	QString v1;
	QString v2;
	QString vDelta;
};

/*!
 * \brief QWidget base plot to build QTGUI plotting tools.
 * \ingroup qtgui_blk
 */
class SCOPY_GUI_EXPORT DisplayPlot : public PrintablePlot
{
	Q_OBJECT

	Q_PROPERTY(QColor line_color1 READ getLineColor1 WRITE setLineColor1)
	Q_PROPERTY(QColor line_color2 READ getLineColor2 WRITE setLineColor2)
	Q_PROPERTY(QColor line_color3 READ getLineColor3 WRITE setLineColor3)
	Q_PROPERTY(QColor line_color4 READ getLineColor4 WRITE setLineColor4)
	Q_PROPERTY(QColor line_color5 READ getLineColor5 WRITE setLineColor5)
	Q_PROPERTY(QColor line_color6 READ getLineColor6 WRITE setLineColor6)
	Q_PROPERTY(QColor line_color7 READ getLineColor7 WRITE setLineColor7)
	Q_PROPERTY(QColor line_color8 READ getLineColor8 WRITE setLineColor8)
	Q_PROPERTY(QColor line_color9 READ getLineColor9 WRITE setLineColor9)

	Q_PROPERTY(int line_width1 READ getLineWidth1 WRITE setLineWidth1)
	Q_PROPERTY(int line_width2 READ getLineWidth2 WRITE setLineWidth2)
	Q_PROPERTY(int line_width3 READ getLineWidth3 WRITE setLineWidth3)
	Q_PROPERTY(int line_width4 READ getLineWidth4 WRITE setLineWidth4)
	Q_PROPERTY(int line_width5 READ getLineWidth5 WRITE setLineWidth5)
	Q_PROPERTY(int line_width6 READ getLineWidth6 WRITE setLineWidth6)
	Q_PROPERTY(int line_width7 READ getLineWidth7 WRITE setLineWidth7)
	Q_PROPERTY(int line_width8 READ getLineWidth8 WRITE setLineWidth8)
	Q_PROPERTY(int line_width9 READ getLineWidth9 WRITE setLineWidth9)

	Q_PROPERTY(Qt::PenStyle line_style1 READ getLineStyle1 WRITE setLineStyle1)
	Q_PROPERTY(Qt::PenStyle line_style2 READ getLineStyle2 WRITE setLineStyle2)
	Q_PROPERTY(Qt::PenStyle line_style3 READ getLineStyle3 WRITE setLineStyle3)
	Q_PROPERTY(Qt::PenStyle line_style4 READ getLineStyle4 WRITE setLineStyle4)
	Q_PROPERTY(Qt::PenStyle line_style5 READ getLineStyle5 WRITE setLineStyle5)
	Q_PROPERTY(Qt::PenStyle line_style6 READ getLineStyle6 WRITE setLineStyle6)
	Q_PROPERTY(Qt::PenStyle line_style7 READ getLineStyle7 WRITE setLineStyle7)
	Q_PROPERTY(Qt::PenStyle line_style8 READ getLineStyle8 WRITE setLineStyle8)
	Q_PROPERTY(Qt::PenStyle line_style9 READ getLineStyle9 WRITE setLineStyle9)

	typedef QwtSymbol::Style QwtSymbolStyle;

	Q_ENUMS(QwtSymbolStyle)
	Q_PROPERTY(QwtSymbolStyle line_marker1 READ getLineMarker1 WRITE setLineMarker1)
	Q_PROPERTY(QwtSymbolStyle line_marker2 READ getLineMarker2 WRITE setLineMarker2)
	Q_PROPERTY(QwtSymbolStyle line_marker3 READ getLineMarker3 WRITE setLineMarker3)
	Q_PROPERTY(QwtSymbolStyle line_marker4 READ getLineMarker4 WRITE setLineMarker4)
	Q_PROPERTY(QwtSymbolStyle line_marker5 READ getLineMarker5 WRITE setLineMarker5)
	Q_PROPERTY(QwtSymbolStyle line_marker6 READ getLineMarker6 WRITE setLineMarker6)
	Q_PROPERTY(QwtSymbolStyle line_marker7 READ getLineMarker7 WRITE setLineMarker7)
	Q_PROPERTY(QwtSymbolStyle line_marker8 READ getLineMarker8 WRITE setLineMarker8)
	Q_PROPERTY(QwtSymbolStyle line_marker9 READ getLineMarker9 WRITE setLineMarker9)

	Q_PROPERTY(int marker_alpha1 READ getMarkerAlpha1 WRITE setMarkerAlpha1)
	Q_PROPERTY(int marker_alpha2 READ getMarkerAlpha2 WRITE setMarkerAlpha2)
	Q_PROPERTY(int marker_alpha3 READ getMarkerAlpha3 WRITE setMarkerAlpha3)
	Q_PROPERTY(int marker_alpha4 READ getMarkerAlpha4 WRITE setMarkerAlpha4)
	Q_PROPERTY(int marker_alpha5 READ getMarkerAlpha5 WRITE setMarkerAlpha5)
	Q_PROPERTY(int marker_alpha6 READ getMarkerAlpha6 WRITE setMarkerAlpha6)
	Q_PROPERTY(int marker_alpha7 READ getMarkerAlpha7 WRITE setMarkerAlpha7)
	Q_PROPERTY(int marker_alpha8 READ getMarkerAlpha8 WRITE setMarkerAlpha8)
	Q_PROPERTY(int marker_alpha9 READ getMarkerAlpha9 WRITE setMarkerAlpha9)

	Q_PROPERTY(QColor zoomer_color READ getZoomerColor WRITE setZoomerColor)
	Q_PROPERTY(QColor palette_color READ getPaletteColor WRITE setPaletteColor)
	Q_PROPERTY(int yaxis_label_font_size READ getYaxisLabelFontSize WRITE setYaxisLabelFontSize)
	Q_PROPERTY(int xaxis_label_font_size READ getXaxisLabelFontSize WRITE setXaxisLabelFontSize)
	Q_PROPERTY(int axes_label_font_size READ getAxesLabelFontSize WRITE setAxesLabelFontSize)

public:
	DisplayPlot(int nplots, QWidget *, bool isdBgraph = false, unsigned int xNumDivs = 10,
		    unsigned int yNumDivs = 10, int qwtAxis = QwtAxis::YLeft);
	virtual ~DisplayPlot();

	virtual void replot() = 0;

	const QColor getLineColor1() const;
	const QColor getLineColor2() const;
	const QColor getLineColor3() const;
	const QColor getLineColor4() const;
	const QColor getLineColor5() const;
	const QColor getLineColor6() const;
	const QColor getLineColor7() const;
	const QColor getLineColor8() const;
	const QColor getLineColor9() const;

	int getLineWidth1() const;
	int getLineWidth2() const;
	int getLineWidth3() const;
	int getLineWidth4() const;
	int getLineWidth5() const;
	int getLineWidth6() const;
	int getLineWidth7() const;
	int getLineWidth8() const;
	int getLineWidth9() const;

	const Qt::PenStyle getLineStyle1() const;
	const Qt::PenStyle getLineStyle2() const;
	const Qt::PenStyle getLineStyle3() const;
	const Qt::PenStyle getLineStyle4() const;
	const Qt::PenStyle getLineStyle5() const;
	const Qt::PenStyle getLineStyle6() const;
	const Qt::PenStyle getLineStyle7() const;
	const Qt::PenStyle getLineStyle8() const;
	const Qt::PenStyle getLineStyle9() const;

	const QwtSymbol::Style getLineMarker1() const;
	const QwtSymbol::Style getLineMarker2() const;
	const QwtSymbol::Style getLineMarker3() const;
	const QwtSymbol::Style getLineMarker4() const;
	const QwtSymbol::Style getLineMarker5() const;
	const QwtSymbol::Style getLineMarker6() const;
	const QwtSymbol::Style getLineMarker7() const;
	const QwtSymbol::Style getLineMarker8() const;
	const QwtSymbol::Style getLineMarker9() const;

	int getMarkerAlpha1() const;
	int getMarkerAlpha2() const;
	int getMarkerAlpha3() const;
	int getMarkerAlpha4() const;
	int getMarkerAlpha5() const;
	int getMarkerAlpha6() const;
	int getMarkerAlpha7() const;
	int getMarkerAlpha8() const;
	int getMarkerAlpha9() const;

	QColor getZoomerColor() const;
	QColor getPaletteColor() const;
	int getAxisLabelFontSize(int axisId) const;
	int getYaxisLabelFontSize() const;
	int getXaxisLabelFontSize() const;
	int getAxesLabelFontSize() const;

	void DetachCurve(unsigned int curveIdx);
	void AttachCurve(unsigned int curveIdx);
	QwtPlotCurve *Curve(unsigned int curveIdx);

	void zoomBaseUpdate(bool force = false);

	void setMinXaxisDivision(double minDivison);
	double minXaxisDivision();
	void setMaxXaxisDivision(double maxDivison);
	double maxXaxisDivision();

	void setMinYaxisDivision(double minDivison);
	double minYaxisDivision();
	void setMaxYaxisDivision(double maxDivison);
	double maxYaxisDivision();

	void setLeftVertAxesCount(int count);
	void removeLeftVertAxis(unsigned int axis);
	int leftVertAxesCount();

	void setUsingLeftAxisScales(bool on);
	bool usingLeftAxisScales();

	// Make sure to create your won PlotNewData method in the derived
	// class:
	// void PlotNewData(...);
	QwtPlotZoomer *getZoomer() const;
	void setZoomerParams(bool bounded, int maxStackDepth);

	void bringCurveToFront(unsigned int curveIdx);
	void enableColoredLabels(bool colored);

	void enableMouseGesturesOnScales(bool enable);

	void setDisplayScale(double value);
	void setAllYAxis(double min, double max);

	HorizHandlesArea *bottomHandlesArea();
	QWidget *rightHandlesArea();
	QWidget *leftHandlesArea();
	virtual QWidget *topHandlesArea();
	VertBar *vBar1();
	VertBar *vBar2();
	bool isLogaritmicPlot() const;
	void setPlotLogaritmic(bool);
	bool isLogaritmicYPlot() const;
	void setPlotYLogaritmic(bool value);
	void setXaxisMajorTicksPos(QList<double>);
	QList<double> getXaxisMajorTicksPos() const;
	void setYaxisMajorTicksPos(QList<double>);
	QList<double> getYaxisMajorTicksPos() const;
	QWidget *getPlotwithElements();

	bool vertCursorsEnabled();
	bool horizCursorsEnabled();
	struct cursorReadoutsText allCursorReadouts() const;
	void trackModeEnabled(bool enabled);
	void repositionCursors();
	void toggleCursors(bool en);
	virtual QString formatXValue(double value, int precision) const;
	virtual QString formatYValue(double value, int precision) const;

	void setCursorAxes(QwtAxisId fixed_axis, QwtAxisId mobile_axis);
	CursorReadouts *getCursorReadouts() const;

	scopy::PlotMagnifier *getMagnifier();
	QVector<scopy::PlotMagnifier *> getMagnifierList();

	VertBar *getVBar1() const;
	VertBar *getVBar2() const;
	HorizBar *getHBar1() const;
	HorizBar *getHBar2() const;

	void adjustHandleAreasSize(bool cursors = false);

public Q_SLOTS:
	virtual void disableLegend();
	virtual void setYaxis(double min, double max);
	virtual void setXaxis(double min, double max);
	virtual void setLineLabel(int which, QString label);
	virtual QString getLineLabel(int which);
	virtual void setLineColor(int which, QColor color);
	void setLineColor(int chnIdx, int colorIdx);
	virtual QColor getLineColor(int which) const;
	virtual void setLineWidth(int which, qreal width);
	virtual int getLineWidth(int which) const;
	virtual void setLineStyle(int which, Qt::PenStyle style);
	virtual const Qt::PenStyle getLineStyle(int which) const;
	virtual void setLineMarker(int which, QwtSymbol::Style marker);
	virtual const QwtSymbol::Style getLineMarker(int which) const;
	virtual void setMarkerAlpha(int which, int alpha);
	virtual int getMarkerAlpha(int which) const;

	virtual void horizAxisScaleIncrease();
	virtual void horizAxisScaleDecrease();
	virtual void vertAxisScaleIncrease();
	virtual void vertAxisScaleDecrease();

	void setActiveVertAxis(unsigned int axisIdx, bool selected = true);
	int activeVertAxis();

	void setVertOffset(double offset, int axisIdx = 0);
	double VertOffset(int axisIdx = 0);
	void setHorizOffset(double offset);
	double HorizOffset();
	void setVertUnitsPerDiv(double upd, int axisIdx = 0);
	double VertUnitsPerDiv(int axisIdx = 0);
	void setHorizUnitsPerDiv(double upd);
	double HorizUnitsPerDiv();

	// Need a function for each curve for setting via stylesheet.
	// Can't use preprocessor directives because we're inside a Q_OBJECT.
	void setLineColor1(QColor);
	void setLineColor2(QColor);
	void setLineColor3(QColor);
	void setLineColor4(QColor);
	void setLineColor5(QColor);
	void setLineColor6(QColor);
	void setLineColor7(QColor);
	void setLineColor8(QColor);
	void setLineColor9(QColor);

	void setLineWidth1(int);
	void setLineWidth2(int);
	void setLineWidth3(int);
	void setLineWidth4(int);
	void setLineWidth5(int);
	void setLineWidth6(int);
	void setLineWidth7(int);
	void setLineWidth8(int);
	void setLineWidth9(int);

	void setLineStyle1(Qt::PenStyle);
	void setLineStyle2(Qt::PenStyle);
	void setLineStyle3(Qt::PenStyle);
	void setLineStyle4(Qt::PenStyle);
	void setLineStyle5(Qt::PenStyle);
	void setLineStyle6(Qt::PenStyle);
	void setLineStyle7(Qt::PenStyle);
	void setLineStyle8(Qt::PenStyle);
	void setLineStyle9(Qt::PenStyle);

	void setLineMarker1(QwtSymbol::Style);
	void setLineMarker2(QwtSymbol::Style);
	void setLineMarker3(QwtSymbol::Style);
	void setLineMarker4(QwtSymbol::Style);
	void setLineMarker5(QwtSymbol::Style);
	void setLineMarker6(QwtSymbol::Style);
	void setLineMarker7(QwtSymbol::Style);
	void setLineMarker8(QwtSymbol::Style);
	void setLineMarker9(QwtSymbol::Style);

	void setMarkerAlpha1(int);
	void setMarkerAlpha2(int);
	void setMarkerAlpha3(int);
	void setMarkerAlpha4(int);
	void setMarkerAlpha5(int);
	void setMarkerAlpha6(int);
	void setMarkerAlpha7(int);
	void setMarkerAlpha8(int);
	void setMarkerAlpha9(int);

	void printWithNoBackground(const QString &toolName = "", bool editScaleDraw = true);

	void setZoomerColor(QColor c);
	void setPaletteColor(QColor c);
	void setAxisLabelFontSize(int axisId, int fs);
	void setYaxisLabelFontSize(int fs);
	void setXaxisLabelFontSize(int fs);
	void setAxesLabelFontSize(int fs);
	void setXaxisMouseGesturesEnabled(bool en);
	void setYaxisMouseGesturesEnabled(int axisId, bool en);

	void setStop(bool on);

	void resizeSlot(QSize *s);

	// Because of the preprocessing of slots in QT, these are not
	// easily separated by the version check. Make one for each
	// version until it's worked out.
	void onPickerPointSelected(const QPointF &p);
	void onPickerPointSelected6(const QPointF &p);

	unsigned int xAxisNumDiv() const;
	unsigned int yAxisNumDiv() const;

	void setVertCursorsEnabled(bool en);
	void setHorizCursorsEnabled(bool en);
	void setVertCursorsHandleEnabled(bool en);
	void setCursorReadoutsVisible(bool en);
	void setHorizCursorsLocked(bool value);
	void setVertCursorsLocked(bool value);

	void setCursorReadoutsTransparency(int value);
	void moveCursorReadouts(CustomPlotPositionButton::ReadoutsPosition position);

	void onPlotMagnified();

Q_SIGNALS:
	void horizScaleDivisionChanged(double);
	void vertScaleDivisionChanged(double);
	void horizScaleOffsetChanged(double);
	void vertScaleOffsetChanged(double);

	void plotPointSelected(const QPointF p);

	// signals that the plot size changed
	void plotSizeChanged();

	void cursorReadoutsChanged(struct cursorReadoutsText);

private Q_SLOTS:
	void onHbar1PixelPosChanged(int);
	void onHbar2PixelPosChanged(int);
	void onVbar1PixelPosChanged(int);
	void onVbar2PixelPosChanged(int);

	void onHorizCursorHandle1Changed(int value);
	void onVertCursorHandle1Changed(int value);
	void onVertCursorHandle2Changed(int value);
	void onHorizCursorHandle2Changed(int value);

protected Q_SLOTS:
	virtual void legendEntryChecked(QwtPlotItem *plotItem, bool on);
	virtual void legendEntryChecked(const QVariant &plotItem, bool on, int index);

	void onHorizAxisOffsetDecrease();
	void onHorizAxisOffsetIncrease();
	void onVertAxisOffsetDecrease();
	void onVertAxisOffsetIncrease();

	virtual void onHCursor1Moved(double);
	virtual void onHCursor2Moved(double);
	virtual void onVCursor1Moved(double);
	virtual void onVCursor2Moved(double);

	void _onXbottomAxisWidgetScaleDivChanged();
	void _onYleftAxisWidgetScaleDivChanged();

#ifdef __ANDROID__
	void mousePressEvent(QMouseEvent *event);
#endif

public:
	PlotLineHandleV *d_vCursorHandle1;
	PlotLineHandleV *d_vCursorHandle2;
	PlotLineHandleH *d_hCursorHandle1;
	PlotLineHandleH *d_hCursorHandle2;

protected:
	enum PlotMarker
	{
		V1Marker = 0,
		V2Marker,
		H1Marker,
		H2Marker
	};

	int m_qwtYAxis;
	int d_nplots;
	std::vector<QwtPlotCurve *> d_plot_curve;

	QString d_yAxisUnit;
	QString d_xAxisUnit;

	QwtPlotPanner *d_panner;
	QVector<QwtPlotZoomer *> d_zoomer;
	QVector<scopy::PlotMagnifier *> d_magnifier;
	QwtPlotGrid *d_grid;

	QwtDblClickPlotPicker *d_picker;

	std::vector<PlotAxisConfiguration *> vertAxes;
	PlotAxisConfiguration *horizAxis;

	QVector<QwtPlotScaleItem *> scaleItems;

	int64_t d_numPoints;

	bool d_stop;

	double d_displayScale;

	QList<QColor> d_CurveColors;
	QList<QColor> d_printColors;

	bool d_autoscale_state;
	int d_activeVertAxis;

	double d_xAxisMin;
	double d_xAxisMax;
	double d_yAxisMin;
	double d_yAxisMax;
	unsigned int d_xAxisNumDiv;
	unsigned int d_yAxisNumDiv;
	bool d_usingLeftAxisScales;

	NumberSeries d_hScaleDivisions;
	NumberSeries d_vScaleDivisions;

	void setXaxisNumDiv(unsigned int);
	void setYaxisNumDiv(unsigned int);
	void bottomHorizAxisInit();
	virtual void configureAxis(int axisPos, int axisIdx);

	void resizeEvent(QResizeEvent *event);

	HorizHandlesArea *d_bottomHandlesArea;
	VertHandlesArea *d_rightHandlesArea;
	VertHandlesArea *d_leftHandlesArea;
	HorizHandlesArea *d_topHandlesArea;

	VertBar *d_vBar1;
	VertBar *d_vBar2;
	HorizBar *d_hBar1;
	HorizBar *d_hBar2;
	SymbolController *d_symbolCtrl;

	struct cursorReadoutsText d_cursorReadoutsText;
	CursorReadouts *d_cursorReadouts;

	bool d_trackMode;
	int d_selected_channel;
	bool d_cursorsEnabled;

	QwtPlotMarker *markerIntersection1;
	QwtPlotMarker *markerIntersection2;

	double getHorizontalCursorIntersection(double time);

private:
	void AddAxisOffset(int axisPos, int axisIdx, double offset);
	bool d_coloredLabels;
	bool d_mouseGesturesEnabled;

	bool d_vertCursorsHandleEnabled;
	bool d_vertCursorsEnabled;
	bool d_horizCursorsEnabled;
	bool horizCursorsLocked;
	bool vertCursorsLocked;

	int pixelPosHandleHoriz1;
	int pixelPosHandleHoriz2;
	int pixelPosHandleVert1;
	int pixelPosHandleVert2;

	bool d_isLogaritmicPlot;
	bool d_isLogaritmicYPlot;
	QList<double> d_majorTicks;
	QList<double> d_majorTicksY;
	bool d_cursorReadoutsVisible;
	PrefixFormatter *d_formatter;

	void setupCursors();
	void setupReadouts();
	void displayIntersection();
	void setupDisplayPlotDiv(bool isdBgraph);
};

/*
 * OscScaleDraw class overrides the way the major values are being displayed.
 */
class SCOPY_GUI_EXPORT OscScaleDraw : public QwtScaleDraw
{
public:
	OscScaleDraw(const QString &unit_type = "");
	OscScaleDraw(PrefixFormatter *, const QString &);
	QwtText label(double) const;

	void setFloatPrecision(unsigned int numDigits);
	unsigned int getFloatPrecison() const;

	void setUnitType(const QString &unit);
	QString getUnitType() const;

	void setColor(QColor color);

	void setDisplayScale(double value);
	void setFormatter(PrefixFormatter *formatter);

	void enableDeltaLabel(bool enable);
	void setUnitsEnabled(bool enable);

protected:
	virtual void draw(QPainter *, const QPalette &) const;

private:
	int m_floatPrecision;
	QString m_unit;
	PrefixFormatter *m_formatter;
	QColor m_color;
	double m_displayScale;
	mutable unsigned int m_nrTicks;
	mutable bool m_shouldDrawMiddleDelta;
	bool m_delta;
	bool m_unitsEn;
};

class SCOPY_GUI_EXPORT OscPlotZoomer : public ExtendingPlotZoomer
{
	Q_OBJECT
public:
	OscPlotZoomer(QwtAxisId xAxis, QwtAxisId yAxis, QWidget *parent, bool doReplot);
	OscPlotZoomer(QWidget *, bool doReplot = true);

	void cancel() { reset(); }

Q_SIGNALS:
	void zoomIn();
	void zoomOut();
	void zoomFinished(bool);

protected:
	virtual void rescale();

private:
	int lastIndex;
};

/*
 * PlotAxisConfiguration class holds a group of settings of an axis
 */
class SCOPY_GUI_EXPORT PlotAxisConfiguration
{
public:
	PlotAxisConfiguration(int axisPos, int axisIdx, DisplayPlot *plot);
	~PlotAxisConfiguration();

	QwtAxisId &axis();

	void setPtsPerDiv(double value);
	double ptsPerDiv();

	void setOffset(double value);
	double offset();

	void setCursorShapeOnHover(Qt::CursorShape shape);

	void setMouseGesturesEnabled(bool en);

private:
	QwtAxisId d_axis;
	DisplayPlot *d_plot;

	Qt::CursorShape d_hoverCursorShape;

	OscAdjuster *d_mouseGestures;

	double d_ptsPerDiv;
	double d_offset;
};

/*
 * EdgelessPlotScaleItem class ensures that the first and last major ticks are ignored
 */
class SCOPY_GUI_EXPORT EdgelessPlotScaleItem : public QwtPlotScaleItem
{
public:
	explicit EdgelessPlotScaleItem(QwtScaleDraw::Alignment = QwtScaleDraw::BottomScale, const double pos = 0.0);
	virtual void updateScaleDiv(const QwtScaleDiv &, const QwtScaleDiv &);
};

/*
 * EdgelessPlotGrid class ensures that the first and last major ticks are ignored
 */
class SCOPY_GUI_EXPORT EdgelessPlotGrid : public QwtPlotGrid
{
public:
	explicit EdgelessPlotGrid();
	virtual void updateScaleDiv(const QwtScaleDiv &, const QwtScaleDiv &);
};

} // namespace scopy

#endif /* M2K_DISPLAY_PLOT_H */
