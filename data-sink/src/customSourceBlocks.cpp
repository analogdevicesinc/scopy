#include <QThread>
#include <cmath>
#include <include/data-sink/customSourceBlocks.h>
#include <QDateTime>
#include <qfile.h>
using namespace scopy::datasink;

FileSourceBlock::FileSourceBlock(QString filename, QString name)
	: SourceBlock(name)
	, m_filename(filename)
{}

FileSourceBlock::~FileSourceBlock() {}

BlockData FileSourceBlock::createData()
{
	QFile file = QFile(m_filename);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "Error: Could not open file";
		return {};
	}

	QTextStream m_stream(&file);

	BlockData map = BlockData();
	int numRows = 0;

	while(!m_stream.atEnd()) {
		QString line = m_stream.readLine();
		QStringList values = line.split(',');

		for(int col = 0; col < values.size(); ++col) {
			if(!m_channels.value(col) && !map.contains(col))
				continue;

			if(m_bufferSize <= numRows) {
				break;
			}

			// REMOVE THIS
			// + rand() % 20
			float num = values[col].toFloat();

			if(numRows == 0) {
				ChannelDataVector data = ChannelDataVector(m_bufferSize);
				map.insert(col, data);
				data.data.push_back(num);
			} else {
				map[col].data.push_back(num);
			}
		}
		numRows++;
	}

	return map;
}

TestSourceBlock::TestSourceBlock(QString name)
	: SourceBlock(name)
{}

TestSourceBlock::~TestSourceBlock() {}

BlockData TestSourceBlock::createData()
{
	BlockData map = BlockData();

	for(int ch = 0; ch < m_channels.size(); ++ch) {
		if(m_channels.value(ch)) {
			ChannelDataVector data = ChannelDataVector(m_bufferSize);

			for(int i = 0; i < m_bufferSize; i++) {
				data.data.push_back(i * pow(10, ch));
			}
			map.insert(ch, data);
		}
	}

	return map;
}

IIOSourceBlock::IIOSourceBlock(iio_device *dev, QString name)
	: SourceBlock(name)
	, m_buf(nullptr)
	, m_current_buf_size(-1)
	, m_timeAxisSR(-1)
{
	m_dev = dev;
	findDevSampleRateAttribute();
}

IIOSourceBlock::~IIOSourceBlock() {}

void IIOSourceBlock::setBufferSize(size_t size)
{
	SourceBlock::setBufferSize(size);
	generateTimeAxis();
}

void IIOSourceBlock::setPlotSize(size_t size)
{
	SourceBlock::setPlotSize(size);
	generateTimeAxis();
}

void IIOSourceBlock::enChannel(bool en, uint id)
{
	SourceBlock::enChannel(en, id);

	if(en) {
		iio_channel_enable(iio_device_get_channel(m_dev, id));
	} else {
		iio_channel_disable(iio_device_get_channel(m_dev, id));
	}
}

struct iio_channel *IIOSourceBlock::getIIOChannel(uint id) { return iio_device_get_channel(m_dev, id); }

iio_device *IIOSourceBlock::iioDev() { return m_dev; }

std::vector<float> IIOSourceBlock::getTimeAxis()
{
	if(m_timeAxis.empty()) {
		generateTimeAxis();
	}

	return m_timeAxis;
}

BlockData IIOSourceBlock::createData()
{
	iio_channel *rx0_i = iio_device_find_channel(m_dev, m_channels.value(0) ? "voltage0" : "voltage1", false);
	// QElapsedTimer timer;
	// QThread::msleep(60);
	// std::cout << "before: " << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss,zzz").toStdString() <<
	// '\n'; timer.start();

	if(m_current_buf_size != m_bufferSize) {
		if(m_buf)
			iio_buffer_destroy(m_buf);
		m_buf = iio_device_create_buffer(m_dev, m_bufferSize, false);
		m_current_buf_size = m_bufferSize;
	}
	// std::cout << "buff: " << timer.elapsed() << " of size: " << m_bufferSize << std::endl;

	BlockData map = BlockData();
	ssize_t nbytes_rx;
	char *p_dat, *p_end;
	ptrdiff_t p_inc;

	if(!m_buf) {
		return {};
	}

	// Refill RX buffer
	nbytes_rx = iio_buffer_refill(m_buf);
	if(nbytes_rx < 0) {
		printf("Error refilling buf %d\n", (int)nbytes_rx);
		return {};
	}

	// READ: Get pointers to RX buf and read IQ from RX buf port 0
	p_inc = iio_buffer_step(m_buf);
	p_end = (char *)iio_buffer_end(m_buf);

	int i;
	for(auto it = m_channels.begin(); it != m_channels.end(); ++it) {
		if(it.value()) {
			ChannelDataVector data = ChannelDataVector(m_bufferSize);

			i = 0;
			for(p_dat = (char *)iio_buffer_first(m_buf, rx0_i); p_dat < p_end; p_dat += p_inc) {
				// // Example: swap I and Q
				// const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
				// const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
				// ((int16_t*)p_dat)[0] = q;
				// ((int16_t*)p_dat)[1] = i;
				data.data.push_back((float)((int16_t *)p_dat)[it.key()]);
				i++;
			}

			map.insert(it.key(), data);
		}
	}

	// iio_buffer_destroy(m_buf);
	count++;
	// std::cout << "iio source:" << count << std::endl;
	// std::cout << "after: " << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss,zzz").toStdString() <<
	// '\n';

	return map;
}

void IIOSourceBlock::generateTimeAxis()
{
	double sr = m_timeAxisSR;
	// sample rate is not found
	if(sr == -1) {
		sr = 1;
	}

	double timeoffset = 0;
	m_timeAxis.clear();

	for(int i = 0; i <= plotSize(); i++) {
		m_timeAxis.push_back(timeoffset + i / sr);
	}
}

