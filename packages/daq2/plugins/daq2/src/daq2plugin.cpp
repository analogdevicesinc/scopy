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

#include "daq2plugin.h"
#include "daq2_api.h"
#include "daq2tool.h"
#include "scopy-daq2_config.h"

#include <QLabel>
#include <QLoggingCategory>
#include <deviceiconbuilder.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <pluginbase/scopyjs.h>
#include <style.h>

#include <component/controller.h>
#include <component/context.h>
#include <component/device.h>
#include <component/backends/iio/iioregisterreader.h>
#include <pkg-manager/pkgmanager.h>

Q_LOGGING_CATEGORY(CAT_DAQ2PLUGIN, "Daq2Plugin")
using namespace scopy::daq2;

namespace {
QString findPackageFile(const QString &dir, const QString &fileName)
{
	const QFileInfoList files = scopy::PkgManager::listFilesInfo({dir}, {fileName});
	if(files.isEmpty()) {
		qWarning(CAT_DAQ2PLUGIN) << "File" << fileName << "not found in" << dir;
		return "";
	}
	return files.first().absoluteFilePath();
}
} // namespace

QList<scopy::RegisterMapToolConfig> Daq2Plugin::registerMapConfiguration(const QVariantMap &query)
{
	Q_UNUSED(query)
	// device name -> this package's template: the converter registers are reached through the HDL device
	const QMap<QString, QString> templates = {{"axi-ad9144-hpc", "ad9144.xml"}, {"axi-ad9680-hpc", "ad9680.xml"}};
	QList<RegisterMapToolConfig> configs;
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return configs;
	}
	for(component::Device *dev : ctx->findChildren<component::Device *>(Qt::FindDirectChildrenOnly)) {
		if(!templates.contains(dev->name()) || !dev->findChild<component::iio::IIORegisterReader *>()) {
			continue;
		}
		RegisterMapToolConfig config;
		config.target = dev->name();
		config.templatePath = findPackageFile("daq2-regmap", templates.value(dev->name()));
		config.access = RegisterMapToolConfig::IIO;
		configs.append(config);
	}
	return configs;
}

bool Daq2Plugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_DAQ2PLUGIN) << "Check DAQ2 compatibility";
	component::ContextHandle ctx = component::Controller::context(m_param);

	if(!ctx) {
		qWarning(CAT_DAQ2PLUGIN) << "No context available for DAQ2";
		return false;
	}

	bool adcFound = ctx->findChild<component::Device *>("axi-ad9680-hpc", Qt::FindDirectChildrenOnly) != nullptr;
	bool dacFound = ctx->findChild<component::Device *>("axi-ad9144-hpc", Qt::FindDirectChildrenOnly) != nullptr;

	return adcFound && dacFound;
}

bool Daq2Plugin::loadPage() { return false; }

bool Daq2Plugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("DAQ2");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();

	return true;
}

void Daq2Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("daq2Tool", "DAQ2",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_debugger.svg"));
}

void Daq2Plugin::unload() {}

QString Daq2Plugin::description() { return "Plugin for DAQ2"; }

QString Daq2Plugin::displayName() { return DAQ2_PLUGIN_DISPLAY_NAME; }

bool Daq2Plugin::onConnect()
{
	m_context = component::Controller::context(m_param);

	if(!m_context) {
		qWarning(CAT_DAQ2PLUGIN) << "No context available for DAQ2";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	DAQ2 *daq2 = new DAQ2(m_context.get(), m_widgetGroup);
	m_toolList[0]->setTool(daq2);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	initApi();
	return true;
}

void Daq2Plugin::initApi()
{
	m_api = new Daq2_API(this);
	m_api->setObjectName("daq2");
	ScopyJS::GetInstance()->registerApi(m_api);
}

bool Daq2Plugin::onDisconnect()
{
	if(m_api) {
		delete m_api;
		m_api = nullptr;
	}

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

	if(m_widgetGroup) {
		delete m_widgetGroup;
		m_widgetGroup = nullptr;
	}

	m_context = {};
	return true;
}

void Daq2Plugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":["m2kplugin"]
	}
)plugin");
}
