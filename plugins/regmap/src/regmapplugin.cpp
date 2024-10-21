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

#include "jsonformatedelement.hpp"
#include "regmapplugin.h"
#include "utils.hpp"
#include "xmlfilemanager.hpp"

#include "registermaptemplate.hpp"
#include "registermapvalues.hpp"
#include "regmapplugin.h"
#include "xmlfilemanager.hpp"
#include <iio.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QWidget>
#include <QVector>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <registermaptool.hpp>
#include <style.h>
#include <src/readwrite/iioregisterreadstrategy.hpp>
#include <src/readwrite/iioregisterwritestrategy.hpp>
#include <pluginbase/preferences.h>
#include <gui/preferenceshelper.h>
#include <gui/deviceinfopage.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>
#include <readwrite/fileregisterreadstrategy.hpp>
#include <readwrite/fileregisterwritestrategy.hpp>
#include "logging_categories.h"
#include <regmap_api.h>

#include "iioutil/connectionprovider.h"
#include "jsonformatedelement.hpp"
#include "scopy-regmap_config.h"
#include "utils.hpp"
#include "utils.hpp"
#include <pluginbase/scopyjs.h>
#if defined __APPLE__
#include <QApplication>
#endif

using namespace scopy;
using namespace regmap;

bool RegmapPlugin::loadPage()
{
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);

	ConnectionProvider *c = ConnectionProvider::GetInstance();
	Connection *conn = c->open(m_param);
	auto deviceInfoPage = new DeviceInfoPage(conn);
	lay->addWidget(deviceInfoPage);
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));
	c->close(m_param);

	return true;
}

bool RegmapPlugin::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
			      "/icons/RegMap.svg);");
	return true;
}

void RegmapPlugin::loadToolList()
{
	ToolMenuEntry *toolMenuEntry = SCOPY_NEW_TOOLMENUENTRY(
		REGMAP_PLUGIN_SCOPY_MODULE, REGMAP_PLUGIN_DISPLAY_NAME,
		":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/tool_calibration.svg");
	m_toolList.append(toolMenuEntry);
	m_toolList.last()->setRunBtnVisible(true);
	m_toolList.last()->setRunEnabled(false);

	Q_EMIT toolListChanged();
}

void RegmapPlugin::unload()
{
	// TODO
}

bool RegmapPlugin::compatible(QString m_param, QString category)
{
	m_name = REGMAP_PLUGIN_DISPLAY_NAME;
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);

	if(!conn) {
		cp->close(m_param);
		return false;
	} else {
		struct iio_context *ctx = conn->context();
		auto deviceCount = iio_context_get_devices_count(ctx);
		for(int i = 0; i < deviceCount; i++) {
			iio_device *dev = iio_context_get_device(ctx, i);
			if(iio_device_find_debug_attr(dev, "direct_reg_access")) {
				cp->close(m_param);
				return true;
			}
		}
	}
	cp->close(m_param);

	return false;
}

void RegmapPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("regmap_color_by_value", "Default");
#if defined __APPLE__
	p->init("additional_regmap_xml_path", QCoreApplication::applicationDirPath() + "/plugins/xmls");
#else
	p->init("additional_regmap_xml_path", REGMAP_XML_PATH_LOCAL);
#endif
}

bool RegmapPlugin::loadPreferencesPage()
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

	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCombo(
		p, "regmap_color_by_value", "Use color to reflect value",
		{"Default", "Bitfield background", "Bitfield text", "Register background", "Register text",
		 "Register background and Bitfield background", "Register text and Bitfield text",
		 "Register background and Bitfield text", "Register text and Bitfield background"},
		generalSection));
	return true;
}

