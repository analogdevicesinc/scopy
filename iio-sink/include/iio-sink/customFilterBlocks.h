#ifndef CUSTOMFILTERBLOCKS_H
#define CUSTOMFILTERBLOCKS_H

#include "filterBlock.h"
#include "scopy-iio-sink_export.h"

namespace scopy::iiosink {
class SCOPY_IIO_SINK_EXPORT AddFilterBlock : public FilterBlock
{
public:
    AddFilterBlock(bool copy = false, QString name = QString());
    ~AddFilterBlock();

private:
    ChannelData *createData() override;
};
}

#endif
