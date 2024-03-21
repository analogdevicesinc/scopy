#include "iiosortfilterproxymodel.h"

using namespace scopy::iiodebugplugin;

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
