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

#ifndef REGISTERMODEL_HPP
#define REGISTERMODEL_HPP

#include <QObject>
#include <registermaptemplate.hpp>

namespace scopy::regmap {
class BitFieldModel;

class RegisterModel : public QObject
{
	Q_OBJECT
public:
	RegisterModel(QString name, uint32_t address, QString description, bool exists, int width, QString notes,
		      QVector<BitFieldModel *> *bitFields, RegisterMapTemplate *registerMapTemplate);

	QString getName() const;
	uint32_t getAddress() const;
	QString getDescription() const;
	bool getExists() const;
	int getWidth() const;
	QString getNotes() const;
	QVector<BitFieldModel *> *getBitFields() const;

	RegisterMapTemplate *registerMapTemaplate() const;
	void setRegisterMapTemaplate(RegisterMapTemplate *newRegisterMapTemaplate);

private:
	QString name;
	uint32_t address;
	QString description;
	bool exists;
	int width;
	QString notes;
	QVector<BitFieldModel *> *bitFields;
	RegisterMapTemplate *m_registerMapTemaplate = nullptr;

Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // REGISTERMODEL_HPP
