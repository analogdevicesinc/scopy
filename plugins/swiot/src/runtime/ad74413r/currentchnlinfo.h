#ifndef CURRENTCHNLINFO_H
#define CURRENTCHNLINFO_H

#include "chnlinfo.h"
#define MIN_CURRENT_VALUE 0
#define MAX_CURRENT_VALUE 25
namespace scopy::swiot {
class CurrentChnlInfo : public ChnlInfo
{
	Q_OBJECT
public:
	explicit CurrentChnlInfo(QString plotUm = "mA", QString hwUm = "mA", iio_channel *iioChnl = nullptr,
				 CommandQueue *cmdQueue = nullptr);
	~CurrentChnlInfo();

	double convertData(unsigned int data) override;
};
} // namespace scopy::swiot

#endif // CURRENTCHNLINFO_H
