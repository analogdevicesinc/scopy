#include "iiostandarditemview.h"

using namespace scopy::iiodebugplugin;

IIOStandardItemView::IIOStandardItemView(IIOStandardItemType type, QWidget *parent)
	: QWidget(parent)
	, m_type(type)
{
	
}
