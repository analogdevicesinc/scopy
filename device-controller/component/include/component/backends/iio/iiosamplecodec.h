#pragma once

#include "component/samplecodec.h"
#include "iioutil/dataformat.h"
#include "iioutil/handles.h"

namespace scopy::iio {
class IChannelOps;
}

namespace scopy::component::iio {

class IIOSampleCodec : public SampleCodec
{
	Q_OBJECT
public:
	IIOSampleCodec(scopy::iio::IChannelOps *ops, scopy::iio::ChannelHandle handle, QObject *parent = nullptr);

	const scopy::iio::DataFormat &dataFormat() const { return m_format; }

	void convert(void *dst, const void *src) const override;
	void convertInverse(void *dst, const void *src) const override;

private:
	scopy::iio::IChannelOps *m_ops;
	scopy::iio::ChannelHandle m_handle;
	scopy::iio::DataFormat m_format;
};

} // namespace scopy::component::iio
