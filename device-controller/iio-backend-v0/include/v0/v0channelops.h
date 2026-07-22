#pragma once

#include "iioutil/ichannelops.h"

namespace scopy::iio {

class V0ChannelOps : public IChannelOps
{
public:
	QString id(ChannelHandle ch) const override;
	QString name(ChannelHandle ch) const override;
	QString label(ChannelHandle ch) const override;
	bool isOutput(ChannelHandle ch) const override;
	bool isScanElement(ChannelHandle ch) const override;
	long index(ChannelHandle ch) const override;
	int channelType(ChannelHandle ch) const override;
	int modifier(ChannelHandle ch) const override;
	unsigned int attrsCount(ChannelHandle ch) const override;
	QString attrName(ChannelHandle ch, unsigned int index) const override;
    DataFormat dataFormat(ChannelHandle ch) const override;

	void enable(ChannelHandle ch, ChannelsMaskHandle mask) override;
	void disable(ChannelHandle ch, ChannelsMaskHandle mask) override;
	bool isEnabled(ChannelHandle ch, ChannelsMaskHandle mask) const override;

	size_t read(ChannelHandle ch, BlockHandle block, void *dst, size_t len, bool raw) const override;
	size_t write(ChannelHandle ch, BlockHandle block, const void *src, size_t len, bool raw) override;

	void convert(ChannelHandle ch, void *dst, const void *src) const override;
	void convertInverse(ChannelHandle ch, void *dst, const void *src) const override;
};

} // namespace scopy::iio
