#pragma once

#include "scopy-core_export.h"

#include <QObject>
#include <QPointer>
#include <QString>

#include <atomic>

namespace scopy {
namespace acq { class AcquisitionEngine; }
namespace decoder {

enum class LogLevel { Info, Warning, Critical };

class SCOPY_CORE_EXPORT DecoderLogger : public QObject
{
	Q_OBJECT
public:
	explicit DecoderLogger(QObject *parent = nullptr);
	~DecoderLogger() override;

	void log(LogLevel lvl, const QString &id, const QString &msg);

	void info    (const QString &id, const QString &msg) { log(LogLevel::Info,     id, msg); }
	void warning (const QString &id, const QString &msg) { log(LogLevel::Warning,  id, msg); }
	void critical(const QString &id, const QString &msg) { log(LogLevel::Critical, id, msg); }

	void setEngine(scopy::acq::AcquisitionEngine *e);
	void setForwardToEngine(bool en) { m_forward.store(en, std::memory_order_relaxed); }
	bool forwardsToEngine() const    { return m_forward.load(std::memory_order_relaxed); }

Q_SIGNALS:
	void messageLogged(int level, QString id, QString message);

private:
	QPointer<scopy::acq::AcquisitionEngine> m_engine;
	std::atomic<bool>                       m_forward{true};
};

} // namespace decoder
} // namespace scopy
