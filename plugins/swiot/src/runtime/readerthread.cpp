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
#include "src/swiot_logging_categories.h"
#include "src/runtime/ad74413r/bufferlogic.h"
#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <iioutil/iiocommand/iiochannelenable.h>
#include <iioutil/iiocommand/iiodevicecreatebuffer.h>
#include <iioutil/iiocommand/iiobufferdestroy.h>
#include <iioutil/iiocommand/iiobufferrefill.h>

#include <iio.h>

using namespace scopy::swiot;

ReaderThread::ReaderThread(bool isBuffered, CommandQueue *cmdQueue, QObject *parent)
	: QThread{parent}
        , isBuffered(isBuffered)
	, m_enabledChnlsNo(0)
	, m_iioBuff(nullptr)
	, m_iioDev(nullptr)
	, m_cmdQueue(cmdQueue)
	, m_refillBufferCommand(nullptr)
	, m_createBufferCommand(nullptr)
	, m_destroyBufferCommand(nullptr)
	, m_requiredBuffersNumber(0)
	, bufferCounter(0)
{
}

ReaderThread::~ReaderThread()
{
	destroyIioBuffer();
}

void ReaderThread::addDioChannel(int index, struct iio_channel *channel) {
	m_dioChannels.insert(index, channel);
	m_dioChannelsReadCommands.insert(index, new IioChannelAttributeRead(m_dioChannels[index], "raw", m_cmdQueue)); //overwrite?

	connect(m_dioChannelsReadCommands[index], &scopy::Command::finished, this, [=, this] (scopy::Command *cmd) {
		IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead*>(cmd);
		if (!tcmd) {
			return;
		}
		int cmdIndex = m_dioChannelsReadCommands.indexOf(cmd);
		if (tcmd->getReturnCode() >= 0) {
			char *res = tcmd->getResult();
			bool ok = false;
			double raw = QString(res).toDouble(&ok);
			if (ok) {
				 Q_EMIT channelDataChanged(cmdIndex, raw);
			}
			qDebug(CAT_SWIOT_MAX14906) << "Channel with index " << cmdIndex << " read raw value: " << raw;
		} else {
			qCritical(CAT_SWIOT_MAX14906) << "Failed to acquire data on DioReaderThread " << tcmd->getReturnCode();
		}
	}, Qt::QueuedConnection);
}

void ReaderThread::runDio() {
	qDebug(CAT_SWIOT_MAX14906) << "DioReaderThread started";
	try {
		if (!this->m_dioChannels.empty()) {
			auto keys = this->m_dioChannels.keys();
			for (int index : keys) {
				m_cmdQueue->enqueue(m_dioChannelsReadCommands[index]);
			}
		}
	} catch (...) {
		qCritical(CAT_SWIOT_MAX14906) << "Failed to acquire data on DioReaderThread";
	}
}

void ReaderThread::addBufferedDevice(iio_device *device) {
        m_iioDev = device;
}

void ReaderThread::enableIioChnls()
{
        if (m_iioDev) {
		auto keys = m_chnlsInfo.keys();
		for(const auto &key : keys) {
			struct iio_channel* iioChnl = m_chnlsInfo[key]->iioChnl();
                        bool isEnabled = iio_channel_is_enabled(iioChnl);
			if (m_chnlsInfo[key]->isEnabled()) {
                                if (!isEnabled) {
                                        iio_channel_enable(iioChnl);
                                }
				qDebug(CAT_SWIOT_AD74413R) << "Chanel en " << key;
                        } else {
                                if (isEnabled) {
                                        iio_channel_disable(iioChnl);
                                }
				qDebug(CAT_SWIOT_AD74413R) << "Chanel dis " << key;
                        }
                }
        }
}

int ReaderThread::getEnabledChnls()
{
	int enChnls = 0;
	auto keys = m_chnlsInfo.keys();
	for (const auto &key : keys) {
		if (iio_channel_is_enabled(m_chnlsInfo[key]->iioChnl())) {
                        enChnls++;
                }
        }
	return enChnls;
}

