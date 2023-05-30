#include "utils.hpp"
#include "dynamicWidget.h"
#include "scopy-regmap_config.h"
#include "jsonformatedelement.hpp"

#include <QDir>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QPushButton>

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

void Utils::applyScopyButtonStyle(QPushButton *button)
{
    scopy::setDynamicProperty(button, "blue_button", true);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

}

QDir Utils::setXmlPath()
{
    QDir xmlsPath(REGMAP_XML_BUILD_PATH);
    if ( xmlsPath.entryList().empty()) {
        xmlsPath.setPath(REGMAP_XML_SYSTEM_PATH);
    }
    return xmlsPath;
}

int Utils::getBitsPerRow()
{
    return bitsPerRow;
}

QList<QString>* Utils::getTemplate(QString devName)
{
    QList<QString> *templates = new QList<QString>();
    // search for SPI template
    foreach (QString key, spiJson->keys()) {
        for (int i = 0; i < spiJson->value(key)->getCompatibleDevices()->size(); i++) {
            if (spiJson->value(key)->getCompatibleDevices()->at(i).contains(devName)) {
                templates->push_back(key);
                break;
            }
        }
    }
    // search for AXI template
    foreach (QString key, axiJson->keys()) {
        for (int i = 0; i < axiJson->value(key)->getCompatibleDevices()->size(); i++) {
            if (axiJson->value(key)->getCompatibleDevices()->at(i).contains(devName)) {
                templates->push_back(key);
                break;
            }
        }
    }

    return templates;
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
    populateJsonTemplateMap(jsonSpiArray,true);
    QJsonArray jsonAxiArray = obj.value(QString("axi")).toArray();
    populateJsonTemplateMap(jsonAxiArray,false);

}

void Utils::populateJsonTemplateMap(QJsonArray jsonArray, bool spi)
{
    for (auto object : jsonArray) {

        QString fileName = object.toObject().value(QString("file_name")).toString();
        bool axiCompatible = object.toObject().value(QString("axi_compatible")).toBool();
        bool useDescriptionAsName = object.toObject().value(QString("use_description_as_name")).toBool();

        QList<QString> *compatibleDevicesList = new QList<QString>();
        QJsonArray compatibleDevices = object.toObject().value(QString("compatible_drivers")).toArray();
        if (!compatibleDevices.isEmpty()) {
            for (auto device : compatibleDevices) {
                compatibleDevicesList->push_back(device.toString());
            }
        }
        if (spi) {
            spiJson->insert(fileName , new JsonFormatedElement(fileName, compatibleDevicesList, axiCompatible, useDescriptionAsName));
        } else {
            axiJson->insert(fileName , new JsonFormatedElement(fileName, compatibleDevicesList, axiCompatible, useDescriptionAsName));
        }
    }
}

void Utils::generateJsonTemplate(QString filePath)
{
    getConfigurationFromJson(filePath);

    foreach (const QString &xmlName, scopy::regmap::Utils::setXmlPath().entryList()) {
        if (xmlName.contains(".xml") ) {
            if (!xmlName.contains("_axi")) {
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
}
