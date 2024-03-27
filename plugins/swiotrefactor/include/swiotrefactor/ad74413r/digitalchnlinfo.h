#ifndef DIGITALCHNLINFO_H
#define DIGITALCHNLINFO_H

#include "chnlinfo.h"
#define MIN_DIGITAL_VALUE -2
#define MAX_DIGITAL_VALUE 2
namespace scopy::swiotrefactor {
class DigitalChnlInfo : public ChnlInfo
{
	Q_OBJECT
public:
	explicit DigitalChnlInfo(QString plotUm = "", QString hwUm = "", iio_channel *iioChnl = nullptr,
				 CommandQueue *cmdQueue = nullptr);
	~DigitalChnlInfo();

	double convertData(unsigned int data) override;
};
} // namespace scopy::swiotrefactor

#endif // DIGITALCHNLINFO_H
