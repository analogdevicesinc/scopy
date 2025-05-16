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

#include "admtplugin.h"
#include "admtcontroller.h"
#include "harmoniccalibration.h"

#include <QLabel>
#include <QLoggingCategory>

#include <iioutil/connectionprovider.h>

Q_LOGGING_CATEGORY(CAT_ADMTPLUGIN, "ADMTPlugin")
using namespace scopy;
using namespace scopy::admt;

const bool isDebug = false;

bool ADMTPlugin::compatible(QString m_param, QString category)
{
	m_name = "ADMT4000";
	bool ret = false;
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);

	if(!conn) {
		qWarning(CAT_ADMTPLUGIN) << "No context available for ADMT";
		return false;
	}

	iio_device *admtDevice = iio_context_find_device(conn->context(), "admt4000");
	if(admtDevice) {
		ret = true;
	}

	ConnectionProvider::close(m_param);
	if(isDebug)
		return true;
	return ret;
}

bool ADMTPlugin::loadPage()
{
	m_page = new QWidget();
	return true;
}

bool ADMTPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void ADMTPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("harmoniccalibration", "Harmonic Calibration",
						  ":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
}

void ADMTPlugin::unload()
{
	delete m_page;
	delete m_admtController;
}

QString ADMTPlugin::description() { return "Plugin for ADMT Harmonic Calibration"; }

bool ADMTPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr)
		return false;
	m_ctx = conn->context();
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	m_admtController = new ADMTController(m_param, this);
	m_admtController->connectADMT();
	harmonicCalibration = new HarmonicCalibration(m_admtController, isDebug);
	m_toolList[0]->setTool(harmonicCalibration);

	connect(m_admtController, &ADMTController::requestDisconnect, this, &ADMTPlugin::disconnectDevice,
		Qt::QueuedConnection);

	return true;
}

bool ADMTPlugin::onDisconnect()
{
	// This method is called when the disconnect button is pressed
	// It must remove all connections that were established on the connection

	dynamic_cast<HarmonicCalibration *>(harmonicCalibration)->requestDisconnect();

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

	m_admtController->disconnectADMT();
	return true;
}

void ADMTPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":102,
	   "category":[
	      "iio"
	   ],
	   "exclude":["*", "!debugger"]
	}
)plugin");
}

QString ADMTPlugin::version() { return "0.1"; }

#include "moc_admtplugin.cpp"
