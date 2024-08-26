#include "markercontroller.h"
#include <qwt_text.h>
#include <freq/fftplotcomponentchannel.h>

using namespace scopy::adc;

MarkerController::MarkerController(FFTPlotComponentChannel *ch, QObject *parent)
	: QObject(parent)
	, m_ch(ch)
	, m_plot(nullptr)
	, m_xAxis(QwtAxis::XBottom)
	, m_yAxis(QwtAxis::YLeft)
{
	m_enabled = false;
	m_complex = false;
	m_handlesVisible = true;
}

void MarkerController::init()
{
	m_plot = m_ch->m_plotComponent->fftPlot()->plot();
	m_xAxis = m_ch->m_plotComponent->fftPlot()->xAxis()->axisId();
	m_yAxis = m_ch->m_plotComponent->fftPlot()->yAxis()->axisId();
	setNrOfMarkers(5);
	setMarkerType(MC_NONE);
}

MarkerController::~MarkerController() {}

void MarkerController::setNrOfMarkers(int n)
{
	for(int i = 0; i < m_markers.count(); i++) {
		m_markers[i]->detach();
		delete m_markers[i];
	}
	m_markers.clear();

	m_nrOfMarkers = n;
	if(m_markerType == MC_NONE) {
		return;
	}

	int nrOfMarkers = m_nrOfMarkers;
	if(m_markerType == MC_IMAGE) {
		nrOfMarkers = 3;
	}

	for(int i = 0; i < nrOfMarkers; i++) {
		QwtPlotMarker *marker = new QwtPlotMarker();
		m_markers.append(marker);
		marker->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QColor(237, 28, 36),
						QPen(QColor(255, 255, 255, 140), 2, Qt::SolidLine), QSize(5, 5)));
		m_markers[i]->setXAxis(m_xAxis);
		m_markers[i]->setYAxis(m_yAxis);
		m_markers[i]->attach(m_plot);
		m_markers[i]->setVisible(m_enabled);
	}
}

void MarkerController::setMarkerType(MarkerTypes v)
{
	if(m_markerType == MC_FIXED) {
		deinitFixedMarker();
	}

	m_markerType = v;
	setNrOfMarkers(m_nrOfMarkers);

	if(v == MC_FIXED) {
		initFixedMarker();
	}

	Q_EMIT markerEnabled(m_enabled && m_markerType != MC_NONE);
	computeMarkers();
}

void MarkerController::setFixedMarkerFrequency(int idx, double freq)
{

	if(idx > m_markerInfo.count() - 1)
		return;
	m_markerInfo[idx].peak.x = freq;
}

void MarkerController::initFixedMarker()
{
	cacheMarkerInfo();
	for(int i = 0; i < m_nrOfMarkers; i++) {
		double initX = popCacheMarkerInfo();
		MarkerInfo mi = {.name = QString("F") + QString::number(i), .marker = m_markers[i], .peak = {initX, 0}};
		m_markerInfo.append(mi);
		PlotAxisHandle *handle =
			new PlotAxisHandle(m_ch->plotComponent()->plot(0), m_ch->m_plotComponent->plot(0)->xAxis());
		connect(handle, &PlotAxisHandle::scalePosChanged, this, [=](double v) {
			m_markerInfo[i].peak.x = v;
			computeMarkers();
		});
		m_fixedHandles.append(handle);

		handle->setPositionSilent(initX);
	}

	setFixedHandleVisible(m_handlesVisible);
}

void MarkerController::setFixedHandleVisible(bool b)
{
	m_handlesVisible = b;
	if(m_markerType == MC_FIXED) {
		for(auto handle : m_fixedHandles) {
			handle->handle()->setVisible(b);
			handle->handle()->raise();
		}
	}
}

void MarkerController::deinitFixedMarker()
{
	for(auto handle : m_fixedHandles) {
		delete handle;
	}
	m_fixedHandles.clear();
}

void MarkerController::computeFixedMarkerFrequency()
{
	for(int i = 0; i < m_nrOfMarkers; i++) {
		m_markerInfo[i].peak.y = m_ch->plotChannel()->getValueAt(m_markerInfo[i].peak.x);
	}
}

const QList<MarkerController::MarkerInfo> &MarkerController::markerInfo() const { return m_markerInfo; }

void MarkerController::computeMarkers()
{

	if(m_enabled == false)
		return;
	if(m_markerType == MC_NONE)
		return;

	if(m_markerType == MC_PEAK) {
		computePeaks();
		computePeakMarkers();
	}

	if(m_markerType == MC_SINGLETONE) {
		computePeaks();
		computeSingleToneMarkers();
	}

	if(m_markerType == MC_FIXED) {
		computeFixedMarkerFrequency();
	}

	if(m_markerType == MC_IMAGE) {
		computePeaks();
		computeImageMarkers();
	}

	attachMarkersToPlot();
	Q_EMIT markerInfoUpdated();
	m_plot->replot();
}

void MarkerController::setAxes(QwtAxisId x, QwtAxisId y)
{
	m_xAxis = x;
	m_yAxis = y;

	for(int i = 0; i < m_markers.count(); i++) {
		m_markers[i]->setXAxis(m_xAxis);
		m_markers[i]->setYAxis(m_yAxis);
	}

	if(m_markerType == MC_FIXED) {
		deinitFixedMarker();
		initFixedMarker();
	}
}

