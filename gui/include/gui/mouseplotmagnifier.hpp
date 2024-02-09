#ifndef MOUSEPLOTMAGNIFIER_H
#define MOUSEPLOTMAGNIFIER_H

#include <QwtPlot>
#include <qwt_scale_map.h>
#include "scopy-gui_export.h"

namespace scopy {

class SCOPY_GUI_EXPORT MousePlotMagnifier : public QObject
{
	Q_OBJECT

public:
	explicit MousePlotMagnifier(QWidget *canvas);
	~MousePlotMagnifier();

	void setEnabled(bool en);
	bool isEnabled() const;

	void setBounded(bool en);
	void setBaseRect(const QRectF &rect);
	QRectF zoomBase() const;
	QwtPlot *getPlot();
	void setFactor(double factor);
	double getFactor() const;
	bool isZoomed() const;

	void setZoomModifier(Qt::KeyboardModifier modifier);
	Qt::KeyboardModifier getZoomModifier();
	void setPanModifier(Qt::KeyboardModifier modifier);
	Qt::KeyboardModifier getPanModifier();

	void setXAxis(QwtAxisId axisId);
	void setYAxis(QwtAxisId axisId);
	void setXAxisEnabled(bool en);
	void setYAxisEnabled(bool en);
	bool isXAxisEnabled() const;
	bool isYAxisEnabled() const;

	void silentZoom(double factor, QPointF cursorPos);
	void zoom(double factor, QPointF cursorPos);
	void silentPan(double factor);
	void pan(double factor);

Q_SIGNALS:
	void reset();
	void zoomed(double factor, QPointF cursorPos = QPointF());
	void panned(double factor);

protected:
	virtual bool eventFilter(QObject *object, QEvent *event) QWT_OVERRIDE;
	void panRescale(double factor);
	void zoomRescale(double factor);
	void setPlotAxisScale(QwtAxisId axisId, double min, double max);

private Q_SLOTS:
	void zoomToBase();

private:
	QWidget *m_canvas;
	double m_factor;
	QPointF m_cursorPos;
	QRectF m_zoomBase;
	QwtAxisId m_xAxis, m_yAxis;
	bool m_xAxisEn, m_yAxisEn;
	bool m_isZoomed;
	bool m_en;
	bool m_bounded;
	Qt::KeyboardModifier m_zoomModifier, m_panModifier;
};
} // namespace scopy

#endif // MOUSEPLOTMAGNIFIER_H