void IIOSourceBlock::findDevSampleRateAttribute()
{
	QStringList list = {"sample_rate", "sampling_rate", "sample_frequency", "sampling_frequency"};
	for(const QString &name : list) {
		const char *attr = iio_device_find_attr(m_dev, name.toStdString().c_str());
		if(attr) {
			m_devSampleRateAttr = QString(attr);
			break;
		}
	}
}

void IIOSourceBlock::populateChannelInfo(uint id)
{
	iio_channel *ch = iio_device_get_channel(m_dev, id);
	if(!ch)
		return;

	QStringList srNames = {"sample_rate", "sampling_rate", "sample_frequency", "sampling_frequency"};
	QStringList scaleNames = {"scale"};

	ChannelInfo info;
	info.fmt = iio_channel_get_data_format(ch);

	for(const QString &name : srNames) {
		const char *attr = iio_channel_find_attr(ch, name.toStdString().c_str());
		if(attr) {
			info.sampleRateAttr = QString(attr);
			break;
		}
	}
	for(const QString &name : scaleNames) {
		const char *attr = iio_channel_find_attr(ch, name.toStdString().c_str());
		if(attr) {
			info.scaleAttr = QString(attr);
			break;
		}
	}

	m_channelInfoMap[id] = info;
	generateTimeAxis();
}

void IIOSourceBlock::removeChannelInfo(uint id) { m_channelInfoMap.remove(id); }

bool IIOSourceBlock::sampleRateAvailable(int id)
{
	if(id >= 0) {
		const ChannelInfo info = m_channelInfoMap.value(id);
		return info.hasSampleRateAttr();
	}

	if(!m_devSampleRateAttr.isEmpty())
		return true;

	for(const auto &info : m_channelInfoMap) {
		if(info.hasSampleRateAttr())
			return true;
	}

	return false;
}

bool IIOSourceBlock::scaleAttributeAvailable(uint id)
{
	const ChannelInfo info = m_channelInfoMap.value(id);
	return info.hasScaleAttr();
}

QString IIOSourceBlock::getDevSampleRateAttr() { return m_devSampleRateAttr; }

QString IIOSourceBlock::getChSampleRateAttr(uint id)
{
	if(!m_channelInfoMap.contains(id))
		return QString();

	return m_channelInfoMap[id].sampleRateAttr;
}

double IIOSourceBlock::readSampleRate(int id)
{
	char buffer[32];
	bool ok = false;

	// if id is specified read from that channel,
	// if not, ready any channel
	if(id != -1) {
		iio_channel *ch = iio_device_get_channel(m_dev, id);
		if(ch && m_channelInfoMap[id].hasSampleRateAttr()) {
			iio_channel_attr_read(ch, m_channelInfoMap[id].sampleRateAttr.toStdString().c_str(), buffer,
					      sizeof(buffer));
			double val = QString(buffer).toDouble(&ok);
			if(ok)
				return val;
		}
	} else {
		for(int i = 0; i <= m_channelInfoMap.count(); i++) {
			iio_channel *ch = iio_device_get_channel(m_dev, i);
			if(ch && m_channelInfoMap[i].hasSampleRateAttr()) {
				iio_channel_attr_read(ch, m_channelInfoMap[i].sampleRateAttr.toStdString().c_str(),
						      buffer, sizeof(buffer));
				double val = QString(buffer).toDouble(&ok);
				if(ok)
					return val;
			}
		}
	}

	// some devices may have it as a device attr
	if(!m_devSampleRateAttr.isEmpty()) {
		iio_device_attr_read(m_dev, m_devSampleRateAttr.toStdString().c_str(), buffer, sizeof(buffer));
		double val = QString(buffer).toDouble(&ok);
		if(ok)
			return val;
	}

	return -1;
}

double IIOSourceBlock::readChannelScale(uint id)
{
	char buffer[32];
	bool ok = false;

	if(m_channelInfoMap.contains(id)) {
		iio_channel *ch = iio_device_get_channel(m_dev, id);
		if(ch && m_channelInfoMap[id].hasScaleAttr()) {
			iio_channel_attr_read(ch, m_channelInfoMap[id].scaleAttr.toStdString().c_str(), buffer,
					      sizeof(buffer));
			double val = QString(buffer).toDouble(&ok);
			if(ok)
				return val;
		}
	}
	return -1;
}

QString IIOSourceBlock::getChScaleAttr(uint id)
{
	if(!m_channelInfoMap.contains(id))
		return QString();

	return m_channelInfoMap[id].scaleAttr;
}

void IIOSourceBlock::setTimeAxisSR(double sr)
{
	m_timeAxisSR = sr;
	generateTimeAxis();
}

const iio_data_format *IIOSourceBlock::getFmt(uint id) const
{
	if(!m_channelInfoMap.contains(id))
		return nullptr;

	return m_channelInfoMap[id].fmt;
}

StaticSourceBlock::StaticSourceBlock(QString name)
	: SourceBlock(name)
	, m_map(BlockData())
{}

StaticSourceBlock::~StaticSourceBlock() {}

void StaticSourceBlock::setData(std::vector<float> xdata, std::vector<float> ydata)
{
	m_map.clear();
	m_map.insert(0, std::move(ydata));
	m_timeAxis = std::move(xdata);
}

std::vector<float> StaticSourceBlock::getTimeAxis() { return m_timeAxis; }

std::vector<float> StaticSourceBlock::getCurrentData()
{
	if(m_map.empty())
		return std::vector<float>();
	return m_map.first().data;
}

BlockData StaticSourceBlock::createData() { return m_map; }
