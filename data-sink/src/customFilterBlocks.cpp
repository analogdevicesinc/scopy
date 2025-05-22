#include <include/data-sink/customFilterBlocks.h>
using namespace scopy::datasink;

AddFilterBlock::AddFilterBlock(bool copy, QString name)
    : FilterBlock(copy, name)
{}
AddFilterBlock::~AddFilterBlock() {}

ChannelData *AddFilterBlock::createData()
{
    // find min size channel
    int size = 0;
    for(auto chData : *m_data) {
        if(size == 0) {
            size = chData->data.size();
        }
        size = std::min(size, (int)chData->data.size());
    }

    if(size <= 0)
        return new ChannelData();

    // add data
    ChannelData *new_data = new ChannelData(size);
    ChannelData *firstCh = m_data->first();

    for(int i = 0; i < size; ++i) {
        for(auto chData : *m_data)
            if(chData == firstCh) {
                new_data->data.push_back(chData->data[i]);
            } else {
                new_data->data[i] += chData->data[i];
            }
    }

    return new_data;
}
