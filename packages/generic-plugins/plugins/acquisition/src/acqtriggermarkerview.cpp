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

#include "acqtriggermarkerview.h"

#include "acqaxis.h"
#include "acqchannel.h"
#include "acqplot.h"

#include <core/acq_engine/triggermarker.h>

#include <gui/plotaxis.h>
#include <gui/plotwidget.h>

#include <QLoggingCategory>

#include <algorithm>

Q_LOGGING_CATEGORY(CAT_ACQ_TRIGGERMARKERVIEW, "AcqTriggerMarkerView")

using namespace scopy;
using namespace scopy::adc;

AcqTriggerMarkerView::AcqTriggerMarkerView(QObject *parent)
	: QObject(parent)
{}

void AcqTriggerMarkerView::setMarker(scopy::acq::TriggerMarker *marker) { m_marker = marker; }

void AcqTriggerMarkerView::setAxisKey(const scopy::acq::DataKey &key) { m_axisKey = key; }

PlotAxis *AcqTriggerMarkerView::findXAxisFor(AcqPlot *plot, const scopy::acq::DataKey &key)
{
	if(!plot || key.key.isEmpty()) {
		return nullptr;
	}

	// The channels, not plot->axisForSource(): that one creates on miss, and only a channel
	// can say an axis is really being drawn against.
	PlotAxis *fallback = nullptr;
	const QList<AcqChannel *> chans = plot->channels();
	for(AcqChannel *ch : chans) {
		if(!ch || !ch->xAxis()) {
			continue;
		}
		AcqAxis *ax = ch->xAxis();
		// isHorizontal() even on the X side: a channel's "X" axis is whichever PlotAxis it
		// was given, and nothing stops a caller handing it a vertical one.
		if(ax->source().key != key || !ax->isHorizontal() || !ax->plotAxis()) {
			continue;
		}
		// Sample index and time share the ramp key, so a key match cannot tell them apart.
		// Prefer the index axis: its interval really is 0..plotSize-1, which makes the
		// position-to-sample map exact rather than merely monotonic.
		if(ax->isSampleIndex()) {
			return ax->plotAxis();
		}
		if(!fallback) {
			fallback = ax->plotAxis();
		}
	}
	return fallback;
}

void AcqTriggerMarkerView::retarget(const QList<AcqPlot *> &plots, int defaultWidth)
{
	if(m_marker.isNull()) {
		return;
	}

	// First plot that has it: one trigger, and no basis for preferring a later plot.
	AcqPlot *target = nullptr;
	PlotAxis *axis = nullptr;
	for(AcqPlot *p : plots) {
		if(PlotAxis *ax = findXAxisFor(p, m_axisKey)) {
			target = p;
			axis = ax;
			break;
		}
	}

	if(!axis) {
		// Detached rather than parked on some other axis. The reader's pick is kept, so this
		// resolves itself once a channel draws X against that key.
		const bool had = m_marker->isAttached();
		m_marker->detach();
		m_plot = nullptr;
		m_axis = nullptr;
		if(had) {
			// Only on the transition: this runs on every plot and channel change.
			qWarning(CAT_ACQ_TRIGGERMARKERVIEW) << "trigger axis" << m_axisKey.toString()
							    << "is not drawn as an X axis on any plot — marker hidden";
		}
		return;
	}

	PlotWidget *w = target->plot();
	if(!w) {
		return;
	}

	m_plot = target;
	m_axis = axis;
	// The window before the attach, so the marker's first sync places the bar against the
	// right one.
	updateWindow(defaultWidth);
	m_marker->attach(w, axis);
}

void AcqTriggerMarkerView::updateWindow(int defaultWidth)
{
	if(m_marker.isNull()) {
		return;
	}

	// The *marker plot's* plotSize: plots can be different widths. The default is only the
	// answer before the first resolve.
	const int n = m_plot.isNull() ? defaultWidth : m_plot->plotSize();
	const int last = std::max(0, n - 1);

	// The axis units that window spans: slot numbers, or slots over the channel's rate for a
	// time axis. Any other X is left as slot numbers — monotonic, which is all the marker
	// needs, and the most a proportional map can claim without scanning the stream.
	double x1 = static_cast<double>(last);
	if(!m_axis.isNull() && !m_plot.isNull()) {
		const QList<AcqChannel *> chans = m_plot->channels();
		for(AcqChannel *ch : chans) {
			AcqAxis *ax = ch ? ch->xAxis() : nullptr;
			if(!ax || ax->plotAxis() != m_axis.data()) {
				continue;
			}
			if(ax->isTime()) {
				const double rate = ch->sampleRate() > 0.0 ? ch->sampleRate() : 1.0;
				x1 = static_cast<double>(last) / rate;
			}
			break;
		}
	}

	m_marker->setWindow(n, 0.0, x1);
	// The same `n` for the processor's fire window and the target spinbox's maximum: the fire
	// index is in these units and the marker maps it back through them, so the three must be
	// one number. Stated rather than applied — this class holds no processor.
	Q_EMIT windowChanged(n);
}

#include "moc_acqtriggermarkerview.cpp"
