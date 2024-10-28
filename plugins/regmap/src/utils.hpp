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

#ifndef UTILS_HPP
#define UTILS_HPP

#include <QObject>

class QDir;
class QPushButton;
class QLayout;

namespace scopy::regmap {
class JsonFormatedElement;

class Utils : public QObject
{
	Q_OBJECT
public:
	explicit Utils(QObject *parent = nullptr);

	static QString convertToHexa(uint32_t value, int size);
	static uint32_t convertQStringToUint32(QString value);
	static uint32_t getBitMask(int offset, int width);
	static void removeLayoutMargins(QLayout *layout);
	static QDir setXmlPath();

	static int getBitsPerRowDetailed();

	static JsonFormatedElement *getJsonTemplate(QString xml);

	static void applyJsonConfig();
	static JsonFormatedElement *getTemplate(QString devName);

private:
	static const int bitsPerRowDetailed = 3;
	static QMap<QString, JsonFormatedElement *> *spiJson;
	static QMap<QString, JsonFormatedElement *> *axiJson;
	static void getConfigurationFromJson(QString filePath);
	static void populateJsonTemplateMap(QJsonArray jsonArray, bool isAxi);
	static void generateJsonTemplate(QString filePath);

signals:
};
} // namespace scopy::regmap
#endif // UTILS_HPP
