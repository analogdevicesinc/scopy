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

#include "extprocplugin.h"

#include <QLoggingCategory>
#include <QLabel>
#include <menusectionwidget.h>
#include "preferenceshelper.h"
#include "extprocinstrument.h"
#include <iioutil/connectionprovider.h>
#include <controller/jsonformat.h>
#include <pluginbase/preferences.h>
#include "scopy-extprocplugin_config.h"

Q_LOGGING_CATEGORY(CAT_EXTPROCPLUGIN, "ExtProcPlugin")
using namespace scopy::extprocplugin;

bool ExtProcPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_EXTPROCPLUGIN) << "compatible";
	bool ret = false;
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr) {
		return ret;
	}

	for(int i = 0; i < iio_context_get_devices_count(conn->context()); i++) {
		iio_device *dev = iio_context_get_device(conn->context(), i);
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {
			struct iio_channel *chn = iio_device_get_channel(dev, j);
			if(!iio_channel_is_output(chn) && iio_channel_is_scan_element(chn)) {
				ret = true;
				goto finish;
			}
		}
	}

finish:
	ConnectionProvider::GetInstance()->close(m_param);
	return ret;
}

bool ExtProcPlugin::loadPage()
{
	// Here you must write the code for the plugin info page
	// Below is an example for an iio device
	/*m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_infoPage = new InfoPage(m_page);
	m_infoPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_page->layout()->addWidget(m_infoPage);
	m_page->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	auto cp = ContextProvider::GetInstance();
	struct iio_context *context = cp->open(m_param);
	ssize_t attributeCount = iio_context_get_attrs_count(context);
	for(int i = 0; i < attributeCount; ++i) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(context, i, &name, &value);
		if(ret < 0) {
			qWarning(CAT_EXTPROCPLUGIN) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();
	return true;
	*/
	return false;
}

void ExtProcPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("ext_cli_path", "");
}

bool ExtProcPlugin::loadPreferencesPage()
{
	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_preferencesPage);

	MenuSectionWidget *generalWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *generalSection = new MenuCollapseSection(
		"General", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, generalWidget);
	generalWidget->contentLayout()->setSpacing(10);
	generalWidget->contentLayout()->addWidget(generalSection);
	generalSection->contentLayout()->setSpacing(10);
	lay->setMargin(0);
	lay->addWidget(generalWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	generalSection->contentLayout()->addWidget(PREFERENCE_FILE_BROWSER(
		p, "ext_cli_path", "CLI path", "Select the directory for the external processing tool.",
		FileBrowserWidget::DIRECTORY, generalSection));

	return true;
}

bool ExtProcPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void ExtProcPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("extprocinstrument", "External Process",
						  ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
}

void ExtProcPlugin::unload()
{ /*delete m_infoPage;*/
}

QString ExtProcPlugin::description() { return EXTPROCPLUGIN_PLUGIN_DESCRIPTION; }

bool ExtProcPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);

	m_iioManager = new IIOManager(conn->context());

	ToolMenuEntry *tme = m_toolList[0];
	ExtProcInstrument *extInstrument = new ExtProcInstrument(tme);
	tme->setTool(extInstrument);
	tme->setEnabled(true);
	tme->setRunBtnVisible(true);
	extInstrument->setAvailableChannels(m_iioManager->getAvailableChannels());

	// The format isn't necessary to be declared here
	CommandFormat *cmdFormat = new JsonFormat();
	m_cmdController = new CMDController(cmdFormat);

	// connect(m_iioManager, &IIOManager::inputFormatChanged, extInstrument,
	// &ExtProcInstrument::onInputFormatChanged);
	connect(m_iioManager, &IIOManager::dataReady, extInstrument, &ExtProcInstrument::onBufferDataReady);

	// input config
	connect(m_iioManager, &IIOManager::inputFormatChanged, m_cmdController, &CMDController::configureInput);
	connect(m_cmdController, &CMDController::inputConfigured, extInstrument,
		&ExtProcInstrument::onInputFormatChanged);
	// analysis config
	connect(extInstrument, &ExtProcInstrument::analysisConfigChanged, m_cmdController,
		&CMDController::configureAnalysis);
	connect(m_cmdController, &CMDController::analysisConfigured, extInstrument,
		&ExtProcInstrument::onAnalysisConfigured);
	// output config
	connect(extInstrument, &ExtProcInstrument::outputConfigured, m_cmdController, &CMDController::configureOutput);
	connect(m_cmdController, &CMDController::outputConfigured, extInstrument, &ExtProcInstrument::onOutputConfig);
	// run
	connect(m_iioManager, &IIOManager::dataReady, m_cmdController, &CMDController::runAnalysis);
	connect(m_cmdController, &CMDController::processDataCompleted, extInstrument,
		&ExtProcInstrument::onRunResponse);
	// analysis types
	connect(m_cmdController, &CMDController::analysisTypesReceived, extInstrument,
		&ExtProcInstrument::onAnalysisTypes);
	// analysis info
	connect(m_cmdController, &CMDController::analysisInfo, extInstrument, &ExtProcInstrument::onAnalysisInfo);
	connect(extInstrument, &ExtProcInstrument::requestAnalysisInfo, m_cmdController,
		&CMDController::getAnalysisInfo);

	connect(m_cmdController, &CMDController::processFinished, extInstrument, &ExtProcInstrument::onProcessFinished);

	connect(extInstrument, &ExtProcInstrument::bufferParamsChanged, m_iioManager,
		&IIOManager::onBufferParamsChanged);
	connect(extInstrument, &ExtProcInstrument::runPressed, m_iioManager, &IIOManager::startAcq);
	connect(extInstrument, &ExtProcInstrument::requestNewData, m_iioManager, &IIOManager::onDataRequest);

	m_cmdController->getAnalysisTypes();

	return true;
}

bool ExtProcPlugin::onDisconnect()
{
	// This method is called when the disconnect button is pressed
	// It must remove all connections that were established on the connection
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			tool->setTool(nullptr);
			delete(w);
		}
	}

	if(m_cmdController) {
		delete m_cmdController;
		m_cmdController = nullptr;
	}
	if(m_iioManager) {
		m_iioManager->deleteLater();
		m_iioManager = nullptr;
	}

	ConnectionProvider::GetInstance()->close(m_param);

	return true;
}

void ExtProcPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":10,
	   "category":[
	      "iio"
	   ]
	}
)plugin");
}
