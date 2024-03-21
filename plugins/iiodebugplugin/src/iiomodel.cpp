#include "iiomodel.h"
#include <QLoggingCategory>

#define BUFFER_SIZE 256
#define SEPARATOR "/"

Q_LOGGING_CATEGORY(CAT_IIOMODEL, "IIOModel")
using namespace scopy::iiodebugplugin;

IIOModel::IIOModel(struct iio_context *context, QString uri, QObject *parent)
	: QObject(parent)
	, m_ctx(context)
	, m_uri(uri)
{
	m_model = new QStandardItemModel(this);
	m_model->setHorizontalHeaderItem(0, new QStandardItem("IIO Tree"));
	iioTreeSetup();
}

QStandardItemModel *IIOModel::getModel() { return m_model; }

QSet<QString> IIOModel::getEntries() { return m_entries; }

void IIOModel::iioTreeSetup()
{
	m_rootString = m_uri;
	generateCtxAttributes();

	// add all devices from context, dfs
	uint ctx_devices_count = iio_context_get_devices_count(m_ctx);
	for(m_currentDeviceIndex = 0; m_currentDeviceIndex < ctx_devices_count; ++m_currentDeviceIndex) {
		setupCurrentDevice();
		generateDeviceAttributes();

		// add all channels to current device
		uint device_channels_count = iio_device_get_channels_count(m_currentDevice);
		for(m_currentChannelIndex = 0; m_currentChannelIndex < device_channels_count; ++m_currentChannelIndex) {
			setupCurrentChannel();
			generateChannelAttributes();

			// add channel to device
			m_currentDeviceItem->appendRow(m_currentChannelItem);
		}

		// add device to ctx
		m_rootItem->appendRow(m_currentDeviceItem);
	}

	m_model->appendRow(m_rootItem);
}

void IIOModel::generateCtxAttributes()
{
	QList<IIOWidget *> ctxList = IIOWidgetFactory::buildAllAttrsForContext(m_ctx);
	m_rootItem = new IIOStandardItem(ctxList, m_rootString, m_rootString, IIOStandardItem::Context);
	m_rootItem->setEditable(false);

	// add attrs from context
	for(IIOWidget *ctxWidget : ctxList) {
		m_entries.insert(ctxWidget->getRecipe().data);
		auto *attrItem = new IIOStandardItem({ctxWidget}, ctxWidget->getRecipe().data,
						     m_rootString + SEPARATOR + ctxWidget->getRecipe().data,
						     IIOStandardItem::ContextAttribute);
		attrItem->setEditable(false);
		m_rootItem->appendRow(attrItem);
	}
}

void IIOModel::setupCurrentDevice()
{
	m_currentDevice = iio_context_get_device(m_ctx, m_currentDeviceIndex);
	m_devList = IIOWidgetFactory::buildAllAttrsForDevice(m_currentDevice);
	m_currentDeviceName = iio_device_get_name(m_currentDevice);
	QString currentDeviceId = iio_device_get_id(m_currentDevice);
	bool is_trigger = iio_device_is_trigger(m_currentDevice);
	if(is_trigger) {
		m_currentDeviceItem =
			new IIOStandardItem({}, m_currentDeviceName, currentDeviceId,
					    m_rootString + SEPARATOR + m_currentDeviceName, IIOStandardItem::Trigger);
	} else {
		m_currentDeviceItem =
			new IIOStandardItem(m_devList, m_currentDeviceName, currentDeviceId,
					    m_rootString + SEPARATOR + m_currentDeviceName, IIOStandardItem::Device);
	}
	m_currentDeviceItem->setDevice(m_currentDevice);
	m_currentDeviceItem->setEditable(false);
	m_entries.insert(m_currentDeviceName);
}

void IIOModel::generateDeviceAttributes()
{
	// add all attrs to current device
	for(int j = 0; j < m_devList.size(); ++j) {
		QString device_attr = iio_device_get_attr(m_currentDevice, j);

		m_entries.insert(device_attr);
		auto *attrItem =
			new IIOStandardItem({m_devList[j]}, m_devList[j]->getRecipe().data,
					    m_rootString + SEPARATOR + m_currentDeviceName + SEPARATOR + device_attr,
					    IIOStandardItem::DeviceAttribute);
		attrItem->setDevice(m_currentDevice);
		attrItem->setEditable(false);
		m_currentDeviceItem->appendRow(attrItem);
	}
}

void IIOModel::setupCurrentChannel()
{
	m_currentChannel = iio_device_get_channel(m_currentDevice, m_currentChannelIndex);
	m_chnlList = IIOWidgetFactory::buildAllAttrsForChannel(m_currentChannel);
	m_currentChannelName = iio_channel_get_id(m_currentChannel);
	QString currentChannelId = iio_channel_get_name(m_currentChannel);

	m_currentChannelItem =
		new IIOStandardItem(m_chnlList, currentChannelId, m_currentChannelName,
				    m_rootString + SEPARATOR + m_currentDeviceName + SEPARATOR + m_currentChannelName,
				    IIOStandardItem::Channel);
	m_currentChannelItem->setChannel(m_currentChannel);

	if(m_currentChannelItem->isScanElement()) {
		m_currentDeviceItem->setBufferCapable(true);
	}

	m_currentChannelItem->setEditable(false);
	m_entries.insert(m_currentChannelName);
}

void IIOModel::generateChannelAttributes()
{
	// add all attrs from channel
	for(int i = 0; i < m_chnlList.size(); ++i) {
		// FIXME: why is there an attr_name and an attrName
		QString attr_name = iio_channel_get_attr(m_currentChannel, i);

		m_entries.insert(attr_name);
		QString attrName = m_chnlList[i]->getRecipe().data;
		auto *attr_item = new IIOStandardItem({m_chnlList[i]}, attrName,
						      m_rootString + SEPARATOR + m_currentDeviceName + SEPARATOR +
							      m_currentChannelName + SEPARATOR + attrName,
						      IIOStandardItem::ChannelAttribute);
		attr_item->setChannel(m_currentChannel);
		attr_item->setEditable(false);
		m_currentChannelItem->appendRow(attr_item);
	}
}
