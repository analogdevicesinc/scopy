/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#include "readerthread.h"

//#include "src/runtime/ad74413r/bufferlogic.h"
#include "swiot_logging_categories.h"

#include <iio.h>

#include <iioutil/iiocommand/iiobuffercancel.h>
#include <iioutil/iiocommand/iiobufferdestroy.h>
#include <iioutil/iiocommand/iiobufferrefill.h>
#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <iioutil/iiocommand/iiochannelenable.h>
#include <iioutil/iiocommand/iiodevicecreatebuffer.h>

using namespace scopy::swiotrefactor;

ReaderThread::ReaderThread(bool isBuffered, CommandQueue *cmdQueue, QObject *parent)
	: QThread{parent}
	, isBuffered(isBuffered)
	, m_enabledChnlsNo(0)
	, m_iioBuff(nullptr)
	, m_iioDev(nullptr)
	, m_cmdQueue(cmdQueue)
	, m_requiredBuffersNumber(0)
	, bufferCounter(0)
	, m_running(false)
	, m_bufferInvalid(false)
	, m_deinit(true)
{}

ReaderThread::~ReaderThread()
{
	if(m_deinit) {
		forcedStop();
	}
}

void ReaderThread::addDioChannel(int index, struct iio_channel *channel) { m_dioChannels.insert(index, channel); }

void ReaderThread::createDioChannelCommand(int index)
{
	Command *dioChannelReadCommand = new IioChannelAttributeRead(m_dioChannels[index], "raw", nullptr);

	connect(
		dioChannelReadCommand, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) {
			IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
			if(!tcmd) {
				return;
			}

			auto channel = tcmd->getChannel();
			if(!channel) {
				return;
			}
			int channelIndex = m_dioChannels.key(channel);
			if(tcmd->getReturnCode() >= 0) {
				char *res = tcmd->getResult();
				bool ok = false;
				double raw = QString(res).toDouble(&ok);
				if(ok) {
					Q_EMIT channelDataChanged(channelIndex, raw);
				}
				qDebug(CAT_SWIOT_MAX14906)
					<< "Channel with index " << channelIndex << " read raw value: " << raw;
			} else {
				qCritical(CAT_SWIOT_MAX14906)
					<< "Failed to acquire data on DioReaderThread " << tcmd->getReturnCode();
			}
		},
		Qt::QueuedConnection);
	m_cmdQueue->enqueue(dioChannelReadCommand);
}

void ReaderThread::runDio()
{
	qDebug(CAT_SWIOT_MAX14906) << "DioReaderThread started";
	try {
		if(!this->m_dioChannels.empty()) {
			auto keys = this->m_dioChannels.keys();
			for(int index : keys) {
				createDioChannelCommand(index);
			}
		}
	} catch(...) {
		qCritical(CAT_SWIOT_MAX14906) << "Failed to acquire data on DioReaderThread";
	}
}

void ReaderThread::addBufferedDevice(iio_device *device) { m_iioDev = device; }

void ReaderThread::initIioChannels()
{
	//	auto vals = m_chnlsInfo.values();
	//	for(const auto &val : vals) {
	//		val->addReadScaleCommand();
	//		val->addReadOffsetCommand();
	//	}
}

void ReaderThread::enableIioChnls()
{
	//	if(m_iioDev) {
	//		auto keys = m_chnlsInfo.keys();
	//		for(const auto &key : keys) {
	//			struct iio_channel *iioChnl = m_chnlsInfo[key]->iioChnl();
	//			bool isEnabled = iio_channel_is_enabled(iioChnl);
	//			if(m_chnlsInfo[key]->isEnabled()) {
	//				if(!isEnabled) {
	//					iio_channel_enable(iioChnl);
	//				}
	//				qDebug(CAT_SWIOT_AD74413R) << "Chanel en " << key;
	//			} else {
	//				if(isEnabled) {
	//					iio_channel_disable(iioChnl);
	//				}
	//				qDebug(CAT_SWIOT_AD74413R) << "Chanel dis " << key;
	//			}
	//		}
	//	}
}

