#include "fftmarkercontroller.hpp"

#include <QwtText>
#include <freq/fftplotcomponentchannel.h>

using namespace scopy;
using namespace adc;

FFTMarkerController::FFTMarkerController(FFTPlotComponentChannel *ch, QObject *parent)
	: PlotMarkerController(ch, parent)
	, m_ch(ch)
{}

FFTMarkerController::~FFTMarkerController() {}

void FFTMarkerController::init()
{
	this->m_plot = m_ch->m_plotComponent->fftPlot()->plot();
	this->m_xAxis = m_ch->m_plotComponent->fftPlot()->xAxis()->axisId();
	this->m_yAxis = m_ch->m_plotComponent->fftPlot()->yAxis()->axisId();
}

void FFTMarkerController::attachMarkersToPlot()
{
	for(auto m : this->m_markerInfo) {
		m.marker->setValue(m.peak.x, m.peak.y);

		QwtText lbl;
		lbl.setText(m.name);
		lbl.setColor(m_ch->m_fftPlotCh->curve()->pen().color());
		m.marker->setLabel(lbl);
		m.marker->setLabelAlignment(Qt::AlignTop);
		m.marker->setSpacing(10);
	}
}

int FFTMarkerController::findPeakNearIdx(int idx, int range)
{
	auto data = m_ch->m_ch->chData();
	int start = (idx - range) > 0 ? idx - range : 0;
	int maxIdx = start;
	double maxVal = data->yData()[start];
	for(int i = start; i <= idx + range && i < data->size(); i++) {
		if(maxVal < data->yData()[i]) {
			maxVal = data->yData()[i];
			maxIdx = i;
		}
	}
	return maxIdx;
}

void FFTMarkerController::computeImageMarkers()
{
	auto data = m_ch->m_ch->chData();
	this->m_markerInfo.clear();
	int dc_idx = (this->m_complex) ? data->size() / 2 : 0;

	if(m_sortedPeakInfo.count() == 0)
		return;

	MarkerInfo dc = {
		.name = QString("DC"), .marker = m_markers[0], .peak = {data->xData()[dc_idx], data->yData()[dc_idx]}};
	MarkerInfo fund = {.name = QString("Fund"), .marker = m_markers[1], .peak = m_sortedPeakInfo[0]};
	int fund_offset = m_sortedPeakInfo[0].idx - dc_idx;
	int idx = dc_idx - fund_offset;
	MarkerInfo imag = {
		.name = QString("Imag"), .marker = m_markers[2], .peak = {data->xData()[idx], data->yData()[idx]}};

	m_markerInfo.append(dc);
	m_markerInfo.append(fund);
	m_markerInfo.append(imag);
}

void FFTMarkerController::computeSingleToneMarkers()
{
	auto data = m_ch->m_ch->chData();
	m_markerInfo.clear();
	int dc_idx = (m_complex) ? data->size() / 2 : 0;

	if(m_sortedPeakInfo.count() == 0)
		return;

	MarkerInfo dc = {
		.name = QString("DC"), .marker = m_markers[0], .peak = {data->xData()[dc_idx], data->yData()[dc_idx]}};
	MarkerInfo fund = {.name = QString("Fund"), .marker = m_markers[1], .peak = m_sortedPeakInfo[0]};
	int fund_offset = m_sortedPeakInfo[0].idx - dc_idx;
	m_markerInfo.append(dc);
	m_markerInfo.append(fund);
	// Compute harmonics - need double PlotCursors::getHorizIntersectionAt(double pos)

	int histeresis = log2(data->size()); // easy hack - if data size is higher, so is the histeresis

	for(int i = 2; i < this->m_nrOfMarkers - 1; i++) {
		int idx = findPeakNearIdx((fund_offset * i + dc_idx), histeresis);
		MarkerInfo mi = {.name = QString::number(i) + "H",
				 .marker = m_markers[i],
				 .peak = {data->xData()[idx], data->yData()[idx]}};
		m_markerInfo.append(mi);
	}
}

void FFTMarkerController::computePeaks()
{
	auto data = m_ch->m_ch->chData();
	m_peakInfo.clear();

	int m_start = 0;
	int m_stop = (m_complex) ? data->size() : data->size() / 2;

	for(int i = m_start + 2; i < m_stop - 1; i++) {
		if(data->yData()[i - 2] < data->yData()[i - 1] &&
		   data->yData()[i - 1] > data->yData()[i]) { // is there a better way to compute a peak ?
			PeakInfo mi = {.x = data->xData()[i - 1], .y = data->yData()[i - 1], .idx = i - 1};
			m_peakInfo.append(mi);
		}
	}

	m_sortedPeakInfo.clear();
	for(auto v : m_peakInfo) {
		m_sortedPeakInfo.append(v);
	}

	std::sort(m_sortedPeakInfo.begin(), m_sortedPeakInfo.end(),
		  [](const PeakInfo a, const PeakInfo b) { return a.y > b.y; });
}

void FFTMarkerController::initFixedMarker()
{
	this->cacheMarkerInfo();
	for(int i = 0; i < m_nrOfMarkers; i++) {
		double initX = this->popCacheMarkerInfo();
		MarkerInfo mi = {.name = QString("F") + QString::number(i), .marker = m_markers[i], .peak = {initX, 0}};
		this->m_markerInfo.append(mi);
		PlotAxisHandle *handle =
			new PlotAxisHandle(m_ch->plotComponent()->plot(0), m_ch->m_plotComponent->plot(0)->xAxis());
		connect(handle, &PlotAxisHandle::scalePosChanged, this, [=](double v) {
			m_markerInfo[i].peak.x = v;
			computeMarkers();
		});
		this->m_fixedHandles.append(handle);

		handle->setPositionSilent(initX);
	}

	this->setFixedHandleVisible(this->m_handlesVisible);
}
