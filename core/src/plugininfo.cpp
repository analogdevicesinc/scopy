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

#include "plugininfo.h"

#include <QFileInfo>

using namespace scopy;

PluginInfo::PluginInfo(const QString &file, PluginState state, QString errorMsg, Plugin *instance)
	: m_filePath(file)
	, m_errorMsg(errorMsg)

{
	setState(state);
	setPluginInstance(instance);
}

PluginInfo::~PluginInfo() {}

void PluginInfo::setState(PluginState state)
{
	m_state = state;
	if(m_state != PLUGIN_LOADED) {
		m_name = QFileInfo(m_filePath).baseName();
		m_description = m_errorMsg.isEmpty() ? getErrorMessage() : m_errorMsg;
	}
}

void PluginInfo::setPluginInstance(Plugin *instance)
{
	m_pluginInstance = instance;
	if(m_pluginInstance) {
		m_name = m_pluginInstance->displayName();
		m_description = m_pluginInstance->description();
	}
}

void PluginInfo::setErrorMsg(const QString &newErrorMsg) { m_errorMsg = newErrorMsg; }

bool PluginInfo::isLoaded() const { return m_state == PLUGIN_LOADED; }

QString PluginInfo::getErrorMessage() const
{
	if(!m_errorMsg.isEmpty()) {
		return m_errorMsg;
	}

	QString errorMsg = "";
	switch(m_state) {
	case PLUGIN_FILE_NOT_FOUND:
		errorMsg = "File: " + m_filePath + " doesn't exist";
		break;
	case PLUGIN_INVALID_LIBRARY:
		errorMsg = "File: " + m_filePath + " doesn't have a valid suffix for a loadable library";
		break;
	case PLUGIN_LOAD_FAILED:
		errorMsg = "Cannot load library " + m_filePath;
		break;
	case PLUGIN_INSTANCE_FAILED:
		errorMsg = "Cannot create QObject instance from loaded library";
		break;
	case PLUGIN_INVALID_PLUGIN:
		errorMsg = "Loaded library instance is not a Plugin*";
		break;
	case PLUGIN_CLONE_FAILED:
		errorMsg = "Clone method does not clone the object or clone name is empty";
		break;
	default:
		errorMsg = "Unknown error!";
		break;
	}

	return errorMsg;
}
