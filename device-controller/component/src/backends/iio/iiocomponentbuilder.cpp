/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "component/backends/iio/iiocomponentbuilder.h"

#include "component/backends/iio/iioattribute.h"
#include "component/backends/iio/iioattributereader.h"
#include "component/backends/iio/iioattributewriter.h"
#include "component/backends/iio/iiochannel.h"
#include "component/backends/iio/iiocontext.h"
#include "component/backends/iio/iiodevice.h"
#include "component/backends/iio/iioinputstream.h"
#include "component/backends/iio/iiooutputstream.h"
#include "component/backends/iio/iioregisterreader.h"
#include "component/backends/iio/iioregisterwriter.h"
#include "component/backends/iio/iiosamplecodec.h"
#include "component/backends/iio/iioscanelement.h"
#include "component/backends/iio/iiotrigger.h"
#include "component/backends/iio/iiotriggerable.h"

#include "iioutil/iattrops.h"
#include "iioutil/ibackend.h"
#include "iioutil/ibufferops.h"
#include "iioutil/ichannelops.h"
#include "iioutil/icontextops.h"
#include "iioutil/ideviceops.h"

#include <algorithm>

using namespace scopy;
using namespace scopy::component::iio;

bool IIOComponentBuilder::build(Context *ctx, ICmdExecutor *executor)
{
	auto *iioCtx = qobject_cast<IIOContext *>(ctx);
	if(!iioCtx || !m_handle.ptr || !executor || !m_backend) {
		return false;
	}

	m_attrOps = m_backend->attrOps();

	buildContextAttributes(iioCtx, executor);
	buildDevices(iioCtx, executor);
	return true;
}

void IIOComponentBuilder::buildContextAttributes(IIOContext *iioCtx, ICmdExecutor *executor)
{
	auto *ctxOps = m_backend->contextOps();
	const auto handle = m_handle;
	const unsigned int count = ctxOps->attrsCount(handle);
	for(unsigned int i = 0; i < count; ++i) {
		const QString name = ctxOps->attrName(handle, i);
		const scopy::iio::AttrHandle attr = m_attrOps->contextAttr(handle, name);
		// Context attributes are always read-only in libiio.
		IIOAttribute *a = makeAttribute(iioCtx, name, attr, executor, /*forceReadOnly=*/true);
		// Context attrs are static strings; read eagerly and cache so clients (e.g. the device
		// info page) can read the value off the Attribute child without an async round-trip.
		scopy::Result<QByteArray> value = m_attrOps->read(attr);
		if(value && a) {
			a->setCachedValue(QString::fromUtf8(value.value()));
		}
	}
}

void IIOComponentBuilder::buildDevices(IIOContext *iioCtx, ICmdExecutor *executor)
{
	auto *ctxOps = m_backend->contextOps();
	const auto handle = m_handle;
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
	dev->setIsHwmon(devOps->isHwmon(dh));

	const AttrMap devAttrs = buildDeviceAttributes(dev, executor);
	buildDebugAttributes(dev, executor);
	buildChannels(dev, devAttrs, executor);
	buildStreams(dev, executor);
	buildTrigger(dev, executor);
}

void IIOComponentBuilder::buildTrigger(IIODevice *dev, ICmdExecutor *executor)
{
	auto *devOps = m_backend->deviceOps();
	const auto dh = dev->handle();

	// A device that IS a trigger carries a source marker and has no trigger of its
	// own, so it gets no Triggerable.
	if(devOps->isTrigger(dh)) {
		new IIOTrigger(devOps, dh, dev);
		return;
	}

	// A trigger gates acquisition, so only a device with an input stream can be
	// assigned one. buildStreams ran above, so the stream children (if any) are present.
	if(!scopy::component::streamsOf<IIOInputStream>(dev).isEmpty()) {
		new IIOTriggerable(devOps, dh, executor, dev);
	}
}

IIOComponentBuilder::AttrMap IIOComponentBuilder::buildDeviceAttributes(IIODevice *dev, ICmdExecutor *executor)
{
	auto *devOps = m_backend->deviceOps();
	const auto dh = dev->handle();
	AttrMap attrs;
	const unsigned int count = devOps->attrsCount(dh);
	for(unsigned int i = 0; i < count; ++i) {
		const QString name = devOps->attrName(dh, i);
		attrs.insert(name, makeAttribute(dev, name, m_attrOps->deviceAttr(dh, name), executor));
	}
	applyAvailable(attrs);
	return attrs;
}

