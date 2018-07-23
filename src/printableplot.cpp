/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "printableplot.h"

using namespace adiscope;

PrintablePlot::PrintablePlot(QWidget *parent) :
        QwtPlot(parent),
        d_plotRenderer(new QwtPlotRenderer(this))
{
        dropBackground(true);
}

void PrintablePlot::dropBackground(bool drop)
{
        d_plotRenderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, drop);
        d_plotRenderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground, drop);
}

void PrintablePlot::printPlot()
{
        legendDisplay = new QwtLegend(this);
        legendDisplay->setDefaultItemMode(QwtLegendData::ReadOnly);
        insertLegend(legendDisplay, QwtPlot::TopLegend);

        updateLegend();

	d_plotRenderer.exportTo(this, "plotScreenshot.png");

        insertLegend(nullptr);
}
