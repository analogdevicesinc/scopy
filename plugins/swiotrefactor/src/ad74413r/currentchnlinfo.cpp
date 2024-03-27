#include "ad74413r/currentchnlinfo.h"

using namespace scopy::swiotrefactor;
CurrentChnlInfo::CurrentChnlInfo(QString plotUm, QString hwUm, iio_channel *iioChnl, CommandQueue *cmdQueue)
	: ChnlInfo(plotUm, hwUm, iioChnl, cmdQueue)
{
	m_rangeValues = {MIN_CURRENT_VALUE, MAX_CURRENT_VALUE};
}

CurrentChnlInfo::~CurrentChnlInfo() {}

double CurrentChnlInfo::convertData(unsigned int data)
{
	double convertedData = 0.0;
	double defaultFactor = m_unitOfMeasureFactor.contains(m_hwUm.at(0)) ? m_unitOfMeasureFactor[m_hwUm.at(0)] : 1;
	double newFactor = m_unitOfMeasureFactor.contains(m_plotUm.at(0)) ? m_unitOfMeasureFactor[m_plotUm.at(0)] : 1;
	double factor = defaultFactor / newFactor;
	data <<= 8;
	data = SWAP_UINT32(data);
	data &= 0x0000FFFF;
	convertedData = (data + m_offsetScalePair.first) * m_offsetScalePair.second * factor;
	return convertedData;
}
