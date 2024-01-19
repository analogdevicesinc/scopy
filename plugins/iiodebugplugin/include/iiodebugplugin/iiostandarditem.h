#ifndef SCOPY_IIOSTANDARDITEM_H
#define SCOPY_IIOSTANDARDITEM_H

#include <QWidget>
#include <QStandardItem>
#include "iiostandarditemmodel.h"
#include "iiostandarditemtypes.h"
#include "iiostandarditemview.h"

namespace scopy::iiodebugplugin {
class IIOStandardItem : public QStandardItem
{
public:
	explicit IIOStandardItem(const QString &text, IIOStandardItemType type = IIOStandardItemType::Undefined);
	~IIOStandardItem() override;

private:
	IIOStandardItemModel *m_itemModel;
	IIOStandardItemView *m_itemView;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_IIOSTANDARDITEM_H
