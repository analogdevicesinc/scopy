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

#ifndef BITFIELDMODEL_HPP
#define BITFIELDMODEL_HPP

#include "bitfieldoption.hpp"

#include <QMap>
#include <QObject>

namespace scopy::regmap {
class BitFieldModel : public QObject
{
	Q_OBJECT
public:
	BitFieldModel(QString name, int width, int regOffset, QString description, QObject *parent = nullptr);

	BitFieldModel(QString name, QString access, int defaultValue, QString description, QString visibility,
		      int width, QString notes, int bitOffset, int regOffset, int sliceWidth,
		      QVector<BitFieldOption *> *options, QObject *parent = nullptr);
	~BitFieldModel();

	QString getName() const;
	QString getAccess() const;
	int getDefaultValue() const;
	QString getDescription() const;
	QString getVisibility() const;
	int getWidth() const;
	QString getNotes() const;
	int getBitOffset() const;
	int getRegOffset() const;
	int getSliceWidth() const;
	QVector<BitFieldOption *> *getOptions() const;

private:
	QString name;
	QString access;
	int defaultValue;
	QString description;
	QString visibility;
	int width;
	QString notes;
	int bitOffset;
	int regOffset;
	int sliceWidth;
	QVector<BitFieldOption *> *options = nullptr;

	bool reserved;

Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // BITFIELDMODEL_HPP
