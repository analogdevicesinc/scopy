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

#ifndef DATASTOREVIEWER_H
#define DATASTOREVIEWER_H

#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <QWidget>
#include <infoiconwidget.h>

#include <core/acq_engine/DataKey.h>

namespace scopy {
class MetricPrefixFormatter;

namespace acq {
class AcquisitionEngine;
class DataStore;
}

namespace adc {

// Live view of every stream in a DataStore, split side by side: the key list on
// the left (key, sample type, chunk size, history use), the selected stream's
// descriptor and newest samples on the right.
//
// The descriptor is what makes a misdrawn plot diagnosable here: without it, a
// trace with no unit or the wrong X source is equally explained by the producing
// block declaring it wrong and by the view reading it wrong. StreamInfo comes
// from the engine, which asks the producing block; AnnotationStreamInfo comes
// from the store.
//
// Selection rather than per-row expansion is what makes a per-cycle refresh
// affordable: exactly one stream's samples are ever listed, so the work the panel
// does at cycle rate does not scale with the number of keys. The panel refreshes
// at cycle rate, so anything it does for every key is in the GUI thread's hot path.
//
// Main-thread only. Nothing here connects to the engine; the owner calls
// refresh() from its own (queued) cycle handler.
class DataStoreViewer : public QWidget
{
	Q_OBJECT
public:
	// `engine` may be null — the panel then simply has no descriptor to report,
	// since a block is the only thing that knows one.
	DataStoreViewer(scopy::acq::DataStore *store, scopy::acq::AcquisitionEngine *engine,
			QWidget *parent = nullptr);

public Q_SLOTS:
	// Re-read the store. Adds and removes rows to match the current key set,
	// retexts the rest, and refills the samples pane for the selected key.
	void refresh();

private:
	// Columns of the key list.
	enum Column
	{
		ColKey = 0,
		ColType,
		ColSamples,
		ColHistory,
		ColCount,
	};

	// Columns of the samples pane.
	enum SampleColumn
	{
		ColIndex = 0,
		ColValue,
		SampleColCount,
	};

	// Columns of the descriptor readout.
	enum InfoColumn
	{
		ColField = 0,
		ColFieldValue,
		InfoColCount,
	};

	void buildUi();

	// Rebuild the descriptor readout for the selected key. Asks the engine every
	// time rather than caching or listening: a block's declaration changes with its
	// channel set and its settings widget, and the engine answers from the block
	// precisely so no copy needs re-syncing at those points.
	void fillInfo();

	// Rebuild the samples pane from the newest samples of the selected key, and the
	// descriptor readout with it. Rows are reused rather than recreated: this runs
	// every cycle, and recreating makes the list flicker and lose the reader's place.
	void fillSamples();

	// The key of the selected row, or an invalid key when nothing is selected.
	scopy::acq::DataKey selectedKey() const;

	// Puts the selected stream on the clipboard, one sample per line. Always the whole
	// stream the store holds, not the window the pane happens to list.
	void copySamples();

	QTreeWidget *m_tree{nullptr};
	QTreeWidget *m_samples{nullptr};
	// Field/value readout of the selected stream's descriptor. Sized to its content
	// rather than sharing the pane's height: the panel spans the short bottom rail,
	// and a readout of six rows that takes half of it leaves no samples visible.
	QTreeWidget *m_info{nullptr};
	QLabel      *m_samplesTitle{nullptr};
	// Disabled with no selection or an unwritten key: there is nothing to copy, and a
	// button that copies an empty clipboard is worse than one that is visibly unavailable.
	QPushButton *m_copyBtn{nullptr};
	// How many newest samples the pane lists. Small by default — see the class
	// comment on why this is the expensive knob.
	QSpinBox    *m_sampleCount{nullptr};

	InfoIconWidget *m_sampleCountInfo{nullptr};

	// Owned (parented here). Held rather than constructed per fill: fillInfo() runs
	// at cycle rate and this is a QObject.
	scopy::MetricPrefixFormatter *m_fmt{nullptr};

	// Not owned. QPointers so a torn-down store or engine can't be read through.
	QPointer<scopy::acq::DataStore>         m_store;
	QPointer<scopy::acq::AcquisitionEngine> m_engine;
};

} // namespace adc
} // namespace scopy

#endif // DATASTOREVIEWER_H
