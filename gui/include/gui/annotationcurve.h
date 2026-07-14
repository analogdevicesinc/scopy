/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 */

#ifndef SCOPY_GUI_ANNOTATIONCURVE_H
#define SCOPY_GUI_ANNOTATIONCURVE_H

#include "scopy-gui_export.h"

#include <QColor>
#include <QFontMetrics>
#include <QHash>
#include <QList>
#include <QString>
#include <QVector>

#include <QwtPlotItem>

namespace scopy {

class PlotAxis;
class PlotAxisHandle;

// Drawing-side annotation representation. Kept local to the gui module so
// this class does not depend on scopy-core (which links scopy-gui). Callers
// convert their domain-specific annotation type (e.g. scopy::acq::Annotation)
// into this struct.
struct SCOPY_GUI_EXPORT AnnotationSpan
{
	quint64 startSample = 0;
	quint64 endSample   = 0;   // == startSample → instant marker
	QString klass;             // row / class name (used for coloring & row bucket)
	QString text;              // label drawn inside the block if it fits
};

// Draws a set of decoder annotations on a QwtPlot as colored rounded-rect
// blocks stacked into rows. One instance per decoder; the vertical band is
// supplied externally: its top edge is a Y-axis scale value (so it follows
// zoom/pan of the main y-axis) and its height is a fixed pixel size.
//
// PulseView-style aggregation collapses annotations that would render into
// fewer than ~1 pixel into a single hatched block.
class SCOPY_GUI_EXPORT AnnotationCurve : public QwtPlotItem
{
public:
	AnnotationCurve(const QString &title, PlotAxis *xAxis, PlotAxis *yAxis,
			PlotAxisHandle *handle = nullptr);
	~AnnotationCurve() override;

	// Full-replace update. Rows are derived from AnnotationSpan::klass;
	// row order is preserved across updates (first-seen wins).
	void setAnnotations(const QVector<AnnotationSpan> &anns);
	void clear();

	// Optional per-class color override. Without an override, a stable
	// HSL hash of the class string is used.
	void setClassColor(const QString &klass, const QColor &c);

	// Total number of samples the annotations index into. If > 0,
	// annotations are laid out proportionally across the current plot
	// x-axis interval (whatever it currently is). If 0, startSample/
	// endSample are used verbatim as x-axis data values.
	void setSampleCount(quint64 n);

	// Band placement. Top edge is expressed in the y-axis' scale
	// coordinates so it moves with y-axis zoom/pan. Height is a fixed
	// canvas-pixel value so annotation rows stay legible independent of
	// the y-scale. The band grows downward from top.
	void setBandTopScale(double yScaleValue);
	void setBandHeightPx(double heightPx);

	// QwtPlotItem
	int  rtti() const override { return QwtPlotItem::Rtti_PlotUserItem + 42; }
	void draw(QPainter *painter, const QwtScaleMap &xMap,
		  const QwtScaleMap &yMap, const QRectF &canvasRect) const override;

	// Hit-test a canvas-pixel point against the currently laid out
	// annotations. Uses the exact same row geometry as draw(). Returns
	// nullptr if the point is outside this curve's band or not on any
	// visible annotation. The returned pointer is valid until the next
	// setAnnotations()/clear() call.
	const AnnotationSpan *hitTest(const QPointF &canvasPos,
				      const QwtScaleMap &xMap,
				      const QwtScaleMap &yMap,
				      const QRectF &canvasRect) const;

private:
	struct Row
	{
		QString      klass;
		QColor       color;
		QVector<int> indices; // into m_anns, sorted by startSample
	};

	QColor colorFor(const QString &klass) const;
	void   rebuildRows();

	void drawRow(QPainter *painter, const QwtScaleMap &xMap,
		     const QwtScaleMap &yMap, const QRectF &canvasRect,
		     const Row &row, double topPx, double bottomPx,
		     double annotLeft) const;

	void drawRowLabel(QPainter *painter, double leftPx,
			  double topPx, double bottomPx,
			  double labelBoxW, const QString &text) const;

	void drawAnnotation(QPainter *painter, const AnnotationSpan &ann,
			    const QColor &color, const QwtScaleMap &xMap,
			    double topPx, double bottomPx) const;

	void drawBlock(QPainter *painter, double startPx, double endPx,
		       const QColor &color, bool classUniform,
		       double topPx, double bottomPx) const;

	static QString elideText(const QString &text, double maxWidth,
				 const QFontMetrics &fm);

	PlotAxis *m_xAxis;
	PlotAxis *m_yAxis;
	PlotAxisHandle *m_handle;

	double sampleToX(quint64 sample, const QwtScaleMap &xMap) const;

	QString                 m_title;
	QVector<AnnotationSpan> m_anns;
	QList<Row>              m_rows;
	QHash<QString, int>     m_rowByClass;
	QHash<QString, QColor>  m_classColor;
	quint64                 m_sampleCount = 0;

	// Band geometry supplied by the caller (typically DecoderOverlay).
	// Default: place at y-scale 0 with a modest height so the item is
	// non-empty even if the caller forgets to configure it.
	double m_bandTopScale = 0.0;
	double m_bandHeightPx = 24.0;
};

} // namespace scopy

#endif // SCOPY_GUI_ANNOTATIONCURVE_H
