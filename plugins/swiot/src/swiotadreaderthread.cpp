#include "swiotadreaderthread.hpp"
#include "qdebug.h"
#include <core/logging_categories.h>

SwiotAdReaderThread::SwiotAdReaderThread(struct iio_device *iioDev) :
	m_enabledChnlsNo(0)
      ,m_iioDev(iioDev)
      ,m_iioBuff(nullptr)
      ,m_offsetScaleValues()
{
	lock = new QMutex();
}

double SwiotAdReaderThread::convertData(unsigned int data, int idx)
{
	double convertedData = 0.0;
	data <<= 8;
	data = SWAP_UINT32(data);
	data &= 0x0000FFFF;
//	convertedData = (data + m_offsetScaleValues[idx].first) * m_offsetScaleValues[idx].second;
	convertedData = data;
	return data;
}

void SwiotAdReaderThread::enableIioChnls()
{
	if (m_iioDev) {
		for(const auto &key : m_chnlsInfo.keys()) {
			struct iio_channel* iioChnl = m_chnlsInfo[key]->iioChnl;
			bool isEnabled = iio_channel_is_enabled(iioChnl);
			if (m_chnlsInfo[key]->isEnabled) {
				if (!isEnabled) {
					iio_channel_enable(iioChnl);
				}
				qDebug(CAT_SWIOT_RUNTIME) << "Chanel en " << key;
			} else {
				if (isEnabled) {
					iio_channel_disable(iioChnl);
				}
				qDebug(CAT_SWIOT_RUNTIME) << "Chanel dis " << key;
			}
		}
	}
}

QVector<std::pair<double, double>> SwiotAdReaderThread::getOffsetScaleVector()
{
	QVector<std::pair<double, double>> offsetScalePairs = {};

	for (const auto &key : m_chnlsInfo.keys()) {
		if (m_chnlsInfo[key]->isEnabled) {
			offsetScalePairs.push_back(m_chnlsInfo[key]->offsetScalePair);
		}
	}
	return offsetScalePairs;
}

int SwiotAdReaderThread::getEnabledChnls()
{
	int enChnls = 0;
	for (const auto &key : m_chnlsInfo.keys()) {
		if (iio_channel_is_enabled(m_chnlsInfo[key]->iioChnl)) {
			enChnls++;
		}
	}

	return enChnls;
}

void SwiotAdReaderThread::createIioBuffer()
{
	m_enabledChnlsNo = getEnabledChnls();
	int possibleBufferSize = m_sampleRate * m_timespan;
	m_offsetScaleValues = getOffsetScaleVector();
	qInfo(CAT_SWIOT_RUNTIME) << "Enabled channels number: " + QString::number(m_enabledChnlsNo);
	if (m_iioDev) {
		if(possibleBufferSize >= MAX_BUFFER_SIZE) {
			m_iioBuff = iio_device_create_buffer(m_iioDev, MAX_BUFFER_SIZE, false);
		} else {
			m_iioBuff = iio_device_create_buffer(m_iioDev, MIN_BUFFER_SIZE, false);
		}
		if (m_iioBuff) {
			qDebug(CAT_SWIOT_RUNTIME) << "Buffer created";
		} else {
			qDebug(CAT_SWIOT_RUNTIME) << "Buffer wasn't created: " + QString(strerror(errno));
		}
	}
}

void SwiotAdReaderThread::destroyIioBuffer()
{
	if (m_iioBuff) {
		qDebug(CAT_SWIOT_RUNTIME) << "Buffer destroyed";
		iio_buffer_destroy(m_iioBuff);
		m_iioBuff = nullptr;
	}
}

void SwiotAdReaderThread::onChnlsChange(QMap<int, struct chnlInfo*> chnlsInfo)
{
	m_chnlsInfo = chnlsInfo;
}

void SwiotAdReaderThread::run()
{
	qDebug(CAT_SWIOT_RUNTIME) << "Thread";
	enableIioChnls();
	createIioBuffer();
	while (!isInterruptionRequested()) {
		if (m_iioBuff) {
			bufferCounter++;
			lock->lock();
//			qDebug(CAT_SWIOT_RUNTIME) << QString::number(bufferCounter)+" Before refill" ;
			int refillBytes = iio_buffer_refill(m_iioBuff);
//			qDebug(CAT_SWIOT_RUNTIME) << QString::number(bufferCounter)+" After refill";
			if (refillBytes > 0) {
				int i = 0;
				int idx = 0;
				double data = 0.0;

				u_int32_t* startAdr = (uint32_t*)iio_buffer_start(m_iioBuff);
				u_int32_t* endAdr = (uint32_t*)iio_buffer_end(m_iioBuff);
				m_bufferData.clear();
				for (int i=0; i < m_enabledChnlsNo; i++){
					m_bufferData.push_back({});
				}
				for (uint32_t* ptr = startAdr; ptr != endAdr; ptr++) {
					idx = i % m_enabledChnlsNo;
					data = convertData(*ptr, idx);
					m_bufferData[idx].push_back(data);
					i++;
				}
				Q_EMIT bufferRefilled(m_bufferData, bufferCounter);
			} else {
				qDebug(CAT_SWIOT_RUNTIME) << "Refill error " << QString(strerror(-refillBytes));

			}
			lock->unlock();
		}
	}
	destroyIioBuffer();
}
