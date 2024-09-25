#include "dacdatamodel.h"
#include "txnode.h"
#include "dac_logging_categories.h"

#include <algorithm>

#include <QtConcurrentRun>
#include <QDebug>
#include <QThread>

using namespace scopy;
using namespace scopy::dac;
DacDataModel::DacDataModel(struct iio_device *dev, QObject *parent)
	: QObject(parent)
	, m_ddsTxs({})
	, m_bufferTxs({})
	, m_activeDds(false)
	, m_activeBuffer(false)
	, m_buffer(nullptr)
	, m_cyclicBuffer(true)
	, m_interrupted(false)
	, m_userBuffersize(0)
	, m_userKernelBufferCount(0)
	, m_decimation(1)
{
	m_dev = dev;
	m_name = iio_device_get_name(m_dev);

	m_isBufferCapable = initBufferDac();
	m_isDds = initDdsDac();

	connect(
		this, &DacDataModel::reqInitBuffer, this,
		[this]() {
			if(m_buffer) {
				initBuffer();
			}
		},
		Qt::QueuedConnection);
}

DacDataModel::~DacDataModel()
{
	deinitBufferDac();
	deinitDdsDac();
}

void DacDataModel::reset()
{
	m_interrupted = false;
	m_userBuffersize = 0;
	m_userKernelBufferCount = 0;
	m_data.clear();
}

struct iio_device *DacDataModel::getDev() const { return m_dev; }

QString DacDataModel::getName() const { return m_name; }

bool DacDataModel::isBufferCapable() const { return m_isBufferCapable; }

bool DacDataModel::isDds() const { return m_isDds; }

void DacDataModel::enableBuffer(bool enabled)
{
	qDebug(CAT_DAC_DATA) << QString("Enable buffer %1").arg(enabled);
	if(m_isDds && enabled) {
		enableDds(false);
	}
	if(m_isBufferCapable) {
		m_activeBuffer = enabled;
		if(!enabled) {
			stop();
		}
	}
}

void DacDataModel::enableDds(bool enable)
{
	qDebug(CAT_DAC_DATA) << QString("Enable DDS %1").arg(enable);
	if(m_isBufferCapable && m_activeBuffer && enable) {
		for(auto node : qAsConst(m_bufferTxs)) {
			enableBufferChannel(node->getUuid(), false);
		}
	}
	if(m_isDds) {
		m_activeDds = enable;
		for(auto tx : qAsConst(m_ddsTxs)) {
			tx->enableDds(enable);
		}
	}
}

QMap<QString, TxNode *> DacDataModel::getBufferTxs() const { return m_bufferTxs; }

QMap<QString, TxNode *> DacDataModel::getDdsTxs() const { return m_ddsTxs; }

void DacDataModel::requestInterruption()
{
	m_interrupted = true;
	if(m_pushThd.isRunning()) {
		m_pushThd.waitForFinished();
	}
	qDebug(CAT_DAC_DATA) << "Thread stopped.";
}

void DacDataModel::setCyclic(bool cyclic)
{
	requestInterruption();
	m_cyclicBuffer = cyclic;
	autoBuffersizeAndKernelBuffers();
	Q_EMIT reqInitBuffer();
}

void DacDataModel::setKernelBuffersCount(unsigned int kernelCount)
{
	if(kernelCount != m_userKernelBufferCount) {
		requestInterruption();
		m_userKernelBufferCount = kernelCount;
		Q_EMIT reqInitBuffer();
	}
}

void DacDataModel::setDecimation(double decimation)
{
	requestInterruption();
	m_decimation = decimation;
	Q_EMIT reqInitBuffer();
}

void DacDataModel::setBuffersize(unsigned int buffersize)
{
	if(m_userBuffersize != buffersize) {
		requestInterruption();
		m_userBuffersize = buffersize;
		Q_EMIT reqInitBuffer();
	}
}

void DacDataModel::setFilesize(unsigned int filesize)
{
	if(m_filesize != filesize) {
		requestInterruption();
		m_filesize = filesize;
		autoBuffersizeAndKernelBuffers();
		Q_EMIT reqInitBuffer();
	}
}

void DacDataModel::enableBufferChannel(QString uuid, bool enable)
{
	requestInterruption();
	auto chn = m_bufferTxs.value(uuid)->getChannel();
	if(!chn) {
		qDebug(CAT_DAC_DATA) << QString("No channel for uuid %1").arg(uuid);
	}
	if(enable) {
		iio_channel_enable(chn);
	} else {
		iio_channel_disable(chn);
	}

	Q_EMIT reqInitBuffer();
}

unsigned int DacDataModel::getEnabledChannelsCount()
{
	unsigned int enChannelsCount = 0;
	for(auto node : qAsConst(m_bufferTxs)) {
		enChannelsCount += iio_channel_is_enabled(node->getChannel()) ? 1 : 0;
	}
	return enChannelsCount;
}

