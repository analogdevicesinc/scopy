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
#include <stylehelper.h>
#include <src/readwrite/componentregisterreadstrategy.hpp>
#include <src/readwrite/componentregisterwritestrategy.hpp>
#include <component/controller.h>
#include <component/context.h>
#include <component/backends/iio/iiodevice.h>
#include <component/backends/iio/iioregisterreader.h>
#include <component/backends/iio/iioregisterwriter.h>
#include <pluginbase/preferences.h>
#include <gui/preferenceshelper.h>
#include <gui/deviceinfopage.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>
#include <readwrite/fileregisterreadstrategy.hpp>
#include <readwrite/fileregisterwritestrategy.hpp>
#include "logging_categories.h"
#include <regmap_api.h>
#include <deviceiconbuilder.h>

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
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("REGMAP");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();
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
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return false;
	}

	const QList<component::iio::IIODevice *> devices =
		ctx->findChildren<component::iio::IIODevice *>(Qt::FindDirectChildrenOnly);
	for(component::iio::IIODevice *dev : devices) {
		if(dev->findChild<component::iio::IIORegisterReader *>()) {
			return true;
		}
	}

	return false;
}

void RegmapPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("regmap_color_by_value", "Default");
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
	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(generalWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	QStringList color_value_options = {"Default",
					   "Bitfield background",
					   "Bitfield text",
					   "Register background",
					   "Register text",
					   "Register background and Bitfield background",
					   "Register text and Bitfield text",
					   "Register background and Bitfield text",
					   "Register text and Bitfield background"};
	generalSection->contentLayout()->addWidget(
		PREFERENCE_COMBO(p, "regmap_color_by_value", "Use color to reflect value",
				 "Select from various configurations available which elements in the Register Map "
				 "are color coded for better visual interpretation.",
				 color_value_options, generalSection));

	return true;
}

bool RegmapPlugin::onConnect()
{
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx)
		return false;

	m_deviceList = new QList<component::iio::IIODevice *>();
	const QList<component::iio::IIODevice *> devices =
		ctx->findChildren<component::iio::IIODevice *>(Qt::FindDirectChildrenOnly);

	for(component::iio::IIODevice *dev : devices) {
		if(dev->findChild<component::iio::IIORegisterReader *>()) {
			qDebug(CAT_REGMAP) << "DEVICE FOUND " << dev->name();
			m_deviceList->push_back(dev);
		}
	}
	m_registerMapWidget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(m_registerMapWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	m_registerMapWidget->setLayout(layout);
	Utils::applyJsonConfig();

	if(m_deviceList && !m_deviceList->isEmpty()) {
		QFileInfoList xmlFiles = Utils::setXmlPath();
		registerMapTool = new RegisterMapTool();
		layout->addWidget(registerMapTool);

		for(int i = 0; i < m_deviceList->size(); ++i) {
			component::iio::IIODevice *dev = m_deviceList->at(i);
			ComponentRegisterReadStrategy *readStrategy = new ComponentRegisterReadStrategy(
				dev->findChild<component::iio::IIORegisterReader *>());
			ComponentRegisterWriteStrategy *writeStrategy = new ComponentRegisterWriteStrategy(
				dev->findChild<component::iio::IIORegisterWriter *>());

			QString devName = dev->name();
			qDebug(CAT_REGMAP) << "CONNECTING TO DEVICE : " << devName;
			JsonFormatedElement *templatePaths = Utils::getTemplate(devName);
			qDebug(CAT_REGMAP) << "templatePaths :" << templatePaths;
			QString templatePath = "";

			if(templatePaths) {
				qDebug(CAT_REGMAP) << "TEMPLATE FORUND FOR DEVICE : " << devName;
				// Find the XML file with matching filename
				for(const QFileInfo &xmlFile : xmlFiles) {
					if(xmlFile.fileName() == templatePaths->getFileName()) {
						templatePath = xmlFile.absoluteFilePath();
						break;
					}
				}
				if(templatePaths->getIsAxiCompatible()) {
					uint32_t axiAddressSpace = Utils::convertQStringToUint32("80000000");
					readStrategy->setAddressSpace(axiAddressSpace);
					writeStrategy->setAddressSpace(axiAddressSpace);
				}
				generateDevice(templatePath, devName, readStrategy, writeStrategy,
					       templatePaths->getBitsPerRow());
			} else {
				generateDevice(templatePath, devName, readStrategy, writeStrategy);
			}
		}

		m_toolList[0]->setEnabled(true);
		m_toolList[0]->setTool(m_registerMapWidget);

		for(auto &tool : m_toolList) {
			tool->setEnabled(true);
			tool->setRunBtnVisible(true);
		}

		Q_EMIT toolListChanged();

		initApi();
		return true;
	}

	return false;
}

bool RegmapPlugin::onDisconnect()
{
	if(m_api) {
		ScopyJS::GetInstance()->unregisterApi(m_api);
		delete m_api;
		m_api = nullptr;
	}

	for(ToolMenuEntry *tme : std::as_const(m_toolList)) {
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

QString RegmapPlugin::pkgName() { return REGMAP_PKG_NAME; }

QWidget *RegmapPlugin::getTool() { return m_registerMapWidget; }

void RegmapPlugin::generateDevice(QString xmlPath, QString devName, IRegisterReadStrategy *readStrategy,
				  IRegisterWriteStrategy *writeStrategy, int bitsPerRow)
{

	RegisterMapTemplate *registerMapTemplate = nullptr;
	if(!xmlPath.isEmpty()) {
		registerMapTemplate = new RegisterMapTemplate(this);
		registerMapTemplate->setBitsPerRow(bitsPerRow);
		XmlFileManager xmlFileManager(xmlPath);
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

void RegmapPlugin::initApi()
{
	m_api = new RegMap_API(this);
	ScopyJS *js = ScopyJS::GetInstance();
	m_api->setObjectName("regmap");
	js->registerApi(m_api);
}
#include "moc_regmapplugin.cpp"
