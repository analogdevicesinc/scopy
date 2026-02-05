/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "pqm_api.h"
#include "pqmplugin.h"
#include "rmsinstrument.h"
#include "harmonicsinstrument.h"
#include "waveforminstrument.h"
#include "settingsinstrument.h"
#include <pluginbase/toolmenuentry.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/filebrowserwidget.h>
#include <iioutil/connectionprovider.h>
#include <QLineEdit>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_PQM_API, "PQM_API")

using namespace scopy::pqm;

PQM_API::PQM_API(PQMPlugin *pqmPlugin)
	: ApiObject()
	, m_pqmPlugin(pqmPlugin)
{}

PQM_API::~PQM_API() {}

RmsInstrument *PQM_API::getRmsInstrument()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_pqmPlugin->m_toolList, "pqmrms");
	if(tool && tool->tool()) {
		return dynamic_cast<RmsInstrument *>(tool->tool());
	}
	return nullptr;
}

HarmonicsInstrument *PQM_API::getHarmonicsInstrument()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_pqmPlugin->m_toolList, "pqmharmonics");
	if(tool && tool->tool()) {
		return dynamic_cast<HarmonicsInstrument *>(tool->tool());
	}
	return nullptr;
}

WaveformInstrument *PQM_API::getWaveformInstrument()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_pqmPlugin->m_toolList, "pqmwaveform");
	if(tool && tool->tool()) {
		return dynamic_cast<WaveformInstrument *>(tool->tool());
	}
	return nullptr;
}

SettingsInstrument *PQM_API::getSettingsInstrument()
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_pqmPlugin->m_toolList, "pqmsettings");
	if(tool && tool->tool()) {
		return dynamic_cast<SettingsInstrument *>(tool->tool());
	}
	return nullptr;
}

QStringList PQM_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_pqmPlugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// RMS instrument methods
bool PQM_API::isRmsRunning()
{
	RmsInstrument *rms = getRmsInstrument();
	return rms ? rms->m_running : false;
}

void PQM_API::setRmsRunning(bool running)
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_pqmPlugin->m_toolList, "pqmrms");
	if(tool) {
		tool->setRunning(running);
		Q_EMIT tool->runClicked(running);
	}
}

void PQM_API::rmsSingleShot()
{
	RmsInstrument *rms = getRmsInstrument();
	if(rms && rms->m_singleBtn && rms->m_singleBtn->isEnabled()) {
		rms->m_singleBtn->setChecked(true);
	}
}

bool PQM_API::isRmsLoggingEnabled()
{
	RmsInstrument *rms = getRmsInstrument();
	return rms && rms->m_logSection ? !rms->m_logSection->collapsed() : false;
}

void PQM_API::setRmsLoggingEnabled(bool enabled)
{
	RmsInstrument *rms = getRmsInstrument();
	if(rms && !isRmsRunning() && rms->m_logSection) {
		rms->m_logSection->setCollapsed(!enabled);
	}
}

QString PQM_API::getRmsLogPath()
{
	RmsInstrument *rms = getRmsInstrument();
	return rms && rms->m_logFileBrowser ? rms->m_logFileBrowser->lineEdit()->text() : QString();
}

void PQM_API::setRmsLogPath(const QString &path)
{
	RmsInstrument *rms = getRmsInstrument();
	if(rms && rms->m_logFileBrowser) {
		rms->m_logFileBrowser->lineEdit()->setText(path);
	}
}

void PQM_API::resetRmsPqEvents()
{
	RmsInstrument *rms = getRmsInstrument();
	if(rms && rms->m_pqEventsBtn) {
		rms->resetEventsBtn();
		Q_EMIT rms->m_pqEventsBtn->clicked(true);
	}
}

bool PQM_API::isRmsPqEvents()
{
	RmsInstrument *rms = getRmsInstrument();
	if(rms && rms->m_pqEventsBtn) {
		return rms->m_pqEventsBtn->isChecked();
	}
	return false;
}

// Harmonics instrument methods
bool PQM_API::isHarmonicsRunning()
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	return harmonics ? harmonics->m_running : false;
}

