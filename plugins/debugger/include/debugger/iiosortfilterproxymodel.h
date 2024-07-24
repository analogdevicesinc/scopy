#ifndef SCOPY_IIOSORTFILTERPROXYMODEL_H
#define SCOPY_IIOSORTFILTERPROXYMODEL_H

#include <QObject>
#include <QModelIndex>
#include <QSortFilterProxyModel>

namespace scopy::debugger {
class IIOSortFilterProxyModel : public QSortFilterProxyModel
{
public:
	IIOSortFilterProxyModel(QObject *parent = nullptr);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};
} // namespace scopy::debugger

#endif // SCOPY_IIOSORTFILTERPROXYMODEL_H
