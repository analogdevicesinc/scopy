/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "dacdatamodel.h"
#include "txnode.h"
#include "dac_logging_categories.h"

#include <pluginbase/preferences.h>

#include <component/device.h>
#include <component/channel.h>
#include <component/streamformat.h>
#include <component/backends/iio/iiochannel.h>
#include <component/backends/iio/iiosamplecodec.h>
#include <component/backends/iio/iiooutputstream.h>
#include <component/backends/iio/iioscanelement.h>

#include <algorithm>

#include <QDebug>
#include <iio.h>

using namespace scopy;
using namespace scopy::dac;
DacDataModel::DacDataModel(component::Device *dev, QObject *parent)
	: QObject(parent)
	, m_ddsTxs({})
	, m_bufferTxs({})
	, m_activeBuffer(false)
	, m_out(nullptr)
	, m_cyclicBuffer(true)
	, m_repeatFileBuffer(true)
	, m_interrupted(false)
	, m_cycleInFlight(false)
	, m_userBuffersize(0)
	, m_userKernelBufferCount(0)
	, m_filesize(0)
	, m_decimation(1)
	, m_debounceTimer(nullptr)
{
	m_dev = dev;
	m_name = m_dev->name();

	m_isBufferCapable = initBufferDac();
	m_isDds = initDdsDac();

	m_debounceTimer = new QTimer(this);
	m_debounceTimer->setSingleShot(true);
	m_debounceTimer->setInterval(DEBOUNCE_TIME_MS);
	connect(m_debounceTimer, &QTimer::timeout, this, &DacDataModel::startPushOperation);

	connect(
		this, &DacDataModel::reqInitBuffer, this,
		[this]() {
			if(isRunning()) {
				initBuffer();
			}
		},
		Qt::QueuedConnection);
}

