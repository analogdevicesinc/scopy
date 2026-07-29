#include "component/backends/iio/iiocomponentbuilder.h"

#include "component/backends/iio/iioattribute.h"
#include "component/backends/iio/iioattributereader.h"
#include "component/backends/iio/iioattributewriter.h"
#include "component/backends/iio/iiochannel.h"
#include "component/backends/iio/iiocontext.h"
#include "component/backends/iio/iiodevice.h"
#include "component/backends/iio/iioinputstream.h"
#include "component/backends/iio/iiooutputstream.h"
#include "component/backends/iio/iioping.h"
#include "component/backends/iio/iiosamplecodec.h"
#include "component/backends/iio/iioscanelement.h"

#include "iioutil/iattrops.h"
#include "iioutil/ibackend.h"
#include "iioutil/ibufferops.h"
#include "iioutil/ichannelops.h"
#include "iioutil/icontextops.h"
#include "iioutil/ideviceops.h"

using namespace scopy;
using namespace scopy::component::iio;

bool IIOComponentBuilder::build(Context *ctx, ICmdExecutor *executor)
{
	auto *iioCtx = qobject_cast<IIOContext *>(ctx);
	if(!iioCtx || !iioCtx->handle().ptr || !executor || !m_backend) {
		return false;
	}

	m_attrOps = m_backend->attrOps();

	setContextIdentity(iioCtx);
	new IIOPing(m_backend->contextOps(), iioCtx->handle(), executor, iioCtx);
	buildContextAttributes(iioCtx, executor);
	buildDevices(iioCtx, executor);
	return true;
}

void IIOComponentBuilder::setContextIdentity(IIOContext *iioCtx)
{
	auto *ctxOps = m_backend->contextOps();
	const auto handle = iioCtx->handle();
	iioCtx->setName(ctxOps->name(handle));
	iioCtx->setDescription(ctxOps->description(handle));
}

void IIOComponentBuilder::buildContextAttributes(IIOContext *iioCtx, ICmdExecutor *executor)
{
	auto *ctxOps = m_backend->contextOps();
	const auto handle = iioCtx->handle();
	const unsigned int count = ctxOps->attrsCount(handle);
	for(unsigned int i = 0; i < count; ++i) {
		const QString name = ctxOps->attrName(handle, i);
		// Context attributes are always read-only in libiio.
		makeAttribute(iioCtx, name, m_attrOps->contextAttr(handle, name), executor, /*forceReadOnly=*/true);
	}
}

void IIOComponentBuilder::buildDevices(IIOContext *iioCtx, ICmdExecutor *executor)
{
	auto *ctxOps = m_backend->contextOps();
	const auto handle = iioCtx->handle();
	const unsigned int count = ctxOps->devicesCount(handle);
	for(unsigned int d = 0; d < count; ++d) {
		const scopy::iio::DeviceHandle dh = ctxOps->getDevice(handle, d);
		if(dh.ptr) {
			buildOneDevice(iioCtx, dh, executor);
		}
	}
}

void IIOComponentBuilder::buildOneDevice(IIOContext *iioCtx, scopy::iio::DeviceHandle dh, ICmdExecutor *executor)
{
	auto *devOps = m_backend->deviceOps();

	auto *dev = new IIODevice(iioCtx);
	dev->setHandle(dh);
	dev->setId(devOps->id(dh));
	dev->setName(devOps->name(dh));
	dev->setLabel(devOps->label(dh));

	buildDeviceAttributes(dev, executor);
	buildDebugAttributes(dev, executor);
	buildChannels(dev, executor);
	buildStreams(dev, executor);
}

void IIOComponentBuilder::buildDeviceAttributes(IIODevice *dev, ICmdExecutor *executor)
{
	auto *devOps = m_backend->deviceOps();
	const auto dh = dev->handle();
	const unsigned int count = devOps->attrsCount(dh);
	for(unsigned int i = 0; i < count; ++i) {
		const QString name = devOps->attrName(dh, i);
		makeAttribute(dev, name, m_attrOps->deviceAttr(dh, name), executor);
	}
}

void IIOComponentBuilder::buildDebugAttributes(IIODevice *dev, ICmdExecutor *executor)
{
	auto *devOps = m_backend->deviceOps();
	const auto dh = dev->handle();
	const unsigned int count = devOps->debugAttrsCount(dh);
	for(unsigned int i = 0; i < count; ++i) {
		const QString name = devOps->debugAttrName(dh, i);
		makeAttribute(dev, name, m_attrOps->debugAttr(dh, name), executor);
	}
}