void DacDataModel::setData(QVector<QVector<int16_t>> data)
{
	m_data = data;
	Q_EMIT reqInitBuffer();
}

void DacDataModel::setSamplingFrequency(unsigned int sr)
{
	if(m_samplingFrequency != sr) {
		m_samplingFrequency = sr;
		requestInterruption();
		autoBuffersizeAndKernelBuffers();
		Q_EMIT reqInitBuffer();
	}
}

void DacDataModel::autoBuffersizeAndKernelBuffers()
{
	auto bf = 0;
	auto kb = 0;
	if(!m_cyclicBuffer) {
		const unsigned int maxTransferSize = 16 * 1024 * 1024;
		const unsigned int minKernelBuffers = 4;
		const double maxTransferTime = 0.1;
		kb = minKernelBuffers;
		while(kb <= 64) {
			bf = m_filesize / kb;
			kb += (m_filesize % kb != 0) ? 1 : 0;
			if((bf / m_samplingFrequency) > maxTransferTime) {
				kb++;
			} else {
				break;
			}
		}
		m_userKernelBufferCount = kb;
		m_userBuffersize = bf;
	} else {
		m_userKernelBufferCount = 0;
		m_userBuffersize = 0;
	}
	Q_EMIT updateBuffersize(m_userBuffersize);
	Q_EMIT updateKernelBuffers(m_userKernelBufferCount);
}

bool DacDataModel::validateBufferParams()
{
	if(!m_isBufferCapable) {
		return false;
	}
	if(!m_activeBuffer) {
		return false;
	}

	auto enabledChannelsCount = getEnabledChannelsCount();
	ssize_t s_size = iio_device_get_sample_size(m_dev);
	if(!s_size || enabledChannelsCount == 0) {
		auto msg = "Unable to create buffer, no channel enabled.";
		qDebug(CAT_DAC_DATA) << msg;
		Q_EMIT log(msg);
		return false;
	}

	if((m_cyclicBuffer && m_data.size() == 0) || (!m_cyclicBuffer && m_userBuffersize == 0)) {
		auto msg = "Unable to create buffer due to data size.";
		qDebug(CAT_DAC_DATA) << msg;
		Q_EMIT log(msg);
		return false;
	}

	if(m_data[0].size() < enabledChannelsCount) {
		auto msg = "Not enough data columns for all enabled channels.";
		qDebug(CAT_DAC_DATA) << msg;
		Q_EMIT log(msg);
		return false;
	}

	if(!txChannelsCheckValidSetup()) {
		auto msg = "Unable to create buffer due to incompatible channels enabled.";
		qDebug(CAT_DAC_DATA) << msg;
		Q_EMIT log(msg);
		return false;
	}

	if(m_cyclicBuffer) {
		m_kernelBufferCount = 4;
		m_buffersize = m_filesize;
	} else {
		m_buffersize = m_userBuffersize;
		m_kernelBufferCount = m_userKernelBufferCount;
	}

	if(m_decimation != 1) {
		if(m_userBuffersize * m_decimation > m_filesize) {
			auto msg = "Unable to create buffer due to high decimation.";
			qDebug(CAT_DAC_DATA) << msg;
			Q_EMIT log(msg);
			return false;
		}
	}

	return true;
}

void DacDataModel::initBuffer()
{
	if(m_pushThd.isRunning()) {
		m_pushThd.cancel();
		qDebug(CAT_DAC_DATA) << "Cancel thread and wait in initBuffer";
		m_pushThd.waitForFinished();
	}
	m_pushThd = QtConcurrent::run(this, &DacDataModel::push);
}

