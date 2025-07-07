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

	BlockData createData() override;
};

class SCOPY_DATA_SINK_EXPORT TestSourceBlock : public SourceBlock
{
public:
	TestSourceBlock(QString name = QString());
	~TestSourceBlock();

private:
	BlockData createData() override;
};

class SCOPY_DATA_SINK_EXPORT IIOSourceBlock : public SourceBlock
{
public:
	IIOSourceBlock(iio_device *dev, QString name = QString());
	~IIOSourceBlock();

	void setBufferSize(size_t size) override;
	bool sampleRateAvailable();
	double readSampleRate();
	struct iio_device *iioDev();
	std::vector<float> getTimeAxis();

public Q_SLOTS:
	void enChannel(bool en, uint id) override;

private:
	void findSRAttribute();
	QString findDevAttribute(QStringList possibleNames);
	QPair<QString, struct iio_channel *> findChAttribute(QStringList possibleNames);
	void generateTimeAxis();

private:
	iio_device *m_dev;
	BlockData createData() override;
	int count = 0;
	QString m_devSampleRateAttribute;
	QPair<QString, struct iio_channel *> m_chSampleRateAttribute;
	iio_buffer *m_buf;
	size_t m_current_buf_size;
	std::vector<float> m_timeAxis;
};
} // namespace scopy::datasink

#endif
