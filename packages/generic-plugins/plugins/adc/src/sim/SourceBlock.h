#pragma once

#include "DataStore.h"

#include <atomic>
#include <stdexcept>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class QWidget;

namespace scopy {
namespace adc {
namespace sim {

class SourceBlock : public QObject
{
	Q_OBJECT
public:
	explicit SourceBlock(const QString &id, QObject *parent = nullptr);
	virtual ~SourceBlock() = default;

	virtual void acquire(DataStore *store) = 0;

	virtual void onStart();
	virtual void onStop();

	void        setBufferSize(std::size_t size);
	std::size_t bufferSize() const;

	void           enableChannel(const QString &channelId, bool en);
	void           disableAllChannels();
	bool           isChannelEnabled(const QString &channelId) const;
	QList<QString> enabledChannels() const;
	QList<QString> channelIds() const { return m_channels.keys(); }

	const QString &id() const { return m_id; }

	bool isEnabled() const { return m_sourceEnabled.load(std::memory_order_relaxed); }
	void setEnabled(bool en) { m_sourceEnabled.store(en, std::memory_order_relaxed); }

	virtual QWidget *createSettingsWidget(QWidget *parent = nullptr);

protected:
	std::size_t         m_bufferSize{1024};
	QMap<QString, bool> m_channels;
	std::atomic<bool>   m_stopRequested{false};
	std::atomic<bool>   m_sourceEnabled{true};
	QString             m_id;
};

} // namespace sim
} // namespace adc
} // namespace scopy
