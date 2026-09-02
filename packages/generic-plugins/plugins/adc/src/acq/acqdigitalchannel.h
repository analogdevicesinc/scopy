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

#ifndef ACQDIGITALCHANNEL_H
#define ACQDIGITALCHANNEL_H

#include "acqchannel.h"

#include <QPointer>
#include <QVector>

namespace scopy {
class PlotAxis;
class PlotAxisHandle;
class PlotWidget;

namespace adc {

class DigitalCurveItem;

// A fixed-height 0/1 track on the plot's shared digital axis.
//
// Differs from AcqCurveChannel in three ways that the base exists to absorb:
//
//   - the visual is a QwtPlotItem, not a PlotChannel, so nothing here goes
//     through PlotWidget::addPlotChannel and the autoscaler/curve-style controls
//     do not apply;
//   - the band is 24 canvas pixels tall regardless of Y zoom, positioned off a
//     draggable PlotAxisHandle rather than by a Y value, so it needs a vertical
//     slot from the plot rather than an axis range — which is why it declines the
//     pooled Y axis and wraps AcqPlot::digitalAxis() instead;
//   - it reads bits, not floats. toBits() maps any non-zero to 1, which is what
//     makes a UInt8 logic stream and an Int32 comparator output look the same
//     here.
//
// X alignment with the analog curves is not a coincidence and not free: the item
// is told the plot's sample count and lays its samples out proportionally across
// the *shared* X axis interval, which is the same convention AnnotationCurve
// uses. Without setSampleCount() the sample indices would be read as X scale
// values and a 1024-sample track would be crammed into the first 1024 units of
// whatever the axis happens to span.
class AcqDigitalChannel : public AcqChannel
{
	Q_OBJECT
public:
	explicit AcqDigitalChannel(const Args &args);
	~AcqDigitalChannel() override;

	QString kindName() const override { return QStringLiteral("DIGITAL"); }

	void reset() override;

protected:
	void attachTo(AcqPlot *plot) override;
	void detachFrom() override;
	void readData(scopy::acq::DataStore *store, int plotSize) override;
	DepthNeed depthNeeded(int plotSize) const override;

	void onEnabledChanged(bool en) override;
	void onColorChanged(const QColor &c) override;

	// Declines the pooled Y axis: the band hangs off a PlotAxisHandle on the plot's
	// shared digital axis, so a private Y scale would be an axis nothing draws against.
	bool wantsPooledYAxis() const override { return false; }
	scopy::PlotAxis *ownYAxis(AcqPlot *plot) override;

private:
	// No plot pointer of our own: the base holds it and plotOwner() returns it.
	QPointer<PlotWidget> m_plot;

	// Not a QPointer: DigitalCurveItem is a QwtPlotItem, not a QObject.
	DigitalCurveItem *m_item{nullptr};
	// Is a QPointer: AxisHandle parents itself to the canvas, so a canvas teardown
	// takes it with no notice to us.
	QPointer<PlotAxisHandle> m_handle;
};

} // namespace adc
} // namespace scopy

#endif // ACQDIGITALCHANNEL_H
