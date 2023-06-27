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
#include <iio.h>
#include "src/runtime/ad74413r/bufferlogic.h"

using namespace scopy::swiot;

ReaderThread::ReaderThread(bool isBuffered, QObject *parent)
	: QThread{parent}
        , isBuffered(isBuffered)
	, m_enabledChnlsNo(0), m_iioBuff(nullptr)
	, m_iioDev(nullptr)
{
        lock = new QMutex();
}

ReaderThread::~ReaderThread()
{
	destroyIioBuffer();
}

void ReaderThread::addDioChannel(int index, struct iio_channel *channel) {
        this->m_dioChannels.insert(index, channel);
}

void ReaderThread::runDio() {
	qDebug(CAT_SWIOT_MAX14906) << "DioReaderThread started";
        try {
		if (!this->m_dioChannels.empty()) {
			auto keys = this->m_dioChannels.keys();
			for (int index : keys) {
				double raw = -1;
				iio_channel_attr_read_double(this->m_dioChannels[index], "raw", &raw);
				qDebug(CAT_SWIOT_MAX14906) << "Channel with index " << index << " read raw value: " << raw;
				Q_EMIT channelDataChanged(index, raw);
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

void ReaderThread::createIioBuffer()
{
	m_enabledChnlsNo = getEnabledChnls();
	m_bufferedChnls = getEnabledBufferedChnls();
	qInfo(CAT_SWIOT_AD74413R) << "Enabled channels number: " + QString::number(m_enabledChnlsNo);
	if (m_iioDev) {
		if(m_samplingFreq >= MAX_BUFFER_SIZE) {
			m_iioBuff = iio_device_create_buffer(m_iioDev, MAX_BUFFER_SIZE, false);
		} else {
			m_iioBuff = iio_device_create_buffer(m_iioDev, MIN_BUFFER_SIZE, false);
		}
		if (m_iioBuff) {
			qDebug(CAT_SWIOT_AD74413R) << "Buffer created";
		} else {
			qDebug(CAT_SWIOT_AD74413R) << "Buffer wasn't created: " + QString(strerror(errno));
		}
	}
}

void ReaderThread::destroyIioBuffer()
{
        if (m_iioBuff) {
		qDebug(CAT_SWIOT_AD74413R) << "Buffer destroyed";
                iio_buffer_destroy(m_iioBuff);
                m_iioBuff = nullptr;
		m_bufferedChnls.clear();
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

void ReaderThread::runBuffered() {
	qDebug(CAT_SWIOT_AD74413R) << "Thread";
        enableIioChnls();
        createIioBuffer();
        while (!isInterruptionRequested()) {
                if (m_iioBuff) {
                        bufferCounter++;
                        lock->lock();
                        int refillBytes = iio_buffer_refill(m_iioBuff);
                        if (refillBytes > 0) {
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
				qDebug(CAT_SWIOT_AD74413R) << "Refill error " << QString(strerror(-refillBytes));

                        }
                        lock->unlock();
                }
        }
        destroyIioBuffer();
}

void ReaderThread::run() {
        if (isBuffered) {
                this->runBuffered();
        } else {
                this->runDio();
        }
}

void ReaderThread::singleDio() {
        this->runDio();
}

