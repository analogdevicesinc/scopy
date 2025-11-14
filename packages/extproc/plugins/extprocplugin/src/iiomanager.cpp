/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <iiomanager.h>
#include <QLoggingCategory>
#include <qtconcurrentrun.h>

Q_LOGGING_CATEGORY(CAT_IIO_MANAGER, "IIOManager");

using namespace scopy::extprocplugin;

IIOManager::IIOManager(iio_context *ctx, QObject *parent)
	: QObject(parent)
	, m_ctx(ctx)
{
	m_dataWriter = new DataWriter(this);
	m_readFw = new QFutureWatcher<void>();
	computeDevMap();

	connect(m_readFw, &QFutureWatcher<void>::finished, this, [this]() {
		if(!m_bufferData.isEmpty()) {
			Q_EMIT dataReady(m_bufferData);
		}
	});
}

IIOManager::~IIOManager() {}

QMap<QString, QList<ChannelInfo>> IIOManager::getAvailableChannels()
{
	QMap<QString, QList<ChannelInfo>> result;
	for(auto it = m_devMap.constBegin(); it != m_devMap.constEnd(); ++it) {
		QList<ChannelInfo> devChnls;
		for(auto chIt = it.value().constBegin(); chIt != it.value().constEnd(); ++chIt) {
			ChannelInfo chInfo{chIt.key(), iio_channel_is_enabled(chIt.value())};
			devChnls.push_back(chInfo);
		}
		result.insert(it.key(), devChnls);
	}
	return result;
}

void IIOManager::startAcq(bool en)
{
	if(m_readFw->isRunning()) {
		m_readFw->waitForFinished();
	}
	m_readFw->setPaused(!en);
	if(!en) {
		destroyBuffer();
	} else if(!m_buffer) {
		updateBufferParams(m_params);
	}
	onDataRequest();
}

void IIOManager::onDataRequest()
{
	if(!m_readFw->isRunning() && !m_readFw->isPaused()) {
		QFuture<void> f = QtConcurrent::run(this, &IIOManager::readBuffer);
		m_readFw->setFuture(f);
	}
}

void IIOManager::onBufferParamsChanged(const BufferParams &params)
{
	updateBufferParams(params);
	notifyInputConfigChanged();
}

void IIOManager::updateBufferParams(const BufferParams &params)
{
	m_params = params;
	QString devName = params.deviceName;
	iio_device *dev = iio_context_find_device(m_ctx, devName.toStdString().c_str());
	if(!dev) {
		qWarning(CAT_IIO_MANAGER) << "The device doesn't exist:" << params.deviceName;
		return;
	}
	destroyBuffer();
	m_enChnlSize = enChannels(devName, params.enChnls);
	m_buffer = iio_device_create_buffer(dev, params.samplesCount, false);
	int64_t dataSize = m_enChnlSize * params.samplesCount * sizeof(float);
	if(!m_dataWriter->openFile(ExtProcUtils::dataInPath(), dataSize)) {
		qWarning(CAT_IIO_MANAGER) << "Failed to map:" << ExtProcUtils::dataInPath();
		return;
	}
	// m_buffer = createMmapIioBuffer(dev, params.samplesCount, &m_originalBufferPtr);
}

void IIOManager::notifyInputConfigChanged()
{
	QString devName = m_params.deviceName;
	iio_device *dev = iio_context_find_device(m_ctx, devName.toStdString().c_str());
	if(!dev) {
		qWarning(CAT_IIO_MANAGER) << "The device doesn't exist:" << m_params.deviceName;
		return;
	}
	InputConfig inputConfig = createInputConfig(dev, m_enChnlSize, m_params.samplesCount);
	if(inputConfig.isValid()) {
		Q_EMIT inputFormatChanged(inputConfig);
	}
}

void IIOManager::computeDevMap()
{
	for(int i = 0; i < iio_context_get_devices_count(m_ctx); i++) {
		iio_device *dev = iio_context_get_device(m_ctx, i);
		QString deviceName = iio_device_get_name(dev);
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {
			struct iio_channel *chn = iio_device_get_channel(dev, j);
			if(!iio_channel_is_output(chn) && iio_channel_is_scan_element(chn)) {
				QString chnlId = iio_channel_get_id(chn);
				m_devMap[deviceName].insert(chnlId, chn);
			}
		}
	}
}

