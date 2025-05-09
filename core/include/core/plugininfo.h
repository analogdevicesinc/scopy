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
 *
 */

#ifndef PLUGININFO_H
#define PLUGININFO_H

#include "pluginbase/plugin.h"
#include <QString>

namespace scopy {
class PluginInfo
{

public:
	enum PluginState
	{
		PLUGIN_UNLOADED,
		PLUGIN_LOADED,
		PLUGIN_FILE_NOT_FOUND,
		PLUGIN_INVALID_LIBRARY,
		PLUGIN_LOAD_FAILED,
		PLUGIN_INSTANCE_FAILED,
		PLUGIN_INVALID_PLUGIN,
		PLUGIN_CLONE_FAILED
	};

	PluginInfo(const QString &file, PluginState state, QString errorMsg = "", Plugin *instance = nullptr);
	~PluginInfo();

	bool isLoaded() const;
	QString getErrorMessage() const;

	QString filePath() const { return m_filePath; }
	QString name() const { return m_name; }
	QString description() const { return m_description; }
	PluginState state() const { return m_state; }
	Plugin *pluginInstance() const { return m_pluginInstance; }

	void setState(PluginState state);
	void setPluginInstance(Plugin *instance);

	void setErrorMsg(const QString &newErrorMsg);

private:
	QString m_filePath;
	QString m_name;
	QString m_errorMsg;
	QString m_description;
	PluginState m_state;
	Plugin *m_pluginInstance;
};
} // namespace scopy

#endif // PLUGININFO_H
