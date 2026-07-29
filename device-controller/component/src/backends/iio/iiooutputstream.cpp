#include "component/backends/iio/iiooutputstream.h"

#include "component/backends/iio/iioscanelement.h"
#include "component/capabilityexecutor.h"
#include "iioutil/commands/bufferclosecommand.h"
#include "iioutil/commands/bufferopencommand.h"
#include "iioutil/commands/bufferpushcommand.h"
#include "iioutil/ibufferops.h"
#include "iioutil/ichannelops.h"

#include <cerrno>
#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::iio;

IIOOutputStream::IIOOutputStream(scopy::iio::IBufferOps *bufOps, scopy::iio::IChannelOps *chOps,
				 scopy::iio::DeviceHandle dev, int nbChannels, scopy::ICmdExecutor *executor,
				 QObject *parent)
	: OutputStream(parent)
	, m_bufOps(bufOps)
	, m_chOps(chOps)
	, m_dev(dev)
	, m_mask(bufOps->createChannelsMask(nbChannels))
	, m_executor(executor)
{
}

IIOOutputStream::~IIOOutputStream()
{
	if(m_open && m_buffer.ptr) {
		m_bufOps->cancelBuffer(m_buffer);
		m_bufOps->closeBuffer(m_buffer);
	}
	m_bufOps->destroyChannelsMask(m_mask);
}

Result<void> IIOOutputStream::setKernelBuffers(unsigned int n)
{
	if(m_open) {
		return Result<void>(Unexpected{Error{-EBUSY, QStringLiteral("cannot set kernelBuffers while open")}});
	}
	m_kernelBuffers = n;
	return Result<void>();
}

Result<void> IIOOutputStream::setCyclic(bool cyclic)
{
	if(m_open) {
		return Result<void>(Unexpected{Error{-EBUSY, QStringLiteral("cannot set cyclic while open")}});
	}
	m_cyclic = cyclic;
	return Result<void>();
}

void IIOOutputStream::applyEnabledChannels(const QList<int> &channels)
{
	const QList<IIOScanElement *> elems = findChildren<IIOScanElement *>(QString(), Qt::FindDirectChildrenOnly);
	for(IIOScanElement *el : elems) {
		el->enable(channels.contains(static_cast<int>(el->index())));
	}
}

QCoro::Task<Result<void>> IIOOutputStream::openInternal(StreamConfig cfg)
{
	applyEnabledChannels(cfg.enabledChannels);

	auto *cmd = new scopy::iio::BufferOpenCommand(m_bufOps, m_dev, 0, m_mask, cfg.samplesCount, m_cyclic,
						      m_kernelBuffers);
	auto result = co_await runCommand(
		m_executor, cmd, [this, cmd](const Result<void> &) { m_buffer = cmd->openedHandle(); },
		[this](const Error &e) { Q_EMIT streamError(e); });

	if(result) {
		m_config = cfg;
		m_open = true;
		buildStreamFormat();
		Q_EMIT openedChanged(true);
	}
	co_return result;
}

Result<void> IIOOutputStream::open(const StreamConfig &cfg) { return QCoro::waitFor(openInternal(cfg)); }

QCoro::Task<void> IIOOutputStream::closeInternal()
{
	if(!m_open || !m_buffer.ptr) {
		co_return;
	}
	m_bufOps->cancelBuffer(m_buffer);
	auto *cmd = new scopy::iio::BufferCloseCommand(m_bufOps, m_buffer);
	co_await m_executor->execute(cmd);
	cmd->deleteLater();
	m_buffer = {};
	m_open = false;
	Q_EMIT openedChanged(false);
}

void IIOOutputStream::close() { QCoro::waitFor(closeInternal()); }

QCoro::Task<Result<void>> IIOOutputStream::pushInternal(scopy::iio::BufferPushCommand *cmd)
{
	co_return co_await runCommand(
		m_executor, cmd, [this](const Result<void> &) { Q_EMIT pushSucceeded(); },
		[this](const Error &e) { Q_EMIT pushFailed(e); });
}

Result<void> IIOOutputStream::push()
{
	return QCoro::waitFor(pushInternal(new scopy::iio::BufferPushCommand(m_bufOps, m_buffer)));
}

QUuid IIOOutputStream::pushAsync()
{
	auto *cmd = new scopy::iio::BufferPushCommand(m_bufOps, m_buffer);
	const QUuid id = cmd->id();
	pushInternal(cmd);
	return id;
}

void IIOOutputStream::buildStreamFormat()
{
	StreamFormat f;
	auto *start = static_cast<char *>(m_bufOps->bufferStart(m_buffer));
	auto *end = static_cast<char *>(m_bufOps->bufferEnd(m_buffer));
	const ptrdiff_t stride = m_bufOps->bufferStep(m_buffer);

	f.data = start;
	f.sampleCount = stride > 0 ? static_cast<size_t>((end - start) / stride) : 0;

	const QList<IIOScanElement *> elems = findChildren<IIOScanElement *>(QString(), Qt::FindDirectChildrenOnly);
	for(IIOScanElement *el : elems) {
		if(!el->isEnabled()) {
			continue;
		}
		const scopy::iio::ChannelHandle ch = el->channelHandle();
		const scopy::iio::DataFormat fmt = m_chOps->dataFormat(ch);
		auto *first = static_cast<char *>(m_bufOps->bufferFirst(m_buffer, ch));

		ChannelFormat cf;
		cf.offset = first - start;
		cf.stride = stride;
		cf.type = dataTypeOf(fmt.bits, fmt.is_signed);
		cf.validBits = static_cast<uint8_t>(fmt.bits);
		cf.shift = static_cast<uint8_t>(fmt.shift);
		cf.scale = fmt.with_scale ? fmt.scale : 1.0;
		cf.offsetPhys = fmt.offset;
		f.order = fmt.is_be ? ByteOrder::BigEndian : ByteOrder::LittleEndian;
		f.channels.append(cf);
	}
	m_format = f;
}
