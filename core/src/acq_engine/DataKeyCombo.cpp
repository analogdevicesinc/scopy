#include "DataKeyCombo.h"

#include "AcquisitionEngine.h"
#include "DataStore.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QSet>
#include <QSignalBlocker>
#include <QStringList>

namespace scopy {
namespace acq {

void populateKeyCombo(QComboBox *box, const DataStore *store, const AcquisitionEngine *engine,
		      bool withSampleIndex)
{
	if(!box) {
		return;
	}

	// Blocked and rebuilt wholesale: clear() emits currentIndexChanged, and a listener
	// would read that as the reader picking whatever lands at index 0.
	QSignalBlocker blocker(box);
	const QString previous = box->currentData().toString();
	box->clear();

	if(withSampleIndex) {
		// First, and so the default for an X picker. Its payload is the engine ramp's real
		// key, so the loop below skips it rather than listing it twice.
		box->addItem(QCoreApplication::translate("DataKeyCombo", "sample index"),
			     AcquisitionEngine::indexRampKey().toString());
	}

	// Declared ∪ written: the two overlap but neither contains the other. A
	// declared-but-unwritten stream is offered on purpose — selecting it before the first
	// cycle is what makes the first window full-depth.
	QSet<DataKey> all;
	if(engine) {
		const QList<DataKey> declared = engine->declaredKeys();
		for(const DataKey &k : declared) {
			all.insert(k);
		}
	}
	if(store) {
		const QList<DataKey> written = store->keys();
		for(const DataKey &k : written) {
			all.insert(k);
		}
	}

	QStringList names;
	names.reserve(all.size());
	for(const DataKey &k : all) {
		names << k.toString();
	}
	// Sorted: a QSet iterates in hash order, which would reshuffle the list every time the
	// key set changed and make the picker unusable.
	names.sort();
	const QString ramp = AcquisitionEngine::indexRampKey().toString();
	for(const QString &n : names) {
		// The ramp already has its own entry above when one was asked for.
		if(withSampleIndex && n == ramp) {
			continue;
		}
		box->addItem(n, n);
	}

	// Keep what was selected where it still exists — a refresh must not silently retarget a
	// selection the reader made and has not acted on yet.
	const int idx = box->findData(previous);
	box->setCurrentIndex(qMax(0, idx));
}

DataKey keyFromCombo(const QComboBox *box)
{
	if(!box) {
		return DataKey();
	}
	const QString data = box->currentData().toString();
	if(data.isEmpty()) {
		return DataKey();
	}
	return DataKey(data);
}

} // namespace acq
} // namespace scopy
