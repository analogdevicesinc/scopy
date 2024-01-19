#include "iiostandarditem.h"

using namespace scopy::iiodebugplugin;

IIOStandardItem::IIOStandardItem(const QString &text, IIOStandardItemModel::IIOType type)
	: QStandardItem(text)
	, m_itemModel(new IIOStandardItemModel(type))
{}

IIOStandardItem::~IIOStandardItem() { delete m_itemModel; }
