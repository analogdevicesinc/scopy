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

#include <QWidget>
#include <QSettings>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QString>
#include <QMainWindow>
#include "pluginbase/toolmenuentry.h"
#include <pluginbase/preferences.h>
#include <pluginbase/resourcemanager.h>


class QJSEngine;
class QPushButton;

extern "C" {
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
	explicit M2kTool(struct iio_context *ctx, ToolMenuEntry *tme,
		      ApiObject *api, const QString& name,
		      QWidget *parent);
	~M2kTool();

	const QString& getName();
	void setName(const QString& name);
	virtual void settingsLoaded();
	virtual void setNativeDialogs(bool nativeDialogs);

	ApiObject* getApi();

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
	struct iio_context *ctx;	
	ApiObject *api;
	QString name;
	bool saveOnExit;
	bool isDetached;
	bool m_running;
	QMainWindow *window;
	ToolMenuEntry *tme;
	Preferences *p;
	bool m_useNativeDialogs;
	QWidget *m_centralWidget;
};
}
}
#endif /* SCOPY_TOOL_HPP */
