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

#ifndef REGISTERMAPTABLE_H
#define REGISTERMAPTABLE_H

#include "irecyclerviewadapter.hpp"

#include <QMap>
#include <QObject>

namespace scopy::regmap {
class RegisterModel;
class RegisterSimpleWidget;
class RecyclerView;

class RegisterMapTable : public IRecyclerViewAdapter
{
	friend class RegMap_API;
	Q_OBJECT
public:
	RegisterMapTable(QMap<uint32_t, RegisterModel *> *registerModels, QWidget *parent);

	QWidget *getWidget();
	void setFilters(QList<uint32_t> filters);
	void valueUpdated(uint32_t address, uint32_t value);
	void scrollTo(uint32_t index);
	void setRegisterSelected(uint32_t address);

	// IRecyclerViewAdapter interface
	void generateWidget(int index);

Q_SIGNALS:
	void requestWidget(int index);
	void registerSelected(uint32_t address);

private:
	RecyclerView *recyclerView = nullptr;
	QMap<uint32_t, RegisterModel *> *registerModels;
	QMap<uint32_t, RegisterSimpleWidget *> *registersMap;
	uint32_t selectedAddress;
};
} // namespace scopy::regmap
#endif // REGISTERMAPTABLE_H
