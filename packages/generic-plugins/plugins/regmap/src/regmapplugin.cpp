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

#include "registermapaccess.hpp"
#include "registermaptemplate.hpp"
#include "regmapplugin.h"
#include "templates/registermaptemplateloader.hpp"
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
#include <component/controller.h>
#include <component/context.h>
#include <component/backends/iio/iiodevice.h>
#include <component/backends/iio/iioregisterreader.h>
#include <pluginbase/messagebroker.h>
#include <pluginbase/preferences.h>
#include <gui/preferenceshelper.h>
#include <core/deviceinfopage.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>
#include "logging_categories.h"
#include <regmap_api.h>
#include <deviceiconbuilder.h>

#include "jsonformatedelement.hpp"
#include <core/deviceimpl.h>
#include <core/dongledevicemanager.h>
#include "scopy-regmap_config.h"
#include "utils.hpp"
#include "utils.hpp"
#include <pluginbase/scopyjs.h>

#if defined __APPLE__
#include <QApplication>
#include <algorithm>
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
	// Claimed statically: the device plugins that configure regmap aren't visible yet here.
	// On a dongle device regmap starts as an empty tool, filled when a provider asks (CONFIGURE_TOPIC).
	if(category == DongleDeviceManager::CATEGORY) {
		return true;
	}

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

	reconfigure();

	// a dongle device starts as an empty tool, its EEPROM configuration plugin fills it on request
	if(registerMapTool->deviceCount() == 0 && !isDongle()) {
		delete m_registerMapWidget;
		m_registerMapWidget = nullptr;
		registerMapTool = nullptr;
		return false;
	}
	if(isDongle()) {
		MessageBroker::GetInstance()->subscribe(this, REGISTER_MAP_CONFIGURE_TOPIC);
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
	MessageBroker::GetInstance()->unsubscribe(this, REGISTER_MAP_CONFIGURE_TOPIC);
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

void RegmapPlugin::messageCallback(QString topic, QString message)
{
	if(topic != REGISTER_MAP_CONFIGURE_TOPIC || message != m_param || !registerMapTool) {
		return;
	}
	qInfo(CAT_REGMAP) << "Configuration requested for" << m_param;
	reconfigure();
}

void RegmapPlugin::reconfigure()
{
	Utils::applyJsonConfig();
	QList<RegisterMapToolConfig> provided;
	if(m_device) {
		provided = m_device->requestRegisterMapConfiguration();
	}
	// plugin-configured devices first, then the generic register map for the rest
	const QList<RegisterMapToolConfig> configured = resolveProviders(provided);
	QSet<QString> covered;
	for(const RegisterMapToolConfig &config : configured) {
		covered.insert(config.target);
	}
	const QList<RegisterMapToolConfig> configs = inContextOrder(genericConfiguration(covered) + configured);

	clearToolConfiguration();
	for(const RegisterMapToolConfig &config : configs) {
		applyConfiguration(config);
	}
}

bool RegmapPlugin::isDongle() const { return m_category == DongleDeviceManager::CATEGORY; }

QList<RegisterMapToolConfig> RegmapPlugin::genericConfiguration(const QSet<QString> &covered)
{
	QList<RegisterMapToolConfig> configs;
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return configs;
	}

	// the regmap-config.json lookup needs Utils::applyJsonConfig(), done in onConnect
	const QFileInfoList xmlFiles = Utils::setXmlPath();
	const QList<component::iio::IIODevice *> devices =
		ctx->findChildren<component::iio::IIODevice *>(Qt::FindDirectChildrenOnly);
	for(component::iio::IIODevice *dev : devices) {
		// a device a plugin configures gets nothing from the generic register map
		if(covered.contains(dev->name())) {
			continue;
		}
		if(dev->findChild<component::iio::IIORegisterReader *>()) {
			qDebug(CAT_REGMAP) << "DEVICE FOUND " << dev->name();
			configs.append(genericEntry(dev->name(), xmlFiles));
		}
	}
	return configs;
}

bool RegmapPlugin::isGeneric(const RegisterMapToolConfig &config) const { return config.provider == m_name; }