int ReaderThread::getEnabledChnls()
{
	int enChnls = 0;
	//	auto keys = m_chnlsInfo.keys();
	//	for(const auto &key : keys) {
	//		if(iio_channel_is_enabled(m_chnlsInfo[key]->iioChnl())) {
	//			enChnls++;
	//		}
	//	}
	return enChnls;
}

// QVector<ChnlInfo *> ReaderThread::getEnabledBufferedChnls()
//{
//	QVector<ChnlInfo *> enabledBufferedChnls = {};

//	auto keys = m_chnlsInfo.keys();
//	for(const auto &key : keys) {
//		if(m_chnlsInfo[key]->isScanElement() && m_chnlsInfo[key]->isEnabled() &&
//		   !m_chnlsInfo[key]->isOutput()) {
//			enabledBufferedChnls.push_back(m_chnlsInfo[key]);
//		}
//	}
//	return enabledBufferedChnls;
//}

void ReaderThread::bufferRefillCommandFinished(scopy::Command *cmd)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	IioBufferRefill *tcmd = dynamic_cast<IioBufferRefill *>(cmd);
	if(!tcmd) {
		return;
	}
	if(m_bufferInvalid) {
		return;
	}
	if(tcmd->getReturnCode() > 0) {
		int i = 0;
		int idx = 0;
		double data = 0.0;

		uint32_t *startAdr = (uint32_t *)iio_buffer_start(m_iioBuff);
		uint32_t *endAdr = (uint32_t *)iio_buffer_end(m_iioBuff);
		m_bufferData.clear();
		for(int i = 0; i < m_enabledChnlsNo; i++) {
			m_bufferData.push_back({});
		}

		for(uint32_t *ptr = startAdr; ptr != endAdr; ptr++) {
			idx = i % m_enabledChnlsNo;
			uint32_t d_ptr = (uint32_t)*ptr;
			//			data = m_bufferedChnls[idx]->convertData(d_ptr);
			m_bufferData[idx].push_back(data);
			i++;
		}
		Q_EMIT bufferRefilled(m_bufferData, bufferCounter);
	} else {
		qDebug(CAT_SWIOT_AD74413R) << "Refill error " << QString(strerror(-tcmd->getReturnCode()));
	}
	start();
}

void ReaderThread::bufferCreateCommandFinished(scopy::Command *cmd)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	IioDeviceCreateBuffer *tcmd = dynamic_cast<IioDeviceCreateBuffer *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() < 0) {
		qDebug(CAT_SWIOT_AD74413R) << "Buffer wasn't created: " + QString(strerror(-tcmd->getReturnCode()));
	} else {
		m_iioBuff = tcmd->getResult();
		m_bufferInvalid = false;
		start();
	}
}

void ReaderThread::bufferCancelCommandFinished(scopy::Command *cmd)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	IioBufferCancel *tcmd = dynamic_cast<IioBufferCancel *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() < 0) {
		qDebug(CAT_SWIOT_AD74413R) << "Buffer wasn't canceled: " + QString(strerror(-tcmd->getReturnCode()));
	}
	lock.unlock();
	destroyIioBuffer();
}

void ReaderThread::bufferDestroyCommandStarted(scopy::Command *cmd)
{
	IioBufferDestroy *tcmd = dynamic_cast<IioBufferDestroy *>(cmd);
	if(!tcmd) {
		return;
	}
	m_bufferInvalid = true;
}

void ReaderThread::bufferDestroyCommandFinished(scopy::Command *cmd)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	IioBufferDestroy *tcmd = dynamic_cast<IioBufferDestroy *>(cmd);
	if(!tcmd) {
		Q_EMIT readerThreadFinished();
		return;
	}
	if(tcmd->getReturnCode() < 0) {
		qDebug(CAT_SWIOT_AD74413R) << "Buffer wasn't destroyed: " + QString(strerror(-tcmd->getReturnCode()));
	} else {
		m_iioBuff = nullptr;
		//		m_bufferedChnls.clear();
	}
	Q_EMIT readerThreadFinished();
}

