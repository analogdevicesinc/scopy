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
 *
 */

#ifndef DAQ2_API_H
#define DAQ2_API_H

#include "scopy-daq2_export.h"
#include <pluginbase/apiobject.h>

namespace scopy::daq2 {

class Daq2Plugin;

class SCOPY_DAQ2_EXPORT Daq2_API : public ApiObject
{
	Q_OBJECT
public:
	explicit Daq2_API(Daq2Plugin *plugin);
	~Daq2_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// ADC test mode — axi-ad9680-hpc voltage0 (Ch0)
	Q_INVOKABLE QString getAdcCh0TestMode();
	Q_INVOKABLE void setAdcCh0TestMode(const QString &mode);

	// ADC test mode — axi-ad9680-hpc voltage1 (Ch1)
	Q_INVOKABLE QString getAdcCh1TestMode();
	Q_INVOKABLE void setAdcCh1TestMode(const QString &mode);

	// Generic widget access
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// Trigger a read refresh on all widgets
	Q_INVOKABLE void refresh();

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);

	Daq2Plugin *m_plugin;
};

} // namespace scopy::daq2

#endif // DAQ2_API_H
