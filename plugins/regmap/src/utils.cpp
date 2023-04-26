#include "utils.hpp"

Utils::Utils(QObject *parent)
    : QObject{parent}
{

}

QString Utils::convertToHexa(uint32_t value, int size)
{
    return QStringLiteral("0x%1").arg(value, (size / bitsPerRow) + 1, 16, QLatin1Char('0'));
}
