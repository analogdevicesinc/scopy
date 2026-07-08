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
// blocks stacked into rows. One instance per decoder; each instance uses its
// own PlotAxis (yAxis) that defines the vertical band it occupies. All rows
// derived from AnnotationSpan::klass stack inside [yAxis->min(), yAxis->max()].
//
// PulseView-style aggregation collapses annotations that would render into
// fewer than ~1 pixel into a single hatched block.
class SCOPY_GUI_EXPORT AnnotationCurve : public QwtPlotItem
{
public:
	AnnotationCurve(const QString &title, PlotAxis *xAxis, PlotAxis *yAxis);
	~AnnotationCurve() override;

	// Full-replace update. Rows are derived from AnnotationSpan::klass;
	// row order is preserved across updates (first-seen wins).
	void setAnnotations(const QVector<AnnotationSpan> &anns);
	void clear();

	// Optional per-class color override. Without an override, a stable
	// HSL hash of the class string is used.
	void setClassColor(const QString &klass, const QColor &c);

	// Optional per-sample x-value lookup. If empty (default), annotation
	// startSample/endSample are used directly as x-axis data values (i.e.
	// the plot x-axis is assumed to be in sample-index units). If set,
	// xValues[i] gives the x-axis data value at sample index i, and
	// annotations are placed at xValues[startSample] .. xValues[endSample].
	// Out-of-range indices are clamped to xValues.first()/xValues.last().
	void setSampleXValues(const QVector<double> &xValues);

	// QwtPlotItem
	int  rtti() const override { return QwtPlotItem::Rtti_PlotUserItem + 42; }
	void draw(QPainter *painter, const QwtScaleMap &xMap,
		  const QwtScaleMap &yMap, const QRectF &canvasRect) const override;

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

	// Map a sample index to an x-axis data value. If m_sampleX is empty,
	// returns the sample index verbatim (legacy sample-index mode).
	double sampleToX(quint64 sample) const;

	QVector<AnnotationSpan> m_anns;   // flat list, sorted by startSample
	QList<Row>              m_rows;
	QHash<QString, int>     m_rowByClass;
	QHash<QString, QColor>  m_classColor;
	QVector<double>         m_sampleX; // optional per-sample x-value lookup
};

} // namespace scopy

#endif // SCOPY_GUI_ANNOTATIONCURVE_H
