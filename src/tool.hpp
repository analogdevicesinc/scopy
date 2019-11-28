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

#ifndef SCOPY_TOOL_HPP
#define SCOPY_TOOL_HPP

#include <QWidget>
#include <QSettings>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QString>
#include <QMainWindow>

#include "preferences.h"

#include "toolmenuitem.h"

class QJSEngine;
class QPushButton;

extern "C" {
struct iio_context;
}

namespace adiscope {
class ApiObject;
class ToolLauncher;

class Tool : public QWidget
{
	Q_OBJECT

public:
	explicit Tool(struct iio_context *ctx, ToolMenuItem *toolMenuItem,
		      ApiObject *api, const QString& name,
		      ToolLauncher *parent);
	~Tool();

	QPushButton *runButton() { return toolMenuItem->getToolStopBtn(); }
	const QString& getName();
	void setName(const QString& name);
	virtual void settingsLoaded();
	virtual void setNativeDialogs(bool nativeDialogs);

Q_SIGNALS:
	void detachedState(bool detached);

public Q_SLOTS:
	virtual void run();
	virtual void stop();
	virtual void single();
	virtual bool isRunning();
	virtual void attached();
	virtual void detached();
	virtual void readPreferences();

private Q_SLOTS:
	void saveState();
	void loadState();

protected:
	struct iio_context *ctx;
	QPushButton *run_button;
	ApiObject *api;
	QSettings *settings;
	QString name;
	Preferences *prefPanel;
	bool saveOnExit;
	bool isDetached;
	bool m_running;
	QMainWindow *window;
	ToolMenuItem *toolMenuItem;
	bool m_useNativeDialogs;
};
}
#endif /* SCOPY_TOOL_HPP */
