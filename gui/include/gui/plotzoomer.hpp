#ifndef PLOTZOOMER_H
#define PLOTZOOMER_H

#include <QRubberBand>
#include <QwtPlot>
#include <qwt_scale_map.h>
#include "scopy-gui_export.h"

namespace scopy {

class SCOPY_GUI_EXPORT PlotZoomer : public QObject
{
	Q_OBJECT

public:
	explicit PlotZoomer(QwtPlot *plot);
	~PlotZoomer();

	QwtPlot *plot();

	void zoom(const QRectF &rect);
	void silentZoom(const QRectF &rect);
	bool isZoomed() const;

	void setBaseRect(const QRectF &rect);
	void setBaseRect();
	QRectF zoomBase() const;
	QRectF getCurrentRect();

	void setEnabled(bool en);
	bool isEnabled() const;

	void setBounded(bool en);
	bool isBoundEn();

	void setMinimumZoom(uint pixels);
	uint getMinimumZoom();

	void setDrawZoomRectEn(bool en);
	bool isDrawZoomRectEn();

	void setBlockZoomEn(bool en);
	bool isBlockZoomEn();

	void setBlockZoomResetEn(bool en);
	bool isBlockZoomResetEn();

	void setXAxis(QwtAxisId axisId);
	void setYAxis(QwtAxisId axisId);
	QwtAxisId getXAxis();
	QwtAxisId getYAxis();

	void setXAxisEn(bool en);
	void setYAxisEn(bool en);
	bool isXAxisEn() const;
	bool isYAxisEn() const;

Q_SIGNALS:
	void reset();
	void zoomed(const QRectF &rect);

protected:
	virtual bool eventFilter(QObject *object, QEvent *event) QWT_OVERRIDE;
	void rescale(const QRectF &rect);
	void onZoomResize(QMouseEvent *event);
	void onZoomStart(QMouseEvent *event);
	void onZoomEnd();

private Q_SLOTS:
	void zoomToBase();

private:
	QwtPlot *m_plot;
	QWidget *m_canvas;
	QRectF m_baseRect;
	QwtAxisId m_xAxis, m_yAxis;
	bool m_xAxisEn, m_yAxisEn;
	bool m_isZoomed;
	bool m_en;
	bool m_bounded;

	QRubberBand *m_rubberBand;
	QPoint m_origin;
	uint m_minZoom;
	bool m_drawZoom;
	bool m_blockReset;
	bool m_blockZoom;
};
} // namespace scopy

#endif // PLOTZOOMER_H
