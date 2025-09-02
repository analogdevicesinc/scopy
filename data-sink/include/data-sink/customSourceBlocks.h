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
	void setPlotSize(size_t size) override;

	void findDevSampleRateAttribute();
	void populateChannelInfo(uint id);
	void removeChannelInfo(uint id);

	bool sampleRateAvailable(int id = -1);
	bool scaleAttributeAvailable(uint id);
	QString getDevSampleRateAttr();
	QString getChSampleRateAttr(uint id);

	double readSampleRate(int id = -1);
	double readChannelScale(uint id);
	QString getChScaleAttr(uint id);

	void setTimeAxisSR(double sr);

	struct iio_device *iioDev();
	std::vector<float> getTimeAxis();
	iio_channel *getIIOChannel(uint id);

	const iio_data_format *getFmt(uint id) const;

public Q_SLOTS:
	void enChannel(bool en, uint id) override;

private:
	void generateTimeAxis();

private:
	iio_device *m_dev;
	BlockData createData() override;
	int count = 0;
	QString m_chSampleRateAttribute;
	QString m_scaleAttribute;
	iio_buffer *m_buf;
	size_t m_current_buf_size;
	std::vector<float> m_timeAxis;

	struct ChannelInfo {
		QString sampleRateAttr;
		QString scaleAttr;
		const iio_data_format *fmt = nullptr;

		bool hasSampleRateAttr() const { return !sampleRateAttr.isEmpty(); }
		bool hasScaleAttr() const { return !scaleAttr.isEmpty(); }
	};

	QString m_devSampleRateAttr;
	QMap<uint, ChannelInfo> m_channelInfoMap;
	double m_timeAxisSR; // this may not always be the iio samplerate. Depends on the XMode used
};

class SCOPY_DATA_SINK_EXPORT StaticSourceBlock : public SourceBlock
{
public:
	StaticSourceBlock(QString name = QString());
	~StaticSourceBlock();

	void setData(std::vector<float> xdata, std::vector<float> ydata);
	std::vector<float> getTimeAxis();
	std::vector<float> getCurrentData();

private:
	BlockData createData() override;

	std::vector<float> m_timeAxis;
	BlockData m_map;
};
} // namespace scopy::datasink

#endif
