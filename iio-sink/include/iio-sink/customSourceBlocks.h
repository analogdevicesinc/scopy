#ifndef CUSTOMSOURCERBLOCKS_H
#define CUSTOMSOURCERBLOCKS_H

#include "sourceBlock.h"
#include "scopy-iio-sink_export.h"
#include <iio.h>

namespace scopy::iiosink {
class SCOPY_IIO_SINK_EXPORT FileSourceBlock : public SourceBlock
{
public:
    FileSourceBlock(QString filename, QString name = QString());
    ~FileSourceBlock();

private:
    QString m_filename;

    BlockData *createData() override;
};

class SCOPY_IIO_SINK_EXPORT IIOSourceBlock : public SourceBlock
{
public:
    IIOSourceBlock(iio_device* dev, QString name = QString());
    ~IIOSourceBlock();

public Q_SLOTS:
    void enChannel(bool en, uint id);

private:
    iio_device* m_dev;
    BlockData *createData() override;
    int count = 0;
};
}

#endif