QVector<ChnlInfo *> ReaderThread::getEnabledBufferedChnls()
{
	QVector<ChnlInfo *> enabledBufferedChnls= {};

	auto keys = m_chnlsInfo.keys();
	for (const auto &key : keys) {
		if (m_chnlsInfo[key]->isScanElement() && m_chnlsInfo[key]->isEnabled()
				&& !m_chnlsInfo[key]->isOutput()) {
			enabledBufferedChnls.push_back(m_chnlsInfo[key]);
		}
	}
	return enabledBufferedChnls;
}

void ReaderThread::bufferRefillCommandFinished(scopy::Command* cmd)
{
	IioBufferRefill *tcmd = dynamic_cast<IioBufferRefill*>(cmd);
	if (!tcmd) {
		return;
	}
	if (tcmd->getReturnCode() > 0) {
		int i = 0;
		int idx = 0;
		double data = 0.0;

		uint32_t* startAdr = (uint32_t*)iio_buffer_start(m_iioBuff);
		uint32_t* endAdr = (uint32_t*)iio_buffer_end(m_iioBuff);
		m_bufferData.clear();
		for (int i=0; i < m_enabledChnlsNo; i++){
			m_bufferData.push_back({});
		}
		for (uint32_t* ptr = startAdr; ptr != endAdr; ptr++) {
			idx = i % m_enabledChnlsNo;
			data = m_bufferedChnls[idx]->convertData(*ptr);
			m_bufferData[idx].push_back(data);
			i++;
		}
		Q_EMIT bufferRefilled(m_bufferData, bufferCounter);
	} else {
		qDebug(CAT_SWIOT_AD74413R) << "Refill error " << QString(strerror(-tcmd->getReturnCode()));
	}
}

void ReaderThread::createBufferRefillCommand()
{
	if (m_refillBufferCommand) {
		disconnect(m_refillBufferCommand, &scopy::Command::finished, this, &ReaderThread::bufferRefillCommandFinished);
		int occurences = m_cmdQueue->remove(m_refillBufferCommand);
		qDebug(CAT_SWIOT_AD74413R) << " Dequeued occurences of refill command " << occurences;
		delete m_refillBufferCommand;
		m_refillBufferCommand = nullptr;
	}
	m_refillBufferCommand = new IioBufferRefill(m_iioBuff, m_cmdQueue);
	connect(m_refillBufferCommand, &scopy::Command::finished, this, &ReaderThread::bufferRefillCommandFinished, Qt::QueuedConnection);
	qDebug(CAT_SWIOT_AD74413R) << "==== Command refill created";
}

void ReaderThread::createBufferDestroyCommand()
{
	if (m_destroyBufferCommand) {
		int occurences = m_cmdQueue->remove(m_destroyBufferCommand);
		qDebug(CAT_SWIOT_AD74413R) << " Dequeued occurences of destroy command " << occurences;
		disconnect(m_destroyBufferCommand, &scopy::Command::finished, this, &ReaderThread::bufferDestroyCommandFinished);
		delete m_destroyBufferCommand;
		m_destroyBufferCommand = nullptr;
	}
	m_destroyBufferCommand = new IioBufferDestroy(m_iioBuff, m_cmdQueue);
	connect(m_destroyBufferCommand, &scopy::Command::finished, this, &ReaderThread::bufferDestroyCommandFinished, Qt::QueuedConnection);
	qDebug(CAT_SWIOT_AD74413R) << "==== Command destroy created";
}

void ReaderThread::bufferCreateCommandFinished(scopy::Command *cmd)
{
	IioDeviceCreateBuffer *tcmd = dynamic_cast<IioDeviceCreateBuffer*>(cmd);
	if (!tcmd) {
		return;
	}
	if (tcmd->getReturnCode() < 0) {
		qDebug(CAT_SWIOT_AD74413R) << "Buffer wasn't created: " + QString(strerror(-tcmd->getReturnCode()));
	} else {
		m_iioBuff = tcmd->getResult();
		if (m_iioBuff) {
			createBufferRefillCommand();
			createBufferDestroyCommand();
			m_condBufferCreated.notify_one();
		}
	}
}

