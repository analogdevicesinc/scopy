#pragma once

#include "scopy-core_export.h"

#include "DataKey.h"
#include "SampleBuffer.h"

#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>

namespace scopy {
namespace acq {

class SCOPY_CORE_EXPORT DataStore : public QObject
{
	Q_OBJECT
public:
	explicit DataStore(QObject *parent = nullptr);

	void write(const DataKey &key, SampleVariant vec);
	void setHistorySize(const DataKey &key, std::size_t n);
	SampleBuffer   read(const DataKey &key) const;
	bool           contains(const DataKey &key) const;
	QList<DataKey> keys() const;
	void clear();
	void reset();

	void          beginCycle();
	QSet<DataKey> cycleKeys() const;

Q_SIGNALS:
	void dataWritten(DataKey key);
	void keysChanged(QList<DataKey> keys);

private:
	QMap<DataKey, SampleBuffer> m_data;
	QSet<DataKey>               m_cycleKeys;
	mutable QMutex              m_mutex;
};

} // namespace acq
} // namespace scopy
