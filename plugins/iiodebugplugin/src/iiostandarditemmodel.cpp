#include "iiostandarditemmodel.h"

using namespace scopy::iiodebugplugin;

IIOStandardItemModel::IIOStandardItemModel(IIOType type, QWidget *parent)
	: QWidget(parent)
	, m_type(type)
{
	switch(m_type) {
	case Undefined:
		break;
	case Context: {
		// IIOWidget->buildAllAttrsForContext
		break;
	}
	case Device: {
		// IIOWidget->buildAllAttrsForDevice
		break;
	}
	case Channel: {
		// IIOWidget->buildAllAttrsForChannel
		break;
	}
	case ContextAttribute: {
		// IIOWidget->buildSingle(
		break;
	}
	case DeviceAttribute: {
		break;
	}
	case ChannelAttribute: {
		break;
	}
	}
}
