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

#include "ad9084_api.h"
#include "ad9084plugin.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_AD9084_API, "AD9084_API")

using namespace scopy::ad9084;

AD9084_API::AD9084_API(AD9084Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

AD9084_API::~AD9084_API() {}

// --- Private helpers ---

QString AD9084_API::rxKey(int channel, const QString &attr)
{
	return "axi-ad9084-rx-hpc/voltage" + QString::number(channel) + "_i_in/" + attr;
}

QString AD9084_API::txKey(int channel, const QString &attr)
{
	return "axi-ad9084-rx-hpc/voltage" + QString::number(channel) + "_i_out/" + attr;
}

QString AD9084_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_AD9084_API) << "Widget manager not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_AD9084_API) << "Widget not found for key:" << key;
		return QString();
	}

	QPair<QString, QString> result = widget->read();
	return result.first;
}

void AD9084_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_AD9084_API) << "Widget manager not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_AD9084_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

// --- Tool management ---

QStringList AD9084_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- RX channel attributes ---

QString AD9084_API::getRxAdcFrequency(int channel) { return readFromWidget(rxKey(channel, "adc_frequency")); }

void AD9084_API::setRxAdcFrequency(int channel, const QString &value)
{
	writeToWidget(rxKey(channel, "adc_frequency"), value);
}

QString AD9084_API::getRxChannelNcoFrequency(int channel)
{
	return readFromWidget(rxKey(channel, "channel_nco_frequency"));
}

void AD9084_API::setRxChannelNcoFrequency(int channel, const QString &value)
{
	writeToWidget(rxKey(channel, "channel_nco_frequency"), value);
}

QString AD9084_API::getRxChannelNcoPhase(int channel) { return readFromWidget(rxKey(channel, "channel_nco_phase")); }

void AD9084_API::setRxChannelNcoPhase(int channel, const QString &value)
{
	writeToWidget(rxKey(channel, "channel_nco_phase"), value);
}

QString AD9084_API::getRxMainNcoFrequency(int channel) { return readFromWidget(rxKey(channel, "main_nco_frequency")); }

void AD9084_API::setRxMainNcoFrequency(int channel, const QString &value)
{
	writeToWidget(rxKey(channel, "main_nco_frequency"), value);
}

QString AD9084_API::getRxMainNcoPhase(int channel) { return readFromWidget(rxKey(channel, "main_nco_phase")); }

void AD9084_API::setRxMainNcoPhase(int channel, const QString &value)
{
	writeToWidget(rxKey(channel, "main_nco_phase"), value);
}

QString AD9084_API::getRxTestMode(int channel) { return readFromWidget(rxKey(channel, "test_mode")); }

void AD9084_API::setRxTestMode(int channel, const QString &value) { writeToWidget(rxKey(channel, "test_mode"), value); }

QString AD9084_API::getRxNyquistZone(int channel) { return readFromWidget(rxKey(channel, "nyquist_zone")); }

void AD9084_API::setRxNyquistZone(int channel, const QString &value)
{
	writeToWidget(rxKey(channel, "nyquist_zone"), value);
}

QString AD9084_API::getRxLoopback(int channel) { return readFromWidget(rxKey(channel, "loopback")); }

void AD9084_API::setRxLoopback(int channel, const QString &value) { writeToWidget(rxKey(channel, "loopback"), value); }

QString AD9084_API::isRxCfirEnabled(int channel) { return readFromWidget(rxKey(channel, "cfir_en")); }

void AD9084_API::setRxCfirEnabled(int channel, const QString &value)
{
	writeToWidget(rxKey(channel, "cfir_en"), value);
}

QString AD9084_API::getRxCfirProfileSel(int channel) { return readFromWidget(rxKey(channel, "cfir_profile_sel")); }

void AD9084_API::setRxCfirProfileSel(int channel, const QString &value)
{
	writeToWidget(rxKey(channel, "cfir_profile_sel"), value);
}

