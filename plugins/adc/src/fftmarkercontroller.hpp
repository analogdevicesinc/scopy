#ifndef FFTMARKERCONTROLLER_HPP
#define FFTMARKERCONTROLLER_HPP

#include <markercontroller.h>

namespace scopy {
namespace adc {

class FFTPlotComponentChannel;

class FFTMarkerController : public MarkerController
{
public:
	FFTMarkerController(FFTPlotComponentChannel *ch, QObject *parent);
	~FFTMarkerController();

	// MarkerController interface
public:
	void init();
	void attachMarkersToPlot() override;

protected:
	int findPeakNearIdx(int idx, int range) override;
	void computeImageMarkers() override;
	void computeSingleToneMarkers() override;
	void computePeaks() override;
	void initFixedMarker() override;

private:
	FFTPlotComponentChannel *m_ch;
};
} // namespace adc
} // namespace scopy
#endif // FFTMARKERCONTROLLER_HPP