void MarkerController::setPlot(QwtPlot *p)
{
	m_plot = p;
	for(int i = 0; i < m_markers.count(); i++) {
		m_markers[i]->attach(m_plot);
		;
	}
	if(m_markerType == MC_FIXED) {
		deinitFixedMarker();
		initFixedMarker();
	}
	setEnabled(m_enabled);
	if(m_enabled && m_markerType != MC_NONE) {
		Q_EMIT markerInfoUpdated();
	}
}

void MarkerController::setComplex(bool b) { m_complex = b; }

void MarkerController::computePeaks()
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

void MarkerController::computePeakMarkers()
{
	m_markerInfo.clear();
	for(int i = 0; i < m_markers.count() && i < m_sortedPeakInfo.count(); i++) {
		MarkerInfo mi = {
			.name = QString("P") + QString::number(i), .marker = m_markers[i], .peak = m_sortedPeakInfo[i]};
		m_markerInfo.append(mi);
	}
}

void MarkerController::computeSingleToneMarkers()
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

	for(int i = 2; i < m_nrOfMarkers - 1; i++) {
		int idx = findPeakNearIdx((fund_offset * i + dc_idx), histeresis);
		MarkerInfo mi = {.name = QString::number(i) + "H",
				 .marker = m_markers[i],
				 .peak = {data->xData()[idx], data->yData()[idx]}};
		m_markerInfo.append(mi);
	}
}

void MarkerController::computeImageMarkers()
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
	int idx = dc_idx - fund_offset;
	MarkerInfo imag = {
		.name = QString("Imag"), .marker = m_markers[2], .peak = {data->xData()[idx], data->yData()[idx]}};

	m_markerInfo.append(dc);
	m_markerInfo.append(fund);
	m_markerInfo.append(imag);
}

int MarkerController::findPeakNearIdx(int idx, int range)
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

void MarkerController::cacheMarkerInfo()
{
	m_markerCache.clear();
	for(auto mi : m_markerInfo) {
		m_markerCache.push_back(mi.peak.x);
	}
	m_markerInfo.clear();
}

double MarkerController::popCacheMarkerInfo()
{
	double ret;
	if(m_markerCache.empty()) {
		ret = 0;
	} else {
		ret = m_markerCache.front();
		m_markerCache.pop_front();
	}
	return ret;
}

void MarkerController::attachMarkersToPlot()
{
	for(auto m : m_markerInfo) {
		m.marker->setValue(m.peak.x, m.peak.y);

		QwtText lbl;
		lbl.setText(m.name);
		lbl.setColor(m_ch->m_fftPlotCh->curve()->pen().color());
		m.marker->setLabel(lbl);
		m.marker->setLabelAlignment(Qt::AlignTop);
		m.marker->setSpacing(10);
	}
}

bool MarkerController::enabled() const { return m_enabled; }

void MarkerController::setEnabled(bool newEnabled)
{
	m_enabled = newEnabled;
	for(int i = 0; i < m_markers.count(); i++) {
		if(m_enabled) {
			m_markers[i]->setVisible(true);
		} else {
			m_markers[i]->setVisible(false);
		}
	}
	Q_EMIT markerEnabled(newEnabled && m_markerType != MC_NONE);
}

MarkerPanel::MarkerPanel(QWidget *parent)
{
	m_panelLayout = new QHBoxLayout(this);
	m_panelLayout->setAlignment(Qt::AlignLeft);
	m_panelLayout->setSpacing(0);
	m_panelLayout->setMargin(0);
	setLayout(m_panelLayout);
}

MarkerPanel::~MarkerPanel() {}

void MarkerPanel::newChannel(QString name, QPen c)
{
	if(m_map.contains(name)) {
		deleteChannel(name);
	}
	QWidget *w = new MarkerLabel(name, c, this);
	m_panelLayout->addWidget(w);
	m_map[name] = w;
}

void MarkerPanel::deleteChannel(QString name)
{
	if(!m_map.contains(name))
		return;
	QWidget *w = m_map[name];
	m_panelLayout->removeWidget(w);
	delete w;
	m_map.remove(name);
}

void MarkerPanel::updateChannel(QString name, QList<MarkerController::MarkerInfo> mi)
{
	dynamic_cast<MarkerLabel *>(m_map[name])->updateInfo(mi);
	setFixedHeight(20 + mi.count() * 20);
}

int MarkerPanel::markerCount() { return m_map.count(); }

MarkerLabel::MarkerLabel(QString name, QPen c, QWidget *parent)
{
	m_lay = new QVBoxLayout(this);
	setLayout(m_lay);
	m_lay->setMargin(0);
	m_name = name;
	m_txt = new QTextEdit();
	m_txt->setTextColor(c.color());
	m_txt->setText(name);
	m_lay->addWidget(m_txt);
	m_mpf = new MetricPrefixFormatter(this);
	m_mpf->setTwoDecimalMode(false);
	setFixedWidth(200);
}

MarkerLabel::~MarkerLabel() {}

QString MarkerLabel::name() { return m_name; }

void MarkerLabel::updateInfo(QList<MarkerController::MarkerInfo> markers)
{
	m_txt->setText(m_name);
	for(auto m : markers) {
		m_txt->append(m.name + ": " + m_mpf->format(m.peak.y, "db", 2) + " @ " +
			      m_mpf->format(m.peak.x, "Hz", 3));
	}
}
