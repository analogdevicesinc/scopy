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

#include "cn0540plugin.h"
#include "cn0540.h"

#include <QLabel>
#include <QLoggingCategory>
#include <iio.h>

#include <iioutil/connectionprovider.h>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_CN0540PLUGIN, "CN0540Plugin")

using namespace scopy::cn0540;

bool CN0540Plugin::compatible(QString m_param, QString category)
{
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);

	if(!conn) {
		qDebug(CAT_CN0540PLUGIN) << "No context available for CN0540";
		return false;
	}

	iio_context *ctx = conn->context();
	bool ret = iio_context_find_device(ctx, "ad7768-1") && iio_context_find_device(ctx, "ltc2606") &&
		iio_context_find_device(ctx, "one-bit-adc-dac");

	if(ret)
		qDebug(CAT_CN0540PLUGIN) << "Found CN0540 devices";

	cp->close(m_param);
	return ret;
}

bool CN0540Plugin::loadPage() { return false; }

bool CN0540Plugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void CN0540Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(
		"cn0540tool", "CN0540",
		":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/gear_wheel.svg"));
}

void CN0540Plugin::unload() {}

QString CN0540Plugin::description() { return "CN0540 precision measurement system plugin ported from iio-oscilloscope"; }

QString CN0540Plugin::displayName() { return "CN0540"; }

bool CN0540Plugin::onConnect()
{
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);

	if(!conn) {
		qWarning(CAT_CN0540PLUGIN) << "No context available for CN0540";
		return false;
	}

	CN0540 *tool = new CN0540(conn->context());
	m_toolList[0]->setTool(tool);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	return true;
}

bool CN0540Plugin::onDisconnect()
{
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

	auto &&cp = ConnectionProvider::GetInstance();
	cp->close(m_param);
	return true;
}

void CN0540Plugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":[""]
	}
)plugin");
}