void PQM_API::setHarmonicsRunning(bool running)
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_pqmPlugin->m_toolList, "pqmharmonics");
	if(tool) {
		tool->setRunning(running);
		Q_EMIT tool->runClicked(running);
	}
}

void PQM_API::harmonicsSingleShot()
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	if(harmonics && harmonics->m_singleBtn && harmonics->m_singleBtn->isEnabled()) {
		harmonics->m_singleBtn->setChecked(true);
	}
}

QString PQM_API::getHarmonicsActiveChannel()
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	if(harmonics && harmonics->m_table) {
		int currentRow = harmonics->m_table->currentRow();
		if(currentRow >= 0 && currentRow < harmonics->m_chnls.keys().size()) {
			return harmonics->m_chnls.keys().at(currentRow);
		}
	}
	return QString();
}

void PQM_API::setHarmonicsActiveChannel(const QString &channel)
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	if(harmonics && harmonics->m_table) {
		int index = harmonics->m_chnls.keys().indexOf(channel);
		if(index >= 0) {
			harmonics->m_table->selectRow(index);
		}
	}
}

QString PQM_API::getHarmonicsType()
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	return harmonics ? harmonics->m_harmonicsType : QString();
}

void PQM_API::setHarmonicsType(const QString &type)
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	if(harmonics && (type == "harmonics" || type == "inter_harmonics")) {
		harmonics->m_harmonicsType = type;
	}
}

bool PQM_API::isHarmonicsLoggingEnabled()
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	return harmonics && harmonics->m_logSection ? !harmonics->m_logSection->collapsed() : false;
}

void PQM_API::setHarmonicsLoggingEnabled(bool enabled)
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	if(harmonics && harmonics->m_logSection) {
		harmonics->m_logSection->setCollapsed(!enabled);
	}
}

QString PQM_API::getHarmonicsLogPath()
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	return harmonics && harmonics->m_logFileBrowser ? harmonics->m_logFileBrowser->lineEdit()->text() : QString();
}

void PQM_API::setHarmonicsLogPath(const QString &path)
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	if(harmonics && harmonics->m_logFileBrowser) {
		harmonics->m_logFileBrowser->lineEdit()->setText(path);
	}
}

void PQM_API::resetHarmonicsPqEvents()
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	if(harmonics && harmonics->m_pqEventsBtn) {
		harmonics->resetEventsBtn();
		Q_EMIT harmonics->m_pqEventsBtn->clicked(true);
	}
}

bool PQM_API::isHarmonicsPqEvents()
{
	HarmonicsInstrument *harmonics = getHarmonicsInstrument();
	if(harmonics && harmonics->m_pqEventsBtn) {
		return harmonics->m_pqEventsBtn->isChecked();
	}
	return false;
}

// Waveform instrument methods
bool PQM_API::isWaveformRunning()
{
	WaveformInstrument *waveform = getWaveformInstrument();
	return waveform ? waveform->m_running : false;
}

void PQM_API::setWaveformRunning(bool running)
{
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryById(m_pqmPlugin->m_toolList, "pqmwaveform");
	if(tool) {
		tool->setRunning(running);
		Q_EMIT tool->runClicked(running);
	}
}

void PQM_API::waveformSingleShot()
{
	WaveformInstrument *waveform = getWaveformInstrument();
	if(waveform && waveform->m_singleBtn && waveform->m_singleBtn->isEnabled()) {
		waveform->m_singleBtn->setChecked(true);
	}
}

double PQM_API::getWaveformTimespan()
{
	WaveformInstrument *waveform = getWaveformInstrument();
	return waveform && waveform->m_timespanSpin ? waveform->m_timespanSpin->value() : 1.0;
}

void PQM_API::setWaveformTimespan(double value)
{
	WaveformInstrument *waveform = getWaveformInstrument();
	if(waveform && waveform->m_timespanSpin) {
		waveform->m_timespanSpin->setValue(value);
	}
}

bool PQM_API::isWaveformRollingMode()
{
	WaveformInstrument *waveform = getWaveformInstrument();
	return waveform && waveform->m_rollingModeSwitch ? waveform->m_rollingModeSwitch->onOffswitch()->isChecked()
							 : false;
}

