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

#ifndef FFTMARKERCONTROLLER_HPP
#define FFTMARKERCONTROLLER_HPP

#include <plotmarkercontroller.h>

namespace scopy {
namespace adc {

class FFTPlotComponentChannel;

class FFTMarkerController : public PlotMarkerController
{
public:
	FFTMarkerController(FFTPlotComponentChannel *ch, QObject *parent);
	~FFTMarkerController();

	// MarkerController interface
public:
	void init();
	void attachMarkersToPlot() override;

protected:
	int findPeakNearIdx(int idx, int range) override;
	void computeImageMarkers() override;
	void computeSingleToneMarkers() override;
	void computePeaks() override;
	void initFixedMarker() override;

private:
	FFTPlotComponentChannel *m_ch;
};
} // namespace adc
} // namespace scopy
#endif // FFTMARKERCONTROLLER_HPP
