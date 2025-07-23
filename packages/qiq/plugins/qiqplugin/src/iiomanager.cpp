#include <iiomanager.h>
#include <QLoggingCategory>
#include <qtconcurrentrun.h>

Q_LOGGING_CATEGORY(CAT_IIO_MANAGER, "IIOManager");

using namespace scopy::qiqplugin;

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

QMap<QString, QStringList> IIOManager::getAvailableChannels()
{
	QMap<QString, QStringList> avlChnls;
	for(auto it = m_devMap.begin(); it != m_devMap.end(); ++it) {
		avlChnls.insert(it.key(), it.value().keys());
	}
	return avlChnls;
}
// must handle single
void IIOManager::startAcq(bool en)
{
	if(m_readFw->isRunning()) {
		m_readFw->waitForFinished();
	}
	m_readFw->setPaused(!en);
	onDataRequest();
}

void IIOManager::onDataRequest()
{
	if(!m_readFw->isRunning() && !m_readFw->isPaused()) {
		QFuture<void> f = QtConcurrent::run(this, &IIOManager::readBuffer);
		m_readFw->setFuture(f);
	}
}

void IIOManager::onBufferParamsChanged(BufferParams params)
{
	QString devName = params.deviceName;
	iio_device *dev = iio_context_find_device(m_ctx, devName.toStdString().c_str());
	if(!dev) {
		return;
		qWarning(CAT_IIO_MANAGER) << "The device doesn't exist:" << params.deviceName;
	}
	if(m_buffer) {
		destroyBuffer();
	}
	int enChnlSize = enChannels(devName, params.enChnls);
	m_buffer = createMmapIioBuffer(dev, params.samplesCount, &m_originalBufferPtr);
	InputConfig inputConfig = createInputConfig(dev, enChnlSize, m_bufferSamples);
	if(inputConfig.isValid()) {
		Q_EMIT inputFormatChanged(inputConfig);
	}
}

void IIOManager::init() {}

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
				iio_channel_disable(chn);
			}
		}
	}
}

void IIOManager::destroyBuffer()
{
	if(!m_buffer) {
		return;
	}
	if(m_originalBufferPtr) {
		union iio_buffer_hack hackBuffer;
		hackBuffer.buffer = m_buffer;
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
	dst.resize(m_bufferSamples * length);

	uintptr_t src_ptr, dst_ptr = (uintptr_t)dst.data();
	uintptr_t end = dst_ptr + m_bufferSamples * length;
	uintptr_t buf_end = (uintptr_t)iio_buffer_end(m_buffer);
	ptrdiff_t buf_step = iio_buffer_step(m_buffer);

	for(src_ptr = (uintptr_t)iio_buffer_first(m_buffer, chnl); src_ptr < buf_end && dst_ptr + length <= end;
	    src_ptr += buf_step, dst_ptr += length) {
		iio_channel_convert(chnl, (void *)dst_ptr, (const void *)src_ptr);
	}
}

QVector<double> IIOManager::toDouble(QByteArray dst)
{
	const char *data = dst.data();
	int length = dst.size() / m_bufferSamples;
	QVector<double> chData;
	chData.resize(m_bufferSamples);

	for(int i = 0; i < m_bufferSamples; ++i) {
		double value = 0.0;
		if(length == 4) {
			const int *intPtr = reinterpret_cast<const int *>(data + i * length);
			value = static_cast<double>(*intPtr);
		} else if(length == 2) { // int16
			const short *int16Ptr = reinterpret_cast<const short *>(data + i * length);
			value = static_cast<double>(*int16Ptr);
		} else if(length == 8) { // double
			const double *doublePtr = reinterpret_cast<const double *>(data + i * length);
			value = *doublePtr;
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
		QVector<double> values = toDouble(rawData);
		if(!values.isEmpty()) {
			m_bufferData.push_back(values);
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

void IIOManager::onReadBufferData() {}

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

QStringList IIOManager::getChannelsFormat(iio_device *dev)
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
			QString dataFormat = channelDataFormat(chnl);
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
	inputConfig.setInputFile(DEFAULT_FILE_PATH);
	inputConfig.setInputFileFormat(FileFormatTypes::BINARY_INTERLEAVED);
	inputConfig.setChnlsFormat(getChannelsFormat(dev));
	inputConfig.setChannelCount(channelCount);
	inputConfig.setSampleCount(bufferSamplesSize);
	inputConfig.setSamplingFrequency(getSamplingFrequency(dev));
	return inputConfig;
}

double IIOManager::calculateBufferSamples(iio_channel *ch, iio_buffer *buffer)
{
	double bufferSamples = 0.0;
	if(buffer && ch) {
		ptrdiff_t buf_step = iio_buffer_step(buffer);
		uintptr_t buf_end = (uintptr_t)iio_buffer_end(buffer);
		uintptr_t buf_start = (uintptr_t)iio_buffer_first(buffer, ch);
		bufferSamples = (buf_end - buf_start) / buf_step;
	}
	return bufferSamples;
}

iio_buffer *IIOManager::createMmapIioBuffer(struct iio_device *dev, size_t samples, void **originalBufferPtr)
{
	union iio_buffer_hack hackBuffer;
	hackBuffer.buffer = iio_device_create_buffer(dev, samples, false);
	if(!hackBuffer.buffer) {
		qWarning(CAT_IIO_MANAGER) << "Failed to create IIO buffer";
		return nullptr;
	}
	QString devName = iio_device_get_name(dev);
	m_bufferSamples = calculateBufferSamples(m_devMap[devName].first(), hackBuffer.buffer);
	int64_t bufferDataSize = iio_device_get_sample_size(dev) * m_bufferSamples;
	if(!m_dataWriter->openFile(DEFAULT_FILE_PATH, bufferDataSize)) {
		qWarning(CAT_IIO_MANAGER) << "Failed to map:" << DEFAULT_FILE_PATH;
		return nullptr;
	}

	void *original_buffer = hackBuffer.fields->buffer;
	if(originalBufferPtr) {
		*originalBufferPtr = original_buffer;
	}

	qInfo(CAT_IIO_MANAGER) << "Original buffer pointer:" << original_buffer;
	qInfo(CAT_IIO_MANAGER) << "Mapped memory pointer:" << (void *)m_dataWriter->mappedData();

	hackBuffer.fields->buffer = m_dataWriter->mappedData();

	if(iio_buffer_start(hackBuffer.buffer) == m_dataWriter->mappedData()) {
		qDebug(CAT_IIO_MANAGER) << "SUCCESS: Buffer hack working correctly!";
	} else {
		qWarning(CAT_IIO_MANAGER) << "WARNING: Buffer hack might not be working";
	}
	return hackBuffer.buffer;
}
