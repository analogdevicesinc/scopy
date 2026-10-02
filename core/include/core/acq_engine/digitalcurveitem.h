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

#pragma once

#include "scopy-core_export.h"

#include <QColor>
#include <QPointer>
#include <QString>
#include <QVector>

#include <QwtPlotItem>

// Complete types required: the QPointer members below.
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>

namespace scopy {
namespace acq {

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
class SCOPY_CORE_EXPORT DigitalCurveItem : public QwtPlotItem
{
public:
	DigitalCurveItem(const QString &title, PlotAxis *xAxis, PlotAxis *yAxis, PlotAxisHandle *handle);
	~DigitalCurveItem() override;

	// Full-replace update. Values are treated as 0/1 (any non-zero => 1).
	void setSamples(const QVector<quint8> &samples);
	void clear();

	// Total sample count for proportional x-layout. 0 = verbatim mapping.
	void setSampleCount(quint64 n);

	void setColor(const QColor &c);
	QColor color() const { return m_color; }

	// QwtPlotItem overrides
	int rtti() const override { return QwtPlotItem::Rtti_PlotUserItem + 43; }
	void draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
		  const QRectF &canvasRect) const override;

private:
	PlotAxis *m_xAxis;
	PlotAxis *m_yAxis;
	QPointer<PlotAxisHandle> m_handle;

	QString m_title;
	QVector<quint8> m_samples;
	quint64 m_sampleCount = 0;
	QColor m_color;
};

} // namespace acq
} // namespace scopy
