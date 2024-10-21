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

#include "pqmplugin.h"

#include <QLoggingCategory>
#include <QPushButton>
#include <QVBoxLayout>
#include <acquisitionmanager.h>
#include <harmonicsinstrument.h>
#include <pqmdatalogger.h>
#include <rmsinstrument.h>
#include <settingsinstrument.h>
#include <style.h>
#include <waveforminstrument.h>

#include <pluginbase/preferences.h>
#include <iioutil/connectionprovider.h>
#include <iioutil/iiopingtask.h>

Q_LOGGING_CATEGORY(CAT_PQMPLUGIN, "PQMPlugin");
using namespace scopy::pqm;

void PQMPlugin::preload() {}

bool PQMPlugin::compatible(QString m_param, QString category)
{
	bool ret = false;
	auto &&cp = ConnectionProvider::GetInstance();

	Connection *conn = cp->open(m_param);
	if(!conn) {
		qInfo(CAT_PQMPLUGIN) << "The context is not compatible with the PQMPlugin!";
		return ret;
	}
	iio_device *pqmDevice = iio_context_find_device(conn->context(), "pqm");
	if(pqmDevice) {
		ret = true;
	}
	cp->close(m_param);

	return ret;
}

bool PQMPlugin::loadPage()
{

	m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_infoPage = new InfoPage(m_page);
	m_infoPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_page->layout()->addWidget(m_infoPage);
	m_page->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	auto cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(conn == nullptr)
		return false;
	struct iio_context *context = conn->context();
	ssize_t attributeCount = iio_context_get_attrs_count(context);
	for(int i = 0; i < attributeCount; ++i) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(context, i, &name, &value);
		if(ret < 0) {
			qWarning(CAT_PQMPLUGIN) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();

	return true;
}

bool PQMPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/pqm/pqm_icon.svg");
	return true;
}

void PQMPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("pqmrms", "Rms",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_network_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("pqmharmonics", "Harmonics",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_spectrum_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("pqmwaveform", "Waveform",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_oscilloscope.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("pqmsettings", "Settings",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void PQMPlugin::unload() { delete m_infoPage; }

QString PQMPlugin::description() { return "Adds functionality specific to PQM board"; }

bool PQMPlugin::onConnect()
{
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);

	if(!conn) {
		return false;
	}
	struct iio_context *ctx = conn->context();
	m_pingTask = new IIOPingTask(ctx, this);

	m_acqManager = new AcquisitionManager(ctx, m_pingTask, this);
	bool hasFwVers = m_acqManager->hasFwVers();

	ToolMenuEntry *rmsTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "pqmrms");
	RmsInstrument *rms = new RmsInstrument(rmsTme, m_param);
	rmsTme->setTool(rms);
	rmsTme->setEnabled(true);
	rmsTme->setRunBtnVisible(true);
	connect(m_acqManager, &AcquisitionManager::pqmAttrsAvailable, rms, &RmsInstrument::onAttrAvailable);

	ToolMenuEntry *harmonicsTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "pqmharmonics");
	HarmonicsInstrument *harmonics = new HarmonicsInstrument(harmonicsTme, m_param);
	harmonics->showThdWidget(hasFwVers);
	harmonicsTme->setTool(harmonics);
	harmonicsTme->setEnabled(true);
	harmonicsTme->setRunBtnVisible(true);
	connect(m_acqManager, &AcquisitionManager::pqmAttrsAvailable, harmonics, &HarmonicsInstrument::onAttrAvailable);
	connect(harmonics, &HarmonicsInstrument::logData, m_acqManager, &AcquisitionManager::logData);

	ToolMenuEntry *waveformTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "pqmwaveform");
	WaveformInstrument *waveform = new WaveformInstrument(waveformTme, m_param);
	waveform->showOneBuffer(hasFwVers);
	waveformTme->setTool(waveform);
	waveformTme->setEnabled(true);
	waveformTme->setRunBtnVisible(true);
	connect(m_acqManager, &AcquisitionManager::bufferDataAvailable, waveform,
		&WaveformInstrument::onBufferDataAvailable, Qt::QueuedConnection);
	connect(waveform, &WaveformInstrument::logData, m_acqManager, &AcquisitionManager::logData);

	SettingsInstrument *settings = new SettingsInstrument();
	m_toolList[3]->setTool(settings);
	m_toolList[3]->setEnabled(true);
	m_toolList[3]->setRunBtnVisible(false);
	connect(m_acqManager, &AcquisitionManager::pqmAttrsAvailable, settings,
		&SettingsInstrument::attributeAvailable);
	connect(settings, &SettingsInstrument::setAttributes, m_acqManager, &AcquisitionManager::setConfigAttr);

	for(auto &tool : m_toolList) {
		connect(tool->tool(), SIGNAL(enableTool(bool, QString)), m_acqManager,
			SLOT(toolEnabled(bool, QString)));
	}
	return true;
}

bool PQMPlugin::onDisconnect()
{
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			disconnect(tool);
			disconnect(tool->tool());
			tool->setTool(nullptr);
			delete(w);
		}
	}
	ResourceManager::close("pqm");
	disconnect(m_acqManager);
	delete m_acqManager;
	m_acqManager = nullptr;
	clearPingTask();
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	cp->close(m_param);
	return true;
}

void PQMPlugin::startPingTask() { m_acqManager->startPing(); }

void PQMPlugin::stopPingTask() { m_acqManager->stopPing(); }

void PQMPlugin::onPausePingTask(bool pause)
{
	if(pause) {
		m_acqManager->stopPing();
	} else {
		m_acqManager->startPing();
	}
}

void PQMPlugin::clearPingTask()
{
	if(m_pingTask) {
		m_pingTask->deleteLater();
		m_pingTask = nullptr;
	}
}

void PQMPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":["*"]
	}
)plugin");
}

#include "moc_pqmplugin.cpp"
