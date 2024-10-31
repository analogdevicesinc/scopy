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

#ifndef BITFIELDSIMPLEWIDGETFACTORY_HPP
#define BITFIELDSIMPLEWIDGETFACTORY_HPP

#include <QObject>

namespace scopy::regmap {
class BitFieldModel;
class BitFieldSimpleWidget;

class BitFieldSimpleWidgetFactory : public QObject
{
	Q_OBJECT
public:
	explicit BitFieldSimpleWidgetFactory(QObject *parent = nullptr);

	BitFieldSimpleWidget *buildWidget(BitFieldModel *model, int streach);
Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // BITFIELDSIMPLEWIDGETFACTORY_HPP
