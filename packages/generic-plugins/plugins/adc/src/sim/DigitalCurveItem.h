#pragma once

#include <QColor>
#include <QPointer>
#include <QString>
#include <QVector>

#include <QwtPlotItem>

namespace scopy {

class PlotAxis;
class PlotAxisHandle;

namespace adc {

// Draws a single digital 0/1 waveform as a step curve at a fixed canvas-pixel
// height (24 px, matching AnnotationCurve row height). The waveform's vertical
// position is driven by a PlotAxisHandle whose scale-space position defines
// the top edge in y-axis scale coordinates; the item then renders the fixed
// 24 px band in canvas pixels regardless of y-axis zoom.
//
// Sample x-values are treated as sample indices [0..sampleCount). If a
// non-zero sampleCount is supplied via setSampleCount(), samples are laid out
// proportionally across the current x-axis interval (same convention as
// AnnotationCurve). Otherwise indices map to x-axis scale values verbatim.
class DigitalCurveItem : public QwtPlotItem
{
public:
	DigitalCurveItem(const QString &title, PlotAxis *xAxis, PlotAxis *yAxis,
	                 PlotAxisHandle *handle);
	~DigitalCurveItem() override;

	// Full-replace update. Values are treated as 0/1 (any non-zero => 1).
	void setSamples(const QVector<quint8> &samples);
	void clear();

	// Total sample count for proportional x-layout. 0 = verbatim mapping.
	void setSampleCount(quint64 n);

	void setColor(const QColor &c);
	QColor color() const { return m_color; }

	// QwtPlotItem overrides
	int  rtti() const override { return QwtPlotItem::Rtti_PlotUserItem + 43; }
	void draw(QPainter *painter, const QwtScaleMap &xMap,
	          const QwtScaleMap &yMap, const QRectF &canvasRect) const override;

private:
	PlotAxis                *m_xAxis;
	PlotAxis                *m_yAxis;
	QPointer<PlotAxisHandle> m_handle;

	QString           m_title;
	QVector<quint8>   m_samples;
	quint64           m_sampleCount = 0;
	QColor            m_color;
};

} // namespace adc
} // namespace scopy
