#include "ad74413r/resistancechnlinfo.h"

using namespace scopy::swiotrefactor;

ResistanceChnlInfo::ResistanceChnlInfo(QString plotUm, QString hwUm, iio_channel *iioChnl, CommandQueue *cmdQueue)
	: ChnlInfo(plotUm, hwUm, iioChnl, cmdQueue)
{
	m_rangeValues = {MIN_RESISTANCE_VALUE, MAX_RESISTANCE_VALUE};
}

ResistanceChnlInfo::~ResistanceChnlInfo() {}

double ResistanceChnlInfo::convertData(unsigned int data)
{
	double convertedData = 0.0;
	data <<= 8;
	data = SWAP_UINT32(data);
	data &= 0x0000FFFF;
	convertedData =
		((ADC_MAX_VALUE - data) != 0) ? ((data * RPULL_UP) / (ADC_MAX_VALUE - data)) : MAX_RESISTANCE_VALUE;
	return convertedData;
}
