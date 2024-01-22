#include "iiostandarditem.h"

using namespace scopy::iiodebugplugin;

IIOStandardItem::IIOStandardItem(struct iio_context *context, const QString &text, IIOStandardItemType type)
	: QStandardItem(text)
	, m_itemModel(new IIOStandardItemModel(type))
	, m_itemView(new IIOStandardItemView(type))
	, m_factory(new IIOWidgetFactory())
	, m_context(context)
{
	if(type == IIOStandardItemType::Context) {

	} else if(type == IIOStandardItemType::ContextAttribute) {
	}
}

IIOStandardItem::IIOStandardItem(struct iio_device *device, const QString &text, IIOStandardItemType type)
	: QStandardItem(text)
	, m_itemModel(new IIOStandardItemModel(type))
	, m_itemView(new IIOStandardItemView(type))
	, m_factory(new IIOWidgetFactory())
	, m_device(device)
{
	if(type == IIOStandardItemType::Device) {
		m_iioWidgets = m_factory->buildAllAttrsForDevice(device);
	} else if(type == IIOStandardItemType::DeviceAttribute) {
	}
}

IIOStandardItem::IIOStandardItem(struct iio_channel *channel, const QString &text, IIOStandardItemType type)
	: QStandardItem(text)
	, m_itemModel(new IIOStandardItemModel(type))
	, m_itemView(new IIOStandardItemView(type))
	, m_factory(new IIOWidgetFactory())
	, m_channel(channel)
{
	QString title = text;
	int dataIndex = title.indexOf(' ');
	if(dataIndex != -1) {
		title.remove(dataIndex, title.length() - dataIndex);
	}

	if(type == IIOStandardItemType::Channel) {
		m_iioWidgets = m_factory->buildAllAttrsForChannel(channel);
	} else if(type == IIOStandardItemType::ChannelAttribute) {
		m_iioWidgets.append(m_factory->buildSingle(
			IIOWidgetFactory::EditableUi | IIOWidgetFactory::InstantSave | IIOWidgetFactory::AttrData,
			{.channel = channel, .data = title}));
	}
}

IIOStandardItem::~IIOStandardItem()
{
	delete m_itemModel;
	delete m_itemView;
	delete m_factory;
}

QList<scopy::IIOWidget *> IIOStandardItem::getIIOWidgets() { return m_iioWidgets; }