DacDataModel::~DacDataModel()
{
	m_interrupted = true;
	if(m_debounceTimer) {
		m_debounceTimer->stop();
	}
	if(m_pushTask) {
		QCoro::waitFor(m_pushTask.value());
		m_pushTask.reset();
	}
	if(m_out && m_out->isOpen()) {
		QCoro::waitFor(m_out->closeAsync());
	}
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

component::Device *DacDataModel::getDev() const { return m_dev; }

QString DacDataModel::getName() const { return m_name; }

bool DacDataModel::isBufferCapable() const { return m_isBufferCapable; }

bool DacDataModel::isDds() const { return m_isDds; }

void DacDataModel::disableBuffer()
{
	qDebug(CAT_DAC_DATA) << QString("Disable buffer.");
	if(m_isBufferCapable && m_activeBuffer) {
		for(auto node : std::as_const(m_bufferTxs)) {
			enableBufferChannel(node->getUuid(), false);
		}
	}
}

void DacDataModel::enableBuffer()
{
	qDebug(CAT_DAC_DATA) << QString("Enable buffer.");
	if(m_isBufferCapable) {
		m_activeBuffer = true;
	}
}

void DacDataModel::enableDds(bool enable)
{
	qDebug(CAT_DAC_DATA) << QString("Enable DDS %1").arg(enable);
	if(m_isDds) {
		for(auto tx : std::as_const(m_ddsTxs)) {
			QCoro::waitFor(tx->enableDds(enable));
		}
	}
}

QMap<QString, TxNode *> DacDataModel::getBufferTxs() const { return m_bufferTxs; }

QMap<QString, TxNode *> DacDataModel::getDdsTxs() const { return m_ddsTxs; }

void DacDataModel::setRepeatFileBuffer(bool repeat)
{
	requestInterruption();
	m_repeatFileBuffer = repeat;
	autoBuffersizeAndKernelBuffers();
	tryInitBuffer();
}

void DacDataModel::requestInterruption()
{
	m_interrupted = true;
	if(m_pushTask) {
		QCoro::waitFor(m_pushTask.value());
		m_pushTask.reset();
	}
	qDebug(CAT_DAC_DATA) << "Cycle stopped.";
}

void DacDataModel::setCyclic(bool cyclic)
{
	requestInterruption();
	m_cyclicBuffer = cyclic;
	autoBuffersizeAndKernelBuffers();
	tryInitBuffer();
}

void DacDataModel::setKernelBuffersCount(unsigned int kernelCount)
{
	if(kernelCount != m_userKernelBufferCount) {
		requestInterruption();
		m_userKernelBufferCount = kernelCount;
		tryInitBuffer();
	}
}

void DacDataModel::setDecimation(double decimation)
{
	requestInterruption();
	m_decimation = decimation;
	tryInitBuffer();
}

void DacDataModel::setBuffersize(unsigned int buffersize)
{
	if(m_userBuffersize != buffersize) {
		requestInterruption();
		m_userBuffersize = buffersize;
		tryInitBuffer();
	}
}

bool DacDataModel::setFilesize(unsigned int filesize)
{
	bool needToUpdate = (m_filesize != filesize);
	if(needToUpdate) {
		requestInterruption();
		m_filesize = filesize;
		autoBuffersizeAndKernelBuffers();
		tryInitBuffer();
	}
	return needToUpdate;
}

component::iio::IIOScanElement *DacDataModel::scanElement(TxNode *node) const
{
	return node ? m_scanElements.value(node->getUuid(), nullptr) : nullptr;
}

void DacDataModel::enableBufferChannel(QString uuid, bool enable)
{
	requestInterruption();
	auto *el = m_scanElements.value(uuid, nullptr);
	if(!el) {
		qDebug(CAT_DAC_DATA) << QString("No scan element for uuid %1").arg(uuid);
		return;
	}
	QCoro::waitFor(el->enableAsync(enable));

	tryInitBuffer();
}

unsigned int DacDataModel::getEnabledChannelsCount()
{
	unsigned int enChannelsCount = 0;
	for(auto node : std::as_const(m_bufferTxs)) {
		auto *el = scanElement(node);
		enChannelsCount += (el && el->isEnabled()) ? 1 : 0;
	}
	return enChannelsCount;
}

void DacDataModel::setData(QVector<QVector<double>> data)
{
	bool updatedAndReqInit = false;
	requestInterruption();
	m_data.clear();
	m_data = data;
	if((m_filesize == 0) || (m_filesize > data.size())) {
		updatedAndReqInit = setFilesize(data.size());
	}
	if(!updatedAndReqInit) {
		tryInitBuffer();
	}
}

void DacDataModel::setSamplingFrequency(unsigned int sr)
{
	if(m_samplingFrequency != sr) {
		m_samplingFrequency = sr;
		requestInterruption();
		autoBuffersizeAndKernelBuffers();
		tryInitBuffer();
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

void DacDataModel::tryInitBuffer()
{
	// Only restart if a buffer run is currently active
	if(isRunning()) {
		Q_EMIT reqInitBuffer();
	}
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
	if(enabledChannelsCount == 0) {
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

	if(m_data[0].size() < enabledChannelsCount && !m_repeatFileBuffer) {
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
	// Always restart timer - latest call wins
	m_debounceTimer->start();
}

void DacDataModel::startPushOperation()
{
	if(m_pushTask) {
		QCoro::waitFor(m_pushTask.value());
		m_pushTask.reset();
	}
	m_pushTask = pushTask();
}

bool DacDataModel::isRunning()
{
	// A run is live while the stream is open (cyclic keeps it open), a push
	// cycle is in flight, or a debounced (re)start is pending.
	return (m_out && m_out->isOpen()) || m_cycleInFlight || (m_debounceTimer && m_debounceTimer->isActive());
}

QCoro::Task<void> DacDataModel::pushTask()
{
	qDebug(CAT_DAC_DATA) << "Start push cycle";
	m_cycleInFlight = true;
	m_interrupted = false;

	if(!validateBufferParams() || !m_out || m_data.isEmpty()) {
		Q_EMIT invalidRunParams();
		m_cycleInFlight = false;
		co_return;
	}

	if(m_out->isOpen()) {
		co_await m_out->closeAsync();
	}

	m_out->setCyclic(m_cyclicBuffer);
	m_out->setKernelBuffers(m_kernelBufferCount);

	// Enabled scan elements, in stream (findChildren) order -> aligns 1:1 with
	// writeFormat().channels after openAsync.
	const QList<component::iio::IIOScanElement *> allEls =
		m_out->findChildren<component::iio::IIOScanElement *>(QString(), Qt::FindDirectChildrenOnly);
	QList<component::iio::IIOScanElement *> enabledEls;
	QList<int> indices;
	for(auto *el : allEls) {
		if(el->isEnabled()) {
			enabledEls.append(el);
			indices.append(static_cast<int>(el->index()));
		}
	}
	unsigned int enChannelsCount = enabledEls.size();
	if(enChannelsCount == 0) {
		m_cycleInFlight = false;
		co_return;
	}

	auto openRes = co_await m_out->openAsync({indices, m_buffersize});
	if(!openRes) {
		QString msg = "Unable to open output stream.";
		qDebug(CAT_DAC_DATA) << msg;
		Q_EMIT log(msg);
		m_cycleInFlight = false;
		co_return;
	}

	// Per enabled channel, resolve the sample codec (device counts -> raw bytes),
	// matched to the scan element by id.
	QVector<component::iio::IIOSampleCodec *> codecs(enChannelsCount, nullptr);
	for(int chIdx = 0; chIdx < enabledEls.size(); ++chIdx) {
		for(auto node : std::as_const(m_bufferTxs)) {
			auto *chn = node->getChannel();
			if(chn && chn->id() == enabledEls[chIdx]->id()) {
				codecs[chIdx] = chn->findChild<component::iio::IIOSampleCodec *>();
				break;
			}
		}
	}

	// Build per-channel int32 sample columns (decimation + repeat over data columns).
	unsigned int additionalSamples = m_cyclicBuffer ? 0 : (m_filesize % m_buffersize);
	unsigned int available_data_columns = m_data[0].size();
	QVector<QVector<int32_t>> allDataC(enChannelsCount);
	for(unsigned int ch = 0; ch < enChannelsCount; ch++) {
		for(unsigned int i = 0; i < m_filesize + additionalSamples; i += m_decimation) {
			unsigned int sampleIdx = std::min(i, m_filesize - 1);
			allDataC[ch].append(static_cast<int32_t>(m_data[sampleIdx][ch % available_data_columns]));
		}
	}

	component::StreamFormat &fmt = m_out->writeFormat();
	char *base = static_cast<char *>(fmt.data);
	unsigned int samplesPerBuffer = fmt.sampleCount;
	unsigned int totalSamples = allDataC[0].size();
	int totalNbBuffers = m_cyclicBuffer ? 1 : (samplesPerBuffer ? (totalSamples / samplesPerBuffer) : 0);

	int bufferIdx = 1;
	while(!m_interrupted && bufferIdx <= totalNbBuffers) {
		unsigned int srcBase = (bufferIdx - 1) * samplesPerBuffer;
		for(int chIdx = 0; chIdx < enabledEls.size(); ++chIdx) {
			auto *codec = codecs[chIdx];
			if(!codec) {
				continue;
			}
			const component::ChannelFormat &cf = fmt.channels.at(chIdx);
			const QVector<int32_t> &col = allDataC[chIdx];
			for(unsigned int s = 0; s < samplesPerBuffer; s++) {
				unsigned int srcIdx = srcBase + s;
				if(srcIdx >= static_cast<unsigned int>(col.size())) {
					break;
				}
				char *dst = base + cf.offset + cf.stride * static_cast<ptrdiff_t>(s);
				codec->convertInverse(dst, &col[srcIdx]);
			}
		}
		auto pushRes = co_await m_out->pushAsync();
		if(!pushRes) {
			QString msg = "Failed to push buffer.";
			qDebug(CAT_DAC_DATA) << msg;
			Q_EMIT log(msg);
			Q_EMIT requestStop();
			break;
		}
		QString logMsg = QString("Pushed %1 samples (%2/%3 buffers)")
					 .arg(samplesPerBuffer)
					 .arg(bufferIdx)
					 .arg(totalNbBuffers);
		qDebug(CAT_DAC_DATA) << logMsg;
		Q_EMIT log(logMsg);
		bufferIdx++;
	}

	// Cyclic output keeps transmitting from the kernel buffer, so leave the stream
	// open; non-cyclic (or interrupted) runs close it once drained.
	if(!m_cyclicBuffer || m_interrupted) {
		if(m_out->isOpen()) {
			co_await m_out->closeAsync();
		}
	}
	if(m_interrupted) {
		Q_EMIT log(QString("Aborting cycle..."));
	}

	m_cycleInFlight = false;

	if(!m_interrupted && !m_cyclicBuffer) {
		Q_EMIT requestStop();
		qDebug(CAT_DAC_DATA) << "Non-cyclic run completed, signaling UI";
	}
}

void DacDataModel::start() { initBuffer(); }

void DacDataModel::stop()
{
	requestInterruption();
	if(!m_isBufferCapable) {
		return;
	}
	if(m_out && m_out->isOpen()) {
		QCoro::waitFor(m_out->closeAsync());
		qDebug(CAT_DAC_DATA) << "Stream closed.";
	}
}

bool DacDataModel::initBufferDac()
{
	unsigned int txCount = 0;
	m_out = m_dev->findChild<component::iio::IIOOutputStream *>();
	const QList<component::Channel *> channels =
		m_dev->findChildren<component::Channel *>(QString(), Qt::FindDirectChildrenOnly);
	QList<component::iio::IIOScanElement *> els;
	if(m_out) {
		els = m_out->findChildren<component::iio::IIOScanElement *>(QString(), Qt::FindDirectChildrenOnly);
	}
	for(auto *chn : channels) {
		if(!chn->isOutput()) {
			continue;
		}
		// Scan-element (buffer TX) channels carry an IIOSampleCodec child.
		if(!chn->findChild<component::iio::IIOSampleCodec *>()) {
			continue;
		}
		txCount++;
		QString id = chn->id();
		QString name = chn->name();
		if(name != "") {
			id += ":" + name;
		}
		QString uuid = m_dev->name() + ":" + id;
		m_bufferTxs.insert(uuid, new TxNode(uuid, chn, this));
		for(auto *el : els) {
			if(el->id() == chn->id()) {
				m_scanElements.insert(uuid, el);
				break;
			}
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
	const QList<component::Channel *> channels =
		m_dev->findChildren<component::Channel *>(QString(), Qt::FindDirectChildrenOnly);
	for(auto *chn : channels) {
		if(!chn->isOutput()) {
			continue;
		}
		auto *iioChn = qobject_cast<component::iio::IIOChannel *>(chn);
		if(!iioChn || iioChn->chanType() != IIO_ALTVOLTAGE) {
			continue;
		}
		ddsTonesCount++;

		// Name should contain TX*_I/Q_F or *A/*B
		QString name = chn->name();
		QString id = chn->id();
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
	int num_enabled = 0;
	int ch_pos = 0;

	if(enabled_mask)
		*enabled_mask = 0;

	for(auto ch : std::as_const(m_bufferTxs)) {
		auto *el = scanElement(ch);
		bool enabled = el && el->isEnabled();
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
		int chnIndex = chnId.mid(idx + toneId.size()).toInt(&ok);
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

void DacDataModel::deinitBufferDac() { disableBuffer(); }

void DacDataModel::deinitDdsDac()
{
	auto reset = Preferences::GetInstance()->get("dac_reset_dds_on_disconnect").toBool();
	if(!reset) {
		return;
	}
	enableDds(false);
}
