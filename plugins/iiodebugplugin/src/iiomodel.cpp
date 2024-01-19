#include "iiomodel.h"
#include "iiostandarditem.h"
#include <QLoggingCategory>

#define BUFFER_SIZE 256
#define PATH_DELIMITER "/"

using namespace scopy::iiodebugplugin;

IIOModel::IIOModel(struct iio_context *context, QObject *parent)
	: QObject(parent)
	, m_ctx(context)
{
	m_model = new QStandardItemModel(this);
	iioTreeSetup();
	connect(m_model, &QStandardItemModel::itemChanged, this,
		[](QStandardItem *item) { qWarning() << item->text() << "\n"; });
}

QStandardItemModel *IIOModel::getModel() { return m_model; }

void IIOModel::demoSetup()
{
	// Create top-level items
	auto *rootItem = new IIOStandardItem("Root");

	// Add child items to the root
	auto *item1 = new IIOStandardItem("Item 1");
	auto *item2 = new IIOStandardItem("Item 2");

	// Add child items to Item 1
	auto *subItem1 = new IIOStandardItem("Subitem 1");
	auto *subItem2 = new IIOStandardItem("Subitem 2");

	// Add child items to Item 2
	auto *subItem3 = new IIOStandardItem("Subitem 3");

	// Add items to the model
	rootItem->appendRow(item1);
	rootItem->appendRow(item2);
	item1->appendRow(subItem1);
	item1->appendRow(subItem2);
	item2->appendRow(subItem3);

	m_model->appendRow(rootItem);
}

void IIOModel::iioTreeSetup()
{
	// TODO: maybe you should break this function in a few more...
	QString rootString = iio_context_get_name(m_ctx);
	auto *rootItem = new IIOStandardItem(rootString, IIOStandardItemModel::Context);
	rootItem->setEditable(false);

	// add all context attrs
	uint ctx_attrs_count = iio_context_get_attrs_count(m_ctx);
	for(int i = 0; i < ctx_attrs_count; ++i) {
		const char *name, *value;

		int result = iio_context_get_attr(m_ctx, i, &name, &value);
		if(result < 0) {
			qWarning() << "Error when reading context attr with id" << i << "\n";
		}

		m_entries.append(rootString + PATH_DELIMITER + name);
		auto *attrItem = new IIOStandardItem(QString(name) + " (" + QString(value) + ")",
						     IIOStandardItemModel::ContextAttribute);
		attrItem->setEditable(false);
		rootItem->appendRow(attrItem);
	}

	// add all devices from context, dfs
	uint ctx_devices_count = iio_context_get_devices_count(m_ctx);
	for(int i = 0; i < ctx_devices_count; ++i) {
		struct iio_device *device = iio_context_get_device(m_ctx, i);
		QString device_name = iio_device_get_name(device);
		bool is_trigger = iio_device_is_trigger(device);
		IIOStandardItem *device_item;
		if(is_trigger) {
			device_item = new IIOStandardItem(device_name + " (trigger)", IIOStandardItemModel::Device);
		} else {
			device_item = new IIOStandardItem(device_name, IIOStandardItemModel::Device);
		}
		device_item->setEditable(false);
		m_entries.append(rootString + PATH_DELIMITER + device_name);

		// add all attrs to current device
		uint device_attrs_count = iio_device_get_attrs_count(device);
		for(int j = 0; j < device_attrs_count; ++j) {
			QString device_attr = iio_device_get_attr(device, j);
			char buffer[BUFFER_SIZE] = {0};

			ssize_t result =
				iio_device_attr_read(device, device_attr.toStdString().c_str(), buffer, BUFFER_SIZE);
			if(result < 0) {
				qWarning() << "Error when reading device attr with id" << j << "from device with id"
					   << i << "\n";
			}

			m_entries.append(rootString + PATH_DELIMITER + device_name + PATH_DELIMITER + device_attr);
			auto *attrItem = new IIOStandardItem(device_attr + " (" + QString(buffer) + ")",
							     IIOStandardItemModel::DeviceAttribute);
			attrItem->setEditable(false);
			device_item->appendRow(attrItem);
		}

		// add all channels to current device
		uint device_channels_count = iio_device_get_channels_count(device);
		for(int j = 0; j < device_channels_count; ++j) {
			struct iio_channel *channel = iio_device_get_channel(device, j);
			QString channel_name = iio_channel_get_id(channel);
			auto *channel_item = new IIOStandardItem(channel_name, IIOStandardItemModel::Channel);
			channel_item->setEditable(false);
			m_entries.append(rootString + PATH_DELIMITER + device_name + PATH_DELIMITER + channel_name);

			// add all attrs from channel
			uint channel_attr_count = iio_channel_get_attrs_count(channel);
			for(int k = 0; k < channel_attr_count; ++k) {
				QString attr_name = iio_channel_get_attr(channel, k);
				char buffer[BUFFER_SIZE] = {0};
				ssize_t result = iio_channel_attr_read(channel, attr_name.toStdString().c_str(), buffer,
								       BUFFER_SIZE);
				if(result < 0) {
					qWarning() << "Error when reading channel attr with id" << k
						   << "from channel with id" << j << "from device with id" << i << "\n";
				}

				m_entries.append(rootString + PATH_DELIMITER + device_name + PATH_DELIMITER +
						 channel_name + PATH_DELIMITER + attr_name);
				auto *attr_item = new IIOStandardItem(attr_name + " (" + buffer + ")",
								      IIOStandardItemModel::ChannelAttribute);
				attr_item->setEditable(false);
				channel_item->appendRow(attr_item);
			}

			// add channel to device
			device_item->appendRow(channel_item);
		}

		// add device to ctx
		rootItem->appendRow(device_item);
	}

	m_model->appendRow(rootItem);
}

QStringList IIOModel::getEntries() { return m_entries; }
