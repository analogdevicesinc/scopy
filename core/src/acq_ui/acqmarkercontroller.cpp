/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "acqmarkercontroller.h"

#include "acqaxis.h"
#include "acqcurvechannel.h"
#include "acqplot.h"

#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotchannel.h>
#include <gui/plotwidget.h>

#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotMarker>
#include <qwt_text.h>

#include <algorithm>
#include <cmath>

using namespace scopy;
using namespace scopy::adc;

AcqMarkerController::AcqMarkerController(AcqCurveChannel *ch, QObject *parent)
	: PlotMarkerController(parent)
	, m_curve(ch)
{}

void AcqMarkerController::bind()
{
	if(!m_curve || !m_curve->plotOwner() || !m_curve->plotOwner()->plot() || !m_curve->xAxis() ||
	   !m_curve->yAxis()) {
		return;
	}
	setAxes(m_curve->xAxis()->plotAxis()->axisId(), m_curve->yAxis()->plotAxis()->axisId());
	setPlot(m_curve->plotOwner()->plot()->plot());
}

QList<PlotMarkerController::MarkerInfo> AcqMarkerController::deltaInfo(const QList<MarkerInfo> &info)
{
	QList<MarkerInfo> out = info;
	for(int i = 1; i < out.size(); ++i) {
		out[i].name = QStringLiteral("Δ") + out[i].name;
		out[i].peak.x -= info[0].peak.x;
		out[i].peak.y -= info[0].peak.y;
	}
	return out;
}

QVector<QPointF> AcqMarkerController::samples() const
{
	QVector<QPointF> out;
	if(!m_curve || !m_curve->plotChannel() || !m_curve->plotChannel()->curve()) {
		return out;
	}
	const QwtSeriesData<QPointF> *data = m_curve->plotChannel()->curve()->data();
	if(!data) {
		return out;
	}
	const size_t n = data->size();
	out.reserve(static_cast<int>(n));
	for(size_t i = 0; i < n; ++i) {
		out.append(data->sample(i));
	}
	return out;
}

void AcqMarkerController::attachMarkersToPlot()
{
	const QColor color = m_curve ? m_curve->color() : QColor(Qt::white);
	for(const MarkerInfo &m : std::as_const(m_markerInfo)) {
		m.marker->setValue(m.peak.x, m.peak.y);
		QwtText lbl(m.name);
		lbl.setColor(color);
		m.marker->setLabel(lbl);
		m.marker->setLabelAlignment(Qt::AlignTop);
		m.marker->setSpacing(10);
	}
}

void AcqMarkerController::moveFixedMarker(int idx, double x)
{
	if(idx < 0 || idx >= m_markerInfo.size()) {
		return;
	}
	setFixedMarkerFrequency(idx, x);
	if(idx < m_fixedHandles.size()) {
		m_fixedHandles[idx]->setPositionSilent(x);
	}
	computeMarkers();
}

bool AcqMarkerController::stepToPeak(int idx, PeakStep step)
{
	if(idx < 0 || idx >= m_markerInfo.size()) {
		return false;
	}
	computePeaks();
	const PeakInfo cur = m_markerInfo[idx].peak;
	const PeakInfo *best = nullptr;
	for(const PeakInfo &p : std::as_const(m_peakInfo)) {
		switch(step) {
		case PeakStep::Left:
			if(p.x < cur.x && (!best || p.x > best->x)) {
				best = &p;
			}
			break;
		case PeakStep::Right:
			if(p.x > cur.x && (!best || p.x < best->x)) {
				best = &p;
			}
			break;
		case PeakStep::Higher:
			if(p.y > cur.y && (!best || p.y < best->y)) {
				best = &p;
			}
			break;
		case PeakStep::Lower:
			if(p.y < cur.y && (!best || p.y > best->y)) {
				best = &p;
			}
			break;
		case PeakStep::Highest:
			if(!best || p.y > best->y) {
				best = &p;
			}
			break;
		}
	}
	if(!best) {
		return false;
	}
	moveFixedMarker(idx, best->x);
	return true;
}

