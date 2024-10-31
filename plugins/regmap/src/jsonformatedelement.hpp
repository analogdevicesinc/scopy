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

#ifndef JSONFORMATEDELEMENT_HPP
#define JSONFORMATEDELEMENT_HPP

#include <QObject>

namespace scopy::regmap {
class JsonFormatedElement
{
public:
	JsonFormatedElement(QString fileName, QList<QString> *compatibleDevices = new QList<QString>(),
			    bool isAxiCompatible = false, bool useRegisterDescriptionAsName = false,
			    bool useBifieldDescriptionAsName = false, int bitsPerRow = 8);
	QString getFileName() const;

	QList<QString> *getCompatibleDevices() const;
	void addCompatibleDevice(QString device);
	bool getIsAxiCompatible() const;
	bool getUseRegisterNameAsDescription() const;
	bool getUseBifieldNameAsDescription() const;
	QString toString();

	int getBitsPerRow() const;

private:
	QString fileName;
	QList<QString> *compatibleDevices;
	bool isAxiCompatible = false;
	bool useRegisterDescriptionAsName;
	bool useBifieldDescriptionAsName;
	int bitsPerRow;
};
} // namespace scopy::regmap
#endif // JSONFORMATEDELEMENT_HPP
