#include "DigitalCurveItem.h"

#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QPen>
#include <QPolygonF>

#include <QwtScaleMap>
#include <QwtText>

#include <algorithm>

namespace scopy {
namespace adc {

namespace {
// Match AnnotationCurve's kMaxRowHeightPx (gui/src/annotationcurve.cpp:52)
// so digital rows and annotation rows stack visually at the same pitch.
constexpr double kRowHeightPx   = 24.0;
constexpr double kLabelHPadPx   = 6.0;
constexpr double kLabelChevronPx= 6.0; // width of the right-side ">" tip
constexpr double kLabelGapPx    = 2.0;
constexpr double kBaselinePadPx = 3.0; // inset so 0/1 lines don't touch band edges
constexpr qreal  kLabelBgAlpha  = 0.55;
} // namespace

DigitalCurveItem::DigitalCurveItem(const QString &title, PlotAxis *xAxis,
                                   PlotAxis *yAxis, PlotAxisHandle *handle)
	: QwtPlotItem(QwtText(title))
	, m_xAxis(xAxis)
	, m_yAxis(yAxis)
	, m_handle(handle)
	, m_title(title)
	, m_color(0x4a, 0xb8, 0xff)
{
	setItemAttribute(QwtPlotItem::AutoScale, false);
	setItemAttribute(QwtPlotItem::Legend, false);
	setRenderHint(QwtPlotItem::RenderAntialiased, false);

	if(m_xAxis && m_yAxis)
		setAxes(m_xAxis->axisId(), m_yAxis->axisId());

	setZ(20.0);
}

DigitalCurveItem::~DigitalCurveItem() = default;

void DigitalCurveItem::setSamples(const QVector<quint8> &samples)
{
	m_samples = samples;
	itemChanged();
}

void DigitalCurveItem::clear()
{
	m_samples.clear();
	itemChanged();
}

void DigitalCurveItem::setSampleCount(quint64 n)
{
	if(m_sampleCount == n) return;
	m_sampleCount = n;
	itemChanged();
}

void DigitalCurveItem::setColor(const QColor &c)
{
	if(m_color == c) return;
	m_color = c;
	itemChanged();
}

void DigitalCurveItem::draw(QPainter *painter, const QwtScaleMap &xMap,
                            const QwtScaleMap &yMap,
                            const QRectF &canvasRect) const
{
	if(!m_handle) return;

	const double bandTopScale = m_handle->getPosition();
	const double bandTopPx    = yMap.transform(bandTopScale);
	const double bandBottomPx = bandTopPx + kRowHeightPx;

	painter->save();
	painter->setClipRect(canvasRect);
	painter->setFont(QApplication::font());

	// Label geometry (computed here but drawn LAST so the waveform stays
	// underneath, matching AnnotationCurve which draws its row label on
	// top of the annotations). Label is anchored to the RIGHT edge of the
	// canvas with a left-pointing "<" chevron.
	const QFontMetrics fm = painter->fontMetrics();
	const double labelBoxW = std::max(40.0,
		fm.horizontalAdvance(m_title) + kLabelHPadPx * 2.0 + kLabelChevronPx);
	const double leftPx   = canvasRect.right() - labelBoxW;
	const double topPx    = bandTopPx;
	const double bottomPx = bandTopPx + kRowHeightPx;

	// The 0-level sits near the bottom of the band, the 1-level near the
	// top, with a small pixel inset so lines don't overlap adjacent rows.
	const double yHi = bandTopPx    + kBaselinePadPx;
	const double yLo = bandBottomPx - kBaselinePadPx;

	const int nSamples = m_samples.size();
	const quint64 total = m_sampleCount > 0
		? m_sampleCount
		: static_cast<quint64>(nSamples);

	// --- Waveform (drawn first, behind label) --------------------------
	if(!m_samples.isEmpty() && total > 0 && nSamples > 0) {
		painter->setPen(QPen(m_color, 1.4));

		// Map sample index -> canvas x pixel. Samples occupy the full
		// x-axis scale range [0..total-1], proportional layout across
		// current view. No left-clamp — the label is drawn on top.
		auto sampleToPx = [&](double sampleIndex) {
			return xMap.transform(sampleIndex);
		};

		const double sampleStride = static_cast<double>(total)
			/ static_cast<double>(nSamples);

		QVector<QPointF> pts;
		pts.reserve(nSamples * 2 + 2);

		double prevY = (m_samples[0] ? yHi : yLo);
		pts.append(QPointF(sampleToPx(0.0), prevY));

		for(int i = 1; i < nSamples; ++i) {
			const double x = sampleToPx(i * sampleStride);
			const double y = (m_samples[i] ? yHi : yLo);
			if(!qFuzzyCompare(y, prevY)) {
				pts.append(QPointF(x, prevY));
				pts.append(QPointF(x, y));
				prevY = y;
			}
		}
		// Trailing horizontal to the right edge of the wave area.
		const double xEnd = sampleToPx((nSamples - 1) * sampleStride);
		pts.append(QPointF(xEnd, prevY));

		painter->drawPolyline(pts.constData(), pts.size());
	}

	// --- Row label (drawn on top of waveform) --------------------------
	// Chevron "<" tip on the LEFT of the label, body extends to the right
	// edge of the canvas.
	const double tipX    = leftPx;
	const double bodyL   = leftPx + kLabelChevronPx;
	const double rightPx = leftPx + labelBoxW;
	const double midY    = (topPx + bottomPx) / 2.0;
	QPolygonF poly;
	poly << QPointF(tipX,    midY)
	     << QPointF(bodyL,   topPx)
	     << QPointF(rightPx, topPx)
	     << QPointF(rightPx, bottomPx)
	     << QPointF(bodyL,   bottomPx);
	painter->setPen(QPen(Style::getColor(json::theme::content_silent), 1.0));
	QColor bg = m_color;
	bg.setAlphaF(kLabelBgAlpha);
	painter->setBrush(bg);
	painter->drawPolygon(poly);

	painter->setPen(Style::getColor(json::theme::content_default));
	const QRectF textRect(bodyL + kLabelHPadPx, topPx,
		labelBoxW - kLabelHPadPx * 2.0 - kLabelChevronPx, kRowHeightPx);
	painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_title);

	painter->restore();
}

} // namespace adc
} // namespace scopy
