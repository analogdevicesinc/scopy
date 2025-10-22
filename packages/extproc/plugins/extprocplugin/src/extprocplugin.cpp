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

Q_LOGGING_CATEGORY(CAT_QIQPLUGIN, "ExtProcPlugin")
using namespace scopy::extprocplugin;

bool ExtProcPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_QIQPLUGIN) << "compatible";
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

void ExtProcPlugin::preload()
{
	QString activationCode = Preferences::get("qiq_code").toString();
	CommandFormat *cmdFormat = new JsonFormat();
	m_qiqController = new CMDController(cmdFormat, this);
	m_qiqController->setActivationCode(activationCode);
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
			qWarning(CAT_QIQPLUGIN) << "Could not read attribute with index:" << i;
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
	p->init("qiq_cli_path", "");
	p->init("qiq_code", "");
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

	generalSection->contentLayout()->addWidget(PREFERENCE_EDIT_VALIDATION(
		p, "qiq_code", "Client code", "Insert the cli client code.",
		[](const QString &input) {
			QString normalized = input.toUpper();
			static const QRegularExpression pattern("^[A-Z0-9]{4}(?:-[A-Z0-9]{4}){3}$");
			return pattern.match(normalized).hasMatch();
		},
		generalSection));

	generalSection->contentLayout()->addWidget(PREFERENCE_FILE_BROWSER(
		p, "qiq_cli_path", "CLI path", "Select the directory for the external processing tool.",
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
{
	if(m_qiqController) {
		delete m_qiqController;
		m_qiqController = nullptr;
	}
}

QString ExtProcPlugin::description() { return EXTPROCPLUGIN_PLUGIN_DESCRIPTION; }

bool ExtProcPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(!conn) {
		return false;
	}

	// Create unified acquisition manager
	m_acquisitionManager = new AcquisitionManager(conn->context());
	m_acquisitionManager->setActiveSource(AcquisitionManager::IIO_DEVICE);

	ToolMenuEntry *tme = m_toolList[0];
	ExtProcInstrument *extInstrument = new ExtProcInstrument(tme);
	tme->setTool(extInstrument);
	tme->setEnabled(true);
	tme->setRunBtnVisible(true);
	extInstrument->setAvailableChannels(m_acquisitionManager->getAvailableChannels());
	extInstrument->setInactive(!m_qiqController->isActivated());

	// Connect unified acquisition manager signals
	connect(m_acquisitionManager, &AcquisitionManager::dataReady, extInstrument,
		&ExtProcInstrument::onBufferDataReady);
	connect(m_acquisitionManager, &AcquisitionManager::inputFormatChanged, m_qiqController,
		&CMDController::configureInput);

	connect(m_qiqController, &CMDController::inputConfigured, extInstrument,
		&ExtProcInstrument::onInputFormatChanged);
	// analysis config
	connect(extInstrument, &ExtProcInstrument::analysisConfigChanged, m_qiqController,
		&CMDController::configureAnalysis);
	connect(m_qiqController, &CMDController::analysisConfigured, extInstrument,
		&ExtProcInstrument::onAnalysisConfigured);
	// output config
	connect(extInstrument, &ExtProcInstrument::outputConfigured, m_qiqController, &CMDController::configureOutput);
	connect(m_qiqController, &CMDController::outputConfigured, extInstrument, &ExtProcInstrument::onOutputConfig);
	// run
	connect(m_acquisitionManager, &AcquisitionManager::dataReady, m_qiqController, &CMDController::runAnalysis);
	connect(m_qiqController, &CMDController::processDataCompleted, extInstrument,
		&ExtProcInstrument::onRunResponse);
	// analysis types
	connect(m_qiqController, &CMDController::analysisTypesReceived, extInstrument,
		&ExtProcInstrument::onAnalysisTypes);
	// analysis info
	connect(m_qiqController, &CMDController::analysisInfo, extInstrument, &ExtProcInstrument::onAnalysisInfo);
	connect(extInstrument, &ExtProcInstrument::requestAnalysisInfo, m_qiqController,
		&CMDController::getAnalysisInfo);

	connect(m_qiqController, &CMDController::processFinished, extInstrument, &ExtProcInstrument::onProcessFinished);

	// Connect control signals to unified acquisition manager
	connect(extInstrument, &ExtProcInstrument::bufferParamsChanged, m_acquisitionManager,
		&AcquisitionManager::onBufferParamsChanged);
	connect(extInstrument, &ExtProcInstrument::runPressed, m_acquisitionManager,
		&AcquisitionManager::startAcquisition);
	connect(extInstrument, &ExtProcInstrument::requestNewData, m_acquisitionManager,
		&AcquisitionManager::onDataRequest);
	connect(extInstrument, &ExtProcInstrument::acqFileSelected, this, [this, extInstrument](const QString &path) {
		bool fileValid = false;
		if(!path.isEmpty() && QFile::exists(path)) {
			m_acquisitionManager->setActiveSource(AcquisitionManager::IQ_FILE);
			fileValid = m_acquisitionManager->getFileReader()->openFile(path);
		}
		if(!fileValid) {
			m_acquisitionManager->setActiveSource(AcquisitionManager::IIO_DEVICE);
		}

		extInstrument->setAvailableChannels(m_acquisitionManager->getAvailableChannels());
		Q_EMIT extInstrument->acqFileCheck(fileValid);
	});
	m_qiqController->getAnalysisTypes();

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
	m_acquisitionManager->deleteLater();
	m_acquisitionManager = nullptr;

	QObject::disconnect(m_qiqController);

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
