#include "utils.hpp"
#include "scopy-regmapplugin_config.h"
#include "jsonformatedelement.hpp"
#include <pluginbase/preferences.h>

#include <QDir>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QPushButton>
#include <QLayout>

#include "logging_categories.h"

using namespace scopy::regmap;

QMap<QString, JsonFormatedElement*>* Utils::spiJson {new QMap<QString,JsonFormatedElement*>()};
QMap<QString, JsonFormatedElement*>* Utils::axiJson {new QMap<QString,JsonFormatedElement*>()};

Utils::Utils(QObject *parent)
    : QObject{parent}
{
}

QString Utils::convertToHexa(uint32_t value, int size)
{
    return QStringLiteral("0x%1").arg(value, (size / getBitsPerRow()) + 1, 16, QLatin1Char('0'));
}

void Utils::removeLayoutMargins(QLayout *layout)
{
    layout->setMargin(0);
    layout->setContentsMargins(0,0,0,0);
}

QDir Utils::setXmlPath()
{
    QDir xmlsPath(REGMAP_XML_BUILD_PATH);
    if ( xmlsPath.entryList().empty()) {
        #ifdef Q_OS_WINDOWS
            xmlsPath.setPath("/plugins/regmap/xmls");
        #else
            xmlsPath.setPath(REGMAP_XML_SYSTEM_PATH);
        #endif
    }

    if(!xmlsPath.entryList().empty()) {
        return xmlsPath;
    }

    qDebug(CAT_REGMAP)<< "No XML folder found";
    return QDir("");
}

int Utils::getBitsPerRow()
{
    return bitsPerRow;
}

int Utils::getBitsPerRowDetailed()
{
    return bitsPerRowDetailed;
}

JsonFormatedElement *Utils::getJsonTemplate(QString xml)
{
    if (spiJson->contains(xml)) {
        return spiJson->value(xml);
    }

    if (axiJson->contains(xml)) {
        return axiJson->value(xml);
    }

    return nullptr;
}

QString Utils::getTemplate(QString devName)
{
    // search for SPI template
    foreach (QString key, spiJson->keys()) {
        for (int i = 0; i < spiJson->value(key)->getCompatibleDevices()->size(); i++) {
            if (spiJson->value(key)->getCompatibleDevices()->at(i).contains(devName)) {
                return QString(key);
            }
        }
    }
    // search for AXI template
    foreach (QString key, axiJson->keys()) {
        for (int i = 0; i < axiJson->value(key)->getCompatibleDevices()->size(); i++) {
            if (axiJson->value(key)->getCompatibleDevices()->at(i).contains(devName)) {
                return QString(key);
            }
        }
    }

    return QString("");
}

void Utils::applyJsonConfig()
{
    QDir xmlsPath = scopy::regmap::Utils::setXmlPath();
    QString filePath = xmlsPath.filePath("regmap-config.json");
    generateJsonTemplate(filePath);
}

void Utils::getConfigurationFromJson(QString filePath)
{
    QString val;
    QFile file;
    file.setFileName(filePath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();

    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());

    QJsonObject obj = d.object();

    QJsonArray jsonSpiArray = obj.value(QString("spi")).toArray();
    populateJsonTemplateMap(jsonSpiArray,false);
    QJsonArray jsonAxiArray = obj.value(QString("axi")).toArray();
    populateJsonTemplateMap(jsonAxiArray,true);

}

void Utils::populateJsonTemplateMap(QJsonArray jsonArray, bool isAxi)
{
    for (auto object : jsonArray) {

        QString fileName = object.toObject().value(QString("file_name")).toString();
        bool useRegisterDescriptionAsName = object.toObject().value(QString("use_register_description_as_name")).toBool();
        bool useBifieldDescriptionAsName = object.toObject().value(QString("use_bitfield_description_as_name")).toBool();

        QList<QString> *compatibleDevicesList = new QList<QString>();
        QJsonArray compatibleDevices = object.toObject().value(QString("compatible_drivers")).toArray();

        qDebug(CAT_REGMAP)<< "fileName : " <<  fileName;
        qDebug(CAT_REGMAP)<< "useRegisterDescriptionAsName : " << useRegisterDescriptionAsName;
        qDebug(CAT_REGMAP)<< "useBifieldDescriptionAsName : " << useBifieldDescriptionAsName;

        if (!compatibleDevices.isEmpty()) {
            for (auto device : compatibleDevices) {
                compatibleDevicesList->push_back(device.toString());
                qDebug(CAT_REGMAP)<< "compatible device : " <<  device.toString();
            }
        }
        spiJson->insert(fileName , new JsonFormatedElement(fileName, compatibleDevicesList, isAxi, useRegisterDescriptionAsName, useBifieldDescriptionAsName));
    }
}

void Utils::generateJsonTemplate(QString filePath)
{
    getConfigurationFromJson(filePath);

    foreach (const QString &xmlName, scopy::regmap::Utils::setXmlPath().entryList()) {
        if (xmlName.contains(".xml") ) {
            auto deviceName = xmlName.toLower();
            deviceName.chop(4);
            if (spiJson->contains(xmlName)) {
                spiJson->value(xmlName)->addCompatibleDevice(deviceName);
            } else {
                JsonFormatedElement *jsonFormatedElement = new JsonFormatedElement(xmlName);
                jsonFormatedElement->addCompatibleDevice(deviceName);
                spiJson->insert(xmlName,jsonFormatedElement);
            }
        }
    }
}