QString AD9084_API::isRxEnabled(int channel) { return readFromWidget(rxKey(channel, "en")); }

void AD9084_API::setRxEnabled(int channel, const QString &value) { writeToWidget(rxKey(channel, "en"), value); }

// --- TX channel attributes ---

QString AD9084_API::getTxDacFrequency(int channel) { return readFromWidget(txKey(channel, "dac_frequency")); }

void AD9084_API::setTxDacFrequency(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "dac_frequency"), value);
}

QString AD9084_API::getTxChannelNcoFrequency(int channel)
{
	return readFromWidget(txKey(channel, "channel_nco_frequency"));
}

void AD9084_API::setTxChannelNcoFrequency(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "channel_nco_frequency"), value);
}

QString AD9084_API::getTxChannelNcoPhase(int channel) { return readFromWidget(txKey(channel, "channel_nco_phase")); }

void AD9084_API::setTxChannelNcoPhase(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "channel_nco_phase"), value);
}

QString AD9084_API::getTxMainNcoFrequency(int channel) { return readFromWidget(txKey(channel, "main_nco_frequency")); }

void AD9084_API::setTxMainNcoFrequency(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "main_nco_frequency"), value);
}

QString AD9084_API::getTxMainNcoPhase(int channel) { return readFromWidget(txKey(channel, "main_nco_phase")); }

void AD9084_API::setTxMainNcoPhase(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "main_nco_phase"), value);
}

QString AD9084_API::getTxChannelNcoGainScale(int channel)
{
	return readFromWidget(txKey(channel, "channel_nco_gain_scale"));
}

void AD9084_API::setTxChannelNcoGainScale(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "channel_nco_gain_scale"), value);
}

QString AD9084_API::getTxChannelNcoTestToneScale(int channel)
{
	return readFromWidget(txKey(channel, "channel_nco_test_tone_scale"));
}

void AD9084_API::setTxChannelNcoTestToneScale(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "channel_nco_test_tone_scale"), value);
}

QString AD9084_API::isTxChannelNcoTestToneEnabled(int channel)
{
	return readFromWidget(txKey(channel, "channel_nco_test_tone_en"));
}

void AD9084_API::setTxChannelNcoTestToneEnabled(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "channel_nco_test_tone_en"), value);
}

QString AD9084_API::getTxMainNcoTestToneScale(int channel)
{
	return readFromWidget(txKey(channel, "main_nco_test_tone_scale"));
}

void AD9084_API::setTxMainNcoTestToneScale(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "main_nco_test_tone_scale"), value);
}

QString AD9084_API::isTxMainNcoTestToneEnabled(int channel)
{
	return readFromWidget(txKey(channel, "main_nco_test_tone_en"));
}

void AD9084_API::setTxMainNcoTestToneEnabled(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "main_nco_test_tone_en"), value);
}

QString AD9084_API::isTxCfirEnabled(int channel) { return readFromWidget(txKey(channel, "cfir_en")); }

void AD9084_API::setTxCfirEnabled(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "cfir_en"), value);
}

QString AD9084_API::getTxCfirProfileSel(int channel) { return readFromWidget(txKey(channel, "cfir_profile_sel")); }

void AD9084_API::setTxCfirProfileSel(int channel, const QString &value)
{
	writeToWidget(txKey(channel, "cfir_profile_sel"), value);
}

QString AD9084_API::isTxEnabled(int channel) { return readFromWidget(txKey(channel, "en")); }

void AD9084_API::setTxEnabled(int channel, const QString &value) { writeToWidget(txKey(channel, "en"), value); }

// --- Generic widget access ---

QStringList AD9084_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString AD9084_API::readWidget(const QString &key) { return readFromWidget(key); }

void AD9084_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

// --- Utility ---

void AD9084_API::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		return;
	}

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

#include "moc_ad9084_api.cpp"
