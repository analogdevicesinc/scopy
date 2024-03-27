#include "ad74413r/voltagechnlinfo.h"

using namespace scopy::swiotrefactor;

VoltageChnlInfo::VoltageChnlInfo(QString plotUm, QString hwUm, iio_channel *iioChnl, CommandQueue *cmdQueue)
	: ChnlInfo(plotUm, hwUm, iioChnl, cmdQueue)
{
	m_rangeValues = {MIN_VOLTAGE_VALUE, MAX_VOLTAGE_VALUE};
}

VoltageChnlInfo::~VoltageChnlInfo() {}

double VoltageChnlInfo::convertData(unsigned int data)
{
	double defaultFactor = m_unitOfMeasureFactor.contains(m_hwUm.at(0)) ? m_unitOfMeasureFactor[m_hwUm.at(0)] : 1;
	double newFactor = m_unitOfMeasureFactor.contains(m_plotUm.at(0)) ? m_unitOfMeasureFactor[m_plotUm.at(0)] : 1;
	double factor = defaultFactor / newFactor;
	double convertedData = 0.0;
	data <<= 8;
	data = SWAP_UINT32(data);
	data &= 0x0000FFFF;
	convertedData = (data + m_offsetScalePair.first) * m_offsetScalePair.second * factor;
	Q_EMIT instantValueChanged(convertedData);
	return convertedData;
}
