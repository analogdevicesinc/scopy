/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "nyquistplotzoomer.h"


using namespace scopy;

NyquistPlotZoomer::NyquistPlotZoomer(QwtPolarCanvas* parent):
    QwtPolarMagnifier(parent), zoom_count(0)
{

}

void NyquistPlotZoomer::zoomIn()
{
	QwtPolarMagnifier::rescale(0.8);
	++zoom_count;
}

void NyquistPlotZoomer::zoomOut()
{
	QwtPolarMagnifier::rescale(1.25);
	if (zoom_count > 0)
		--zoom_count;
}

bool NyquistPlotZoomer::isZoomed()
{
	return (zoom_count != 0);
}

void NyquistPlotZoomer::cancelZoom()
{
	zoom_count = 0;
	QwtPolarMagnifier::unzoom();
}
