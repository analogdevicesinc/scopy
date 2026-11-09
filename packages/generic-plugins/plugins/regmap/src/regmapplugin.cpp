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
#include <readwrite/componentregisterreadstrategy.hpp>
#include <readwrite/componentregisterwritestrategy.hpp>
#include <component/controller.h>
#include <component/context.h>
#include <component/backends/iio/iiodevice.h>
#include <component/backends/iio/iioregisterreader.h>
#include <component/backends/iio/iioregisterwriter.h>
#include <pluginbase/preferences.h>
#include <gui/preferenceshelper.h>
#include <core/deviceinfopage.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>
#include <readwrite/fileregisterreadstrategy.hpp>
#include <readwrite/fileregisterwritestrategy.hpp>
#include "logging_categories.h"
#include <regmap_api.h>
#include <deviceiconbuilder.h>

#include "jsonformatedelement.hpp"
#include "regmapconfigkeys.h"
#include <core/deviceimpl.h>
#include <readwrite/csvregisterstrategy.hpp>
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
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		// no IIO context, so no device info to show
		return false;
	}

	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	auto deviceInfoPage = new DeviceInfoPage(ctx.get());
	lay->addWidget(deviceInfoPage);
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

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
	m_registerMapWidget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(m_registerMapWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	m_registerMapWidget->setLayout(layout);
	registerMapTool = new RegisterMapTool();
	layout->addWidget(registerMapTool);

	Utils::applyJsonConfig();
	QList<ToolConfiguration> provided;
	if(m_device) {
		provided = m_device->requestToolConfiguration(configkeys::TOOL_ID);
	}
	const QList<ToolConfiguration> configs = mergeConfigurations(defaultConfiguration(), provided);

	clearToolConfiguration();
	for(const ToolConfiguration &config : configs) {
		applyConfiguration(config);
	}

	if(registerMapTool->deviceCount() == 0) {
		delete m_registerMapWidget;
		m_registerMapWidget = nullptr;
		registerMapTool = nullptr;
		return false;
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
		if(tme->tool()) {
			tme->tool()->deleteLater();
		}
		tme->setTool(nullptr);
	}
	// registerMapTool is a child of m_registerMapWidget, deleted above
	m_registerMapWidget = nullptr;
	registerMapTool = nullptr;

	Q_EMIT toolListChanged();

	return true;
}

QList<ToolConfiguration> RegmapPlugin::defaultConfiguration()
{
	QList<ToolConfiguration> configs;
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return configs;
	}

	// the regmap-config.json lookup needs Utils::applyJsonConfig(), done in onConnect
	const QFileInfoList xmlFiles = Utils::setXmlPath();
	const QList<component::iio::IIODevice *> devices =
		ctx->findChildren<component::iio::IIODevice *>(Qt::FindDirectChildrenOnly);
	for(component::iio::IIODevice *dev : devices) {
		if(dev->findChild<component::iio::IIORegisterReader *>()) {
			qDebug(CAT_REGMAP) << "DEVICE FOUND " << dev->name();
			configs.append(defaultEntry(dev->name(), xmlFiles));
		}
	}
	return configs;
}

ToolConfiguration RegmapPlugin::defaultEntry(const QString &devName, const QFileInfoList &xmlFiles)
{
	ToolConfiguration config;
	config.toolId = configkeys::TOOL_ID;
	config.target = devName;
	config.provider = name();
	config.properties.insert(configkeys::ACCESS, configkeys::ACCESS_DEFAULT);

	QString templatePath = "";
	JsonFormatedElement *templatePaths = Utils::getTemplate(devName);
	if(templatePaths) {
		qDebug(CAT_REGMAP) << "TEMPLATE FOUND FOR DEVICE : " << devName;
		// Find the XML file with matching filename
		for(const QFileInfo &xmlFile : xmlFiles) {
			if(xmlFile.fileName() == templatePaths->getFileName()) {
				templatePath = xmlFile.absoluteFilePath();
				break;
			}
		}
		if(templatePaths->getIsAxiCompatible()) {
			config.properties.insert(configkeys::ADDRESS_SPACE, Utils::convertQStringToUint32("80000000"));
		}
		config.properties.insert(configkeys::BITS_PER_ROW, templatePaths->getBitsPerRow());
	}
	config.properties.insert(configkeys::TEMPLATE, templatePath);
	return config;
}

