#include "swiotadreaderthread.hpp"
#include "qdebug.h"
#include <core/logging_categories.h>

SwiotAdReaderThread::SwiotAdReaderThread() :
	m_iioBuff(nullptr)
      ,m_offsetScaleValues()
      ,m_enabledChnlsNo(0)
{}

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

void SwiotAdReaderThread::onBufferCreated(struct iio_buffer* iioBuff, int enabledChnlsNo, std::vector<std::pair<double, double>> offsetScaleValues)
{
	m_iioBuff = iioBuff;
	m_enabledChnlsNo = enabledChnlsNo;
	m_offsetScaleValues = offsetScaleValues;
}

void SwiotAdReaderThread::onBufferDestroyed()
{
	m_iioBuff = nullptr;
}

void SwiotAdReaderThread::run()
{
	while (!isInterruptionRequested()) {
//		qDebug(CAT_SWIOT_RUNTIME) << "Thread ";
//		sleep(1000);
		if (m_iioBuff) {
			int refillBytes = iio_buffer_refill(m_iioBuff);
			if (refillBytes > 0) {
				int i = 0;
				int idx = 0;
				double data = 0.0;
				if (!m_iioBuff) {
					break;
				}
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
				Q_EMIT bufferRefilled(m_bufferData);
			} else {
				qDebug(CAT_SWIOT_RUNTIME) << "Refill error " << QString(strerror(-refillBytes));

			}
		}
	}
}
