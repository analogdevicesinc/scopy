#include "v0channelops.h"
#include <iio.h>

namespace scopy::iio {

static iio_channel *chn(ChannelHandle h) { return static_cast<iio_channel *>(h.ptr); }

QString V0ChannelOps::id(ChannelHandle h) const
{
	const char *s = iio_channel_get_id(chn(h));
	return s ? QString::fromUtf8(s) : QString();
}

QString V0ChannelOps::name(ChannelHandle h) const
{
	const char *s = iio_channel_get_name(chn(h));
	return s ? QString::fromUtf8(s) : QString();
}

QString V0ChannelOps::label(ChannelHandle h) const
{
	// v0 has no iio_channel_get_label
	return {};
}

bool V0ChannelOps::isOutput(ChannelHandle h) const { return iio_channel_is_output(chn(h)); }

bool V0ChannelOps::isScanElement(ChannelHandle h) const { return iio_channel_is_scan_element(chn(h)); }

long V0ChannelOps::index(ChannelHandle h) const { return iio_channel_get_index(chn(h)); }

int V0ChannelOps::channelType(ChannelHandle h) const { return static_cast<int>(iio_channel_get_type(chn(h))); }

int V0ChannelOps::modifier(ChannelHandle h) const { return static_cast<int>(iio_channel_get_modifier(chn(h))); }

unsigned int V0ChannelOps::attrsCount(ChannelHandle h) const { return iio_channel_get_attrs_count(chn(h)); }

QString V0ChannelOps::attrName(ChannelHandle h, unsigned int index) const
{
	const char *s = iio_channel_get_attr(chn(h), index);
	return s ? QString::fromUtf8(s) : QString();
}

DataFormat V0ChannelOps::dataFormat(ChannelHandle h) const
{
	const iio_data_format *fmt = iio_channel_get_data_format(chn(h));
	DataFormat df = {};
	if(fmt) {
		df.length = fmt->length;
		df.bits = fmt->bits;
		df.shift = fmt->shift;
		df.is_signed = fmt->is_signed;
		df.is_fully_defined = fmt->is_fully_defined;
		df.is_be = fmt->is_be;
		df.with_scale = fmt->with_scale;
		df.scale = fmt->scale;
		df.repeat = fmt->repeat;
		df.offset = 0.0; // v0 has no offset field
	}
	return df;
}

void V0ChannelOps::enable(ChannelHandle h, ChannelsMaskHandle /*mask*/) { iio_channel_enable(chn(h)); }

void V0ChannelOps::disable(ChannelHandle h, ChannelsMaskHandle /*mask*/) { iio_channel_disable(chn(h)); }

bool V0ChannelOps::isEnabled(ChannelHandle h, ChannelsMaskHandle /*mask*/) const
{
	return iio_channel_is_enabled(chn(h));
}

size_t V0ChannelOps::read(ChannelHandle h, BlockHandle block, void *dst, size_t len, bool raw) const
{
	// v0: BlockHandle.ptr is iio_buffer*
	auto *buf = static_cast<iio_buffer *>(block.ptr);
	if(raw) {
		return iio_channel_read_raw(chn(h), buf, dst, len);
	}
	return iio_channel_read(chn(h), buf, dst, len);
}

size_t V0ChannelOps::write(ChannelHandle h, BlockHandle block, const void *src, size_t len, bool raw)
{
	auto *buf = static_cast<iio_buffer *>(block.ptr);
	if(raw) {
		return iio_channel_write_raw(chn(h), buf, src, len);
	}
	return iio_channel_write(chn(h), buf, src, len);
}

void V0ChannelOps::convert(ChannelHandle h, void *dst, const void *src) const { iio_channel_convert(chn(h), dst, src); }

void V0ChannelOps::convertInverse(ChannelHandle h, void *dst, const void *src) const
{
	iio_channel_convert_inverse(chn(h), dst, src);
}

} // namespace scopy::iio
