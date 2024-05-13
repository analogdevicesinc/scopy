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
#include <src/readwrite/iioregisterreadstrategy.hpp>
#include <src/readwrite/iioregisterwritestrategy.hpp>
#include <pluginbase/preferences.h>
#include <pluginbase/preferenceshelper.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>
#include <readwrite/fileregisterreadstrategy.hpp>
#include <readwrite/fileregisterwritestrategy.hpp>
#include "logging_categories.h"

#include "iioutil/connectionprovider.h"
#include "jsonformatedelement.hpp"
#include "scopy-regmapplugin_config.h"
#include "utils.hpp"
#include "utils.hpp"
#if defined __APPLE__
#include <QApplication>
#endif

using namespace scopy;
using namespace regmap;

bool RegmapPlugin::loadPage()
{
	// TODO
	m_page = new QWidget();

	return true;
}

bool RegmapPlugin::loadIcon()
{
	m_icon = new QLabel("");
	m_icon->setStyleSheet("border-image: url(:/gui/icons/scopy-default/icons/RegMap.svg);");
	return true;
}

void RegmapPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("regmap", "Register Map", ":/gui/icons/scopy-default/icons/RegMap.svg"));
}

void RegmapPlugin::unload()
{
	// TODO
}

bool RegmapPlugin::compatible(QString m_param, QString category)
{
	m_name = "REGMAP";
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
	p->init("additional_regmap_xml_path", QCoreApplication::applicationDirPath() + "/plugins/plugins/xmls");
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
	MenuCollapseSection *generalSection =
		new MenuCollapseSection("General", MenuCollapseSection::MHCW_NONE, generalWidget);
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

		return true;
	}

	return false;
}

bool RegmapPlugin::onDisconnect()
{
	// TODO
	auto &&cp = ConnectionProvider::GetInstance();
	cp->close(m_param);

	if(m_registerMapWidget)
		delete m_registerMapWidget;
	if(m_deviceList)
		delete m_deviceList;

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

QString RegmapPlugin::description() { return "Register map tool"; }

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
