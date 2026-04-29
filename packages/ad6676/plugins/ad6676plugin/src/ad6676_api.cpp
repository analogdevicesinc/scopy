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
 */

#include "ad6676_api.h"
#include "ad6676plugin.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>
#include <cmath>

Q_LOGGING_CATEGORY(CAT_AD6676_API, "AD6676_API")

using namespace scopy::ad6676;

AD6676_API::AD6676_API(Ad6676Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

AD6676_API::~AD6676_API() {}

// --- Key helper ---

QString AD6676_API::chnKey(const QString &attr) { return QString("axi-ad6676-hpc/voltage0_in/") + attr; }

// --- Private helpers ---

QString AD6676_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_AD6676_API) << "Widget group not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_AD6676_API) << "Widget not found for key:" << key;
		return QString();
	}

	return widget->read().first;
}

void AD6676_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_AD6676_API) << "Widget group not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_AD6676_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

// --- Tool management ---

QStringList AD6676_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- ADC Settings ---

QString AD6676_API::getAdcFrequency()
{
	QString raw = readFromWidget(chnKey("adc_frequency"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 3);
}
void AD6676_API::setAdcFrequency(const QString &val)
{
	writeToWidget(chnKey("adc_frequency"), QString::number(val.toDouble() * 1e6, 'f', 0));
}

// --- Bandwidth Settings ---

QString AD6676_API::getBandwidth()
{
	QString raw = readFromWidget(chnKey("bandwidth"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 3);
}
void AD6676_API::setBandwidth(const QString &val)
{
	writeToWidget(chnKey("bandwidth"), QString::number(val.toDouble() * 1e6, 'f', 0));
}

QString AD6676_API::getBwMarginLow() { return readFromWidget(chnKey("bw_margin_low")); }
void AD6676_API::setBwMarginLow(const QString &val) { writeToWidget(chnKey("bw_margin_low"), val); }

QString AD6676_API::getBwMarginHigh() { return readFromWidget(chnKey("bw_margin_high")); }
void AD6676_API::setBwMarginHigh(const QString &val) { writeToWidget(chnKey("bw_margin_high"), val); }

QString AD6676_API::getBwMarginIf() { return readFromWidget(chnKey("bw_margin_if")); }
void AD6676_API::setBwMarginIf(const QString &val) { writeToWidget(chnKey("bw_margin_if"), val); }

// --- Receive Settings ---

QString AD6676_API::getIntermediateFrequency()
{
	QString raw = readFromWidget(chnKey("intermediate_frequency"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}
void AD6676_API::setIntermediateFrequency(const QString &val)
{
	writeToWidget(chnKey("intermediate_frequency"), QString::number(val.toDouble() * 1e6, 'f', 0));
}

QString AD6676_API::getSamplingFrequency()
{
	QString raw = readFromWidget(chnKey("sampling_frequency"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 3);
}

QString AD6676_API::getHardwareGain()
{
	QString raw = readFromWidget(chnKey("hardwaregain"));
	// IIO returns e.g. "-10.000000 dB" — strip the unit suffix
	int idx = raw.indexOf(" dB");
	if(idx != -1)
		raw = raw.left(idx).trimmed();
	return raw;
}
void AD6676_API::setHardwareGain(const QString &val) { writeToWidget(chnKey("hardwaregain"), val); }

QString AD6676_API::getScale()
{
	QString raw = readFromWidget(chnKey("scale"));
	if(raw.isEmpty())
		return raw;
	double linear = raw.toDouble();
	if(linear <= 0.0)
		return QString("0");
	int dBFS = static_cast<int>(20.0 * std::log10(1.0 / linear) + 0.5);
	return QString::number(dBFS);
}
void AD6676_API::setScale(const QString &val)
{
	double dBFS = val.toDouble();
	double linear = std::pow(10.0, -dBFS / 20.0);
	writeToWidget(chnKey("scale"), QString::number(linear, 'g', 10));
}

// --- Shuffler Settings ---

QString AD6676_API::getShufflerControl() { return readFromWidget(chnKey("shuffler_control")); }
void AD6676_API::setShufflerControl(const QString &val) { writeToWidget(chnKey("shuffler_control"), val); }

QString AD6676_API::getShufflerThresh() { return readFromWidget(chnKey("shuffler_thresh")); }
void AD6676_API::setShufflerThresh(const QString &val) { writeToWidget(chnKey("shuffler_thresh"), val); }

// --- Test Settings ---

QString AD6676_API::getTestMode() { return readFromWidget(chnKey("test_mode")); }
void AD6676_API::setTestMode(const QString &val) { writeToWidget(chnKey("test_mode"), val); }

// --- Generic widget access ---

QStringList AD6676_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString AD6676_API::readWidget(const QString &key) { return readFromWidget(key); }

void AD6676_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

// --- Utility ---

void AD6676_API::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		return;
	}

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

#include "moc_ad6676_api.cpp"
