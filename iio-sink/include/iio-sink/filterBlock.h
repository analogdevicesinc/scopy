#ifndef FILTERBLOCK_H
#define FILTERBLOCK_H

#include <qdebug.h>
#include "basicBlock.h"
#include "scopy-iio-sink_export.h"
#include <QMap>
#include <QObject>

namespace scopy::iiosink {
class SCOPY_IIO_SINK_EXPORT FilterBlock : public BasicBlock
{
    Q_OBJECT
public:
    FilterBlock(bool copy = false, QString name = QString());
    ~FilterBlock();

    void doCopy(bool copy);
    void onNewData(ChannelData *chData, uint ch);
    void addConnectedChannel(uint ch);
    void removeConnectedChannel(uint ch);

private:
    bool areChannelsReady();
    void resetChannels();

    virtual ChannelData *createData() = 0; // <buffer, size>

protected:
    bool m_copy;
    BlockData *m_data; // <channel, <buffer, size>>
    QMap<uint, bool> m_connectedChannels; // <channel, data_recieved>
};
}

#endif
