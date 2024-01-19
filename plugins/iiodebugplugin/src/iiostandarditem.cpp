#include "iiostandarditem.h"

using namespace scopy::iiodebugplugin;

IIOStandardItem::IIOStandardItem(const QString &text, IIOStandardItemType type)
	: QStandardItem(text)
	, m_itemModel(new IIOStandardItemModel(type))
	, m_itemView(new IIOStandardItemView(type))
{
}

IIOStandardItem::~IIOStandardItem()
{
	delete m_itemModel;
	delete m_itemView;
}