bool RegmapPlugin::configureTool(const QString &device, const QString &templatePath, IRegisterReadStrategy *read,
				 IRegisterWriteStrategy *write)
{
	const ToolConfiguration defaults = defaultEntry(device, Utils::setXmlPath());
	if(!read || !write) {
		IRegisterReadStrategy *defaultRead = nullptr;
		IRegisterWriteStrategy *defaultWrite = nullptr;
		if(!createIioStrategies(device, defaults.properties.value(configkeys::ADDRESS_SPACE, 0).toUInt(),
					defaultRead, defaultWrite)) {
			qWarning(CAT_REGMAP) << "No default read/write for device" << device;
			return false;
		}
		if(read) {
			delete defaultRead;
		} else {
			read = defaultRead;
		}
		if(write) {
			delete defaultWrite;
		} else {
			write = defaultWrite;
		}
	}
	return setupDevice(device, templatePath, defaults.properties.value(configkeys::BITS_PER_ROW, 8).toInt(), read,
			   write);
}

void RegmapPlugin::clearToolConfiguration(const QString &device)
{
	if(!registerMapTool) {
		return;
	}
	if(device.isEmpty()) {
		registerMapTool->clear();
	} else {
		registerMapTool->removeDevice(device);
	}
}

int RegmapPlugin::providerPriority(const QString &provider)
{
	Plugin *p = m_device ? m_device->getPluginByName(provider) : nullptr;
	return p ? p->metadata().value("priority").toInt() : 0;
}

QList<ToolConfiguration> RegmapPlugin::mergeConfigurations(const QList<ToolConfiguration> &defaults,
							   const QList<ToolConfiguration> &provided)
{
	// A provider entry replaces the default for its device only, a device only a provider knows is added,
	// and between two providers the higher priority wins.
	QList<ToolConfiguration> merged = defaults;
	QMap<QString, int> providerPrio; // target -> priority of the provider that configured it
	const QFileInfoList xmlFiles = Utils::setXmlPath();

	for(ToolConfiguration config : provided) {
		if(config.toolId != configkeys::TOOL_ID || config.target.isEmpty()) {
			continue;
		}
		// bitsPerRow and addressSpace are optional, regmap-config.json gives them as today
		const ToolConfiguration jsonDefaults = defaultEntry(config.target, xmlFiles);
		for(const QString &key : {configkeys::BITS_PER_ROW, configkeys::ADDRESS_SPACE}) {
			if(!config.properties.contains(key) && jsonDefaults.properties.contains(key)) {
				config.properties.insert(key, jsonDefaults.properties.value(key));
			}
		}

		const int prio = providerPriority(config.provider);
		int idx = -1;
		for(int i = 0; i < merged.size(); i++) {
			if(merged[i].target == config.target) {
				idx = i;
				break;
			}
		}

		if(idx < 0) {
			merged.append(config);
			providerPrio.insert(config.target, prio);
		} else if(!providerPrio.contains(config.target) || prio > providerPrio.value(config.target)) {
			merged[idx] = config;
			providerPrio.insert(config.target, prio);
		} else {
			qInfo(CAT_REGMAP) << "Ignoring" << config.provider << "for" << config.target << ":"
					  << merged[idx].provider << "has a higher priority";
		}
	}
	return merged;
}

bool RegmapPlugin::applyConfiguration(const ToolConfiguration &config)
{
	for(const QString &key : config.properties.keys()) {
		if(!configkeys::ALL.contains(key)) {
			qWarning(CAT_REGMAP) << "Unknown configuration key" << key << "for device" << config.target
					     << "from" << config.provider;
		}
	}

	IRegisterReadStrategy *read = nullptr;
	IRegisterWriteStrategy *write = nullptr;
	if(!createStrategies(config, read, write)) {
		return false;
	}

	qDebug(CAT_REGMAP) << "CONNECTING TO DEVICE : " << config.target << "configured by" << config.provider;
	return setupDevice(config.target, config.properties.value(configkeys::TEMPLATE).toString(),
			   config.properties.value(configkeys::BITS_PER_ROW, 8).toInt(), read, write);
}

