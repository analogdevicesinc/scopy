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

#ifndef ADRV9009PLUGIN_API_H
#define ADRV9009PLUGIN_API_H

#include "scopy-adrv9009plugin_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::adrv9009 {

class Adrv9009Plugin;

class SCOPY_ADRV9009PLUGIN_EXPORT Adrv9009Plugin_API : public ApiObject
{
	Q_OBJECT
public:
	explicit Adrv9009Plugin_API(Adrv9009Plugin *plugin);
	~Adrv9009Plugin_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// Generic IIOWidget access (full attribute coverage)
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);
	Q_INVOKABLE void refresh();

	// Convenience: main tool
	Q_INVOKABLE void triggerRefresh();
	Q_INVOKABLE void triggerMcsSync();

	// Convenience: advanced tool
	Q_INVOKABLE void triggerAdvancedRefresh();
	Q_INVOKABLE void switchAdvancedTab(const QString &tabName);

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);

	Adrv9009Plugin *m_plugin;
};

} // namespace scopy::adrv9009

#endif // ADRV9009PLUGIN_API_H
