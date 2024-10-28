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

#ifndef DETACHEDTOOLMANAGER_H
#define DETACHEDTOOLMANAGER_H

#include "detachedtoolwindow.h"
#include "scopy-core_export.h"

#include <QMap>
#include <QWidget>

#include <pluginbase/toolmenuentry.h>

namespace scopy {

class SCOPY_CORE_EXPORT DetachedToolWindowManager : public QObject
{
	Q_OBJECT
public:
	DetachedToolWindowManager(QObject *parent = nullptr);
	~DetachedToolWindowManager();

	void add(QString id, ToolMenuEntry *tme);
	bool remove(QString id);
	bool contains(QString key);
	QWidget *getWidget(QString key);

public Q_SLOTS:
	void show(QString id);

private:
	QMap<QString, DetachedToolWindow *> map;
};
} // namespace scopy

#endif // DETACHEDTOOLMANAGER_H
