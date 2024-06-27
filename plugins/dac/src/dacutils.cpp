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
