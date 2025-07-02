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

#ifndef ADC_INTERFACES_H
#define ADC_INTERFACES_H

#include "scopy-adc_export.h"
#include <QString>
// #include <signalpath.h>
#include "measurementcontroller.h"
#include <gui/plotmarkercontroller.h>
#include <gui/interfaces.h>
#include <timechannelsigpath.h>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT Channel : public DataProcessor
{
public:
	virtual datasink::SignalPath *sigpath() = 0;
};

class TimePlotComponent;
typedef struct
{
	std::vector<float> x;
	std::vector<float> y;
	TimePlotComponent *targetPlot;
	QString name;
} SnapshotRecipe;

class SCOPY_ADC_EXPORT MeasurementProvider
{
public:
	virtual MeasureManagerInterface *getMeasureManager() = 0;
};

} // namespace adc
} // namespace scopy

#endif // ADC_INTERFACES_H
