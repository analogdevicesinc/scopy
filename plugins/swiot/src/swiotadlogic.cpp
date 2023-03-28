#include "swiotadlogic.hpp"

using namespace adiscope;

SwiotAdLogic::SwiotAdLogic(struct iio_device* iioDev) :
	m_plotChnlsNo(0)
      ,m_iioDev(iioDev)
{
	if (m_iioDev) {
		iio_device_set_kernel_buffers_count(m_iioDev, MAX_KERNEL_BUFFER);
		m_chnlsNumber = iio_device_get_channels_count(m_iioDev);
		createChannels();
	}
}

SwiotAdLogic::~SwiotAdLogic()
{
	if (m_chnlsInfo.size() > 0) {
		qDeleteAll(m_chnlsInfo);
		m_chnlsInfo.clear();
	}
}

void SwiotAdLogic::createChannels()
{
	if(m_iioDev) {
		int chnlsNumber = iio_device_get_channels_count(m_iioDev);
		int plotChnlsNo = 0;
		bool isOutput = false;
		const QRegExp rx("[^0-9]+");
		for (int i = 0; i < chnlsNumber; i++) {
			struct iio_channel* iioChnl = iio_device_get_channel(m_iioDev, i);
			struct chnlInfo* chnlInfo = new struct chnlInfo;
			int chnlIdx = -1;
			QString chnlId(iio_channel_get_id(iioChnl));
			double offset = 0.0;
			double scale = 0.0;
			int erno = 0;
			const auto&& parts = chnlId.split(rx);

			iio_channel_disable(iioChnl);
			isOutput = iio_channel_is_output(iioChnl);
			erno = iio_channel_attr_read_double(iioChnl, "offset", &offset);
			iio_channel_attr_read_double(iioChnl, "scale", &scale);
			if (erno < 0) {
				scale = -1;
				offset = -1;
			} else {
				plotChnlsNo = (!isOutput) ? (plotChnlsNo + 1) : plotChnlsNo;
			}
			chnlInfo->chnlId = chnlId;
			chnlInfo->iioChnl = iioChnl;
			chnlInfo->isEnabled = false;
			chnlInfo->isOutput = isOutput;
			chnlInfo->offsetScalePair = {offset, scale};
			if (parts.size() > 1) {
				if(parts[1].compare("")){
					chnlIdx = parts[1].toInt();
					chnlIdx = (isOutput) ? (chnlIdx + m_chnlsNumber) : chnlIdx;
				}
			}
			m_chnlsInfo[chnlIdx] = chnlInfo;
		}
		m_plotChnlsNo = plotChnlsNo;
	}

}

struct iio_channel* SwiotAdLogic::getIioChnl(int chnlIdx, bool outputPriority)
{
	struct iio_channel* iioChnl = nullptr;
	if (outputPriority) {
		iioChnl = (m_chnlsInfo.contains(chnlIdx + m_chnlsNumber)) ?
					m_chnlsInfo[chnlIdx + m_chnlsNumber]->iioChnl : m_chnlsInfo[chnlIdx]->iioChnl;
	} else {
		iioChnl = m_chnlsInfo[chnlIdx]->iioChnl;
	}

	return iioChnl;
}

bool SwiotAdLogic::verifyEnableChanges(std::vector<bool> enabledChnls)
{
	bool changes = false;
	for (int i = 0; i < enabledChnls.size(); i++) {
		if (m_chnlsInfo.contains(i)) {
			if (enabledChnls[i] != m_chnlsInfo[i]->isEnabled) {
				m_chnlsInfo[i]->isEnabled = enabledChnls[i] ;
				changes = true;
			}
		}
	}
	if (changes) {
		Q_EMIT chnlsChanged(m_chnlsInfo);
	}
	return changes;
}

int SwiotAdLogic::getPlotChnlsNo()
{
	return m_plotChnlsNo;
}