void DacDataModel::push()
{
	qDebug(CAT_DAC_DATA) << "Start push thread";
	unsigned int totalSize = 0;
	m_interrupted = false;
	QVector<QVector<int16_t>> allDataC = {};
	unsigned int enChannelsCount = getEnabledChannelsCount();
	bool valid = validateBufferParams();
	if(!valid) {
		Q_EMIT invalidRunParams();
		return;
	}

	if(m_buffer) {
		iio_buffer_destroy(m_buffer);
		m_buffer = nullptr;
	}

	iio_device_set_kernel_buffers_count(m_dev, m_kernelBufferCount);

	m_buffer = iio_device_create_buffer(m_dev, m_buffersize, m_cyclicBuffer);
	if(!m_buffer) {
		QString logMsg = QString("Unable to create buffer: %1").arg(strerror(errno));
		qDebug(CAT_DAC_DATA) << logMsg;
		Q_EMIT log(logMsg);
		return;
	}

	unsigned int additionalSamples = 0;
	unsigned int sampleIdx = 0;
	if(!m_cyclicBuffer) {
		additionalSamples = m_filesize % m_buffersize;
	}
	for(int ch = 0; ch < enChannelsCount; ch++) {
		allDataC.push_back({});
		for(unsigned int i = 0; i < m_filesize + additionalSamples; i += m_decimation) {
			sampleIdx = std::min(i, m_filesize - 1);
			allDataC[ch].append(m_data[sampleIdx][ch]);
		}
		totalSize += allDataC[ch].size();
	}

	int dataIdx = 0;
	int bufferIdx = 1;
	int totalNbBuffers = totalSize / (m_buffersize * enChannelsCount);
	while(!m_interrupted && bufferIdx <= totalNbBuffers) {
		int chnIdx = 0;
		for(auto ch : qAsConst(m_bufferTxs)) {
			if(!iio_channel_is_enabled(ch->getChannel())) {
				continue;
			}
			uintptr_t dst_ptr, src_ptr = (uintptr_t)(allDataC[chnIdx].data()),
					   end = src_ptr + allDataC[chnIdx].size() * sizeof(int16_t);
			uintptr_t buf_end = (uintptr_t)iio_buffer_end(m_buffer);
			ptrdiff_t buf_step = iio_buffer_step(m_buffer);
			unsigned int ch_len = iio_channel_get_data_format(ch->getChannel())->length / 8;

			for(dst_ptr = (uintptr_t)iio_buffer_first(m_buffer, ch->getChannel());
			    dst_ptr < buf_end && src_ptr + ch_len <= end; dst_ptr += buf_step, src_ptr += ch_len) {
				iio_channel_convert_inverse(ch->getChannel(), (void *)dst_ptr, (const void *)(src_ptr));
			}
			chnIdx++;
		}

		ssize_t bytes = iio_buffer_push(m_buffer);
		QString logMsg = QString("Pushed %1 samples, %2 bytes (%3/%4 buffers)")
					 .arg(m_buffersize)
					 .arg(bytes)
					 .arg(bufferIdx)
					 .arg(totalNbBuffers);
		qDebug(CAT_DAC_DATA) << logMsg;
		Q_EMIT log(logMsg);
		bufferIdx++;
	}
	if(m_interrupted) {
		Q_EMIT log(QString("Aborting thread..."));
	}
}

void DacDataModel::start() { initBuffer(); }

void DacDataModel::stop()
{
	requestInterruption();
	if(!m_isBufferCapable) {
		return;
	}
	if(m_buffer) {
		iio_buffer_cancel(m_buffer);
		iio_buffer_destroy(m_buffer);
		m_buffer = nullptr;
		qDebug(CAT_DAC_DATA) << "Buffer destroyed.";
	}
}

bool DacDataModel::initBufferDac()
{
	unsigned int txCount = 0;
	unsigned int channelCount = iio_device_get_channels_count(m_dev);
	for(unsigned int i = 0; i < channelCount; i++) {
		struct iio_channel *chn = iio_device_get_channel(m_dev, i);
		if(!iio_channel_is_output(chn))
			continue;
		if(iio_channel_is_scan_element(chn)) {
			txCount++;
			QString id = iio_channel_get_id(chn);
			QString name = iio_channel_get_name(chn);
			if(name != "") {
				id += ":" + name;
			}
			QString uuid = iio_device_get_name(m_dev);
			uuid += ":" + id;
			m_bufferTxs.insert(uuid, new TxNode(uuid, chn, this));
		}
	}
	return (txCount != 0);
}

/**
 * TX1_I_F1, TX1_I_F2, TX1_Q_F1, TX1_Q_F2 -> one channel, dual tone, complex
 * 1A, 1B -> one channel, dual tone, not complex
 */
bool DacDataModel::initDdsDac()
{
	unsigned int ddsTonesCount = 0;
	unsigned int channelCount = iio_device_get_channels_count(m_dev);
	for(unsigned int i = 0; i < channelCount; i++) {
		struct iio_channel *chn = iio_device_get_channel(m_dev, i);
		if(!iio_channel_is_output(chn))
			continue;
		iio_chan_type chnType = iio_channel_get_type(chn);
		if(chnType != IIO_ALTVOLTAGE)
			continue;
		ddsTonesCount++;

		// Name should contain TX*_I/Q_F or *A/*B
		QString name = iio_channel_get_name(chn);
		QString id = iio_channel_get_id(chn);
		if(name == "") {
			name = generateToneName(id);
		}
		QStringList txNodesNames = generateTxNodesForChannel(name);
		QString txNameCurrent = txNodesNames.at(0);
		TxNode *parentTxNode = m_ddsTxs.value(txNameCurrent, nullptr);
		if(!parentTxNode) {
			parentTxNode = new TxNode(txNameCurrent, nullptr, this);
			m_ddsTxs.insert(txNameCurrent, parentTxNode);
		}
		if(txNodesNames.size() == 2) {
			QString toneName = txNodesNames.at(1);
			parentTxNode->addChildNode(toneName, chn);
		} else if(txNodesNames.size() == 3) {
			QString complexChnName = txNodesNames.at(1);
			QString toneName = txNodesNames.at(2);
			TxNode *complexChnNode = parentTxNode->addChildNode(complexChnName, nullptr);
			complexChnNode->addChildNode(toneName, chn);
		}
	}
	return (ddsTonesCount != 0);
}

