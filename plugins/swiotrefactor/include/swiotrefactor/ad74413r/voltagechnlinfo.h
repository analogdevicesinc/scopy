#ifndef VOLTAGECHNLINFO_H
#define VOLTAGECHNLINFO_H

#include "chnlinfo.h"
#define MIN_VOLTAGE_VALUE -10
#define MAX_VOLTAGE_VALUE 10
namespace scopy::swiotrefactor {
class VoltageChnlInfo : public ChnlInfo
{
	Q_OBJECT
public:
	explicit VoltageChnlInfo(QString plotUm = "V", QString hwUm = "mV", iio_channel *iioChnl = nullptr,
				 CommandQueue *cmdQueue = nullptr);
	~VoltageChnlInfo();

	double convertData(unsigned int data) override;
};
} // namespace scopy::swiotrefactor

#endif // VOLTAGECHNLINFO_H
