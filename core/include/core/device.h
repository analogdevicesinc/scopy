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

#ifndef DEVICE_H
#define DEVICE_H

#include "pluginbase/toolmenuentry.h"
#include "scopy-core_export.h"

#include <QObject>
#include <QSettings>

namespace scopy {
class SCOPY_CORE_EXPORT Device
{
public:
	virtual ~Device(){};
	virtual QString id() = 0;
	virtual QString category() = 0;
	virtual QString displayName() = 0;
	virtual QString param() = 0;
	virtual QString displayParam() = 0;
	virtual QWidget *icon() = 0;

	virtual QWidget *page() = 0;

	virtual QList<ToolMenuEntry *> toolList() = 0;
	virtual void init() = 0;
	virtual void preload() = 0;
	virtual void loadPlugins() = 0;
	virtual void unloadPlugins() = 0;

public Q_SLOTS:
	virtual void connectDev() = 0;
	virtual void disconnectDev() = 0;
	virtual void showPage() = 0;
	virtual void hidePage() = 0;
	virtual void save(QSettings &) = 0;
	virtual void load(QSettings &) = 0;

Q_SIGNALS:
	virtual void toolListChanged() = 0;
	virtual void connecting() = 0;
	virtual void connected() = 0;
	virtual void disconnected() = 0;
	virtual void requestedRestart() = 0;
	virtual void requestTool(QString) = 0;
};
} // namespace scopy

#endif // DEVICE_H
