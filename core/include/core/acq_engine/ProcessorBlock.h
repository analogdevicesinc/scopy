#pragma once

#include "scopy-core_export.h"

#include "Block.h"
#include "DataKey.h"
#include "DataStore.h"

#include <QList>
#include <QString>

namespace scopy {
namespace acq {

// Transforms DataStore streams: reads its watchedKeys, writes derived keys.
//
// The engine runs process() once per cycle, but only after every watched key has
// been written that cycle — that ordering constraint is the whole scheduling
// contract, so a processor chain is expressed purely by which keys each link
// watches. A processor whose inputs never arrive is skipped for the cycle.
//
// process() runs on the worker thread. Configuration set from the GUI must
// therefore be either atomic or mutex-guarded.
class SCOPY_CORE_EXPORT ProcessorBlock : public Block
{
	Q_OBJECT
public:
	explicit ProcessorBlock(const QString &name, QObject *parent = nullptr);

	virtual const QList<DataKey> &watchedKeys() const { return m_watchedKeys; }
	virtual void setWatchedKeys(const QList<DataKey> &keys) { m_watchedKeys = keys; }

	virtual void process(DataStore *store) = 0;

	// Called before each run so stateful processors can drop carry-over.
	virtual void reset() {}

protected:
	QList<DataKey> m_watchedKeys;
};

} // namespace acq
} // namespace scopy