void IIOManager::destroyBuffer()
{
	if(!m_buffer) {
		return;
	}
	// If we used the buffer hack, restore the original buffer pointer before cleanup
	// This ensures iio_buffer_destroy() operates on the correct internal state
	if(m_originalBufferPtr) {
		union iio_buffer_hack hackBuffer;
		hackBuffer.buffer = m_buffer;
		// Restore the original buffer pointer that was saved during createMmapIioBuffer()
		hackBuffer.fields->buffer = m_originalBufferPtr;
		m_originalBufferPtr = nullptr;
	}

	iio_buffer_destroy(m_buffer);
	m_buffer = nullptr;
}

void IIOManager::chnlRead(iio_channel *chnl, QByteArray &dst)
{
	if(!m_buffer || !chnl) {
		return;
	}
	unsigned int length = iio_channel_get_data_format(chnl)->length / 8;
	dst.resize(m_params.samplesCount * length);

	uintptr_t src_ptr, dst_ptr = (uintptr_t)dst.data();
	uintptr_t end = dst_ptr + m_params.samplesCount * length;
	uintptr_t buf_end = (uintptr_t)iio_buffer_end(m_buffer);
	ptrdiff_t buf_step = iio_buffer_step(m_buffer);

	for(src_ptr = (uintptr_t)iio_buffer_first(m_buffer, chnl); src_ptr < buf_end && dst_ptr + length <= end;
	    src_ptr += buf_step, dst_ptr += length) {
		iio_channel_convert(chnl, (void *)dst_ptr, (const void *)src_ptr);
	}
}

QVector<float> IIOManager::toFloat(QByteArray dst)
{
	const char *data = dst.data();
	int length = dst.size() / m_params.samplesCount;
	QVector<float> chData;
	chData.resize(m_params.samplesCount);

	for(int i = 0; i < m_params.samplesCount; ++i) {
		float value = 0.0;
		if(length == 4) {
			const int *intPtr = reinterpret_cast<const int *>(data + i * length);
			value = static_cast<float>(*intPtr);
		} else if(length == 2) { // int16
			const short *int16Ptr = reinterpret_cast<const short *>(data + i * length);
			value = static_cast<float>(*int16Ptr);
		}
		chData[i] = value;
	}

	return chData;
}

void IIOManager::readAllChannels(QString deviceName)
{
	QMap<QString, iio_channel *> channels = m_devMap[deviceName];
	m_bufferData.clear();
	for(iio_channel *ch : qAsConst(channels)) {
		if(!iio_channel_is_enabled(ch)) {
			continue;
		}
		QByteArray rawData;
		chnlRead(ch, rawData);
		QVector<float> values = toFloat(rawData);
		if(!values.isEmpty()) {
			m_bufferData.push_back(values);
		}
	}
}

void IIOManager::writeToMappedFile()
{
	if(m_bufferData.isEmpty() || m_bufferData.first().isEmpty()) {
		qWarning(CAT_IIO_MANAGER) << "Data is not available!";
		return;
	}
	if(!m_dataWriter->mappedData()) {
		qWarning(CAT_IIO_MANAGER) << "Couldn't access the DataWriteer mapped data!";
		return;
	}
	int dataIdx = 0;
	float *interleaved = reinterpret_cast<float *>(m_dataWriter->mappedData());
	for(int sample = 0; sample < m_params.samplesCount; sample++) {
		for(int chIdx = 0; chIdx < m_enChnlSize; chIdx++) {
			interleaved[dataIdx++] = m_bufferData[chIdx][sample];
		}
	}
}

void IIOManager::readBuffer()
{
	if(!m_buffer) {
		return;
	}
	const iio_device *dev = iio_buffer_get_device(m_buffer);
	QString devName = iio_device_get_name(dev);
	iio_buffer_refill(m_buffer);
	readAllChannels(devName);
	writeToMappedFile();
}

int IIOManager::enChannels(QString deviceName, QStringList enChnls)
{
	int chnls = 0;
	QMap<QString, iio_channel *> devChnls = m_devMap[deviceName];
	for(auto it = devChnls.begin(); it != devChnls.end(); ++it) {
		if(enChnls.contains(it.key())) {
			iio_channel_enable(it.value());
			chnls++;
		} else {
			iio_channel_disable(it.value());
		}
	}
	return chnls;
}

