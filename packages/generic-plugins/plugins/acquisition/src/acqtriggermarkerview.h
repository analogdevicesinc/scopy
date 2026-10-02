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

#ifndef ACQTRIGGERMARKERVIEW_H
#define ACQTRIGGERMARKERVIEW_H

#include <core/acq_engine/datakey.h>

#include <QList>
#include <QObject>
#include <QPointer>

namespace scopy {
class PlotAxis;

namespace acq {
class TriggerMarker;
}

namespace adc {

class AcqPlot;

// Which X axis the trigger's draggable bar rides, and what window its sample map spans.
//
// Split out of AcqPlotManager because it is a self-contained question: given the plots as
// they are now and the key the reader picked, find the one X axis to attach to. The manager
// owns the plots and calls retarget() whenever that answer can change; nothing here holds a
// plot list of its own.
//
// Everything is borrowed. The marker belongs to whoever built it from the processor, and the
// axis is shared with every channel drawing that source and outlives the marker. The plot and
// axis are kept only so the window can be measured in the right plot's width and the right
// axis's units — the attach decision is the marker's own.
class AcqTriggerMarkerView : public QObject
{
	Q_OBJECT
public:
	explicit AcqTriggerMarkerView(QObject *parent = nullptr);

	// The bar itself. Null detaches this view from it without touching it.
	void setMarker(scopy::acq::TriggerMarker *marker);

	// Which X source the bar rides, as the reader picked it. May name a key no channel
	// draws yet: the marker stays detached until one does, and starts working the moment
	// it happens.
	void setAxisKey(const scopy::acq::DataKey &key);
	const scopy::acq::DataKey &axisKey() const { return m_axisKey; }

	// Put the marker on whatever the key resolves to against `plots` now: the first plot
	// with an X axis for it, or detached when no plot has one. Idempotent and the single
	// entry point — the reader's pick, and a plot or channel coming or going, all change
	// the same answer.
	//
	// Call at the *end* of an add or remove, which is what makes it safe: by then the dying
	// plot or channel is already off the caller's lists, so there is nothing to exclude.
	//
	// `defaultWidth` is the window to report before anything resolves.
	void retarget(const QList<AcqPlot *> &plots, int defaultWidth);

	// Re-measure the window without re-resolving the axis, for a width change.
	void updateWindow(int defaultWidth);

Q_SIGNALS:
	// The window the marker's sample↔axis map now spans, in samples: the marker plot's own
	// width. The processor's fire window and the target spinbox's maximum are the same
	// number, so they follow this rather than each deriving it. Emitted on a retarget and on
	// a width change.
	void windowChanged(int samples);

private:
	// The X PlotAxis a channel on `plot` draws against for `key`, or null when no channel
	// there reads X from it. Walks plot->channels() rather than calling
	// AcqPlot::axisForSource, which would create one on miss — an axis nobody draws on does
	// not zoom with the plot, which is the bug this path exists to avoid.
	static scopy::PlotAxis *findXAxisFor(AcqPlot *plot, const scopy::acq::DataKey &key);

	QPointer<scopy::acq::TriggerMarker> m_marker;
	QPointer<AcqPlot> m_plot;
	QPointer<scopy::PlotAxis> m_axis;
	scopy::acq::DataKey m_axisKey;
};

} // namespace adc
} // namespace scopy

#endif // ACQTRIGGERMARKERVIEW_H
