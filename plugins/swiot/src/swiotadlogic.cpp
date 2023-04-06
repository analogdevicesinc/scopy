#include "swiotadlogic.hpp"
#include <core/logging_categories.h>

using namespace adiscope;

SwiotAdLogic::SwiotAdLogic(struct iio_device* iioDev) :
	m_plotChnlsNo(0)
      ,m_offset(0)
      ,m_scale(0)
      ,m_iioDev(iioDev)
      ,m_iioBuff(nullptr)
{
	if (m_iioDev) {
		createChannels();
	}
}

SwiotAdLogic::~SwiotAdLogic()
{
}

void SwiotAdLogic::createChannels()
{
	if(m_iioDev) {
		int chnlsNumber = iio_device_get_channels_count(m_iioDev);
		int plotChnlsNo = 0;
		bool isOutput = false;
		bool first = true;
		for (int i = 0; i < chnlsNumber; i++) {
			struct iio_channel* iioChnl = iio_device_get_channel(m_iioDev, i);
			QString chnlId(iio_channel_get_id(iioChnl));
			isOutput = iio_channel_is_output(iioChnl);
			iio_channel_disable(iioChnl);
			m_channels.insert(chnlId, iioChnl);

			plotChnlsNo = (!isOutput) ? plotChnlsNo + 1 : plotChnlsNo;
			if (first && !isOutput) {
				iio_channel_attr_read_double(iioChnl, "offset", &m_offset);
				iio_channel_attr_read_double(iioChnl, "scale", &m_scale);
				first = false;
			}
		}
//1 is the input channel used for FAULTS
		m_plotChnlsNo = plotChnlsNo - 1;
	}

}

std::vector<struct iio_channel*> SwiotAdLogic::getChnlsByIndex(int chnlIdx)
{
	QString voltage("voltage" +QString::number(chnlIdx));
	QString current("current" +QString::number(chnlIdx));
	QString resistence("resistence" +QString::number(chnlIdx));
	std::vector<struct iio_channel*> iioChnls;

	if (m_channels.contains(voltage)) {
		iioChnls.push_back(m_channels[voltage]);
	}
	if (m_channels.contains(current)) {
		iioChnls.push_back(m_channels[current]);
	}
	if (m_channels.contains(resistence)) {
		iioChnls.push_back(m_channels[resistence]);
	}

	return iioChnls;

}

struct iio_channel* SwiotAdLogic::getIioChnl(int chnlIdx, bool outputPriority)
{
	std::vector<struct iio_channel*> chnls = getChnlsByIndex(chnlIdx);
	struct iio_channel* iioChnl = nullptr;
	int chnlsNumber = chnls.size();
	switch (chnlsNumber) {
		case 1:
			iioChnl = chnls.back();
			break;
		case 2:
			if (outputPriority) {
				iioChnl = (iio_channel_is_output(chnls.front())) ? chnls.front() : chnls.back();
			} else {
				iioChnl = (iio_channel_is_output(chnls.front())) ? chnls.back() : chnls.front();
			}

			break;
	}
	return iioChnl;
}

void SwiotAdLogic::enableIioChnls(bool changes)
{
	if (changes && m_channels.size() > 0 && m_enabledChannels.size() > 0) {
		for(int i = 0; i < m_enabledChannels.size(); i++) {
			struct iio_channel* iioChnl = getIioChnl(i, false);
			if (iioChnl) {
				if (m_enabledChannels[i]) {
					iio_channel_enable(iioChnl);
					qDebug(CAT_SWIOT_RUNTIME) << "Chanel en " << i;

				} else {
					iio_channel_disable(iioChnl);
					qDebug(CAT_SWIOT_RUNTIME) << "Chanel dis " << i;
				}
			}
		}
	}

}

bool SwiotAdLogic::verifyEnableChanges(std::vector<bool> enabledChnls)
{
	bool changes = false;
	if (m_enabledChannels.size() == 0) {
		m_enabledChannels = enabledChnls;
		changes = true;
	} else {
		changes = (m_enabledChannels == enabledChnls) ? false : true;
		m_enabledChannels = enabledChnls;
	}
	return changes;
}

void SwiotAdLogic::createIioBuffer(int bufferSize)
{
	int enabledChnlsNo = getEnabledChnls();
	qInfo(CAT_SWIOT_RUNTIME) << "Enabled channels number: " + QString::number(enabledChnlsNo);
	if (m_iioDev) {
		m_iioBuff = iio_device_create_buffer(m_iioDev, bufferSize, false);
		if (m_iioBuff) {
			qDebug(CAT_SWIOT_RUNTIME) << "Buffer created";
		} else {
			qDebug(CAT_SWIOT_RUNTIME) << "Buffer wasn't created";
		}
	}

	Q_EMIT bufferCreated(m_iioBuff, enabledChnlsNo, m_scale, m_offset);

}

void SwiotAdLogic::destroyIioBuffer()
{
	Q_EMIT bufferDestroyed();
	if (m_iioBuff) {
		iio_buffer_destroy(m_iioBuff);
		m_iioBuff = nullptr;
	}
}

int SwiotAdLogic::getEnabledChnls()
{
	int enChnls = 0;

	for (const QString &key : m_channels.keys()) {
		if (iio_channel_is_enabled(m_channels[key])) {
			enChnls++;
		}
	}

	//to be deleted----------
//	int verifyEnChannels = 0;
//	if (m_enabledChannels.size() > 0) {
//		verifyEnChannels = std::count(m_enabledChannels.begin(), m_enabledChannels.end(), true);
//	}

//	if (enChnls != verifyEnChannels) {
//		qDebug(CAT_SWIOT_RUNTIME) << "Something went wrong";
//	}
	//------------------------

	return enChnls;
}

int SwiotAdLogic::getPlotChnlsNo()
{
	return m_plotChnlsNo;
}
