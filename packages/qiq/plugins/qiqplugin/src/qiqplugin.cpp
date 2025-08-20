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

#include "qiqplugin.h"

#include <QLoggingCategory>
#include <QLabel>

#include "qiqinstrument.h"

#include <iioutil/connectionprovider.h>

#include <qiqcontroller/jsonformat.h>

Q_LOGGING_CATEGORY(CAT_QIQPLUGIN, "QIQPlugin")
using namespace scopy::qiqplugin;

bool QIQPlugin::compatible(QString m_param, QString category)
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

bool QIQPlugin::loadPage()
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

bool QIQPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void QIQPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("qiqinstrument", "QIQInstrument",
						  ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
}

void QIQPlugin::unload()
{ /*delete m_infoPage;*/
}

QString QIQPlugin::description() { return "A new plugin"; }

bool QIQPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);

	m_iioManager = new IIOManager(conn->context());

	QIQInstrument *qiqInstrument = new QIQInstrument();
	m_toolList[0]->setTool(qiqInstrument);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);
	qiqInstrument->setAvailableChannels(m_iioManager->getAvailableChannels());

	// The format isn't necessary to be declared here
	CommandFormat *cmdFormat = new JsonFormat();
	m_qiqController = new QIQController(cmdFormat);

	// connect(m_iioManager, &IIOManager::inputFormatChanged, qiqInstrument, &QIQInstrument::onInputFormatChanged);
	connect(m_iioManager, &IIOManager::dataReady, qiqInstrument, &QIQInstrument::bufferDataReady);

	// input config
	connect(m_iioManager, &IIOManager::inputFormatChanged, m_qiqController, &QIQController::configureInput);
	connect(m_qiqController, &QIQController::inputConfigured, qiqInstrument, &QIQInstrument::onInputFormatChanged);
	// analysis config
	connect(qiqInstrument, &QIQInstrument::analysisConfigChanged, m_qiqController,
		&QIQController::configureAnalysis);
	connect(m_qiqController, &QIQController::analysisConfigured, qiqInstrument,
		&QIQInstrument::onAnalysisConfigured);
	// output config
	connect(qiqInstrument, &QIQInstrument::outputConfigured, m_qiqController, &QIQController::configureOutput);
	connect(m_qiqController, &QIQController::outputConfigured, qiqInstrument, &QIQInstrument::onOutputConfig);
	// run
	connect(m_iioManager, &IIOManager::dataReady, m_qiqController, &QIQController::runAnalysis);
	connect(m_qiqController, &QIQController::processDataCompleted, qiqInstrument, &QIQInstrument::onRunResponse);
	// analysis types
	connect(m_qiqController, &QIQController::analysisTypesReceived, qiqInstrument, &QIQInstrument::onAnalysisTypes);
	// analysis info
	connect(m_qiqController, &QIQController::analysisInfo, qiqInstrument, &QIQInstrument::onAnalysisInfo);
	connect(qiqInstrument, &QIQInstrument::requestAnalysisInfo, m_qiqController, &QIQController::getAnalysisInfo);

	connect(qiqInstrument, &QIQInstrument::bufferParamsChanged, m_iioManager, &IIOManager::onBufferParamsChanged);
	connect(qiqInstrument, &QIQInstrument::runPressed, m_iioManager, &IIOManager::startAcq);
	connect(qiqInstrument, &QIQInstrument::requestNewData, m_iioManager, &IIOManager::onDataRequest);

	m_qiqController->getAnalysisTypes();

	return true;
}

bool QIQPlugin::onDisconnect()
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

	m_iioManager->deleteLater();
	m_iioManager = nullptr;

	ConnectionProvider::GetInstance()->close(m_param);

	return true;
}

void QIQPlugin::initMetadata()
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
