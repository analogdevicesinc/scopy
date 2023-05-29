#include "utils.hpp"
#include "dynamicWidget.h"
#include "scopy-regmap_config.h"

#include <QDir>
#include <QPushButton>

using namespace scopy::regmap;
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
