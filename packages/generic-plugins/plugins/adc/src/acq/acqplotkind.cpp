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

#include "acqplotkind.h"

#include <gui/plotwidget.h>
#include <gui/waterfallplotwidget.h>

#include <QCoreApplication>

using namespace scopy;
using namespace scopy::adc;

// Four switches rather than a registry of Entry structs keyed by kind. There is no
// registration step and no runtime table: a kind maps to one widget class, known here, so
// adding a third kind is an enum value and one case in each switch below.
//
// Every switch has a Basic default, so an out-of-range kind draws a plain plot instead of
// producing a null widget for callers to handle.

PlotWidget *scopy::adc::createPlotWidget(AcqPlotKind kind, QWidget *parent)
{
	switch(kind) {
	case AcqPlotKind::Waterfall:
		return new WaterfallPlotWidget(parent);
	case AcqPlotKind::Basic:
	default:
		return new PlotWidget(parent);
	}
}

QString scopy::adc::plotKindName(AcqPlotKind kind)
{
	switch(kind) {
	case AcqPlotKind::Waterfall:
		return QCoreApplication::translate("AcqPlotKind", "Waterfall");
	case AcqPlotKind::Basic:
	default:
		return QCoreApplication::translate("AcqPlotKind", "Plot");
	}
}

QList<AcqPlotKind> scopy::adc::allPlotKinds() { return {AcqPlotKind::Basic, AcqPlotKind::Waterfall}; }

bool scopy::adc::plotKindPoolsAxes(AcqPlotKind kind)
{
	switch(kind) {
	case AcqPlotKind::Waterfall:
		// It owns an inverted time Y axis and an Hz X axis and rescales them itself, so a
		// pooled pair would be two unused axes and two settings sections that lie.
		return false;
	case AcqPlotKind::Basic:
	default:
		return true;
	}
}
