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

#include "annotationcurve.h"

#include "plotaxis.h"
#include "style.h"
#include "style_attributes.h"

#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QPen>
#include <QPolygonF>

#include <QwtPlot>
#include <QwtScaleMap>
#include <QwtText>

#include <algorithm>
#include <cmath>

namespace scopy {

namespace {

// Padding inside a rectangular annotation block before text (px).
constexpr double kTextPadding    = 6.0;
// Minimum ratio of block width the label + "..." needs, else no text drawn.
constexpr double kMinLabelRatio  = 0.35;
// Row spacing (vertical px between rows within the band). Tight packing.
constexpr double kRowSpacingPx   = 0.0;
// Max annotation-block pixel height regardless of band size.
constexpr double kMaxRowHeightPx = 24.0;
// Corner radius factor (r = height / kCornerRadiusDiv).
constexpr double kCornerRadiusDiv = 4.0;
// Chevron label geometry (per row).
constexpr double kLabelHPadPx    = 6.0;  // horizontal text padding inside label
constexpr double kLabelChevronPx = 6.0;  // width of the right-side ">" tip
constexpr double kLabelGapPx     = 2.0;  // gap after chevron before annotations

} // namespace

AnnotationCurve::AnnotationCurve(const QString &title, PlotAxis *xAxis, PlotAxis *yAxis)
	: QwtPlotItem(QwtText(title))
	, m_xAxis(xAxis)
	, m_yAxis(yAxis)
{
	setItemAttribute(QwtPlotItem::AutoScale, false);
	setItemAttribute(QwtPlotItem::Legend, false);
	setRenderHint(QwtPlotItem::RenderAntialiased, true);

	if(m_xAxis && m_yAxis)
		setAxes(m_xAxis->axisId(), m_yAxis->axisId());

	setZ(20.0); // above the grid, below markers/cursors
}

AnnotationCurve::~AnnotationCurve() = default;

void AnnotationCurve::setAnnotations(const QVector<AnnotationSpan> &anns)
{
	m_anns = anns;
	std::sort(m_anns.begin(), m_anns.end(),
		  [](const AnnotationSpan &a, const AnnotationSpan &b) {
			  return a.startSample < b.startSample;
		  });
	rebuildRows();
	itemChanged();
}

void AnnotationCurve::clear()
{
	m_anns.clear();
	m_rows.clear();
	m_rowByClass.clear();
	itemChanged();
}

void AnnotationCurve::setClassColor(const QString &klass, const QColor &c)
{
	m_classColor[klass] = c;
	auto it = m_rowByClass.find(klass);
	if(it != m_rowByClass.end())
		m_rows[it.value()].color = c;
	itemChanged();
}

void AnnotationCurve::setSampleCount(quint64 n)
{
	if(m_sampleCount == n)
		return;
	m_sampleCount = n;
	itemChanged();
}

double AnnotationCurve::sampleToX(quint64 sample, const QwtScaleMap &xMap) const
{
	if(m_sampleCount == 0)
		return static_cast<double>(sample);
	const double s1 = xMap.s1();
	const double s2 = xMap.s2();
	const double frac = std::min(1.0,
		static_cast<double>(sample) / static_cast<double>(m_sampleCount));
	return s1 + frac * (s2 - s1);
}

QColor AnnotationCurve::colorFor(const QString &klass) const
{
	auto it = m_classColor.constFind(klass);
	if(it != m_classColor.constEnd())
		return it.value();

	// Deterministic HSL hash. Same recipe as m2k AnnotationCurve
	// (annotationcurve.cpp:603-607) but keyed on the string.
	const int h = static_cast<int>(qHash(klass) % 360);
	QColor c;
	c.setHsl(h, 180, 170);
	return c;
}

void AnnotationCurve::rebuildRows()
{
	// Preserve existing row order; only append newly-seen classes.
	QHash<QString, int> newRowByClass = m_rowByClass;
	QList<Row>          newRows       = m_rows;
	for(Row &r : newRows)
		r.indices.clear();

	for(int i = 0; i < m_anns.size(); ++i) {
		const QString &klass = m_anns[i].klass;
		auto it = newRowByClass.find(klass);
		if(it == newRowByClass.end()) {
			Row r;
			r.klass = klass;
			r.color = colorFor(klass);
			newRows.append(r);
			it = newRowByClass.insert(klass, newRows.size() - 1);
		}
		newRows[it.value()].indices.append(i);
	}

	m_rowByClass = std::move(newRowByClass);
	m_rows       = std::move(newRows);
}

void AnnotationCurve::draw(QPainter *painter, const QwtScaleMap &xMap,
			   const QwtScaleMap &yMap, const QRectF &canvasRect) const
{
	if(m_rows.isEmpty())
		return;

	const int nRows = m_rows.size();

	// Compute vertical band from the yAxis interval.
	const double yTopVal    = m_yAxis ? m_yAxis->max() : yMap.s2();
	const double yBottomVal = m_yAxis ? m_yAxis->min() : yMap.s1();

	const double bandTopPx    = yMap.transform(yTopVal);
	const double bandBottomPx = yMap.transform(yBottomVal);
	const double bandHeightPx = bandBottomPx - bandTopPx;

	if(bandHeightPx <= 0)
		return;

	const double rowStrideHeightPx = bandHeightPx / nRows;
	const double rowHeightPx       = std::min(
		std::max(rowStrideHeightPx - kRowSpacingPx, 4.0), kMaxRowHeightPx);

	painter->save();
	painter->setClipRect(canvasRect);
	painter->setFont(QApplication::font());

	// Compute a single label-column width from the widest class name so all
	// row labels align vertically.
	const QFontMetrics fm = painter->fontMetrics();
	int maxTextW = 0;
	for(const Row &row : m_rows)
		maxTextW = std::max(maxTextW, fm.horizontalAdvance(row.klass));
	const double labelBoxW = maxTextW + kLabelHPadPx * 2 + kLabelChevronPx;
	const double annotLeft = canvasRect.left() + labelBoxW + kLabelGapPx;

	for(int r = 0; r < nRows; ++r) {
		const double rowCenterPx = bandTopPx + rowHeightPx * (r + 0.5);
		const double topPx       = rowCenterPx - rowHeightPx / 2.0;
		const double bottomPx    = rowCenterPx + rowHeightPx / 2.0;
		drawRow(painter, xMap, yMap, canvasRect, m_rows[r],
			topPx, bottomPx, annotLeft);
	}

	// Row labels: sharp-edged box with a chevron ">" right edge.
	for(int r = 0; r < nRows; ++r) {
		const QString &klass = m_rows[r].klass;
		if(klass.isEmpty())
			continue;
		const double rowCenterPx = bandTopPx + rowHeightPx * (r + 0.5);
		const double topPx       = rowCenterPx - rowHeightPx / 2.0;
		const double bottomPx    = rowCenterPx + rowHeightPx / 2.0;
		drawRowLabel(painter, canvasRect.left(), topPx, bottomPx,
			     labelBoxW, klass);
	}

	painter->restore();
}

const AnnotationSpan *AnnotationCurve::hitTest(const QPointF &canvasPos,
					       const QwtScaleMap &xMap,
					       const QwtScaleMap &yMap,
					       const QRectF &canvasRect) const
{
	if(m_rows.isEmpty() || m_anns.isEmpty())
		return nullptr;
	if(!canvasRect.contains(canvasPos))
		return nullptr;

	const int nRows = m_rows.size();
	const double yTopVal    = m_yAxis ? m_yAxis->max() : yMap.s2();
	const double yBottomVal = m_yAxis ? m_yAxis->min() : yMap.s1();
	const double bandTopPx    = yMap.transform(yTopVal);
	const double bandBottomPx = yMap.transform(yBottomVal);
	const double bandHeightPx = bandBottomPx - bandTopPx;
	if(bandHeightPx <= 0)
		return nullptr;

	const double rowStrideHeightPx = bandHeightPx / nRows;
	const double rowHeightPx       = std::min(
		std::max(rowStrideHeightPx - kRowSpacingPx, 4.0), kMaxRowHeightPx);

	// Find which row (if any) the y-coord falls into.
	const double y = canvasPos.y();
	if(y < bandTopPx || y > bandBottomPx)
		return nullptr;

	int rowIndex = -1;
	for(int r = 0; r < nRows; ++r) {
		const double rowCenterPx = bandTopPx + rowHeightPx * (r + 0.5);
		const double topPx    = rowCenterPx - rowHeightPx / 2.0;
		const double bottomPx = rowCenterPx + rowHeightPx / 2.0;
		if(y >= topPx && y <= bottomPx) {
			rowIndex = r;
			break;
		}
	}
	if(rowIndex < 0)
		return nullptr;

	const double x = canvasPos.x();
	const Row &row = m_rows[rowIndex];

	// Return the annotation whose pixel span contains x. For instant
	// markers (start==end) accept a small radius around the point.
	const double kInstantRadiusPx = std::max(3.0, rowHeightPx / 2.0);
	for(int idx : row.indices) {
		const AnnotationSpan &a = m_anns[idx];
		const double aStartPx = xMap.transform(sampleToX(a.startSample, xMap));
		const double aEndPx   = xMap.transform(sampleToX(a.endSample,   xMap));
		if(a.startSample == a.endSample) {
			if(std::abs(x - aStartPx) <= kInstantRadiusPx)
				return &a;
		} else {
			if(x >= aStartPx && x <= aEndPx)
				return &a;
		}
	}
	return nullptr;
}

void AnnotationCurve::drawRow(QPainter *painter, const QwtScaleMap &xMap,
			      const QwtScaleMap & /*yMap*/,
			      const QRectF &canvasRect, const Row &row,
			      double topPx, double bottomPx,
			      double annotLeft) const
{
	if(row.indices.isEmpty())
		return;

	// Reserve the label column at the canvas left; annotations must not
	// render underneath the label box.
	painter->save();
	painter->setClipRect(canvasRect);

	const QFontMetrics fm   = painter->fontMetrics();
	const double minLabelPx = fm.horizontalAdvance(QStringLiteral("XX"));

	// PulseView-style block aggregation
	// (see pv/views/trace/decodetrace.cpp:717-800):
	//   walk annotations in sample order; if consecutive labels would
	//   render less than a pixel apart, coalesce them into one block.
	double  blockStartPx = 0.0;
	double  prevEndPx    = -std::numeric_limits<double>::infinity();
	int     blockCount   = 0;
	QString blockKlass;
	bool    blockClassUniform = true;
	int     prevIdx = -1;

	auto flushBlock = [&]() {
		if(blockCount == 1) {
			// Single annotation → draw normally.
			drawAnnotation(painter, m_anns[prevIdx], row.color, xMap,
				       topPx, bottomPx);
		} else if(blockCount > 1) {
			const QColor c = blockClassUniform
				? colorFor(blockKlass)
				: Style::getColor(json::theme::content_silent);
			drawBlock(painter, blockStartPx, prevEndPx, c,
				  blockClassUniform, topPx, bottomPx);
		}
		blockCount = 0;
	};

	for(int idx : row.indices) {
		const AnnotationSpan &a = m_anns[idx];

		const double aStartPx = xMap.transform(sampleToX(a.startSample, xMap));
		const double aEndPx   = xMap.transform(sampleToX(a.endSample, xMap));
		const double widthPx  = aEndPx - aStartPx;
		const double deltaPx  = aEndPx - prevEndPx;

		// Off-screen quick reject.
		if(aEndPx < canvasRect.left() - 4.0 || aStartPx > canvasRect.right() + 4.0) {
			// Include in block so aggregation still spans it, but don't
			// waste time on solo text-fit checks.
			if(blockCount == 0) {
				blockStartPx      = aStartPx;
				blockKlass        = a.klass;
				blockClassUniform = true;
			} else if(a.klass != blockKlass) {
				blockClassUniform = false;
			}
			prevEndPx = aEndPx;
			prevIdx   = idx;
			++blockCount;
			continue;
		}

		// Does this annotation deserve to be drawn as a solo block?
		bool aIsSeparate = false;
		if(widthPx >= minLabelPx) {
			const double textW = fm.horizontalAdvance(a.text);
			if(textW + kTextPadding * 2 <= widthPx)
				aIsSeparate = true;
			else if(widthPx >= minLabelPx * 2) // wide enough for elided text
				aIsSeparate = true;
		}

		// Flush accumulated block when there's a visible gap or a solo block starts.
		if(std::abs(deltaPx) > 1.0 || aIsSeparate) {
			flushBlock();
		}

		if(aIsSeparate) {
			drawAnnotation(painter, a, row.color, xMap, topPx, bottomPx);
			prevEndPx = -std::numeric_limits<double>::infinity();
			blockCount = 0;
		} else {
			if(blockCount == 0) {
				blockStartPx      = aStartPx;
				blockKlass        = a.klass;
				blockClassUniform = true;
			} else if(a.klass != blockKlass) {
				blockClassUniform = false;
			}
			prevEndPx = aEndPx;
			prevIdx   = idx;
			++blockCount;
		}
	}

	flushBlock();
	painter->restore();
}

void AnnotationCurve::drawRowLabel(QPainter *painter, double leftPx,
				   double topPx, double bottomPx,
				   double labelBoxW, const QString &text) const
{
	// Sharp-edged box with a chevron ">" right edge. Uses default
	// application palette colors so the label is theme-agnostic.
	const double heightPx = bottomPx - topPx;
	const double bodyR    = leftPx + labelBoxW - kLabelChevronPx;
	const double tipX     = leftPx + labelBoxW;
	const double midY     = (topPx + bottomPx) / 2.0;

	QPolygonF poly;
	poly << QPointF(leftPx, topPx)
	     << QPointF(bodyR,  topPx)
	     << QPointF(tipX,   midY)
	     << QPointF(bodyR,  bottomPx)
	     << QPointF(leftPx, bottomPx);

	painter->save();
	painter->setPen(QPen(Style::getColor(json::theme::content_silent), 1.0));
	QColor bg = Style::getColor(json::theme::background_primary);
	bg.setAlphaF(0.55);
	painter->setBrush(bg);
	painter->drawPolygon(poly);

	painter->setPen(Style::getColor(json::theme::content_default));
	const QRectF textRect(leftPx + kLabelHPadPx,
			      topPx,
			      labelBoxW - kLabelHPadPx * 2 - kLabelChevronPx,
			      heightPx);
	painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
	painter->restore();
}

void AnnotationCurve::drawAnnotation(QPainter *painter, const AnnotationSpan &ann,
				     const QColor &color, const QwtScaleMap &xMap,
				     double topPx, double bottomPx) const
{
	const double startPx = xMap.transform(sampleToX(ann.startSample, xMap));
	const double endPx   = xMap.transform(sampleToX(ann.endSample, xMap));
	const double heightPx = bottomPx - topPx;
	const double centerY  = (topPx + bottomPx) / 2.0;

	painter->save();
	painter->setPen(QPen(color.darker(150), 1.0));
	painter->setBrush(color);

	if(ann.startSample == ann.endSample) {
		// Instant marker: circle centered on the sample.
		const double r = heightPx / 2.0;
		painter->drawEllipse(QPointF(startPx, centerY), r, r);

		// Optional label to the right if it fits.
		if(!ann.text.isEmpty()) {
			const QFontMetrics fm = painter->fontMetrics();
			const QRectF textRect(startPx + r + 2.0,
					      topPx,
					      fm.horizontalAdvance(ann.text) + 4.0,
					      heightPx);
			painter->setPen(Style::getColor(json::theme::content_inverse));
			painter->drawText(textRect, Qt::AlignCenter, ann.text);
		}
		painter->restore();
		return;
	}

	// Range: rounded rectangle.
	const double widthPx = endPx - startPx;
	const QRectF rect(startPx, topPx, widthPx, heightPx);
	const double radius = heightPx / kCornerRadiusDiv;
	painter->drawRoundedRect(rect, radius, radius);

	if(!ann.text.isEmpty()) {
		const QFontMetrics fm  = painter->fontMetrics();
		const double innerW    = widthPx - kTextPadding * 2;
		if(innerW >= fm.horizontalAdvance(QStringLiteral("X"))) {
			const QString shown = elideText(ann.text, innerW, fm);
			if(!shown.isEmpty()) {
				painter->setPen(Style::getColor(json::theme::content_inverse));
				painter->drawText(rect, Qt::AlignCenter, shown);
			}
		}
	}

	painter->restore();
}

void AnnotationCurve::drawBlock(QPainter *painter, double startPx, double endPx,
				const QColor &color, bool classUniform,
				double topPx, double bottomPx) const
{
	const double widthPx  = endPx - startPx;
	const double heightPx = bottomPx - topPx;

	painter->save();

	if(classUniform) {
		painter->setPen(QPen(color.darker(150), 1.0));
		painter->setBrush(QBrush(color, Qt::Dense4Pattern));
	} else {
		const QColor gray = Style::getColor(json::theme::content_silent);
		painter->setPen(QPen(gray.darker(150), 1.0));
		painter->setBrush(QBrush(gray, Qt::Dense4Pattern));
	}

	if(widthPx < 1.0) {
		painter->drawLine(QPointF(startPx, topPx), QPointF(startPx, bottomPx));
	} else {
		const QRectF rect(startPx, topPx, widthPx, heightPx);
		const double radius = heightPx / kCornerRadiusDiv;
		painter->drawRoundedRect(rect, radius, radius);
	}

	painter->restore();
}

QString AnnotationCurve::elideText(const QString &text, double maxWidth,
				   const QFontMetrics &fm)
{
	if(text.isEmpty())
		return QString();
	if(fm.horizontalAdvance(text) <= maxWidth)
		return text;

	static const QString kEllipsis = QStringLiteral("...");
	const double ellipsisW = fm.horizontalAdvance(kEllipsis);
	if(ellipsisW > maxWidth * (1.0 - kMinLabelRatio))
		return QString(); // no room even for ellipsis

	// Trim from the right until it fits with ellipsis appended.
	int lo = 0, hi = text.size();
	while(lo < hi) {
		int mid = (lo + hi + 1) / 2;
		const QString candidate = text.left(mid) + kEllipsis;
		if(fm.horizontalAdvance(candidate) <= maxWidth)
			lo = mid;
		else
			hi = mid - 1;
	}
	if(lo == 0)
		return QString();
	return text.left(lo) + kEllipsis;
}

} // namespace scopy
