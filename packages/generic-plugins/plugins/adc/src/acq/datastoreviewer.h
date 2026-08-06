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

#include <QCheckBox>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <QWidget>

#include <core/acq_engine/DataKey.h>

namespace scopy {
namespace acq {
class DataStore;
}

namespace adc {

// Live view of every stream in a DataStore, split side by side: the key list on
// the left (key, sample type, chunk size, history use), the selected stream's
// newest samples on the right.
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
	explicit DataStoreViewer(scopy::acq::DataStore *store, QWidget *parent = nullptr);

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

	void buildUi();

	// Rebuild the samples pane from the newest samples of the selected key. Reuses
	// rows rather than recreating them: this runs every cycle, and recreating makes
	// the list flicker and lose the reader's place.
	void fillSamples();

	// The key of the selected row, or an invalid key when nothing is selected.
	scopy::acq::DataKey selectedKey() const;

	// Puts the selected stream on the clipboard, one sample per line. Always the whole
	// stream the store holds, not the window the pane happens to list.
	void copySamples();

	QTreeWidget *m_tree{nullptr};
	QTreeWidget *m_samples{nullptr};
	QLabel      *m_samplesTitle{nullptr};
	// Disabled with no selection or an unwritten key: there is nothing to copy, and a
	// button that copies an empty clipboard is worse than one that is visibly unavailable.
	QPushButton *m_copyBtn{nullptr};
	// How many newest samples the pane lists. Small by default — see the class
	// comment on why this is the expensive knob.
	QSpinBox    *m_sampleCount{nullptr};
	QCheckBox   *m_hex{nullptr};

	// Not owned. A QPointer so a torn-down store can't be read through.
	QPointer<scopy::acq::DataStore> m_store;
};

} // namespace adc
} // namespace scopy

#endif // DATASTOREVIEWER_H
