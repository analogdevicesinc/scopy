#ifndef CHNLINFOBUILDER_H
#define CHNLINFOBUILDER_H

#include "src/runtime/ad74413r/chnlinfo.h"

namespace scopy::swiot {
class ChnlInfoBuilder
{
public:
	enum ChnlInfoType
	{
		VOLTAGE = 0,
		CURRENT = 1,
		RESISTANCE = 2
	};

	static int decodeId(QString function)
	{
		if(function.compare("v") == 0) {
			return VOLTAGE;
		} else if(function.compare("c") == 0) {
			return CURRENT;
		} else if(function.compare("r") == 0) {
			return RESISTANCE;
		} else {
			return -1;
		}
	}
	static ChnlInfo *build(iio_channel *iioChnl, QString id, CommandQueue *cmdQueue);
};
} // namespace scopy::swiot

#endif // CHNLINFOBUILDER_H
