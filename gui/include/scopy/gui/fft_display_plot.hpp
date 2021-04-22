#ifndef FFTDISPLAYPLOT_HPP
#define FFTDISPLAYPLOT_HPP

namespace scopy {
namespace gui {

class FftDisplayPlot
{

public:
	enum class AverageType
	{
		SAMPLE = 0,
		PEAK_HOLD = 1,
		PEAK_HOLD_CONTINUOUS = 2,
		MIN_HOLD = 3,
		MIN_HOLD_CONTINUOUS = 4,
		LINEAR_RMS = 5,
		LINEAR_DB = 6,
		EXPONENTIAL_RMS = 7,
		EXPONENTIAL_DB = 8,
	};

	enum class MagnitudeType
	{
		DBFS = 0,
		DBV = 1,
		DBU = 2,
		VPEAK = 3,
		VRMS = 4,
		VROOTHZ = 5
	};

	enum class MarkerType
	{
		MANUAL = 0,
		PEAK = 1,
		DELTA = 2,
		FIXED = 3,
	};
};

} // namespace gui
} // namespace scopy

#endif // FFTDISPLAYPLOT_H
