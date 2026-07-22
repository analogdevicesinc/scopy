#include "v1channelops.h"
#include <iio/iio.h>

namespace scopy::iio {

static iio_channel *chn(ChannelHandle h) { return static_cast<iio_channel *>(h.ptr); }
static iio_channels_mask *chMask(ChannelsMaskHandle h) { return static_cast<iio_channels_mask *>(h.ptr); }

QString V1ChannelOps::id(ChannelHandle h) const
{
    const char *s = iio_channel_get_id(chn(h));
    return s ? QString::fromUtf8(s) : QString();
}

QString V1ChannelOps::name(ChannelHandle h) const
{
    const char *s = iio_channel_get_name(chn(h));
    return s ? QString::fromUtf8(s) : QString();
}

QString V1ChannelOps::label(ChannelHandle h) const
{
    const char *s = iio_channel_get_label(chn(h));
    return s ? QString::fromUtf8(s) : QString();
}

bool V1ChannelOps::isOutput(ChannelHandle h) const { return iio_channel_is_output(chn(h)); }

bool V1ChannelOps::isScanElement(ChannelHandle h) const { return iio_channel_is_scan_element(chn(h)); }

long V1ChannelOps::index(ChannelHandle h) const { return iio_channel_get_index(chn(h)); }

int V1ChannelOps::channelType(ChannelHandle h) const { return static_cast<int>(iio_channel_get_type(chn(h))); }

int V1ChannelOps::modifier(ChannelHandle h) const { return static_cast<int>(iio_channel_get_modifier(chn(h))); }

unsigned int V1ChannelOps::attrsCount(ChannelHandle h) const { return iio_channel_get_attrs_count(chn(h)); }

QString V1ChannelOps::attrName(ChannelHandle h, unsigned int index) const
{
    const struct iio_attr *attr = iio_channel_get_attr(chn(h), index);
    const char *s = iio_attr_get_name(attr);
    return s ? QString::fromUtf8(s) : QString();
}

DataFormat V1ChannelOps::dataFormat(ChannelHandle h) const
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
        df.offset = fmt->offset;
    }
    return df;
}

void V1ChannelOps::enable(ChannelHandle h, ChannelsMaskHandle mask) { iio_channel_enable(chn(h), chMask(mask)); }

void V1ChannelOps::disable(ChannelHandle h, ChannelsMaskHandle mask) { iio_channel_disable(chn(h), chMask(mask)); }

bool V1ChannelOps::isEnabled(ChannelHandle h, ChannelsMaskHandle mask) const
{
    return iio_channel_is_enabled(chn(h), chMask(mask));
}

size_t V1ChannelOps::read(ChannelHandle h, BlockHandle block, void *dst, size_t len, bool raw) const
{
    return iio_channel_read(chn(h), static_cast<const iio_block *>(block.ptr), dst, len, raw);
}

size_t V1ChannelOps::write(ChannelHandle h, BlockHandle block, const void *src, size_t len, bool raw)
{
    return iio_channel_write(chn(h), static_cast<iio_block *>(block.ptr), src, len, raw);
}

void V1ChannelOps::convert(ChannelHandle h, void *dst, const void *src) const { iio_channel_convert(chn(h), dst, src); }

void V1ChannelOps::convertInverse(ChannelHandle h, void *dst, const void *src) const
{
    iio_channel_convert_inverse(chn(h), dst, src);
}

} // namespace scopy::iio
