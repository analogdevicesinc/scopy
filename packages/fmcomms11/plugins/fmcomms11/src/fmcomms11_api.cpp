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

#include "fmcomms11_api.h"
#include "fmcomms11plugin.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_FMCOMMS11_API, "FMCOMMS11_API")

using namespace scopy::fmcomms11;

Fmcomms11_API::Fmcomms11_API(Fmcomms11Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

Fmcomms11_API::~Fmcomms11_API() {}

// --- Private helpers ---

QString Fmcomms11_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_FMCOMMS11_API) << "Widget group not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_FMCOMMS11_API) << "Widget not found for key:" << key;
		return QString();
	}

	QPair<QString, QString> result = widget->read();
	return result.first;
}

void Fmcomms11_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_FMCOMMS11_API) << "Widget group not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_FMCOMMS11_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

QString Fmcomms11_API::ncoKey()
{
	// altvoltage4_out is the primary NCO channel; fall back to altvoltage2_out
	const QString primary = "axi-ad9162-hpc/altvoltage4_out/frequency_nco";
	const QString fallback = "axi-ad9162-hpc/altvoltage2_out/frequency_nco";

	if(m_plugin->m_widgetGroup && m_plugin->m_widgetGroup->get(primary)) {
		return primary;
	}
	return fallback;
}

// --- Tool management ---

QStringList Fmcomms11_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- ADC attributes ---

QString Fmcomms11_API::getAdcSamplingFrequency()
{
	return readFromWidget("axi-ad9625-hpc/voltage0_in/sampling_frequency");
}

QString Fmcomms11_API::getAdcScale() { return readFromWidget("axi-ad9625-hpc/voltage0_in/scale"); }

void Fmcomms11_API::setAdcScale(const QString &value) { writeToWidget("axi-ad9625-hpc/voltage0_in/scale", value); }

QString Fmcomms11_API::getAdcTestMode() { return readFromWidget("axi-ad9625-hpc/voltage0_in/test_mode"); }

void Fmcomms11_API::setAdcTestMode(const QString &value)
{
	writeToWidget("axi-ad9625-hpc/voltage0_in/test_mode", value);
}

// --- Input attenuator (HMC1119) ---

QString Fmcomms11_API::getInputAttenuation()
{
	return readFromWidget("hmc1119/voltage0_out/hardwaregain").split(" ").first();
}

void Fmcomms11_API::setInputAttenuation(const QString &value)
{
	writeToWidget("hmc1119/voltage0_out/hardwaregain", value);
}

// --- DAC attributes ---

QString Fmcomms11_API::getDacSamplingFrequency()
{
	return readFromWidget("axi-ad9162-hpc/altvoltage0_out/sampling_frequency");
}

QString Fmcomms11_API::getNcoFrequency()
{
	QString raw = readFromWidget(ncoKey());
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

void Fmcomms11_API::setNcoFrequency(const QString &value)
{
	writeToWidget(ncoKey(), QString::number(value.toDouble() * 1e6, 'f', 0));
}

QString Fmcomms11_API::isFir85Enabled() { return readFromWidget("axi-ad9162-hpc/fir85_enable"); }

void Fmcomms11_API::setFir85Enabled(const QString &value) { writeToWidget("axi-ad9162-hpc/fir85_enable", value); }

// --- Output amplifier (ADL5240) ---

QString Fmcomms11_API::getOutputGain()
{
	return readFromWidget("adl5240/voltage0_out/hardwaregain").split(" ").first();
}

void Fmcomms11_API::setOutputGain(const QString &value) { writeToWidget("adl5240/voltage0_out/hardwaregain", value); }

// --- Generic widget access ---

QStringList Fmcomms11_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString Fmcomms11_API::readWidget(const QString &key) { return readFromWidget(key); }

void Fmcomms11_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

// --- Utility ---

void Fmcomms11_API::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		return;
	}

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

#include "moc_fmcomms11_api.cpp"
