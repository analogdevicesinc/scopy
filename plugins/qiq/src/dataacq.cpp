#include "dataacq.h"

#include <iioutil/connectionprovider.h>
#include <common/scopyconfig.h>
#include <common/debugtimer.h>

using namespace scopy::qiqplugin;

DataAcq::DataAcq(iio_context *ctx, QObject *parent)
	: QObject(parent)
	, m_ctx(ctx)
	, m_buffer(nullptr)
	, m_mappedData(nullptr)
{
}

DataAcq::~DataAcq()
{
	destroyBuffer();
	if(m_mappedData) {
		m_file.unmap(m_mappedData);
	}
	m_file.remove();
}

void DataAcq::readDeviceData()
{
	if(!m_buffer) {
		return;
	}

	m_data.clear();
	m_data.resize(m_chnls.size());
	DebugTimer timer(scopy::config::settingsFolderPath() + QDir::separator() + "benchmark.csv");

	iio_buffer_refill(m_buffer);

	int ch_index = 0;
	for(iio_channel *ch : qAsConst(m_chnls)) {
		if(!iio_channel_is_enabled(ch))
			continue;

		QVector<short> chData(m_samples);
		chnlRead(ch, chData.data());
		for(int i = 0; i < chData.size(); i++) {
			m_data[ch_index].push_back(chData[i]);
		}
		++ch_index;
	}
	DEBUGTIMER_LOG(timer, "Get device samples:");
	timer.restartTimer();
	writeToMappedFile();
	DEBUGTIMER_LOG(timer, "File writing:");
	Q_EMIT dataAvailable(m_data, m_dataSize, FILE_PATH);
}

void DataAcq::chnlRead(iio_channel *ch, short *dst)
{

	uintptr_t src_ptr, dst_ptr = (uintptr_t)dst, end = dst_ptr + m_samples * sizeof(short);
	unsigned int length = iio_channel_get_data_format(ch)->length / 8;
	uintptr_t buf_end = (uintptr_t)iio_buffer_end(m_buffer);
	ptrdiff_t buf_step = iio_buffer_step(m_buffer);

	for(src_ptr = (uintptr_t)iio_buffer_first(m_buffer, ch); src_ptr < buf_end && dst_ptr + length <= end;
	    src_ptr += buf_step, dst_ptr += length) {
		iio_channel_convert(ch, (void *)dst_ptr, (const void *)src_ptr);
	}
}

void DataAcq::onConfigPressed(AcqSetup::AcqConfig config)
{
	int dataSize = config.samples * config.enChnls.size() * sizeof(short);
	if(!m_mappedData || dataSize != m_dataSize) {
		m_dataSize = dataSize;
		mapFile(FILE_PATH);
	}
	m_samples = config.samples;
	QStringList enChnls = config.enChnls;
	QString deviceName = config.deviceName;
	iio_device *dev = iio_context_find_device(m_ctx, deviceName.toStdString().c_str());
	m_chnls.clear();
	for(int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);
		if(!iio_channel_is_output(chn) && iio_channel_is_scan_element(chn)) {
			QString chnlId(iio_channel_get_id(chn));
			if(enChnls.contains(chnlId)) {
				iio_channel_enable(chn);
				m_chnls.push_back(chn);
			} else {
				iio_channel_disable(chn);
			}
		}
	}
	createBuffer(dev);
}

void DataAcq::destroyBuffer()
{
	if(m_buffer) {
		iio_buffer_destroy(m_buffer);
		m_buffer = nullptr;
	}
}

void DataAcq::createBuffer(iio_device *dev)
{
	destroyBuffer();
	m_buffer = iio_device_create_buffer(dev, m_samples, false);
}

void DataAcq::initDevicesAndChnls()
{
	QMap<QString, QStringList> map;
	for(int i = 0; i < iio_context_get_devices_count(m_ctx); i++) {
		iio_device *dev = iio_context_get_device(m_ctx, i);
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {
			QString devName(iio_device_get_name(dev));
			struct iio_channel *chn = iio_device_get_channel(dev, j);
			if(!iio_channel_is_output(chn) && iio_channel_is_scan_element(chn)) {
				iio_channel_disable(chn);
				if(!map.contains(devName)) {
					map.insert(devName, {});
				}
				map[devName].append(iio_channel_get_id(chn));
			}
		}
	}
	Q_EMIT contextInfo(map);
}

void DataAcq::mapFile(const QString &filePath)
{
	m_file.setFileName(filePath);
	if(!m_file.open(QIODevice::ReadWrite)) {
		qCritical() << "Failed to open m_file:" << m_file.errorString();
		return;
	}

	if(!m_file.resize(m_dataSize)) {
		qCritical() << "Failed to resize m_file:" << m_file.errorString();
		m_file.close();
		return;
	}

	m_mappedData = m_file.map(0, m_dataSize);
	if(!m_mappedData) {
		qCritical() << "Failed to map m_file:" << m_file.errorString();
	}
	m_file.close();
}

void DataAcq::writeToMappedFile()
{
	if(m_data.isEmpty() || m_data.first().isEmpty()) {
		qWarning() << "Empty data";
		return;
	}
	if(!m_mappedData) {
		qWarning() << "Map failed!";
		return;
	}

	int rows = m_data.size();
	int cols = m_data.first().size();
	size_t totalBytesNeeded = rows * cols * sizeof(short);
	if(m_dataSize < totalBytesNeeded) {
		qWarning() << "Not enough space!";
		return;
	}

	short *dest = reinterpret_cast<short *>(m_mappedData);
	size_t index = 0;

	for(int col = 0; col < cols; ++col) {
		for(int row = 0; row < rows; ++row) {
			dest[index++] = m_data[row][col];
		}
	}
}
