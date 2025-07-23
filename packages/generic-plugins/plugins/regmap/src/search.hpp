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

#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "scopy-regmap_export.h"

#include <QObject>

namespace scopy::regmap {
class RegisterModel;
class SCOPY_REGMAP_EXPORT Search : public QObject
{
	Q_OBJECT
public:
	explicit Search(QObject *parent = nullptr);

	static QList<uint32_t> searchForRegisters(QMap<uint32_t, RegisterModel *> *registers, QString searchParam);
Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // SEARCH_HPP
