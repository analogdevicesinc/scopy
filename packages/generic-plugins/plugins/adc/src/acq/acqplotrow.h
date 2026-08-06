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

#ifndef ACQPLOTROW_H
#define ACQPLOTROW_H

#include <QObject>
#include <QPointer>

namespace scopy {
class PlotAxis;
class PlotWidget;

namespace adc {

// One plot in the manager's splitter, plus the axes its channels share.
//
// A QObject rather than a QWidget: the PlotWidget goes straight into the
// splitter, so a wrapper widget would add a layout level for nothing.
//
// Axes are shared per row and never destroyed. PlotWidget has no removal path —
// see the commented-out declaration at gui/include/gui/plotwidget.h:73,
// "not supported by Qwt" — while PlotAxis's constructor registers itself into
// PlotWidget::m_plotAxis and bumps the QwtPlot's axis count, and its destructor
// is empty. Deleting a per-channel axis therefore leaves a dangling pointer for
// PlotNavigator to walk. The cost of sharing is that Y range and autoscale are
// per-row rather than per-channel; the alternative is a use-after-free.
class AcqPlotRow : public QObject
{
	Q_OBJECT
public:
	// `plot` is already in the manager's splitter and parented to it. `exclusive`
	// means a repr brought this plot via createOwnPlot(), so no other channel may
	// attach here.
	AcqPlotRow(PlotWidget *plot, bool exclusive, QObject *parent = nullptr);
	~AcqPlotRow() override;

	// Null once the plot has been destroyed — always check before use.
	PlotWidget *plot() const;
	bool isExclusive() const { return m_exclusive; }

	// The one axis hosting fixed-height items (digital tracks, annotation bands)
	// for this row. Created on first use, interval [0,1], invisible: items place
	// themselves in canvas pixels off their own handle, so the axis exists only to
	// host handles and items. Same construction as
	// DigitalTrackManager::setPlot() (src/sim/DigitalTrackManager.cpp:71-80).
	PlotAxis *digitalAxis();

	// Next free vertical slot for a fixed-height item, in digitalAxis scale coords.
	// Walks down in 26 px steps (24 px band + 2 px gap) — the pitch DigitalCurveItem
	// and AnnotationCurve both render at. Lifted from
	// DigitalTrackManager::nextHandlePos() (:120-157) including its fallback for a
	// canvas that has not laid out yet.
	double nextDigitalSlot();

public Q_SLOTS:
	void replot();

private:
	QPointer<PlotWidget> m_plot;
	PlotAxis *m_digitalAxis{nullptr};
	const bool m_exclusive;

	double m_nextSlot{0.0};
	bool m_slotInit{false};
};

} // namespace adc
} // namespace scopy

#endif // ACQPLOTROW_H