bool RegmapPlugin::createStrategies(const ToolConfiguration &config, IRegisterReadStrategy *&read,
				    IRegisterWriteStrategy *&write)
{
	read = nullptr;
	write = nullptr;
	const QString access = config.properties.value(configkeys::ACCESS, configkeys::ACCESS_DEFAULT).toString();

	if(access == configkeys::ACCESS_DEFAULT) {
		return createIioStrategies(config.target,
					   config.properties.value(configkeys::ADDRESS_SPACE, 0).toUInt(), read, write);
	}
	if(access == configkeys::ACCESS_NONE) {
		return true;
	}
	if(access == configkeys::ACCESS_FILE) {
		const QString path = config.properties.value(configkeys::FILE_PATH).toString();
		if(path.isEmpty() || !QFileInfo::exists(path)) {
			qWarning(CAT_REGMAP)
				<< "Register values file" << path << "not found for device" << config.target;
			return false;
		}
		QSharedPointer<CsvRegisterFile> file(new CsvRegisterFile(path));
		read = new CsvRegisterReadStrategy(file);
		write = new CsvRegisterWriteStrategy(file);
		return true;
	}
	qWarning(CAT_REGMAP) << "Unsupported access" << access << "for device" << config.target;
	return false;
}

bool RegmapPlugin::createIioStrategies(const QString &device, uint32_t addressSpace, IRegisterReadStrategy *&read,
				       IRegisterWriteStrategy *&write)
{
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return false;
	}

	const QList<component::iio::IIODevice *> devices =
		ctx->findChildren<component::iio::IIODevice *>(Qt::FindDirectChildrenOnly);
	for(component::iio::IIODevice *dev : devices) {
		if(dev->name() != device) {
			continue;
		}
		component::iio::IIORegisterReader *reader = dev->findChild<component::iio::IIORegisterReader *>();
		if(!reader) {
			return false;
		}
		ComponentRegisterReadStrategy *readStrategy = new ComponentRegisterReadStrategy(reader);
		ComponentRegisterWriteStrategy *writeStrategy =
			new ComponentRegisterWriteStrategy(dev->findChild<component::iio::IIORegisterWriter *>());
		if(addressSpace) {
			readStrategy->setAddressSpace(addressSpace);
			writeStrategy->setAddressSpace(addressSpace);
		}
		read = readStrategy;
		write = writeStrategy;
		return true;
	}
	return false;
}

bool RegmapPlugin::setupDevice(const QString &device, const QString &templatePath, int bitsPerRow,
			       IRegisterReadStrategy *read, IRegisterWriteStrategy *write)
{
	if(!registerMapTool) {
		delete read;
		delete write;
		return false;
	}
	if(registerMapTool->hasDevice(device)) {
		registerMapTool->removeDevice(device);
	}

	RegisterMapTemplate *registerMapTemplate = nullptr;
	if(!templatePath.isEmpty()) {
		registerMapTemplate = new RegisterMapTemplate();
		registerMapTemplate->setBitsPerRow(bitsPerRow);
		XmlFileManager xmlFileManager(templatePath);
		auto aux = xmlFileManager.getAllRegisters(registerMapTemplate);
		if(!aux->isEmpty()) {
			registerMapTemplate->setRegisterList(aux);
		} else {
			qWarning(CAT_REGMAP) << "No registers in template" << templatePath << "for device" << device;
			delete registerMapTemplate;
			registerMapTemplate = nullptr;
		}
	}

	// no read and no write strategy: template only, the register map has no values
	RegisterMapValues *registerMapValues = nullptr;
	if(read || write) {
		registerMapValues = new RegisterMapValues();
		registerMapValues->setDeviceName(device);
		if(read) {
			read->setParent(registerMapValues);
			registerMapValues->setReadStrategy(read);
		}
		if(write) {
			write->setParent(registerMapValues);
			registerMapValues->setWriteStrategy(write);
		}
	}

	registerMapTool->addDevice(device, registerMapTemplate, registerMapValues);
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
	setupDevice(devName, xmlPath, bitsPerRow, readStrategy, writeStrategy);
}

void RegmapPlugin::initApi()
{
	m_api = new RegMap_API(this);
	ScopyJS *js = ScopyJS::GetInstance();
	m_api->setObjectName("regmap");
	js->registerApi(m_api);
}
#include "moc_regmapplugin.cpp"
