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

#include "fmcomms11_api.h"
#include "fmcomms11plugin.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_FMCOMMS11_API, "FMCOMMS11_API")

using namespace scopy::fmcomms11;

FMCOMMS11_API::FMCOMMS11_API(Fmcomms11Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

FMCOMMS11_API::~FMCOMMS11_API() {}

// --- Private helpers ---

QString FMCOMMS11_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_FMCOMMS11_API) << "Widget manager not available";
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

void FMCOMMS11_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_FMCOMMS11_API) << "Widget manager not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_FMCOMMS11_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

// --- Tool management ---

QStringList FMCOMMS11_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- RX: ADC (axi-ad9625-hpc) ---

QString FMCOMMS11_API::getAdcSamplingFrequency()
{
	return readFromWidget("axi-ad9625-hpc/voltage0_in/sampling_frequency");
}

QString FMCOMMS11_API::getAdcScale() { return readFromWidget("axi-ad9625-hpc/voltage0_in/scale"); }

QString FMCOMMS11_API::getTestMode() { return readFromWidget("axi-ad9625-hpc/voltage0_in/test_mode"); }

void FMCOMMS11_API::setTestMode(const QString &value)
{
	writeToWidget("axi-ad9625-hpc/voltage0_in/test_mode", value);
}

// --- RX: VGA (adl5240) ---

QString FMCOMMS11_API::getVgaGain() { return readFromWidget("adl5240/voltage0_out/hardwaregain"); }

void FMCOMMS11_API::setVgaGain(const QString &value) { writeToWidget("adl5240/voltage0_out/hardwaregain", value); }

// --- TX: DAC (axi-ad9162-hpc) ---

QString FMCOMMS11_API::getDacSamplingFrequency()
{
	return readFromWidget("axi-ad9162-hpc/voltage0_i_out/sampling_frequency");
}

QString FMCOMMS11_API::getFir85Enable(const QString &channel)
{
	return readFromWidget("axi-ad9162-hpc/" + channel + "_out/fir85_enable");
}

void FMCOMMS11_API::setFir85Enable(const QString &channel, const QString &value)
{
	writeToWidget("axi-ad9162-hpc/" + channel + "_out/fir85_enable", value);
}

// --- TX: Attenuator (hmc1119) ---

QString FMCOMMS11_API::getAttenuation() { return readFromWidget("hmc1119/voltage0_out/hardwaregain"); }

void FMCOMMS11_API::setAttenuation(const QString &value) { writeToWidget("hmc1119/voltage0_out/hardwaregain", value); }

// --- TX: DDS tones ---

QString FMCOMMS11_API::getDdsFrequency(int tone)
{
	QString chn = QString("altvoltage%1").arg(tone);
	return readFromWidget("axi-ad9162-hpc/" + chn + "_out/frequency");
}

void FMCOMMS11_API::setDdsFrequency(int tone, const QString &value)
{
	QString chn = QString("altvoltage%1").arg(tone);
	writeToWidget("axi-ad9162-hpc/" + chn + "_out/frequency", value);
}

QString FMCOMMS11_API::getDdsPhase(int tone)
{
	QString chn = QString("altvoltage%1").arg(tone);
	return readFromWidget("axi-ad9162-hpc/" + chn + "_out/phase");
}

void FMCOMMS11_API::setDdsPhase(int tone, const QString &value)
{
	QString chn = QString("altvoltage%1").arg(tone);
	writeToWidget("axi-ad9162-hpc/" + chn + "_out/phase", value);
}

QString FMCOMMS11_API::getDdsScale(int tone)
{
	QString chn = QString("altvoltage%1").arg(tone);
	return readFromWidget("axi-ad9162-hpc/" + chn + "_out/scale");
}

void FMCOMMS11_API::setDdsScale(int tone, const QString &value)
{
	QString chn = QString("altvoltage%1").arg(tone);
	writeToWidget("axi-ad9162-hpc/" + chn + "_out/scale", value);
}

QString FMCOMMS11_API::getDdsEnable(int tone)
{
	QString chn = QString("altvoltage%1").arg(tone);
	return readFromWidget("axi-ad9162-hpc/" + chn + "_out/raw");
}

void FMCOMMS11_API::setDdsEnable(int tone, const QString &value)
{
	QString chn = QString("altvoltage%1").arg(tone);
	writeToWidget("axi-ad9162-hpc/" + chn + "_out/raw", value);
}

// --- TX: NCO ---

QString FMCOMMS11_API::getNcoFrequency()
{
	return readFromWidget("axi-ad9162-hpc/altvoltage4_out/frequency_nco");
}

void FMCOMMS11_API::setNcoFrequency(const QString &value)
{
	writeToWidget("axi-ad9162-hpc/altvoltage4_out/frequency_nco", value);
}

// --- LO/PLL (adf4355) ---

QString FMCOMMS11_API::getLoFrequency(int channel)
{
	QString chn = QString("altvoltage%1").arg(channel);
	return readFromWidget("adf4355/" + chn + "_out/frequency");
}

void FMCOMMS11_API::setLoFrequency(int channel, const QString &value)
{
	QString chn = QString("altvoltage%1").arg(channel);
	writeToWidget("adf4355/" + chn + "_out/frequency", value);
}

QString FMCOMMS11_API::getLoPowerdown(int channel)
{
	QString chn = QString("altvoltage%1").arg(channel);
	return readFromWidget("adf4355/" + chn + "_out/powerdown");
}

void FMCOMMS11_API::setLoPowerdown(int channel, const QString &value)
{
	QString chn = QString("altvoltage%1").arg(channel);
	writeToWidget("adf4355/" + chn + "_out/powerdown", value);
}

QString FMCOMMS11_API::getLoRefinFrequency(int channel)
{
	QString chn = QString("altvoltage%1").arg(channel);
	return readFromWidget("adf4355/" + chn + "_out/refin_frequency");
}

// --- Generic widget access ---

QStringList FMCOMMS11_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString FMCOMMS11_API::readWidget(const QString &key) { return readFromWidget(key); }

void FMCOMMS11_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

// --- Utility ---

void FMCOMMS11_API::refresh()
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
