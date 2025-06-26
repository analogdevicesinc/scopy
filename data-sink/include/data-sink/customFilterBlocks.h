#ifndef CUSTOMFILTERBLOCKS_H
#define CUSTOMFILTERBLOCKS_H

#include "filterBlock.h"
#include "scopy-data-sink_export.h"

namespace scopy::datasink {
class SCOPY_DATA_SINK_EXPORT AddFilterBlock : public FilterBlock
{
public:
    AddFilterBlock(bool copy = false, QString name = QString());
    ~AddFilterBlock();

private:
    ChannelDataVector *createData() override;
};
}

#endif
