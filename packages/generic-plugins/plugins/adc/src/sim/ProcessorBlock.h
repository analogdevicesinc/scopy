#pragma once

#include "DataKey.h"
#include "DataStore.h"

#include <atomic>
#include <stdexcept>
#include <QList>
#include <QObject>
#include <QString>

class QWidget;

namespace scopy {
namespace adc {
namespace sim {

class ProcessorBlock : public QObject
{
	Q_OBJECT
public:
	explicit ProcessorBlock(const QString &name, QObject *parent = nullptr);
	virtual ~ProcessorBlock() = default;

	virtual const QList<DataKey> &watchedKeys() const { return m_watchedKeys; }
	void setWatchedKeys(const QList<DataKey> &keys) { m_watchedKeys = keys; }

	virtual void process(DataStore *store) = 0;
	virtual void reset() {}

	bool           isEnabled() const { return m_enabled.load(std::memory_order_relaxed); }
	void           setEnabled(bool en) { m_enabled.store(en, std::memory_order_relaxed); }
	const QString &name() const { return m_name; }

	virtual QWidget *createSettingsWidget(QWidget *parent = nullptr);

protected:
	QString           m_name;
	std::atomic<bool> m_enabled{true};
	QList<DataKey>    m_watchedKeys;
};

} // namespace sim
} // namespace adc
} // namespace scopy
