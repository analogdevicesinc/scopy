#include "deviceregistermap.hpp"
#include "registermapinstrument.hpp"
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
#include <src/readwrite/iioregisterreadstrategy.hpp>
#include <src/readwrite/iioregisterwritestrategy.hpp>
#include "logging_categories.h"

#include "iioutil/contextprovider.h"
#include "scopy-regmap_config.h"
#include "utils.hpp"

using namespace scopy;

bool REGMAPPlugin::loadPage()
{
    //TODO
    m_page = new QWidget();

    return true;
}

bool REGMAPPlugin::loadIcon()
{
    m_icon = new QLabel("");
    m_icon->setStyleSheet("border-image: url(:/icons/scopy-light/icons/locked.svg);");
    return true;
}

void REGMAPPlugin::loadToolList()
{
    m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("regmap","Register Map",":/gui/icons/scopy-default/icons/tool_debugger.svg"));
}

void REGMAPPlugin::unload()
{
    //TODO
    auto &&cp = ContextProvider::GetInstance();
    cp->close(m_param);
}

bool REGMAPPlugin::compatible(QString m_param, QString category)
{
    m_name="REGMAP";
    auto &&cp = ContextProvider::GetInstance();
    iio_context* ctx = cp->open(m_param);

    if (!ctx) {
        cp->close(m_param);
        return false;
    } else {

        auto deviceCount = iio_context_get_devices_count(ctx);
        for (int i = 0; i < deviceCount; i++) {
            iio_device *dev = iio_context_get_device(ctx, i);
            if (iio_device_find_debug_attr(dev,"direct_reg_access")) {
                cp->close(m_param);
                // if we have regmap we create the config file
                scopy::regmap::Utils::applyJsonConfig();
                return true;
            }
        }
    }
    cp->close(m_param);

    return false;
}

void REGMAPPlugin::preload()
{
    //    auto &&cp = ContextProvider::GetInstance();
    //    iio_context* ctx = cp->open(m_uri);

    //    m_deviceList = new QList<iio_device*>();

    //    auto deviceCount = iio_context_get_devices_count(ctx);

    //    for (int i = 0; i < deviceCount; i++) {
    //        iio_device *dev = iio_context_get_device(ctx, i);
    //        qDebug(CAT_REGMAP)<<"DEVICE FOUND " << iio_device_get_name(dev);
    //        m_deviceList->push_back(dev);
    //    }

}

bool REGMAPPlugin::onConnect()
{
    auto &&cp = ContextProvider::GetInstance();
    iio_context* ctx = cp->open(m_param);

    m_deviceList = new QList<iio_device*>();

    auto deviceCount = iio_context_get_devices_count(ctx);


    for (int i = 0; i < deviceCount; i++) {
        iio_device *dev = iio_context_get_device(ctx, i);
        if (iio_device_find_debug_attr(dev,"direct_reg_access")) {
            qDebug(CAT_REGMAP)<<"DEVICE FOUND " << iio_device_get_name(dev);
            m_deviceList->push_back(dev);
        }
    }
    //TODO
    m_registerMapWidget = new QWidget();
    QVBoxLayout *layout  = new QVBoxLayout();
    m_registerMapWidget->setLayout(layout);

    if (m_deviceList && !m_deviceList->isEmpty()) {
        QDir xmlsPath = scopy::regmap::Utils::setXmlPath();
        RegisterMapInstrument *regMapInstrument = new RegisterMapInstrument();

        for (int i = 0; i < m_deviceList->size(); ++i) {
            iio_device *dev = m_deviceList->at(i);
            QString devName = QString::fromStdString(iio_device_get_name(dev));
            qDebug(CAT_REGMAP)<<"CONNECTING TO DEVICE : " << devName;

            if (isBufferCapable(dev)) {
                qDebug(CAT_REGMAP)<<"DEVICE :" << devName << " IS BUFFER CAPABLE";
            }


            QList<QString> *templatePaths = scopy::regmap::Utils::getTemplate(devName);
            if (!templatePaths->empty()) {
                for (int i = 0 ; i < templatePaths->size(); i++) {
                    qDebug(CAT_REGMAP)<<"TEMPLATE FORUND FOR DEVICE : " << devName;
                    regMapInstrument->addTab( dev, devName, xmlsPath.absoluteFilePath(templatePaths->at(i)));
                }

            } else {
                //TODO GROUP ALL DEVICES IN ONE WITH A COMBOBOX
                regMapInstrument->addTab(dev, iio_device_get_name(dev));
            }


        }
        layout->addWidget(regMapInstrument);

        m_toolList[0]->setEnabled(true);
        m_toolList[0]->setTool(m_registerMapWidget);
        return true;
    }

    return false;
}

bool REGMAPPlugin::onDisconnect()
{
    //TODO
    delete m_deviceList;
    delete m_registerMapWidget;

    return true;
}

void REGMAPPlugin::initMetadata()
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

QWidget *REGMAPPlugin::getTool()
{
    return m_registerMapWidget;
}


struct iio_device* REGMAPPlugin::getIioDevice(iio_context* ctx, const char *dev_name){
    auto deviceCount = iio_context_get_devices_count(ctx);

    for (int i = 0; i < deviceCount; i++) {
        iio_device *dev = iio_context_get_device(ctx, i);
        if (strcasecmp(iio_device_get_name(dev), dev_name) == 0) {
            return dev;
        }
    }
    return nullptr;
}

bool REGMAPPlugin::isBufferCapable(iio_device *dev)
{
    unsigned int i;

    for (i = 0; i < iio_device_get_channels_count(dev); i++) {
        struct iio_channel *chn = iio_device_get_channel(dev, i);

        if (iio_channel_is_scan_element(chn)){
            return true;
        }
    }

    return false;
}