void IIOComponentBuilder::buildDebugAttributes(IIODevice *dev, ICmdExecutor *executor)
{
	auto *devOps = m_backend->deviceOps();
	const auto dh = dev->handle();
	const unsigned int count = devOps->debugAttrsCount(dh);
	bool hasRegAccess = false;
	AttrMap attrs;
	for(unsigned int i = 0; i < count; ++i) {
		const QString name = devOps->debugAttrName(dh, i);
		attrs.insert(name, makeAttribute(dev, name, m_attrOps->debugAttr(dh, name), executor));
		if(name.contains("direct_reg_access")) {
			hasRegAccess = true;
		}
	}
	applyAvailable(attrs);

	// Register access — only where the device exposes a debug/register plane.
	if(hasRegAccess) {
		new IIORegisterReader(devOps, dh, executor, dev);
		new IIORegisterWriter(devOps, dh, executor, dev);
	}
}

void IIOComponentBuilder::buildChannels(IIODevice *dev, const AttrMap &devAttrs, ICmdExecutor *executor)
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
		chan->setChanType(chOps->channelType(ch));

		AttrMap attrs;
		const unsigned int attrCount = chOps->attrsCount(ch);
		for(unsigned int i = 0; i < attrCount; ++i) {
			const QString name = chOps->attrName(ch, i);
			attrs.insert(name, makeAttribute(chan, name, m_attrOps->channelAttr(ch, name), executor));
		}
		// Some drivers expose a channel attr's "_available" at device level.
		applyAvailable(attrs, &devAttrs);

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

	// libiio v1 exposes multiple buffers per device; attach one stream per buffer
	// index per present direction, each with its own mask and scan elements. A
	// single-buffer device (buffersCount 0 or 1) collapses to one stream per direction.
	const unsigned buffers = std::max(1u, devOps->buffersCount(dh));
	for(unsigned b = 0; b < buffers; ++b) {
		if(!inputs.isEmpty()) {
			auto *stream = new IIOInputStream(bufOps, chOps, dh, nbChannels, b, executor, dev);
			for(const scopy::iio::ChannelHandle ch : inputs) {
				auto *el = new IIOScanElement(chOps, ch, stream->mask(), executor, stream);
				el->setIndex(chOps->index(ch));
				el->setId(chOps->id(ch));
				el->setIsOutput(false);
			}
			buildBufferAttributes(stream, dh, b, executor);
		}
		if(!outputs.isEmpty()) {
			auto *stream = new IIOOutputStream(bufOps, chOps, dh, nbChannels, b, executor, dev);
			for(const scopy::iio::ChannelHandle ch : outputs) {
				auto *el = new IIOScanElement(chOps, ch, stream->mask(), executor, stream);
				el->setIndex(chOps->index(ch));
				el->setId(chOps->id(ch));
				el->setIsOutput(true);
			}
			buildBufferAttributes(stream, dh, b, executor);
		}
	}
}

void IIOComponentBuilder::buildBufferAttributes(QObject *stream, scopy::iio::DeviceHandle dh, unsigned int bufferIndex,
						ICmdExecutor *executor)
{
	auto *devOps = m_backend->deviceOps();
	AttrMap attrs;
	const unsigned int count = devOps->bufferAttrsCount(dh, bufferIndex);
	for(unsigned int i = 0; i < count; ++i) {
		const QString name = devOps->bufferAttrName(dh, bufferIndex, i);
		attrs.insert(name, makeAttribute(stream, name, m_attrOps->bufferAttr(dh, bufferIndex, name), executor));
	}
	applyAvailable(attrs);
}

IIOAttribute *IIOComponentBuilder::makeAttribute(QObject *parent, const QString &attrName,
						 scopy::iio::AttrHandle handle, ICmdExecutor *executor,
						 bool forceReadOnly)
{
	auto *attr = new IIOAttribute(parent);
	attr->setHandle(handle);
	attr->setName(attrName);

	attr->addReadCapability(new IIOAttributeReader(m_attrOps, handle, executor));
	if(!(forceReadOnly || isReadOnlyAttr(attrName))) {
		attr->addWriteCapability(new IIOAttributeWriter(m_attrOps, handle, executor));
	}
	return attr;
}

void IIOComponentBuilder::applyAvailable(const AttrMap &scope, const AttrMap *fallback)
{
	static const QString kSuffix = QStringLiteral("_available");
	for(auto it = scope.cbegin(); it != scope.cend(); ++it) {
		if(it.key().endsWith(kSuffix)) {
			continue;
		}
		const QString availName = it.key() + kSuffix;
		IIOAttribute *avail = scope.value(availName);
		if(!avail && fallback) {
			avail = fallback->value(availName);
		}
		if(!avail) {
			continue;
		}

		double min = 0, step = 0, max = 0;
		if(m_attrOps->getRange(avail->handle(), min, step, max)) {
			it.value()->setRange({min, step, max});
			continue; // range and list are mutually exclusive
		}
		QStringList values;
		if(m_attrOps->getAvailable(avail->handle(), values) && !values.isEmpty()) {
			it.value()->setOptions(values);
		}
	}
}

bool IIOComponentBuilder::isReadOnlyAttr(const QString &attrName)
{
	return attrName.endsWith(QStringLiteral("_available")) || attrName == QStringLiteral("name") ||
		attrName == QStringLiteral("label");
}