RegisterMapToolConfig RegmapPlugin::genericEntry(const QString &devName, const QFileInfoList &xmlFiles)
{
	RegisterMapToolConfig config;
	config.target = devName;
	config.provider = name();
	config.access = RegisterMapToolConfig::IIO;

	JsonFormatedElement *templatePaths = Utils::getTemplate(devName);
	if(templatePaths) {
		qDebug(CAT_REGMAP) << "TEMPLATE FOUND FOR DEVICE : " << devName;
		// Find the XML file with matching filename
		for(const QFileInfo &xmlFile : xmlFiles) {
			if(xmlFile.fileName() == templatePaths->getFileName()) {
				config.templatePath = xmlFile.absoluteFilePath();
				break;
			}
		}
		if(templatePaths->getIsAxiCompatible()) {
			config.addressSpace = Utils::convertQStringToUint32("80000000");
		}
		config.bitsPerRow = templatePaths->getBitsPerRow();
	}
	return config;
}

bool RegmapPlugin::configureTool(const QString &device, const QString &templatePath,
				 component::IRegisterReadStrategy *read, component::IRegisterWriteStrategy *write)
{
	const RegisterMapToolConfig defaults = genericEntry(device, Utils::setXmlPath());
	// read back only when both come from the same place: the device's IIO pair, or both from the caller
	const bool readBack = (read == nullptr) == (write == nullptr);
	if(!read || !write) {
		component::RegisterStrategies iio;
		if(!findIioStrategies(device, defaults.addressSpace.value_or(0), iio)) {
			qWarning(CAT_REGMAP) << "No default read/write for device" << device;
			return false;
		}
		if(!read) {
			read = iio.read;
		}
		if(!write) {
			write = iio.write;
		}
	}
	return setupDevice(device, templatePath, defaults.bitsPerRow.value_or(8), read, write, readBack);
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

QList<RegisterMapToolConfig> RegmapPlugin::resolveProviders(const QList<RegisterMapToolConfig> &provided)
{
	// One entry per device, applied as the plugin gave it: no fill-in from regmap's defaults.
	// Between two providers the higher priority wins.
	QList<RegisterMapToolConfig> resolved;
	QMap<QString, int> providerPrio; // target -> priority of the provider that configured it

	for(const RegisterMapToolConfig &config : provided) {
		if(config.target.isEmpty()) {
			continue;
		}
		const int prio = providerPriority(config.provider);
		int idx = -1;
		for(int i = 0; i < resolved.size(); i++) {
			if(resolved[i].target == config.target) {
				idx = i;
				break;
			}
		}

		if(idx < 0) {
			resolved.append(config);
			providerPrio.insert(config.target, prio);
		} else if(prio > providerPrio.value(config.target)) {
			resolved[idx] = config;
			providerPrio.insert(config.target, prio);
		} else {
			qInfo(CAT_REGMAP) << "Ignoring" << config.provider << "for" << config.target << ":"
					  << resolved[idx].provider << "has a higher priority";
		}
	}
	return resolved;
}

QList<RegisterMapToolConfig> RegmapPlugin::inContextOrder(QList<RegisterMapToolConfig> configs)
{
	QStringList order;
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(ctx) {
		for(component::iio::IIODevice *dev :
		    ctx->findChildren<component::iio::IIODevice *>(Qt::FindDirectChildrenOnly)) {
			order.append(dev->name());
		}
	}
	// devices only a provider knows (not in the context) go last, in the order they were given
	auto rank = [&order](const RegisterMapToolConfig &config) {
		const int idx = order.indexOf(config.target);
		return idx < 0 ? order.size() : idx;
	};
	std::stable_sort(
		configs.begin(), configs.end(),
		[&rank](const RegisterMapToolConfig &a, const RegisterMapToolConfig &b) { return rank(a) < rank(b); });
	return configs;
}

bool RegmapPlugin::applyConfiguration(const RegisterMapToolConfig &config)
{
	component::RegisterStrategies strategies;
	if(!findStrategies(config, strategies)) {
		return false;
	}

	const bool generic = isGeneric(config);
	if(generic) {
		qInfo(CAT_REGMAP) << "Register map" << config.target << ": generic";
	} else {
		qInfo(CAT_REGMAP) << "Register map" << config.target << ": configured by" << config.provider;
	}
	// one access for the reader and the writer, so they are the same type and a write is read back.
	// A plugin does the configuration it needs: its entry gets nothing from regmap-config.json.
	return setupDevice(config.target, config.templatePath, config.bitsPerRow.value_or(8), strategies.read,
			   strategies.write, true, generic, config.useRegisterDescriptionAsName,
			   config.useBitFieldDescriptionAsName);
}

bool RegmapPlugin::findStrategies(const RegisterMapToolConfig &config, component::RegisterStrategies &strategies)
{
	strategies = {};

	switch(config.access) {
	case RegisterMapToolConfig::IIO:
		return findIioStrategies(config.target, config.addressSpace.value_or(0), strategies);
	case RegisterMapToolConfig::None:
		return true;
	case RegisterMapToolConfig::File: {
		if(config.filePath.isEmpty() || !QFileInfo::exists(config.filePath)) {
			qWarning(CAT_REGMAP)
				<< "Register values file" << config.filePath << "not found for device" << config.target;
			return false;
		}
		strategies = component::fileRegisterStrategies(config.filePath);
		return true;
	}
	}
	qWarning(CAT_REGMAP) << "Unsupported access" << config.access << "for device" << config.target;
	return false;
}

bool RegmapPlugin::findIioStrategies(const QString &device, uint32_t addressSpace,
				     component::RegisterStrategies &strategies)
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
		// the device's pair for this address space; the device controller owns it
		strategies = component::registerStrategies(dev, addressSpace);
		return strategies.read != nullptr;
	}
	return false;
}