void IIOComponentBuilder::buildChannels(IIODevice *dev, ICmdExecutor *executor)
{
	auto *devOps = m_backend->deviceOps();
	auto *chOps = m_backend->channelOps();
	const auto dh = dev->handle();

	const unsigned int count = devOps->channelsCount(dh);
	for(unsigned int c = 0; c < count; ++c) {
		const scopy::iio::ChannelHandle ch = devOps->getChannel(dh, c);
		if(!ch.ptr) {
			continue;
		}

		auto *chan = new IIOChannel(dev);
		chan->setHandle(ch);
		chan->setId(chOps->id(ch));
		chan->setName(chOps->name(ch));
		chan->setLabel(chOps->label(ch));
		chan->setIsOutput(chOps->isOutput(ch));

		const unsigned int attrs = chOps->attrsCount(ch);
		for(unsigned int i = 0; i < attrs; ++i) {
			const QString name = chOps->attrName(ch, i);
			makeAttribute(chan, name, m_attrOps->channelAttr(ch, name), executor);
		}

		if(chOps->isScanElement(ch)) {
			new IIOSampleCodec(chOps, ch, chan);
		}
	}
}

void IIOComponentBuilder::buildStreams(IIODevice *dev, ICmdExecutor *executor)
{
	auto *devOps = m_backend->deviceOps();
	auto *chOps = m_backend->channelOps();
	auto *bufOps = m_backend->bufferOps();
	const auto dh = dev->handle();

	const unsigned int count = devOps->channelsCount(dh);
	QList<scopy::iio::ChannelHandle> inputs, outputs;
	for(unsigned int c = 0; c < count; ++c) {
		const scopy::iio::ChannelHandle ch = devOps->getChannel(dh, c);
		if(!ch.ptr || !chOps->isScanElement(ch)) {
			continue;
		}
		(chOps->isOutput(ch) ? outputs : inputs).append(ch);
	}

	const int nbChannels = static_cast<int>(count);

	if(!inputs.isEmpty()) {
		auto *stream = new IIOInputStream(bufOps, chOps, dh, nbChannels, executor, dev);
		for(const scopy::iio::ChannelHandle ch : inputs) {
			auto *el = new IIOScanElement(chOps, ch, stream->mask(), executor, stream);
			el->setIndex(chOps->index(ch));
			el->setName(chOps->name(ch));
			el->setIsOutput(false);
		}
		buildBufferAttributes(stream, dh, executor);
	}
	if(!outputs.isEmpty()) {
		auto *stream = new IIOOutputStream(bufOps, chOps, dh, nbChannels, executor, dev);
		for(const scopy::iio::ChannelHandle ch : outputs) {
			auto *el = new IIOScanElement(chOps, ch, stream->mask(), executor, stream);
			el->setIndex(chOps->index(ch));
			el->setName(chOps->name(ch));
			el->setIsOutput(true);
		}
		buildBufferAttributes(stream, dh, executor);
	}
}

void IIOComponentBuilder::buildBufferAttributes(QObject *stream, scopy::iio::DeviceHandle dh, ICmdExecutor *executor)
{
	auto *devOps = m_backend->deviceOps();
	const unsigned int buffers = devOps->buffersCount(dh);
	for(unsigned int b = 0; b < buffers; ++b) {
		const unsigned int attrs = devOps->bufferAttrsCount(dh, b);
		for(unsigned int i = 0; i < attrs; ++i) {
			const QString name = devOps->bufferAttrName(dh, b, i);
			makeAttribute(stream, name, m_attrOps->bufferAttr(dh, b, name), executor);
		}
	}
}

scopy::component::Attribute *IIOComponentBuilder::makeAttribute(QObject *parent, const QString &attrName,
								scopy::iio::AttrHandle handle, ICmdExecutor *executor,
								bool forceReadOnly)
{
	auto *attr = new IIOAttribute(parent);
	attr->setHandle(handle);
	attr->setName(attrName);

	double min = 0, step = 0, max = 0;
	if(m_attrOps->getRange(handle, min, step, max)) {
		attr->setRange({min, step, max});
	}
	QStringList values;
	if(m_attrOps->getAvailable(handle, values) && !values.isEmpty()) {
		attr->setOptions(values);
	}

	attr->addReadCapability(new IIOAttributeReader(m_attrOps, handle, executor));
	if(!(forceReadOnly || isReadOnlyAttr(attrName))) {
		attr->addWriteCapability(new IIOAttributeWriter(m_attrOps, handle, executor));
	}
	return attr;
}

bool IIOComponentBuilder::isReadOnlyAttr(const QString &attrName)
{
	return attrName.endsWith(QStringLiteral("_available")) || attrName == QStringLiteral("name") ||
		attrName == QStringLiteral("label");
}