bool RegmapPlugin::onConnect()
{
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(conn == nullptr)
		return false;

	iio_context *ctx = conn->context();
	m_deviceList = new QList<iio_device *>();
	auto deviceCount = iio_context_get_devices_count(ctx);

	for(int i = 0; i < deviceCount; i++) {
		iio_device *dev = iio_context_get_device(ctx, i);
		if(iio_device_find_debug_attr(dev, "direct_reg_access")) {
			qDebug(CAT_REGMAP) << "DEVICE FOUND " << iio_device_get_name(dev);
			m_deviceList->push_back(dev);
		}
	}
	m_registerMapWidget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(m_registerMapWidget);
	layout->setMargin(0);
	m_registerMapWidget->setLayout(layout);
	Utils::applyJsonConfig();

	if(m_deviceList && !m_deviceList->isEmpty()) {
		QDir xmlsPath = Utils::setXmlPath();
		registerMapTool = new RegisterMapTool();
		layout->addWidget(registerMapTool);

		for(int i = 0; i < m_deviceList->size(); ++i) {
			iio_device *dev = m_deviceList->at(i);
			IIORegisterReadStrategy *iioReadStrategy = new IIORegisterReadStrategy(dev);
			IIORegisterWriteStrategy *iioWriteStrategy = new IIORegisterWriteStrategy(dev);

			QString devName = QString::fromStdString(iio_device_get_name(dev));
			qDebug(CAT_REGMAP) << "CONNECTING TO DEVICE : " << devName;
			JsonFormatedElement *templatePaths = Utils::getTemplate(devName);
			qDebug(CAT_REGMAP) << "templatePaths :" << templatePaths;
			QString templatePath = "";

			if(templatePaths) {
				qDebug(CAT_REGMAP) << "TEMPLATE FORUND FOR DEVICE : " << devName;
				templatePath = xmlsPath.absoluteFilePath(templatePaths->getFileName());
				if(templatePaths->getIsAxiCompatible()) {
					uint32_t axiAddressSpace = Utils::convertQStringToUint32("80000000");
					iioReadStrategy->setAddressSpace(axiAddressSpace);
					iioWriteStrategy->setAddressSpace(axiAddressSpace);
				}
				generateDevice(templatePath, dev, devName, iioReadStrategy, iioWriteStrategy,
					       templatePaths->getBitsPerRow());
			} else {
				generateDevice(templatePath, dev, devName, iioReadStrategy, iioWriteStrategy);
			}
		}

		m_toolList[0]->setEnabled(true);
		m_toolList[0]->setTool(m_registerMapWidget);
		InitApi();

		for(auto &tool : m_toolList) {
			tool->setEnabled(true);
			tool->setRunBtnVisible(true);
		}

		Q_EMIT toolListChanged();

		return true;
	}

	return false;
}

bool RegmapPlugin::onDisconnect()
{
	// TODO
	auto &&cp = ConnectionProvider::GetInstance();
	cp->close(m_param);

	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		tme->tool()->deleteLater();
		tme->setTool(nullptr);
	}

	Q_EMIT toolListChanged();

	return true;
}

void RegmapPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":3,
	   "category":[
	      "iio"
       ]
	}
)plugin");
}

QString RegmapPlugin::description() { return REGMAP_PLUGIN_DESCRIPTION; }
QString RegmapPlugin::displayName() { return REGMAP_PLUGIN_DISPLAY_NAME; }

QWidget *RegmapPlugin::getTool() { return m_registerMapWidget; }

void RegmapPlugin::generateDevice(QString xmlPath, struct iio_device *dev, QString devName,
				  IRegisterReadStrategy *readStrategy, IRegisterWriteStrategy *writeStrategy,
				  int bitsPerRow)
{

	RegisterMapTemplate *registerMapTemplate = nullptr;
	if(!xmlPath.isEmpty()) {
		registerMapTemplate = new RegisterMapTemplate(this);
		registerMapTemplate->setBitsPerRow(bitsPerRow);
		XmlFileManager xmlFileManager(dev, xmlPath);
		auto aux = xmlFileManager.getAllRegisters(registerMapTemplate);
		if(!aux->isEmpty()) {
			registerMapTemplate->setRegisterList(aux);
		}
	}

	RegisterMapValues *registerMapValues = new RegisterMapValues();
	registerMapValues->setReadStrategy(readStrategy);
	registerMapValues->setWriteStrategy(writeStrategy);

	registerMapTool->addDevice(devName, registerMapTemplate, registerMapValues);
}

struct iio_device *RegmapPlugin::getIioDevice(iio_context *ctx, const char *dev_name)
{
	auto deviceCount = iio_context_get_devices_count(ctx);

	for(int i = 0; i < deviceCount; i++) {
		iio_device *dev = iio_context_get_device(ctx, i);
		if(strcasecmp(iio_device_get_name(dev), dev_name) == 0) {
			return dev;
		}
	}
	return nullptr;
}

bool RegmapPlugin::isBufferCapable(iio_device *dev)
{
	unsigned int i;

	for(i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if(iio_channel_is_scan_element(chn)) {
			return true;
		}
	}

	return false;
}

void RegmapPlugin::InitApi()
{
	api = new RegMap_API(this);
	ScopyJS *js = ScopyJS::GetInstance();
	api->setObjectName("regmap");
	js->registerApi(api);
}
#include "moc_regmapplugin.cpp"
