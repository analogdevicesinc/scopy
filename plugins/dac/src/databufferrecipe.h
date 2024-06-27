#ifndef DATABUFFERRECIPE_H
#define DATABUFFERRECIPE_H

#include "scopy-dac_export.h"

namespace scopy {
namespace dac {
struct SCOPY_DAC_EXPORT DataBufferRecipe
{
	double scale = 0.0;
	int offset = 0;
	int decimation = 0;
	bool scaled = false;
	double phase = 0.0;
	int frequency = 0;
};
} // namespace dac
} // namespace scopy
#endif // DATABUFFERRECIPE_H
