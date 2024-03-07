#ifndef GENERICTRACKER_H
#define GENERICTRACKER_H

#include <QLabel>
#include <QwtPlot>
#include <QwtWidgetOverlay>
#include <qwt_scale_map.h>
#include "plot_utils.hpp"
#include "scopy-gui_export.h"

namespace scopy {

class SCOPY_GUI_EXPORT GenericTracker : public QwtWidgetOverlay
{
	Q_OBJECT

public:
	explicit GenericTracker(QwtPlot *plot);
	~GenericTracker();

	QwtPlot *plot() const;

	void setXAxis(QwtAxisId axisId);
	QwtAxisId getXAxis();
	void setYAxis(QwtAxisId axisId);
	QwtAxisId getYAxis();

	void setXAxisEn(bool en);
	bool isXAxisEn() const;
	void setYAxisEn(bool en);
	bool isYAxisEn() const;

	void setXAxisUnit(QString unit);
	QString getXAxisUnit();
	void setYAxisUnit(QString unit);
	QString getYAxisUnit();

	void setXFormatter(PrefixFormatter *formatter);
	PrefixFormatter *getXFormatter();
	void setYFormatter(PrefixFormatter *formatter);
	PrefixFormatter *getYFormatter();

	void setColor(QColor color);

protected:
	virtual void drawOverlay(QPainter *painter) const override;
	virtual bool eventFilter(QObject *object, QEvent *event) QWT_OVERRIDE;
	QRect trackerRect(QSizeF size) const;
	QwtText *trackerText(QPoint pos) const;

private:
	QwtPlot *m_plot;
	QWidget *m_canvas;
	QwtAxisId m_xAxis, m_yAxis;
	bool m_xAxisEn, m_yAxisEn;
	QString m_xAxisUnit, m_yAxisUnit;
	QPoint m_mousePos;
	QColor m_color;
	PrefixFormatter *m_xFormatter, *m_yFormatter;
};
} // namespace scopy

#endif // GENERICTRACKER_H
