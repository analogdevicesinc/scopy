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

#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMainWindow>
#include <QMouseEvent>
#include <QSettings>
#include <QString>
#include <QWidget>

#include <scopy/core/api_object.hpp>
#include <scopy/gui/preferences.hpp>
#include <scopy/gui/tool_menu_item.hpp>

class QJSEngine;
class QPushButton;

extern "C"
{
	struct iio_context;
}

namespace scopy {
namespace gui {

class ToolLauncher;

class Tool : public QWidget
{
	Q_OBJECT

public:
	explicit Tool(struct iio_context* m_ctx, ToolMenuItem* m_toolMenuItem, scopy::core::ApiObject* m_api,
		      const QString& m_name, ToolLauncher* parent);
	~Tool();

	QPushButton* runButton() { return m_toolMenuItem->getToolStopBtn(); }
	const QString& getName();
	void setName(const QString& m_name);
	virtual void settingsLoaded();
	virtual void setNativeDialogs(bool nativeDialogs);

	scopy::core::ApiObject* getApi();

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
	struct iio_context* m_ctx;
	QPushButton* m_run_button;
	scopy::core::ApiObject* m_api;
	QSettings* m_settings;
	QString m_name;
	Preferences* m_prefPanel;
	bool m_saveOnExit;
	bool m_isDetached;
	bool m_running;
	QMainWindow* m_window;
	ToolMenuItem* m_toolMenuItem;
	bool m_useNativeDialogs;
};
} // namespace gui
} // namespace scopy

#endif /* SCOPY_TOOL_HPP */
