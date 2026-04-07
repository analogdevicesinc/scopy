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
 */

#ifndef CN0540_API_H
#define CN0540_API_H

#include "scopy-cn0540_export.h"

#include <QStringList>
#include <pluginbase/apiobject.h>

namespace scopy::cn0540 {

class CN0540;
class CN0540Plugin;

class SCOPY_CN0540_EXPORT CN0540_API : public ApiObject
{
	Q_OBJECT
public:
	explicit CN0540_API(CN0540Plugin *plugin);
	~CN0540_API();

	Q_INVOKABLE QStringList getTools();
	Q_INVOKABLE void refresh();

	// Power Control
	Q_INVOKABLE QString getSwFF();
	Q_INVOKABLE QString getShutdown();
	Q_INVOKABLE void setShutdown(const QString &enabled);
	Q_INVOKABLE QString getConstantCurrent();
	Q_INVOKABLE void setConstantCurrent(const QString &enabled);

	// ADC Driver Settings
	Q_INVOKABLE QString getFdaEnabled();
	Q_INVOKABLE void setFdaEnabled(const QString &enabled);
	Q_INVOKABLE QString getFdaMode();
	Q_INVOKABLE void setFdaMode(const QString &mode);

	// Sensor Calibration
	Q_INVOKABLE QString getInputVoltage();
	Q_INVOKABLE QString getShiftVoltage();
	Q_INVOKABLE void setShiftVoltage(const QString &mV);
	Q_INVOKABLE QString getSensorVoltage();
	Q_INVOKABLE void calibrate();

	// Voltage Monitor
	Q_INVOKABLE QStringList getVoltageMonitor();

	// IIOWidget access
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

private:
	CN0540 *getTool() const;
	CN0540Plugin *m_plugin;
};

} // namespace scopy::cn0540
#endif // CN0540_API_H