int DacDataModel::getTxChannelEnabledCount(unsigned *enabled_mask)
{
	bool enabled;
	int num_enabled = 0;
	int ch_pos = 0;

	if(enabled_mask)
		*enabled_mask = 0;

	for(auto ch : qAsConst(m_bufferTxs)) {
		bool enabled = iio_channel_is_enabled(ch->getChannel());
		if(enabled) {
			num_enabled++;
			if(enabled_mask)
				*enabled_mask |= 1 << ch_pos;
		}
		ch_pos++;
	}

	qDebug(CAT_DAC_DATA) << QString("Enabled channels %1").arg(num_enabled);
	return num_enabled;
}

bool DacDataModel::txChannelsCheckValidSetup()
{
	int enabled_channels;
	unsigned int mask;

	enabled_channels = getTxChannelEnabledCount(&mask);

	return (dmaValidSelection(mask, m_bufferTxs.count()) && enabled_channels > 0);
}

bool DacDataModel::dmaValidSelection(unsigned mask, unsigned channel_count)
{
	static const unsigned long eight_channel_masks[] = {0x01, 0x02, 0x04, 0x08, 0x03, 0x0C, /* 1 & 2 chan */
							    0x10, 0x20, 0x40, 0x80, 0x30, 0xC0, /* 1 & 2 chan */
							    0x33, 0xCC, 0xC3, 0x3C, 0x0F, 0xF0, /* 4 chan */
							    0xFF,				/* 8chan */
							    0x00};
	static const unsigned long four_channel_masks[] = {0x01, 0x02, 0x04, 0x08, 0x03, 0x0C, 0x0F, 0x00};
	bool ret = true;
	unsigned int i;

	if(channel_count == 8) {
		ret = false;
		for(i = 0; i < sizeof(eight_channel_masks) / sizeof(eight_channel_masks[0]); i++)
			if(mask == eight_channel_masks[i])
				return true;
	} else if(channel_count == 4) {
		ret = false;
		for(i = 0; i < sizeof(four_channel_masks) / sizeof(four_channel_masks[0]); i++)
			if(mask == four_channel_masks[i])
				return true;
	}

	return ret;
}

// returns a list of nodes - this is specifically ordered
// for channels of 1A/B type it returns: {"1", "1A/B"}
// for channels of TX*_I/Q_F* type it returns: {"TX*", "TX_I/Q", "TX*_I/Q_F*"}
QStringList DacDataModel::generateTxNodesForChannel(QString name)
{
	QStringList txNodes;
	QStringList id_list = name.split("_");
	QString tone_uuid = name;
	QString complex_chn_uuid = "";
	QString tx_uuid = "";
	if(id_list.size() == 3) {						     // TX*_Q/I_F* complex format
		complex_chn_uuid = name.mid(0, name.size() - id_list[2].size() - 1); // TX*_Q/I
		tx_uuid = id_list[0];
		txNodes << tx_uuid;
		txNodes << complex_chn_uuid;
	} else if(id_list.size() == 1) { // *A/*B non-complex format
		tx_uuid = name[0];
		txNodes << tx_uuid;
	}
	txNodes << tone_uuid;
	return txNodes;
}

QString DacDataModel::generateToneName(QString chnId)
{
	bool ok;
	QString name = "";
	int idx = chnId.indexOf(toneId);
	if(idx != -1) {
		int chnIndex = chnId.midRef(idx + toneId.size()).toInt(&ok);
		if(!ok)
			return name;
		int txIndex = (chnIndex / MAX_NB_TONES) + 1; // TX indexing from 1
		int toneIndex = (chnIndex % 2) + 1;	     // assuming dual tones, indexing from 1
		QString typeIQ = (chnIndex & 0x02) ? Q_CHANNEL
						   : I_CHANNEL; /* First two indexes are I, next two are Q and so on*/
		name = "TX" + QString::number(txIndex) + "_" + typeIQ + "_F" + QString::number(toneIndex);
	}
	return name;
}

void DacDataModel::deinitBufferDac() { enableBuffer(false); }

void DacDataModel::deinitDdsDac() { enableDds(false); }
