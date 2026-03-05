/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef IMUANALYZER_API_H
#define IMUANALYZER_API_H

#include "scopy-imuanalyzer_export.h"
#include <pluginbase/apiobject.h>
#include <QString>

namespace scopy {

class IMUAnalyzer;
class IMUAnalyzerInterface;

class SCOPY_IMUANALYZER_EXPORT IMUAnalyzer_API : public ApiObject
{
	Q_OBJECT
public:
	explicit IMUAnalyzer_API(IMUAnalyzer *plugin);
	~IMUAnalyzer_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// Run control
	Q_INVOKABLE bool isRunning();
	Q_INVOKABLE void setRunning(bool running);

	// Measure panel
	Q_INVOKABLE bool isMeasureEnabled();
	Q_INVOKABLE void setMeasureEnabled(bool enabled);

	// Sensor data
	Q_INVOKABLE double getRotationX();
	Q_INVOKABLE double getRotationY();
	Q_INVOKABLE double getRotationZ();
	Q_INVOKABLE double getAccelerationX();
	Q_INVOKABLE double getAccelerationY();
	Q_INVOKABLE double getAccelerationZ();

	// 3D view
	Q_INVOKABLE void resetView();

	// 2D view
	Q_INVOKABLE void setDisplayPoints(const QString &mode);

private:
	IMUAnalyzerInterface *getIMUInterface();

	IMUAnalyzer *m_plugin;
};
} // namespace scopy
#endif // IMUANALYZER_API_H
