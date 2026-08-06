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

#include "acqwaterfallrepr.h"

#include "acqplotrow.h"

#include <core/acq_engine/DataStore.h>

#include <gui/waterfallplotwidget.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuspinbox.h>

#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::adc;

namespace {

// Enough history to see a slow drift, few enough that the two per-cycle copies of
// the whole snapshot (plan T10) stay off the frame budget.
constexpr int kMinRows = 8;
constexpr int kMaxRows = 512;

} // namespace

WaterfallRepr::WaterfallRepr() = default;

WaterfallRepr::~WaterfallRepr() { detach(); }

PlotWidget *WaterfallRepr::createOwnPlot(QWidget *parent)
{
	if(m_wf.isNull()) {
		m_wf = new WaterfallPlotWidget(parent);
	}
	return m_wf.data();
}

void WaterfallRepr::attach(AcqPlotRow *row, const QString &name, const QColor &color)
{
	// The row's plot is the widget createOwnPlot() just returned — the manager put it
	// in an exclusive row and handed the row back. Nothing to add to it: the
	// spectrogram is the plot's own item, so there is no per-channel visual here.
	Q_UNUSED(name)
	Q_UNUSED(color)
	if(!row || m_wf.isNull()) {
		return;
	}

	m_wf->setNumRows(m_rows);
	m_wf->setIntensityRange(m_minDb, m_maxDb);
	// Only gates the internal ChannelData path, which is unused here — but leaving it
	// false would make a later setChannel() silently draw nothing.
	m_wf->setWaterfallEnabled(true);
	m_wf->setVisible(m_enabled);
}

void WaterfallRepr::detach()
{
	// Nothing to unhook. The plot is a child of the manager's splitter, so it is
	// destroyed with it, and the QPointer is how we find out. Deleting it here would
	// be wrong for the same reason it is wrong in the item reprs: destruction order
	// between the plot and the channel owning this repr is unspecified.
	m_wf = nullptr;
}

std::size_t WaterfallRepr::claimDepth(int plotSize, std::size_t bufferSize) const
{
	// plotSize and bufferSize are both irrelevant: history is the Y axis here, so one
	// retained chunk is one pixel row regardless of how wide a chunk is. Claiming
	// exactly m_rows and not one more matters — every extra retained chunk is copied
	// by snapshot() under the store mutex and then discarded by setMaxRows().
	Q_UNUSED(plotSize)
	Q_UNUSED(bufferSize)
	return static_cast<std::size_t>(qMax(1, m_rows));
}

void WaterfallRepr::pull(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, int plotSize)
{
	if(!store || m_wf.isNull()) {
		return;
	}

	// One chunk per row, oldest last — the order snapshot() already gives us and the
	// order setHistorySnapshot() documents, so no reversal here.
	const scopy::acq::SampleBuffer buf = store->snapshot(key);
	if(buf.empty()) {
		return;
	}

	std::vector<QVector<float>> snap;
	snap.reserve(buf.depth());
	for(std::size_t i = 0; i < buf.depth(); ++i) {
		// toFloat, not toFloatView: each row has to survive the call, so an owned
		// vector is the only correct choice — a view would alias one shared scratch
		// buffer and every row would end up holding the last chunk.
		snap.push_back(scopy::acq::toFloat(buf.sample(i)));
	}
	if(snap.empty() || snap.front().isEmpty()) {
		// An annotation stream converts to empty here. makeRepr refuses those, so this
		// is the key-exists-but-holds-nothing case.
		return;
	}

	// The bin frequencies, read the same way CurveRepr reads its X key. Only pushed
	// on a change: it rescales the axis and drops the raster cache.
	if(!m_xKey.key.isEmpty()) {
		const QVector<float> freq = store->windowFloat(m_xKey, plotSize);
		if(freq.size() >= 2) {
			const double start = freq.first();
			const double stop = freq.last();
			if(start != m_freqStart || stop != m_freqStop) {
				m_freqStart = start;
				m_freqStop = stop;
				m_wf->setFrequencyRange(start, stop);
			}
		}
	}

	m_wf->setHistorySnapshot(std::move(snap));
}

void WaterfallRepr::reset()
{
	if(!m_wf.isNull()) {
		m_wf->clearData();
	}
	// Deliberately not resetting m_freqStart/m_freqStop: the FFT configuration does
	// not change across a stop/start, so re-pushing the same range would only cost a
	// rescale on the first cycle of every run.
}

void WaterfallRepr::setRows(int rows)
{
	rows = qBound(kMinRows, rows, kMaxRows);
	if(rows == m_rows) {
		return;
	}
	m_rows = rows;
	if(!m_wf.isNull()) {
		m_wf->setNumRows(rows);
	}
	// The one repr whose depth requirement moves on its own. Without this the store
	// would keep retaining the old count and the visible history would not follow the
	// spinbox — the plot would just crop or starve.
	requestReclaim();
}

QWidget *WaterfallRepr::createSettingsWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);

	gui::MenuSpinbox *rowsSpin = new gui::MenuSpinbox(QObject::tr("History rows"), m_rows, "rows", kMinRows,
							  kMaxRows, true, false, false, w);
	rowsSpin->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	QObject::connect(rowsSpin, &gui::MenuSpinbox::valueChanged, rowsSpin,
			 [this](double v) { setRows(static_cast<int>(v)); });

	// The Z range, not an axis range — the colour map spans it — so this is a pair of
	// spinboxes rather than a MenuPlotAxisRangeControl, which needs a PlotAxis.
	gui::MenuSpinbox *minSpin =
		new gui::MenuSpinbox(QObject::tr("Intensity min"), m_minDb, "dB", -300, 300, true, false, false, w);
	minSpin->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	gui::MenuSpinbox *maxSpin =
		new gui::MenuSpinbox(QObject::tr("Intensity max"), m_maxDb, "dB", -300, 300, true, false, false, w);
	maxSpin->setIncrementMode(gui::MenuSpinbox::IS_FIXED);

	auto pushIntensity = [this]() {
		if(!m_wf.isNull() && m_minDb < m_maxDb) {
			m_wf->setIntensityRange(m_minDb, m_maxDb);
		}
	};
	// Inverted or equal bounds are ignored rather than clamped: clamping would fight
	// the reader typing a new range one field at a time.
	QObject::connect(minSpin, &gui::MenuSpinbox::valueChanged, minSpin, [this, pushIntensity](double v) {
		m_minDb = v;
		pushIntensity();
	});
	QObject::connect(maxSpin, &gui::MenuSpinbox::valueChanged, maxSpin, [this, pushIntensity](double v) {
		m_maxDb = v;
		pushIntensity();
	});

	MenuOnOffSwitch *aaBtn = new MenuOnOffSwitch(QObject::tr("SMOOTHING"), w, false);
	QObject::connect(aaBtn->onOffswitch(), &QAbstractButton::toggled, aaBtn, [this](bool on) {
		if(!m_wf.isNull()) {
			m_wf->setAntialiasing(on);
		}
	});

	lay->addWidget(rowsSpin);
	lay->addWidget(minSpin);
	lay->addWidget(maxSpin);
	lay->addWidget(aaBtn);
	return w;
}

void WaterfallRepr::setEnabled(bool en)
{
	m_enabled = en;
	if(m_wf.isNull()) {
		return;
	}
	// A whole splitter row rather than one item, so disabling hides the plot. The
	// channel already stops pulling, so nothing accumulates while hidden — the
	// history restarts from the disable point, which is the honest thing to show.
	m_wf->setVisible(en);
}
