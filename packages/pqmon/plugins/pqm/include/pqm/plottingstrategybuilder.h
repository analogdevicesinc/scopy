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

#ifndef PLOTTINGSTRATEGYBUILDER_H
#define PLOTTINGSTRATEGYBUILDER_H
#include "rollingstrategy.h"
#include "swtriggerstrategy.h"

namespace scopy::pqm {
class PlottingStrategyBuilder
{
public:
	static PlottingStrategy *build(QString mode, int samplingFreq, QString triggeredBy = "")
	{
		if(mode.compare("rolling") == 0) {
			return new RollingStrategy(samplingFreq);
		} else {
			return new SwTriggerStrategy(samplingFreq, triggeredBy);
		}
	}
};
} // namespace scopy::pqm

#endif // PLOTTINGSTRATEGYBUILDER_H