void ReaderThread::createIioBuffer()
{
	//	std::unique_lock<std::mutex> lock(m_mutex);
	//	m_enabledChnlsNo = getEnabledChnls();
	//	m_bufferedChnls = getEnabledBufferedChnls();
	//	qInfo(CAT_SWIOT_AD74413R) << "Enabled channels number: " + QString::number(m_enabledChnlsNo);
	//	if(m_iioDev) {
	//		Command *createBufferCommand;
	//		if(m_samplingFreq >= MAX_BUFFER_SIZE) {
	//			createBufferCommand = new IioDeviceCreateBuffer(m_iioDev, MAX_BUFFER_SIZE, false,
	// nullptr); 		} else { 			createBufferCommand = new
	// IioDeviceCreateBuffer(m_iioDev, MIN_BUFFER_SIZE, false, nullptr);
	//		}
	//		connect(createBufferCommand, &scopy::Command::finished, this,
	//			&ReaderThread::bufferCreateCommandFinished, Qt::QueuedConnection);
	//		m_cmdQueue->enqueue(createBufferCommand);
	//	}
}

void ReaderThread::cancelIioBuffer()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if(m_iioBuff) {
		Command *cancelBufferCommand = new IioBufferCancel(m_iioBuff, nullptr);
		connect(cancelBufferCommand, &scopy::Command::finished, this,
			&ReaderThread::bufferCancelCommandFinished, Qt::QueuedConnection);
		m_cmdQueue->enqueue(cancelBufferCommand);
	}
}

void ReaderThread::destroyIioBuffer()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if(m_iioBuff) {
		Command *destroyBufferCommand = new IioBufferDestroy(m_iioBuff, nullptr);
		connect(destroyBufferCommand, &scopy::Command::finished, this,
			&ReaderThread::bufferDestroyCommandFinished, Qt::QueuedConnection);
		m_bufferInvalid = true;
		m_cmdQueue->enqueue(destroyBufferCommand);
	}
}

// void ReaderThread::onChnlsChange(QMap<int, ChnlInfo *> chnlsInfo) { m_chnlsInfo = chnlsInfo; }

void ReaderThread::onSamplingFrequencyComputed(double samplingFrequency) { m_samplingFreq = samplingFrequency; }

void ReaderThread::runBuffered(int requiredBuffersNumber)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if(!m_iioBuff || !m_running) {
		m_running = false;
		return;
	}
	if((m_requiredBuffersNumber != 0) && (bufferCounter >= m_requiredBuffersNumber)) {
		return;
	}
	if(m_bufferInvalid) {
		return;
	}
	if(m_iioBuff) {
		Command *refillBufferCommand = new IioBufferRefill(m_iioBuff, nullptr);
		connect(refillBufferCommand, &scopy::Command::finished, this,
			&ReaderThread::bufferRefillCommandFinished, Qt::QueuedConnection);
		m_cmdQueue->enqueue(refillBufferCommand);
		bufferCounter++;
	}
}

void ReaderThread::requestStop()
{
	requestInterruption();
	if(isBuffered && m_running) {
		m_running = false;
		destroyIioBuffer();
	}
	wait();
}

void ReaderThread::run()
{
	if(isBuffered) {
		this->runBuffered(m_requiredBuffersNumber);
	} else {
		this->runDio();
	}
}

void ReaderThread::startCapture(int requiredBuffersNumber)
{
	if(isBuffered) {
		bufferCounter = 0;
		m_requiredBuffersNumber = requiredBuffersNumber;
		enableIioChnls();
		initIioChannels();
		createIioBuffer();
		m_running = true;
	} else {
		this->start();
	}
}

void ReaderThread::singleDio() { this->runDio(); }

void ReaderThread::forcedStop()
{
	requestInterruption();
	if(isBuffered && m_running) {
		m_running = false;
		cancelIioBuffer();
	}
	wait();
}

void ReaderThread::handleConnectionDestroyed()
{
	m_deinit = false;
	requestInterruption();
}