bool RegmapPlugin::setupDevice(const QString &device, const QString &templatePath, int bitsPerRow,
			       component::IRegisterReadStrategy *read, component::IRegisterWriteStrategy *write,
			       bool readBack, bool useRegmapConfig,
			       std::optional<bool> useRegisterDescriptionAsName,
			       std::optional<bool> useBitFieldDescriptionAsName)
{
	if(!registerMapTool) {
		return false;
	}
	if(registerMapTool->hasDevice(device)) {
		registerMapTool->removeDevice(device);
	}

	RegisterMapTemplate *registerMapTemplate = nullptr;
	if(!templatePath.isEmpty()) {
		registerMapTemplate = new RegisterMapTemplate();
		// generic and API paths: regmap-config.json settings for this template file. A plugin entry: only the
		// plugin's own options. The configured bitsPerRow wins, as before.
		RegisterMapTemplateOptions options;
		if(useRegmapConfig) {
			options = RegisterMapTemplateOptions::fromConfig(
				Utils::getJsonTemplate(QFileInfo(templatePath).fileName()));
		}
		options.bitsPerRow = bitsPerRow;
		if(useRegisterDescriptionAsName) {
			options.useRegisterDescriptionAsName = *useRegisterDescriptionAsName;
		}
		if(useBitFieldDescriptionAsName) {
			options.useBitFieldDescriptionAsName = *useBitFieldDescriptionAsName;
		}
		if(!RegisterMapTemplateLoader::load(templatePath, options, registerMapTemplate) ||
		   registerMapTemplate->getRegisterList()->isEmpty()) {
			qWarning(CAT_REGMAP) << "No registers in template" << templatePath << "for device" << device;
			delete registerMapTemplate;
			registerMapTemplate = nullptr;
		}
	}

	// no read and no write strategy: template only, the register map has no values.
	// The access only holds the strategies: they stay the device controller's.
	RegisterMapAccess *registerMapAccess = nullptr;
	if(read || write) {
		registerMapAccess = new RegisterMapAccess(read, write, readBack, device);
	}

	registerMapTool->addDevice(device, registerMapTemplate, registerMapAccess);
	return true;
}

void RegmapPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":3,
	   "category":[
	      "iio",
	      "dongle"
       ]
	}
)plugin");
}

QString RegmapPlugin::description() { return REGMAP_PLUGIN_DESCRIPTION; }
QString RegmapPlugin::displayName() { return REGMAP_PLUGIN_DISPLAY_NAME; }

QString RegmapPlugin::pkgName() { return REGMAP_PKG_NAME; }

QWidget *RegmapPlugin::getTool() { return m_registerMapWidget; }

void RegmapPlugin::initApi()
{
	m_api = new RegMap_API(this);
	ScopyJS *js = ScopyJS::GetInstance();
	m_api->setObjectName("regmap");
	js->registerApi(m_api);
}
#include "moc_regmapplugin.cpp"
