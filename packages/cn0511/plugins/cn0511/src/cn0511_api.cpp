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

#include "cn0511_api.h"
#include "cn0511plugin.h"
#include "cn0511.h"

#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_CN0511_API, "Cn0511_API")

using namespace scopy::cn0511;

Cn0511_API::Cn0511_API(Cn0511Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

Cn0511_API::~Cn0511_API() {}

CN0511 *Cn0511_API::getTool()
{
	ToolMenuEntry *entry = ToolMenuEntry::findToolMenuEntryById(m_plugin->m_toolList, "cn0511Tool");
	if(entry && entry->tool()) {
		return dynamic_cast<CN0511 *>(entry->tool());
	}
	return nullptr;
}

QString Cn0511_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_CN0511_API) << "Widget group not available";
		return QString();
	}
	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_CN0511_API) << "Widget not found for key:" << key;
		return QString();
	}
	return widget->read().first;
}

void Cn0511_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_CN0511_API) << "Widget group not available";
		return;
	}
	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_CN0511_API) << "Widget not found for key:" << key;
		return;
	}
	widget->writeAsync(value);
}

QStringList Cn0511_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

QString Cn0511_API::getFrequency()
{
	CN0511 *tool = getTool();
	if(!tool || !tool->m_freqSpinBox) {
		qWarning(CAT_CN0511_API) << "CN0511 tool not available";
		return QString();
	}
	return QString::number(tool->m_freqSpinBox->value());
}

void Cn0511_API::setFrequency(const QString &val)
{
	CN0511 *tool = getTool();
	if(!tool || !tool->m_freqSpinBox) {
		qWarning(CAT_CN0511_API) << "CN0511 tool not available";
		return;
	}
	tool->m_freqSpinBox->setValue(val.toDouble());
}

QString Cn0511_API::getAmplitude()
{
	CN0511 *tool = getTool();
	if(!tool || !tool->m_ampSpinBox) {
		qWarning(CAT_CN0511_API) << "CN0511 tool not available";
		return QString();
	}
	return QString::number(tool->m_ampSpinBox->value());
}

void Cn0511_API::setAmplitude(const QString &val)
{
	CN0511 *tool = getTool();
	if(!tool || !tool->m_ampSpinBox) {
		qWarning(CAT_CN0511_API) << "CN0511 tool not available";
		return;
	}
	tool->m_ampSpinBox->setValue(val.toDouble());
}

void Cn0511_API::applyCalibration()
{
	CN0511 *tool = getTool();
	if(!tool) {
		qWarning(CAT_CN0511_API) << "CN0511 tool not available";
		return;
	}
	tool->applyCalibration();
}

QString Cn0511_API::isDacAmpEnabled() { return readFromWidget("ad9166-amp/en"); }

void Cn0511_API::setDacAmpEnabled(const QString &val) { writeToWidget("ad9166-amp/en", val); }

void Cn0511_API::refresh()
{
	CN0511 *tool = getTool();
	if(!tool || !tool->m_refreshButton) {
		qWarning(CAT_CN0511_API) << "CN0511 tool not available";
		return;
	}
	Q_EMIT tool->m_refreshButton->clicked();
}

#include "moc_cn0511_api.cpp"
