#include "ad74413r/digitalchnlinfo.h"

using namespace scopy::swiotrefactor;

DigitalChnlInfo::DigitalChnlInfo(QString plotUm, QString hwUm, iio_channel *iioChnl, CommandQueue *cmdQueue)
	: ChnlInfo(plotUm, hwUm, iioChnl, cmdQueue)
{
	m_rangeValues = {MIN_DIGITAL_VALUE, MAX_DIGITAL_VALUE};
}

DigitalChnlInfo::~DigitalChnlInfo() {}

double DigitalChnlInfo::convertData(unsigned int data)
{
	double convertedData = 0.0;
	data <<= 8;
	data = SWAP_UINT32(data);
	data &= 0x0000FFFF;
	convertedData = (data + m_offsetScalePair.first) * m_offsetScalePair.second;
	return convertedData;
}
