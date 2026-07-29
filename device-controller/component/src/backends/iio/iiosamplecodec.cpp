#include "component/backends/iio/iiosamplecodec.h"

#include "iioutil/ichannelops.h"

using namespace scopy::component::iio;

IIOSampleCodec::IIOSampleCodec(scopy::iio::IChannelOps *ops, scopy::iio::ChannelHandle handle, QObject *parent)
	: SampleCodec(parent)
	, m_ops(ops)
	, m_handle(handle)
	, m_format(ops->dataFormat(handle))
{
}

void IIOSampleCodec::convert(void *dst, const void *src) const { m_ops->convert(m_handle, dst, src); }

void IIOSampleCodec::convertInverse(void *dst, const void *src) const { m_ops->convertInverse(m_handle, dst, src); }
