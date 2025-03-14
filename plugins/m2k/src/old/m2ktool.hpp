/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef M2K_TOOL_HPP
#define M2K_TOOL_HPP

#include "pluginbase/toolmenuentry.h"

#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMainWindow>
#include <QMouseEvent>
#include <QSettings>
#include <QString>
#include <QWidget>

#include <pluginbase/preferences.h>
#include <pluginbase/resourcemanager.h>

class QJSEngine;
class QPushButton;

extern "C"
{
	struct iio_context;
}

namespace scopy {
class ApiObject;
class ToolLauncher;

namespace m2k {
class M2kTool : public QWidget, public ResourceUser
{
	Q_OBJECT

public:
	explicit M2kTool(ToolMenuEntry *tme, ApiObject *api, const QString &name, QWidget *parent);
	~M2kTool();

	const QString &getName();
	void setName(const QString &name);
	virtual void settingsLoaded();

	ApiObject *getApi();

	ToolMenuEntry *getTme() const;

Q_SIGNALS:
	void detachedState(bool detached);

public Q_SLOTS:
	virtual void run();
	virtual void stop();
	virtual void single();
	virtual bool isRunning();
	virtual void readPreferences();

protected:
	ApiObject *api;
	QString name;
	bool saveOnExit;
	bool isDetached;
	bool m_running;
	QMainWindow *window;
	ToolMenuEntry *tme;
	Preferences *p;
	QWidget *m_centralWidget;
};
} // namespace m2k
} // namespace scopy
#endif /* SCOPY_TOOL_HPP */
