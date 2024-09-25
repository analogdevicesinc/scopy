#include "iiotreescan.h"
#include <QLoggingCategory>
#include <QApplication>
#include <connectionprovider.h>

Q_LOGGING_CATEGORY(CAT_IIOTREESCAN, "IIOTreeScan")

using namespace scopy;

IIOTreeScan::IIOTreeScan(iio_context *ctx, QObject *parent)
	: QObject{parent}
{
	m_rootItem = new IIOItem(iio_context_get_name(ctx), "", IIOItem::CONTEXT, ctx, nullptr);
	m_rootItem->setEditable(false);

	int ctxAttrs = iio_context_get_attrs_count(ctx);
	for(int i = 0; i < ctxAttrs; ++i) {
		const char *ctxAttrName;
		const char *ctxAttrValue;
		int res = iio_context_get_attr(ctx, i, &ctxAttrName, &ctxAttrValue);
		if(res < 0) {
			qWarning(CAT_IIOTREESCAN) << "Error when reading the context attr" << i;
			continue;
		}
		IIOItem *item = new IIOItem(ctxAttrName, "", IIOItem::CONTEXT_ATTR, ctx, m_rootItem);
		item->setEditable(false);
		m_rootItem->addChild(item);
	}

	int ctxDevs = iio_context_get_devices_count(ctx);
	for(int i = 0; i < ctxDevs; ++i) {
		iio_device *dev = iio_context_get_device(ctx, i);
		QString devName = iio_device_get_name(dev);
		QString devId = iio_device_get_id(dev);
		IIOItem *devItem = new IIOItem(devName, devId, IIOItem::DEVICE, dev, m_rootItem);
		devItem->setEditable(false);

		int devAttrCount = iio_device_get_attrs_count(dev);
		for(int j = 0; j < devAttrCount; ++j) {
			QString devAttrName = iio_device_get_attr(dev, j);
			IIOItem *devAttrItem = new IIOItem(devAttrName, "", IIOItem::DEVICE_ATTR, dev, devItem);
			devAttrItem->setEditable(false);
			devItem->addChild(devAttrItem);
		}

		int devChannelCount = iio_device_get_channels_count(dev);
		for(int j = 0; j < devChannelCount; ++j) {
			iio_channel *chnl = iio_device_get_channel(dev, j);
			QString channelName = iio_channel_get_name(chnl);
			QString channelId = iio_channel_get_id(chnl);
			IIOItem *chnlItem = new IIOItem(channelName, channelId, IIOItem::CHANNEL, chnl, devItem);
			chnlItem->setEditable(false);

			int chnlAttrCount = iio_channel_get_attrs_count(chnl);
			for(int k = 0; k < chnlAttrCount; ++k) {
				QString channelAttrName = iio_channel_get_attr(chnl, k);
				IIOItem *chnlAttrItem =
					new IIOItem(channelAttrName, "", IIOItem::CHANNEL_ATTR, chnl, chnlItem);
				chnlAttrItem->setEditable(false);
				chnlItem->addChild(chnlAttrItem);
			}
			devItem->addChild(chnlItem);
		}
		m_rootItem->addChild(devItem);
	}
}

IIOTreeScan::~IIOTreeScan() { qDebug(CAT_IIOTREESCAN) << "IIOTreeScan object dtor"; }

IIOItem *IIOTreeScan::getRoot() { return m_rootItem; }

#include "moc_iiotreescan.cpp"
