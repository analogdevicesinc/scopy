#ifndef SCOPY_IIOSTANDARDITEM_H
#define SCOPY_IIOSTANDARDITEM_H

#include <QWidget>
#include <QStandardItem>
#include "iiostandarditemmodel.h"

namespace scopy::iiodebugplugin {
class IIOStandardItem : public QStandardItem
{
public:
	explicit IIOStandardItem(const QString &text,
				 IIOStandardItemModel::IIOType type = IIOStandardItemModel::Undefined);
	~IIOStandardItem() override;

private:
	IIOStandardItemModel *m_itemModel;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_IIOSTANDARDITEM_H
