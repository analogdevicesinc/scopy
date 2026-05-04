/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "daq2_api.h"
#include "daq2plugin.h"

#include <QLoggingCategory>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <pluginbase/toolmenuentry.h>

Q_LOGGING_CATEGORY(CAT_DAQ2_API, "Daq2_API")

using namespace scopy::daq2;

static const QString ADC_CH0_TEST_MODE_KEY = "axi-ad9680-hpc/voltage0_in/test_mode";
static const QString ADC_CH1_TEST_MODE_KEY = "axi-ad9680-hpc/voltage1_in/test_mode";

static const QStringList AD9680_TEST_MODES = {"off",	       "midscale_short",  "pos_fullscale",
					      "neg_fullscale", "checkerboard",	  "pn9",
					      "pn23",	       "one_zero_toggle", "ramp"};

Daq2_API::Daq2_API(Daq2Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

Daq2_API::~Daq2_API() {}

QStringList Daq2_API::getTools()
{
	QStringList tools;
	if(!m_plugin) {
		qWarning(CAT_DAQ2_API) << "Plugin not available";
		return tools;
	}
	for(auto *entry : m_plugin->m_toolList) {
		tools.append(entry->id());
	}
	return tools;
}

QString Daq2_API::getAdcCh0TestMode() { return readFromWidget(ADC_CH0_TEST_MODE_KEY); }

void Daq2_API::setAdcCh0TestMode(const QString &mode)
{
	if(!AD9680_TEST_MODES.contains(mode)) {
		qWarning(CAT_DAQ2_API) << "Invalid test_mode:" << mode << "Valid:" << AD9680_TEST_MODES;
		return;
	}
	writeToWidget(ADC_CH0_TEST_MODE_KEY, mode);
}

QString Daq2_API::getAdcCh1TestMode() { return readFromWidget(ADC_CH1_TEST_MODE_KEY); }

void Daq2_API::setAdcCh1TestMode(const QString &mode)
{
	if(!AD9680_TEST_MODES.contains(mode)) {
		qWarning(CAT_DAQ2_API) << "Invalid test_mode:" << mode << "Valid:" << AD9680_TEST_MODES;
		return;
	}
	writeToWidget(ADC_CH1_TEST_MODE_KEY, mode);
}

QStringList Daq2_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_DAQ2_API) << "Widget group not available";
		return {};
	}
	return m_plugin->m_widgetGroup->keys();
}

QString Daq2_API::readWidget(const QString &key) { return readFromWidget(key); }

void Daq2_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

void Daq2_API::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_DAQ2_API) << "Widget group not available";
		return;
	}
	for(const QString &key : m_plugin->m_widgetGroup->keys()) {
		IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
		if(widget)
			widget->readAsync();
	}
}

QString Daq2_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_DAQ2_API) << "Widget group not available";
		return {};
	}
	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_DAQ2_API) << "Widget not found for key:" << key;
		return {};
	}
	return widget->read().first;
}

void Daq2_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_DAQ2_API) << "Widget group not available";
		return;
	}
	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_DAQ2_API) << "Widget not found for key:" << key;
		return;
	}
	widget->writeAsync(value);
}

#include "moc_daq2_api.cpp"
