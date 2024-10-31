/*
 * Copyright (c) 2024 Analog Devices Inc.
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
 *
 */

#ifndef BasicTracker_H
#define BasicTracker_H

#include <QLabel>
#include <QwtPlot>
#include <QwtWidgetOverlay>
#include <qwt_scale_map.h>
#include "plot_utils.hpp"
#include "scopy-gui_export.h"

namespace scopy {

class SCOPY_GUI_EXPORT BasicTracker : public QwtWidgetOverlay
{
	Q_OBJECT

public:
	explicit BasicTracker(QwtPlot *plot);
	~BasicTracker();

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

#endif // BasicTracker_H
