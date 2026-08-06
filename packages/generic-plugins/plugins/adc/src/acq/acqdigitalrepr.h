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

#ifndef ACQDIGITALREPR_H
#define ACQDIGITALREPR_H

#include "acqchannelrepr.h"

#include <QPointer>
#include <QVector>

namespace scopy {
class PlotAxisHandle;

namespace adc {

class DigitalCurveItem;

// A fixed-height 0/1 track on the row's shared digital axis.
//
// Differs from CurveRepr in three ways that the interface exists to absorb:
//
//   - the visual is a QwtPlotItem, not a PlotChannel, so nothing here goes
//     through PlotWidget::addPlotChannel and the autoscaler/curve-style controls
//     do not apply;
//   - the band is 24 canvas pixels tall regardless of Y zoom, positioned off a
//     draggable PlotAxisHandle rather than by a Y value, so it needs a vertical
//     slot from the row rather than an axis range;
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
class DigitalRepr : public AcqChannelRepr
{
public:
	DigitalRepr();
	~DigitalRepr() override;

	QString kindName() const override { return QStringLiteral("DIGITAL"); }

	void attach(AcqPlotRow *row, const QString &name, const QColor &color) override;
	void detach() override;
	void pull(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, int plotSize) override;
	void reset() override;
	std::size_t claimDepth(int plotSize, std::size_t bufferSize) const override;
	QWidget *createSettingsWidget(QWidget *parent) override;
	void setEnabled(bool en) override;
	void setColor(const QColor &c) override;

private:
	QPointer<PlotWidget> m_plot;
	QPointer<AcqPlotRow> m_row;

	// Not a QPointer: DigitalCurveItem is a QwtPlotItem, not a QObject.
	DigitalCurveItem *m_item{nullptr};
	// Is a QPointer: AxisHandle parents itself to the canvas, so a canvas teardown
	// takes it with no notice to us.
	QPointer<PlotAxisHandle> m_handle;

	QColor m_color;
	bool m_enabled{true};
};

} // namespace adc
} // namespace scopy

#endif // ACQDIGITALREPR_H