void PQM_API::setWaveformRollingMode(bool enabled)
{
	WaveformInstrument *waveform = getWaveformInstrument();
	if(waveform && waveform->m_rollingModeSwitch) {
		waveform->m_rollingModeSwitch->onOffswitch()->setChecked(enabled);
	}
}

QString PQM_API::getWaveformTriggeredBy()
{
	WaveformInstrument *waveform = getWaveformInstrument();
	return waveform && waveform->m_triggeredBy ? waveform->m_triggeredBy->combo()->currentText() : QString();
}

void PQM_API::setWaveformTriggeredBy(const QString &channel)
{
	WaveformInstrument *waveform = getWaveformInstrument();
	if(waveform && waveform->m_triggeredBy) {
		int index = waveform->m_triggeredBy->combo()->findText(channel);
		if(index >= 0) {
			waveform->m_triggeredBy->combo()->setCurrentIndex(index);
		}
	}
}

bool PQM_API::isWaveformLoggingEnabled()
{
	WaveformInstrument *waveform = getWaveformInstrument();
	return waveform && waveform->m_logSection ? !waveform->m_logSection->collapsed() : false;
}

void PQM_API::setWaveformLoggingEnabled(bool enabled)
{
	WaveformInstrument *waveform = getWaveformInstrument();
	if(waveform && waveform->m_logSection) {
		waveform->m_logSection->setCollapsed(!enabled);
	}
}

QString PQM_API::getWaveformLogPath()
{
	WaveformInstrument *waveform = getWaveformInstrument();
	return waveform && waveform->m_logFileBrowser ? waveform->m_logFileBrowser->lineEdit()->text() : QString();
}

void PQM_API::setWaveformLogPath(const QString &path)
{
	WaveformInstrument *waveform = getWaveformInstrument();
	if(waveform && waveform->m_logFileBrowser) {
		waveform->m_logFileBrowser->lineEdit()->setText(path);
	}
}

// Settings instrument methods
void PQM_API::settingsRead()
{
	SettingsInstrument *settings = getSettingsInstrument();
	if(settings) {
		settings->m_readEnabled = true;
		Q_EMIT settings->enableTool(true);
	}
}

void PQM_API::settingsWrite()
{
	SettingsInstrument *settings = getSettingsInstrument();
	if(settings) {
		Q_EMIT settings->setAttributes(settings->m_pqmAttr);
	}
}

QString PQM_API::getSettingsAttributeValue(const QString &attrName)
{
	SettingsInstrument *settings = getSettingsInstrument();
	if(settings && settings->m_pqmAttr[DEVICE_NAME].contains(attrName)) {
		return settings->m_pqmAttr[DEVICE_NAME][attrName];
	}
	return QString();
}

void PQM_API::setSettingsAttributeValue(const QString &attrName, const QString &value)
{
	SettingsInstrument *settings = getSettingsInstrument();
	if(settings) {
		settings->m_pqmAttr[DEVICE_NAME][attrName] = value;
	}
}

bool PQM_API::triggerPqEvent(bool enable)
{
	Connection *conn = ConnectionProvider::GetInstance()->open(m_pqmPlugin->m_param);
	if(!conn) {
		qWarning(CAT_PQM_API) << "Failed to open connection for PQ event trigger";
		return false;
	}

	iio_device *pqmDevice = iio_context_find_device(conn->context(), "pqm");
	if(!pqmDevice) {
		qWarning(CAT_PQM_API) << "PQM device not found";
		return false;
	}

	iio_channel *countChannel = iio_device_find_channel(pqmDevice, "count0", true);
	if(!countChannel) {
		qWarning(CAT_PQM_API) << "dips channel not found";
		return false;
	}

	const char *value = enable ? "1" : "0";
	int ret = iio_channel_attr_write(countChannel, "countEvent", value);
	if(ret < 0) {
		qWarning(CAT_PQM_API) << "Failed to write countEvent attribute:" << ret;
		return false;
	}

	qInfo(CAT_PQM_API) << "PQ event triggered:" << (enable ? "enabled" : "disabled");
	return true;
}

QString PQM_API::getLogFilePath()
{
	return m_pqmPlugin->m_acqManager ? m_pqmPlugin->m_acqManager->getLogFilePath() : QString();
}

#include "moc_pqm_api.cpp"
