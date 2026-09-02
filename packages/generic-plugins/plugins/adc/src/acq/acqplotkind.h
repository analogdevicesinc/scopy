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

#ifndef ACQPLOTKIND_H
#define ACQPLOTKIND_H

#include <QList>
#include <QString>

class QWidget;

namespace scopy {
class PlotWidget;

namespace adc {

// What kind of plot widget a plot is. The reader picks one when adding a plot.
enum class AcqPlotKind
{
	Basic = 0,
	Waterfall = 1,
};

// The widget for a kind. This is the one thing here worth a translation unit of its own:
// acqplotkind.cpp is the only file that includes waterfallplotwidget.h, which is what
// keeps every concrete widget class out of AcqPlot and AcqPlotManager.
//
// Never null — an unrecognised kind gets a plain PlotWidget rather than nothing, so no
// caller has to handle a plot that failed to build.
scopy::PlotWidget *createPlotWidget(AcqPlotKind kind, QWidget *parent);

// Shown in the add-plot picker and as a read-only label on the plot's page.
QString plotKindName(AcqPlotKind kind);

// Every kind, in enum order, for the add-plot picker.
QList<AcqPlotKind> allPlotKinds();

// Whether channels on this kind of plot take their own axes out of AcqPlot's pool. False
// for a widget that manages its own axes (a waterfall owns an inverted time Y and an Hz
// X and rescales them itself); its channels' AcqAxis then wrap the widget's built-in
// pair, source-fixed.
//
// Not a compatibility check — this decides which axes are actually allocated, in
// AcqChannel::acquireAxes/releaseAxes.
bool plotKindPoolsAxes(AcqPlotKind kind);

} // namespace adc
} // namespace scopy

#endif // ACQPLOTKIND_H
