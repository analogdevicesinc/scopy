#include "dacutils.h"

#include <float.h>
#include <qmath.h>

using namespace scopy;
using namespace scopy::dac;

#define SCALE_MINUS_INFINITE -91

double DacUtils::dbFullScaleConvert(double scale, bool inverse)
{
	if(inverse) {
		if(scale == 0)
			return -DBL_MAX;
		return (int)((20 * log10(scale)) - 0.5);
	} else {
		if(scale == SCALE_MINUS_INFINITE)
			return 0;
		return pow(10, scale / 20.0);
	}
}

bool DacUtils::checkDdsChannel(iio_channel *chn)
{
	iio_chan_type chnType = iio_channel_get_type(chn);
	if(chnType != IIO_ALTVOLTAGE) {
		return false;
	}
	auto freq = iio_channel_find_attr(chn, "frequency");
	if(!freq) {
		return false;
	}
	auto scale = iio_channel_find_attr(chn, "scale");
	if(!scale) {
		return false;
	}
	auto phase = iio_channel_find_attr(chn, "phase");
	if(!phase) {
		return false;
	}
	return true;
}
