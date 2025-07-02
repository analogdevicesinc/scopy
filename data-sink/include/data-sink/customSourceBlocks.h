#ifndef CUSTOMSOURCERBLOCKS_H
#define CUSTOMSOURCERBLOCKS_H

#include "sourceBlock.h"
#include "scopy-data-sink_export.h"
#include <iio.h>

namespace scopy::datasink {
class SCOPY_DATA_SINK_EXPORT FileSourceBlock : public SourceBlock
{
public:
    FileSourceBlock(QString filename, QString name = QString());
    ~FileSourceBlock();

private:
    QString m_filename;

    BlockData *createData() override;
};

class SCOPY_DATA_SINK_EXPORT TestSourceBlock : public SourceBlock
{
public:
	TestSourceBlock(QString name = QString());
	~TestSourceBlock();

private:
	BlockData *createData() override;
};

class SCOPY_DATA_SINK_EXPORT IIOSourceBlock : public SourceBlock
{
public:
    IIOSourceBlock(iio_device* dev, QString name = QString());
    ~IIOSourceBlock();

    bool sampleRateAvailable();
    double readSampleRate();
    struct iio_device* iioDev();

public Q_SLOTS:
    void enChannel(bool en, uint id) override;

private:
    QString findAttribute(QStringList possibleNames, iio_device *dev);

private:
    iio_device* m_dev;
    BlockData *createData() override;
    int count = 0;
    QString m_sampleRateAttribute;
    iio_buffer *m_buf;
    size_t m_current_buf_size;
};
}

#endif
