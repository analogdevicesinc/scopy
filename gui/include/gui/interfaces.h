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

#ifndef INTERFACES_H
#define INTERFACES_H

#include "scopy-gui_export.h"
#include <measurementpanel.h>
#include <menuwidget.h>
#include <plotwidget.h>
#include <iioutil/iiounits.h>
#include "plotmarkercontroller.h"

namespace scopy {

using namespace gui;

class SCOPY_GUI_EXPORT Menu
{
public:
	virtual MenuWidget *menu() = 0;
};

class SCOPY_GUI_EXPORT DataProcessor
{
public:
	virtual void onNewData(const float *xData, const float *yData, size_t size, bool copy) = 0;
};

class SCOPY_GUI_EXPORT SamplingInfoComponent
{
public:
	virtual SamplingInfo samplingInfo() = 0;
	virtual void setSamplingInfo(SamplingInfo p) = 0;
};

class SCOPY_GUI_EXPORT FFTChannel
{
public:
	virtual void setPowerOffset(double) = 0;
	virtual void setWindow(int) = 0;
	virtual void setWindowCorrection(bool) = 0;
};

class SCOPY_GUI_EXPORT SampleRateProvider
{
public:
	virtual bool sampleRateAvailable() = 0;
	virtual double sampleRate() = 0;
};

typedef enum
{
	YMODE_COUNT,
	YMODE_FS,
	YMODE_SCALE
} YMode;

class SCOPY_GUI_EXPORT ScaleProvider
{
public:
	virtual YMode ymode() const = 0;
	virtual void setYMode(YMode newYmode) = 0;
	virtual bool scaleAvailable() const = 0;
	virtual bool yLock() const = 0;
	virtual double yMin() const = 0;
	virtual double yMax() const = 0;
	virtual IIOUnit unit() const = 0;
};

class SCOPY_GUI_EXPORT MeasurementPanelInterface
{
public:
	virtual MeasurementsPanel *measurePanel() const = 0;
	virtual StatsPanel *statsPanel() const = 0;
	virtual MarkerPanel *markerPanel() const = 0;
	virtual void enableMeasurementPanel(bool) = 0;
	virtual void enableStatsPanel(bool) = 0;
	virtual void enableMarkerPanel(bool) = 0;
};
} // namespace scopy

#endif // INTERFACES_H
