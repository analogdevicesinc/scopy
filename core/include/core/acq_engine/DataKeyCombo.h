#pragma once

#include "scopy-core_export.h"

#include "DataKey.h"

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
SCOPY_CORE_EXPORT void populateKeyCombo(QComboBox *box, const DataStore *store,
					const AcquisitionEngine *engine, bool withSampleIndex);

// The key `box` currently names. Empty when nothing is selected. The sample-index entry needs no
// special case: its payload *is* the engine ramp's key.
SCOPY_CORE_EXPORT DataKey keyFromCombo(const QComboBox *box);

} // namespace acq
} // namespace scopy
