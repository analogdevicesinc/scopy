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

#include "datakey.h"

class QComboBox;

namespace scopy {
namespace acq {

class DataStore;
class AcquisitionEngine;

// Fills a combo box with the DataStore streams a reader can point something at: declared ∪
// written, sorted, itemData = the key string, selection preserved across a rebuild. `store` and
// `engine` may each be null; the invariants are commented at the implementation.
//
// Takes QComboBox* rather than MenuCombo* so both the styled menu pickers and plain-Qt widgets
// share it; a MenuCombo caller passes combo->combo().
SCOPY_CORE_EXPORT void populateKeyCombo(QComboBox *box, const DataStore *store, const AcquisitionEngine *engine,
					bool withSampleIndex);

// The key `box` currently names. Empty when nothing is selected. The sample-index entry needs no
// special case: its payload *is* the engine ramp's key.
SCOPY_CORE_EXPORT DataKey keyFromCombo(const QComboBox *box);

} // namespace acq
} // namespace scopy
