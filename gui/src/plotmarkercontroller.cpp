/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "plotmarkercontroller.h"
#include <plotcomponent.h>
#include <qwt_text.h>

using namespace scopy;

PlotMarkerController::PlotMarkerController(PlotComponentChannel *ch, QObject *parent)
	: QObject(parent)
	, m_ch(ch)
	, m_plot(nullptr)
	, m_xAxis(QwtAxis::XBottom)
	, m_yAxis(QwtAxis::YLeft)
{
	init();
}

PlotMarkerController::PlotMarkerController(QObject *parent)
	: QObject(parent)
	, m_plot(nullptr)
	, m_xAxis(QwtAxis::XBottom)
	, m_yAxis(QwtAxis::YLeft)
{
	init();
}

void PlotMarkerController::init()
{
	m_enabled = false;
	m_complex = false;
	m_handlesVisible = true;
	setNrOfMarkers(5);
	setMarkerType(MC_NONE);
}

PlotMarkerController::~PlotMarkerController() {}

void PlotMarkerController::setNrOfMarkers(int n)
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

	if(m_markerType == MC_FIXED) {
		deinitFixedMarker();
		initFixedMarker();
	}
}

void PlotMarkerController::setMarkerType(MarkerTypes v)
{
	m_markerType = v;
	setNrOfMarkers(m_nrOfMarkers);

	Q_EMIT markerEnabled(m_enabled && m_markerType != MC_NONE);
	computeMarkers();
}

void PlotMarkerController::setFixedMarkerFrequency(int idx, double freq)
{

	if(idx > m_markerInfo.count() - 1)
		return;
	m_markerInfo[idx].peak.x = freq;
}

void PlotMarkerController::setFixedHandleVisible(bool b)
{
	m_handlesVisible = b;
	if(m_markerType == MC_FIXED) {
		for(auto handle : qAsConst(m_fixedHandles)) {
			handle->handle()->setVisible(b);
			handle->handle()->raise();
		}
	}
}

void PlotMarkerController::deinitFixedMarker()
{
	for(auto handle : qAsConst(m_fixedHandles)) {
		delete handle;
	}
	m_fixedHandles.clear();
}

void PlotMarkerController::computeFixedMarkerFrequency()
{
	for(int i = 0; i < m_nrOfMarkers; i++) {
		m_markerInfo[i].peak.y = m_ch->plotChannel()->getValueAt(m_markerInfo[i].peak.x);
	}
}

const QList<PlotMarkerController::MarkerInfo> &PlotMarkerController::markerInfo() const { return m_markerInfo; }

void PlotMarkerController::computeMarkers()
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

void PlotMarkerController::setAxes(QwtAxisId x, QwtAxisId y)
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

void PlotMarkerController::setPlot(QwtPlot *p)
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

void PlotMarkerController::setComplex(bool b) { m_complex = b; }

void PlotMarkerController::computePeakMarkers()
{
	m_markerInfo.clear();
	for(int i = 0; i < m_markers.count() && i < m_sortedPeakInfo.count(); i++) {
		MarkerInfo mi = {
			.name = QString("P") + QString::number(i), .marker = m_markers[i], .peak = m_sortedPeakInfo[i]};
		m_markerInfo.append(mi);
	}
}

void PlotMarkerController::cacheMarkerInfo()
{
	m_markerCache.clear();
	for(const auto &mi : qAsConst(m_markerInfo)) {
		m_markerCache.push_back(mi.peak.x);
	}
	m_markerInfo.clear();
}

double PlotMarkerController::popCacheMarkerInfo()
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

PlotComponentChannel *PlotMarkerController::ch() const { return m_ch; }

void PlotMarkerController::setCh(PlotComponentChannel *newCh) { m_ch = newCh; }

bool PlotMarkerController::enabled() const { return m_enabled; }

void PlotMarkerController::setEnabled(bool newEnabled)
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

void MarkerPanel::updateChannel(QString name, QList<PlotMarkerController::MarkerInfo> mi)
{
	dynamic_cast<MarkerLabel *>(m_map[name])->updateInfo(mi);
	setFixedHeight(25 + mi.count() * 20);
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

void MarkerLabel::updateInfo(QList<PlotMarkerController::MarkerInfo> markers)
{
	m_txt->setText(m_name);
	for(const auto &m : markers) {
		m_txt->append(m.name + ": " + m_mpf->format(m.peak.y, "dB", 2) + " @ " +
			      m_mpf->format(m.peak.x, "Hz", 3));
	}
}

#include "moc_plotmarkercontroller.cpp"