QString channelDataFormat(struct iio_channel *chnl)
{
	if(!chnl) {
		return QString();
	}
	const struct iio_data_format *format = iio_channel_get_data_format(chnl);
	if(!format) {
		return QString();
	}
	QString formatString;
	QString endianness = format->is_be ? "be" : "le";
	QString signedness = format->is_signed ? "s" : "u";

	formatString = QString("%1:%2%3/%4>>%5")
			       .arg(endianness, signedness, QString::number(format->bits),
				    QString::number(format->length), QString::number(format->shift));

	return formatString;
}

QStringList IIOManager::getChannelsFormat(iio_device *dev, bool floatFormat)
{
	QStringList formatList;
	if(!dev) {
		return formatList;
	}
	QString devName = iio_device_get_name(dev);
	QMap<QString, iio_channel *> devChnls = m_devMap[devName];
	for(auto it = devChnls.begin(); it != devChnls.end(); ++it) {
		struct iio_channel *chnl = it.value();
		if(!chnl) {
			continue;
		}

		if(iio_channel_is_enabled(chnl)) {
			QString dataFormat = floatFormat ? ChannelFormatTypes::FLOAT32 : channelDataFormat(chnl);
			if(!dataFormat.isEmpty()) {
				formatList.append(dataFormat);
			}
		}
	}
	return formatList;
}

double IIOManager::getSamplingFrequency(iio_device *dev)
{
	double samplingFreq = 0.0;

	if(iio_device_attr_read_double(dev, "sampling_frequency", &samplingFreq) >= 0) {
		return samplingFreq;
	}
	const QString devName = iio_device_get_name(dev);
	iio_channel *channel = m_devMap[devName].first();
	if(channel && iio_channel_attr_read_double(channel, "sampling_frequency", &samplingFreq) >= 0) {
		return samplingFreq;
	}

	return 0.0;
}

InputConfig IIOManager::createInputConfig(iio_device *dev, int channelCount, int64_t bufferSamplesSize)
{
	InputConfig inputConfig;
	inputConfig.setInputFile(ExtProcUtils::dataInPath());
	inputConfig.setInputFileFormat(FileFormatTypes::BINARY_INTERLEAVED);
	inputConfig.setChnlsFormat(getChannelsFormat(dev, true));
	inputConfig.setChannelCount(channelCount);
	inputConfig.setSampleCount(bufferSamplesSize);
	inputConfig.setSamplingFrequency(getSamplingFrequency(dev));
	return inputConfig;
}

iio_buffer *IIOManager::createMmapIioBuffer(struct iio_device *dev, size_t samples, void **originalBufferPtr)
{
	// Create a standard IIO buffer, then hack it to use memory-mapped file storage
	union iio_buffer_hack hackBuffer;
	hackBuffer.buffer = iio_device_create_buffer(dev, samples, false);
	if(!hackBuffer.buffer) {
		qWarning(CAT_IIO_MANAGER) << "Failed to create IIO buffer";
		return nullptr;
	}

	// Calculate required buffer size and create memory-mapped file
	int64_t bufferDataSize = iio_device_get_sample_size(dev) * m_params.samplesCount;
	if(!m_dataWriter->openFile(ExtProcUtils::dataInPath(), bufferDataSize)) {
		qWarning(CAT_IIO_MANAGER) << "Failed to map:" << ExtProcUtils::dataInPath();
		return nullptr;
	}

	// Save the original buffer pointer so we can restore it during cleanup
	void *original_buffer = hackBuffer.fields->buffer;
	if(originalBufferPtr) {
		*originalBufferPtr = original_buffer;
	}

	qInfo(CAT_IIO_MANAGER) << "Original buffer pointer:" << original_buffer;
	qInfo(CAT_IIO_MANAGER) << "Mapped memory pointer:" << (void *)m_dataWriter->mappedData();

	// HACK: Replace the internal buffer pointer with our memory-mapped data
	// This redirects all IIO buffer operations to use the memory-mapped file
	hackBuffer.fields->buffer = m_dataWriter->mappedData();

	// Verify the hack worked by checking if buffer start returns our mapped pointer
	if(iio_buffer_start(hackBuffer.buffer) == m_dataWriter->mappedData()) {
		qDebug(CAT_IIO_MANAGER) << "SUCCESS: Buffer hack working correctly!";
	} else {
		qWarning(CAT_IIO_MANAGER) << "WARNING: Buffer hack might not be working";
	}
	return hackBuffer.buffer;
}
