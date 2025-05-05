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

#include "iiosortfilterproxymodel.h"

using namespace scopy::debugger;

bool IIOSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	// Get the source index for the current row
	QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

	// Get the item data from the source index
	QVariant itemData = sourceModel()->data(sourceIndex, Qt::DisplayRole);

	// Check if the item's data contains the filter string
	QString filterString = filterRegExp().pattern(); // Get the filter string
	if(itemData.toString().contains(filterString, Qt::CaseInsensitive)) {
		return true;
	}

	// Check recursively for children
	for(int i = 0; i < sourceModel()->rowCount(sourceIndex); ++i) {
		if(filterAcceptsRow(i, sourceIndex)) {
			return true;
		}
	}

	return false;
}

IIOSortFilterProxyModel::IIOSortFilterProxyModel(QObject *parent)
	: QSortFilterProxyModel(parent)
{}
