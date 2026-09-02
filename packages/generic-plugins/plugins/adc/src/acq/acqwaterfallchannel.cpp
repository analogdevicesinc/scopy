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

#include "acqwaterfallchannel.h"

#include "acqaxis.h"
#include "acqchannelregistry.h"
#include "acqplot.h"

#include <core/acq_engine/DataStore.h>

// For QPointer<PlotAxis> in AcqChannel: destroying one needs the complete type, and
// acqchannel.h only forward-declares it.
#include <gui/plotaxis.h>
#include <gui/waterfallplotwidget.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuspinbox.h>

#include <QSpinBox>
#include <QVBoxLayout>

#include <optional>
#include <variant>

using namespace scopy;
using namespace scopy::adc;

namespace {

// A chunk's identity: the address of its sample storage. Stable while the chunk is
// retained, which is all that is needed to recognise chunks already drawn.
//
// constData(), never data(): the latter detaches a shared QVector and deep-copies
// the very chunk we are trying to avoid copying.
const void *chunkId(const scopy::acq::SampleVariant &v)
{
	return std::visit([](const auto &vec) -> const void * { return static_cast<const void *>(vec.constData()); },
			  v);
}

} // namespace

// Any stream. A spectrogram of a time-domain stream is noise rather than an error, so
// nothing here could usefully prevent it — only the producer naming Waterfall for a
// stream that is one can, which is exactly why the kind is declared and never inferred.
REGISTER_ACQ_CHANNEL_KIND(AcqWaterfallChannel, scopy::acq::ReprKind::Waterfall)

AcqWaterfallChannel::AcqWaterfallChannel(const Args &args)
	: AcqChannel(args)
	, m_kMinRows(10)
	, m_kMaxRows(2000)
{
}

AcqWaterfallChannel::~AcqWaterfallChannel() { detach(); }

void AcqWaterfallChannel::attachTo(AcqPlot *plot)
{
	// On a Waterfall plot the widget already *is* a WaterfallPlotWidget, built by that
	// plot kind's factory, and there is nothing to add to it: the spectrogram is the
	// plot's own item, so there is no per-channel visual.
	//
	// The cast fails when this kind was put on a plot that is not a waterfall. Nothing
	// stops that, and nothing needs to — every method here begins with this same null
	// check, so the channel exists in the rail and draws nothing.
	if(m_wf.isNull()) {
		m_wf = qobject_cast<WaterfallPlotWidget *>(plot->plot());
	}
	if(m_wf.isNull()) {
		return;
	}

	m_wf->setNumRows(m_rows);
	m_wf->setIntensityRange(m_minDb, m_maxDb);
	// Only gates the internal ChannelData path, which is unused here — but leaving it
	// false would make a later setChannel() silently draw nothing.
	m_wf->setWaterfallEnabled(true);
}

void AcqWaterfallChannel::detachFrom()
{
	// Nothing to unhook. The plot is a child of the dock the manager built, so it is
	// destroyed with it, and the QPointer is how we find out. Deleting it here would
	// be wrong for the same reason it is wrong in the item kinds: destruction order
	// between the plot and this channel is unspecified.
	m_wf = nullptr;
}

DepthNeed AcqWaterfallChannel::depthNeeded(int plotSize) const
{
	// plotSize is irrelevant: history is the Y axis here, so one retained chunk is one
	// pixel row regardless of how wide a chunk is — which is exactly why this is a
	// chunk claim. Claiming exactly m_rows and not one more matters: every extra
	// retained chunk is copied by snapshot() under the store mutex and then discarded
	// by setMaxRows().
	Q_UNUSED(plotSize)
	return DepthNeed::chunks(static_cast<std::size_t>(qMax(1, m_rows)));
}

void AcqWaterfallChannel::readData(scopy::acq::DataStore *store, int plotSize)
{
	Q_UNUSED(plotSize)
	if(m_wf.isNull()) {
		return;
	}

	// The cheapest possible "is there anything new" probe: one chunk, so the store
	// mutex is held for a single refcount bump rather than a whole-history copy. On a
	// cycle where this key was not written — common, since the heartbeat is
	// independent of any one stream — the answer costs nothing and we do nothing at
	// all: no snapshot, no conversion, no repaint.
	const std::optional<scopy::acq::SampleVariant> newest = store->latest(key());
	if(!newest || chunkId(*newest) == m_lastChunk) {
		return;
	}

	// Something is new. The scan below compares pointers only and never touches
	// sample data, so it costs the refcount bumps the old code already paid — without
	// the per-row allocate-convert-copy that followed them.
	const scopy::acq::SampleBuffer buf = store->snapshot(key());
	const int depth = static_cast<int>(buf.depth());
	if(depth == 0) {
		return;
	}

	// Chunks newer than the last one drawn. No match anywhere means the whole retained
	// history is new — first cycle, post-reset, or we fell behind by more than the
	// depth. See the class comment.
	int fresh = depth;
	for(int i = 0; i < depth; ++i) {
		if(chunkId(buf.sample(static_cast<std::size_t>(i))) == m_lastChunk) {
			fresh = i;
			break;
		}
	}

	// Oldest of the new chunks first, so the ring ends up newest-last. Deferred so the
	// several-new-rows case costs one raster invalidation rather than one per row.
	bool appended = false;
	for(int i = fresh - 1; i >= 0; --i) {
		const scopy::acq::FloatView v =
			scopy::acq::toFloatView(buf.sample(static_cast<std::size_t>(i)), m_scratch);
		if(v.size == 0) {
			// An annotation stream converts to empty, as does a chunk holding nothing.
			// Either way there is no row to append.
			continue;
		}
		m_wf->appendRowDeferred(v.data, static_cast<size_t>(v.size));
		appended = true;
	}
	if(!appended) {
		return;
	}
	// Only after a successful append: leaving it unset on an empty stream keeps the
	// next cycle re-examining rather than silently accepting a history it never drew.
	m_lastChunk = chunkId(buf.sample(0));
	m_wf->endAppend();

	updateFrequencyRange(store);
}

