/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 */

#ifndef ADALM2000POWERSUPPLY_API_H
#define ADALM2000POWERSUPPLY_API_H

#include "scopy-adalm2000_export.h"

#include <pluginbase/apiobject.h>

namespace scopy::adalm2000 {

class Adalm2000PowerSupplyTool;

class SCOPY_ADALM2000_EXPORT Adalm2000PowerSupply_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(bool dac1_enabled READ dac1Enabled WRITE setDac1Enabled)
	Q_PROPERTY(bool dac2_enabled READ dac2Enabled WRITE setDac2Enabled)
	Q_PROPERTY(double dac1_value READ dac1Value WRITE setDac1Value)
	Q_PROPERTY(double dac2_value READ dac2Value WRITE setDac2Value)
	Q_PROPERTY(bool sync READ sync WRITE setSync)
	Q_PROPERTY(int tracking_percent READ trackingPercent WRITE setTrackingPercent)

public:
	explicit Adalm2000PowerSupply_API(Adalm2000PowerSupplyTool *tool);
	~Adalm2000PowerSupply_API() override;

	bool dac1Enabled() const;
	void setDac1Enabled(bool en);

	bool dac2Enabled() const;
	void setDac2Enabled(bool en);

	double dac1Value() const;
	void setDac1Value(double volts);

	double dac2Value() const;
	void setDac2Value(double volts);

	bool sync() const;
	void setSync(bool tracking);

	int trackingPercent() const;
	void setTrackingPercent(int percent);

	// channel is 0 (positive rail) or 1 (negative rail); omit it to act on both.
	Q_INVOKABLE bool run(int channel = -1);
	Q_INVOKABLE bool stop(int channel = -1);

private:
	bool setRailsEnabled(int channel, bool on);

	Adalm2000PowerSupplyTool *m_tool;
};

} // namespace scopy::adalm2000

#endif // ADALM2000POWERSUPPLY_API_H
