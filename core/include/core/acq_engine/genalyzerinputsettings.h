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

#pragma once

#include "scopy-core_export.h"

#include <QWidget>

class QComboBox;
class QLabel;

namespace scopy {
namespace acq {

class AcquisitionEngine;
class DataStore;
class GenalyzerFFTProcessor;

// GUI panel for which streams a GenalyzerFFTProcessor transforms, and with that
// for the transform itself: one picked stream is a real FFT, two are a complex
// I/Q pair. The mode selector comes first and the pickers follow it, because the
// mode is the reader's intent and the key count is only how the block stores it.
//
// This is what replaces finding an I/Q pair by channel-name convention. The
// combos list whatever the DataStore declares or holds — see populateKeyCombo —
// so a source that names its channels anything at all is selectable, and a block
// with nothing picked yet shows that state instead of quietly never running.
//
// Retargeting is only offered while it is safe: the engine reads watchedKeys()
// on the worker thread, and skips that read only for a disabled block, so the
// pickers grey out while the engine runs with this block enabled and the hint
// says what to do about it.
class SCOPY_CORE_EXPORT GenalyzerInputSettings : public QWidget
{
	Q_OBJECT
public:
	// `store` and `engine` may each be null — the combos then list nothing and
	// the pickers stay editable, which is all a block with no engine could offer.
	explicit GenalyzerInputSettings(GenalyzerFFTProcessor *proc, DataStore *store, AcquisitionEngine *engine,
					QWidget *parent = nullptr);

private Q_SLOTS:
	// Pull mode and keys back from the block. Also the slot for its
	// inputsChanged signal, so a retarget from elsewhere shows up here.
	void syncFromProcessor();

	// Re-fill the combos from the current key set, preserving each selection.
	void refreshKeys();

private:
	void setupUI();

	// Push the widgets' intent into the block. A complex mode missing either
	// half pushes an empty list rather than a one-key real transform: dropping
	// to a mode the reader did not pick would be a worse answer than idling.
	void pushInputs();

	// Row visibility, labels and the editable/greyed state, from mode + run state.
	void updateState();

	GenalyzerFFTProcessor *m_proc{nullptr};
	DataStore *m_store{nullptr};
	AcquisitionEngine *m_engine{nullptr};

	QComboBox *m_modeCombo{nullptr};
	QComboBox *m_firstKey{nullptr};
	QComboBox *m_secondKey{nullptr};
	QWidget *m_firstRow{nullptr};
	QWidget *m_secondRow{nullptr};
	QLabel *m_firstLabel{nullptr};
	QLabel *m_hint{nullptr};

	// Set while the widgets are being populated, so a half-built state does not
	// write itself back through the same signals a reader's edit uses.
	bool m_building{false};
};

} // namespace acq
} // namespace scopy