void AcqWaterfallChannel::updateFrequencyRange(scopy::acq::DataStore *store)
{
	// The bin frequencies, from whichever stream the X axis names — the caller's choice
	// if it made one, otherwise the producer's declared companion. Only pushed on a
	// change: it rescales the axis and drops the raster cache.
	//
	// Skipped for the two index-based modes rather than for an empty key: xKey() is never
	// empty now — the sample index is a real stream — and a spectrogram scaled to
	// 0..plotSize "Hz" would be a worse lie than leaving the widget's own range alone.
	if(xAxis() && xAxis()->source().isIndexBased()) {
		return;
	}

	// latest(), not windowFloat(): a bin-frequency chunk is one full spectrum, so its
	// own first and last values *are* the range. The window version concatenated and
	// sliced a whole plotSize-long copy every cycle to read two floats out of it.
	const std::optional<scopy::acq::SampleVariant> v = store->latest(xKey());
	if(!v) {
		return;
	}
	const scopy::acq::FloatView freq = scopy::acq::toFloatView(*v, m_scratch);
	if(freq.size < 2) {
		return;
	}
	const double start = freq.data[0];
	const double stop = freq.data[freq.size - 1];
	if(start == m_freqStart && stop == m_freqStop) {
		return;
	}
	m_freqStart = start;
	m_freqStop = stop;
	m_wf->setFrequencyRange(start, stop);
}

void AcqWaterfallChannel::reset()
{
	if(!m_wf.isNull()) {
		m_wf->clearData();
	}
	// The ring is empty now, so the next cycle must re-prime from the full snapshot
	// rather than appending one row onto a history it thinks is still there.
	m_lastChunk = nullptr;
	// Deliberately not resetting m_freqStart/m_freqStop: the FFT configuration does
	// not change across a stop/start, so re-pushing the same range would only cost a
	// rescale on the first cycle of every run.
}

void AcqWaterfallChannel::setRows(int rows)
{
	rows = qBound(m_kMinRows, rows, m_kMaxRows);
	if(rows == m_rows) {
		return;
	}
	m_rows = rows;
	if(!m_wf.isNull()) {
		m_wf->setNumRows(rows);
	}
	// The one kind whose depth requirement moves on its own. Without this the store
	// would keep retaining the old count and the visible history would not follow the
	// spinbox — the plot would just crop or starve.
	requestReclaim();
}

QWidget *AcqWaterfallChannel::createKindSettings(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);

	QWidget *historyWid = new QWidget(w);
	QHBoxLayout *historyLay = new QHBoxLayout(historyWid);
	historyLay->setContentsMargins(0, 0, 0, 0);
	QSpinBox *rowsSpin = new QSpinBox(historyWid);
	historyLay->addWidget(new QLabel(tr("History rows"), historyWid));
	historyLay->addSpacing(10);
	historyLay->addWidget(rowsSpin);
	rowsSpin->setRange(m_kMinRows, m_kMaxRows);
	// Seeded, so the control opens showing the row count actually in effect rather
	// than the range minimum.
	rowsSpin->setValue(m_rows);
	connect(rowsSpin, &QSpinBox::valueChanged, this, [this](int v) { setRows(v); });

	// The Z range, not an axis range — the colour map spans it — so this is a pair of
	// spinboxes rather than a MenuPlotAxisRangeControl, which needs a PlotAxis.
	gui::MenuSpinbox *minSpin =
		new gui::MenuSpinbox(tr("Intensity min"), m_minDb, "dB", -300, 300, true, false, false, w);
	minSpin->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	gui::MenuSpinbox *maxSpin =
		new gui::MenuSpinbox(tr("Intensity max"), m_maxDb, "dB", -300, 300, true, false, false, w);
	maxSpin->setIncrementMode(gui::MenuSpinbox::IS_FIXED);

	auto pushIntensity = [this]() {
		if(!m_wf.isNull() && m_minDb < m_maxDb) {
			m_wf->setIntensityRange(m_minDb, m_maxDb);
		}
	};
	// Inverted or equal bounds are ignored rather than clamped: clamping would fight
	// the reader typing a new range one field at a time.
	connect(minSpin, &gui::MenuSpinbox::valueChanged, this, [this, pushIntensity](double v) {
		m_minDb = v;
		pushIntensity();
	});
	connect(maxSpin, &gui::MenuSpinbox::valueChanged, this, [this, pushIntensity](double v) {
		m_maxDb = v;
		pushIntensity();
	});

	MenuOnOffSwitch *aaBtn = new MenuOnOffSwitch(tr("SMOOTHING"), w, false);
	connect(aaBtn->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		if(!m_wf.isNull()) {
			m_wf->setAntialiasing(on);
		}
	});

	// historyWid, not rowsSpin: re-parenting the spinbox out of its row layout is what
	// used to orphan the "History rows" label.
	lay->addWidget(historyWid);
	lay->addWidget(minSpin);
	lay->addWidget(maxSpin);
	lay->addWidget(aaBtn);
	return w;
}

void AcqWaterfallChannel::onEnabledChanged(bool en)
{
	if(m_wf.isNull()) {
		return;
	}
	// The whole plot widget rather than one item, so disabling blanks the dock. The
	// channel already stops reading, so nothing accumulates while hidden — the
	// history restarts from the disable point, which is the honest thing to show.
	m_wf->setVisible(en);
}

#include "moc_acqwaterfallchannel.cpp"
