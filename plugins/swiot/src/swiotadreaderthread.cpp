#include "swiotadreaderthread.hpp"
#include "qdebug.h"
#include <core/logging_categories.h>

SwiotAdReaderThread::SwiotAdReaderThread() :
	m_iioBuff(nullptr)
      ,m_enabledChnlsNo(0)
      ,m_chnlsChanged(false)
{}

double SwiotAdReaderThread::convertData(unsigned int data)
{
	double convertedData = 0.0;
	data <<= 8;
	data = SWAP_UINT32(data);
	convertedData = (data + m_offset) * m_scale;
	return convertedData;
}

void SwiotAdReaderThread::onBufferCreated(struct iio_buffer* iioBuff, int enabledChnlsNo, double scale, double offset)
{
	m_iioBuff = iioBuff;
	m_enabledChnlsNo = enabledChnlsNo;
}

void SwiotAdReaderThread::onBufferDestroyed()
{
	m_iioBuff = nullptr;
}

void SwiotAdReaderThread::onChnlsStatusChanged()
{
	m_chnlsChanged = true;
}

void SwiotAdReaderThread::run()
{
	while (!isInterruptionRequested()) {
		qDebug(CAT_SWIOT_RUNTIME) << "Thread ";
		sleep(1);
		m_chnlsChanged =false;
		if (m_iioBuff) {
			int refillBytes = iio_buffer_refill(m_iioBuff);
			if (refillBytes > 0) {
				//aici sa trimit datele
				int idx = 0;
				double data = 0.0;
				m_bufferData.clear();
				for (int i=0; i < m_enabledChnlsNo; i++){
					m_bufferData.push_back({});
				}
				for (uint32_t* ptr = (uint32_t*)iio_buffer_start(m_iioBuff); ptr != (uint32_t*)iio_buffer_end(m_iioBuff); ptr++) {
					//process data from uint32_t to double
					if (m_chnlsChanged) {
						m_iioBuff = nullptr;
						break;
					}
					data = convertData(*ptr);
					m_bufferData[idx].push_back(data);
					idx = (idx < m_enabledChnlsNo-1) ? idx + 1 : 0;
				}
				Q_EMIT bufferRefilled(m_bufferData);
			}
		}
	}
}
