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
#ifndef TOOL_LAUNCHER_API_HPP
#define TOOL_LAUNCHER_API_HPP

#include "tool_launcher.hpp"

namespace adiscope {

class ToolLauncher_API: public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(bool menu_opened READ menu_opened WRITE open_menu
		   STORED false);

	Q_PROPERTY(int focused_instrument READ focused_instrument WRITE focus_instrument );

	Q_PROPERTY(bool hidden READ hidden WRITE hide STORED false);

	Q_PROPERTY(QString previous_ip READ getPreviousIp WRITE addIp
		   SCRIPTABLE false);

	Q_PROPERTY(bool maximized READ maximized WRITE setMaximized);

	Q_PROPERTY(bool always_on_top READ alwaysOnTop WRITE setAlwaysOnTop);

	Q_PROPERTY(bool skip_calibration READ calibration_skipped WRITE skip_calibration);

	Q_PROPERTY(bool debugger READ debugger_enabled WRITE enable_debugger)

	Q_PROPERTY(QString index_file READ getIndexFile WRITE setIndexFile)

	Q_PROPERTY(bool manual_calibration READ manual_calibration_enabled WRITE enable_manual_calibration)

public:
	explicit ToolLauncher_API(ToolLauncher *tl) : ApiObject(), tl(tl) {}
	~ToolLauncher_API() {}

	QString getIndexFile() const;
	void setIndexFile(const QString &indexFile);

	bool menu_opened() const;
	void open_menu(bool open);

	bool hidden() const;
	void hide(bool hide);

	bool calibration_skipped();
	void skip_calibration(bool);

	int focused_instrument();
	void focus_instrument(int);

	bool debugger_enabled();
	void enable_debugger(bool);

	bool manual_calibration_enabled() const;
	void enable_manual_calibration(bool);

	const QString& getPreviousIp()
	{
		return tl->previousIp;
	}
	void addIp(const QString& ip);

	bool maximized()
	{
		return tl->isMaximized();
	}
	void setMaximized(bool m)
	{
		if (m) {
			tl->showMaximized();
		} else {
			tl->showNormal();
		}
	}

	bool alwaysOnTop() const
	{
		return !!(tl->windowFlags() & Qt::WindowStaysOnTopHint);
	}
	void setAlwaysOnTop(bool on)
	{
		tl->setWindowFlag(Qt::WindowStaysOnTopHint, on);
		tl->show();
	}

	Q_INVOKABLE QList<QString> usb_uri_list();
	Q_INVOKABLE bool connect(const QString& uri);
	Q_INVOKABLE void disconnect();

	Q_INVOKABLE void load(const QString& file);
	Q_INVOKABLE void save(const QString& file);
	Q_INVOKABLE void save(QSettings *settings);
	Q_INVOKABLE void sync();
	Q_INVOKABLE bool reset();
	Q_INVOKABLE bool enableExtern(bool);
	Q_INVOKABLE bool enableCalibScript(bool);

private:
	ToolLauncher *tl;
};
}
#endif // TOOL_LAUNCHER_API_HPP