void ReaderThread::bufferDestroyCommandFinished(scopy::Command *cmd)
{
	IioBufferDestroy *tcmd = dynamic_cast<IioBufferDestroy*>(cmd);
	if (!tcmd) {
		return;
	}
	if (tcmd->getReturnCode() < 0) {
		qDebug(CAT_SWIOT_AD74413R) << "Buffer wasn't destroyed: " + QString(strerror(-tcmd->getReturnCode()));
	} else {
		m_iioBuff = nullptr;
		m_bufferedChnls.clear();
		qDebug(CAT_SWIOT_AD74413R) << "Buffer destroyed";
	}
}

void ReaderThread::createIioBuffer()
{
	m_enabledChnlsNo = getEnabledChnls();
	m_bufferedChnls = getEnabledBufferedChnls();
	qInfo(CAT_SWIOT_AD74413R) << "Enabled channels number: " + QString::number(m_enabledChnlsNo);
	if (m_iioDev) {
		if (m_createBufferCommand) {
			int occurences = m_cmdQueue->remove(m_createBufferCommand);
			qDebug(CAT_SWIOT_AD74413R) << " Dequeued occurences of create command " << occurences;
			disconnect(m_createBufferCommand, &scopy::Command::finished, this, &ReaderThread::bufferCreateCommandFinished);
			delete m_createBufferCommand;
			m_createBufferCommand = nullptr;
		}

		if(m_samplingFreq >= MAX_BUFFER_SIZE) {
			m_createBufferCommand = new IioDeviceCreateBuffer(m_iioDev, MAX_BUFFER_SIZE, false, m_cmdQueue);
		} else {
			m_createBufferCommand = new IioDeviceCreateBuffer(m_iioDev, MIN_BUFFER_SIZE, false, m_cmdQueue);
		}
		connect(m_createBufferCommand, &scopy::Command::finished, this, &ReaderThread::bufferCreateCommandFinished, Qt::QueuedConnection);
		m_cmdQueue->enqueue(m_createBufferCommand);
	}
}

void ReaderThread::destroyIioBuffer()
{
        if (m_iioBuff) {
		m_cmdQueue->enqueue(m_destroyBufferCommand);
		qDebug(CAT_SWIOT_AD74413R) << "===== Command enqueued " << m_destroyBufferCommand;
		int occurences = m_cmdQueue->remove(m_refillBufferCommand);
		qDebug(CAT_SWIOT_AD74413R) << "Removed occurences of refill command " << occurences;
        }
}

void ReaderThread::onChnlsChange(QMap<int, ChnlInfo*> chnlsInfo)
{
        m_chnlsInfo = chnlsInfo;
}

void ReaderThread::onSamplingFreqWritten(int samplingFreq)
{
	m_samplingFreq = samplingFreq;
}

void ReaderThread::runBuffered(int requiredBuffersNumber) {
	qDebug(CAT_SWIOT_AD74413R) << "Thread";
        enableIioChnls();
        createIioBuffer();
	bufferCounter = 0;
	while (!isInterruptionRequested()) {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_condBufferCreated.wait(lock, [=, this] { return !!m_iioBuff; });

		if ((m_requiredBuffersNumber != 0) ) {
			m_cond.wait(lock, [=, this] {
				// If we reached the required nb of buffers (single mode) then wait until
				// interrupt is requested
				return (bufferCounter < m_requiredBuffersNumber);
			});
		}
		if (isInterruptionRequested()) {
			break;
		}

		if (m_iioBuff) {
			if (m_refillBufferCommand) {
				m_cmdQueue->enqueue(m_refillBufferCommand);
				bufferCounter++;
			}
		}
	}
        destroyIioBuffer();
}

void ReaderThread::requestStop()
{
	//std::lock_guard<std::mutex> lock(m_mutex);
	if (isRunning()) {
		requestInterruption();
	}
	bufferCounter = 0;
	m_cond.notify_one();
}

void ReaderThread::run() {
	if (isBuffered) {
		this->runBuffered(m_requiredBuffersNumber);
        } else {
                this->runDio();
        }
}

void ReaderThread::startCapture(int requiredBuffersNumber)
{
	m_requiredBuffersNumber = requiredBuffersNumber;
	this->start();

}

void ReaderThread::singleDio() {
        this->runDio();
}