void AcqMarkerController::computePeaks()
{
	m_data = samples();
	m_peakInfo.clear();
	m_sortedPeakInfo.clear();
	const int n = m_data.size();
	if(n < 3) {
		return;
	}

	double lo = -INFINITY, hi = INFINITY;
	if(m_plot) {
		const QwtInterval iv = m_plot->axisInterval(m_xAxis);
		lo = iv.minValue();
		hi = iv.maxValue();
	}

	for(int i = 1; i < n - 1; ++i) {
		const QPointF &p = m_data[i];
		if(p.x() < lo || p.x() > hi) {
			continue;
		}
		if(m_data[i - 1].y() < p.y() && p.y() >= m_data[i + 1].y()) {
			m_peakInfo.append({p.x(), p.y(), i});
		}
	}

	m_sortedPeakInfo = m_peakInfo;
	std::sort(m_sortedPeakInfo.begin(), m_sortedPeakInfo.end(),
		  [](const PeakInfo &a, const PeakInfo &b) { return a.y > b.y; });
}

int AcqMarkerController::findPeakNearIdx(int idx, int range)
{
	const int n = m_data.size();
	if(n == 0) {
		return 0;
	}
	const int start = std::clamp(idx - range, 0, n - 1);
	const int stop = std::clamp(idx + range, 0, n - 1);
	int maxIdx = start;
	for(int i = start; i <= stop; ++i) {
		if(m_data[i].y() > m_data[maxIdx].y()) {
			maxIdx = i;
		}
	}
	return maxIdx;
}

void AcqMarkerController::computeSingleToneMarkers()
{
	m_markerInfo.clear();
	if(m_sortedPeakInfo.isEmpty() || m_markers.size() < 2 || m_data.isEmpty()) {
		return;
	}
	const PeakInfo fund = m_sortedPeakInfo[0];
	m_markerInfo.append({QStringLiteral("DC"), m_markers[0], {m_data[0].x(), m_data[0].y(), 0}});
	m_markerInfo.append({QStringLiteral("Fund"), m_markers[1], fund});

	// Wider search on longer spectra, where a harmonic smears over more bins.
	const int range = static_cast<int>(std::log2(std::max(2, static_cast<int>(m_data.size()))));
	for(int h = 2; h < m_markers.size(); ++h) {
		const int want = fund.idx * h;
		if(want < 0 || want >= m_data.size()) {
			break;
		}
		const int idx = findPeakNearIdx(want, range);
		m_markerInfo.append({QString::number(h) + QStringLiteral("H"),
				     m_markers[h],
				     {m_data[idx].x(), m_data[idx].y(), idx}});
	}
}

void AcqMarkerController::computeImageMarkers() { m_markerInfo.clear(); }

void AcqMarkerController::initFixedMarker()
{
	cacheMarkerInfo();
	PlotWidget *plot = (m_curve && m_curve->plotOwner()) ? m_curve->plotOwner()->plot() : nullptr;
	PlotAxis *xAxis = (m_curve && m_curve->xAxis()) ? m_curve->xAxis()->plotAxis() : nullptr;

	for(int i = 0; i < m_markers.size(); ++i) {
		const double x = popCacheMarkerInfo();
		m_markerInfo.append({QStringLiteral("F") + QString::number(i), m_markers[i], {x, 0.0, 0}});
		if(!plot || !xAxis) {
			continue;
		}
		auto *handle = new PlotAxisHandle(plot, xAxis);
		connect(handle, &PlotAxisHandle::scalePosChanged, this, [this, i](double v) {
			if(i < m_markerInfo.size()) {
				m_markerInfo[i].peak.x = v;
				computeMarkers();
			}
		});
		m_fixedHandles.append(handle);
		handle->setPositionSilent(x);
	}
	setFixedHandleVisible(m_handlesVisible);
}

double AcqMarkerController::valueAt(double x)
{
	if(!m_curve || !m_curve->plotChannel()) {
		return 0.0;
	}
	return m_curve->plotChannel()->getValueAt(x);
}

#include "moc_acqmarkercontroller.cpp"
