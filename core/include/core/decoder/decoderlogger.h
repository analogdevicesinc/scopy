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

#include <QObject>
#include <QPointer>
#include <QString>

#include <atomic>

namespace scopy {
namespace acq {
class AcquisitionEngine;
}
namespace decoder {

enum class LogLevel
{
	Info,
	Warning,
	Critical
};

class SCOPY_CORE_EXPORT DecoderLogger : public QObject
{
	Q_OBJECT
public:
	explicit DecoderLogger(QObject *parent = nullptr);
	~DecoderLogger() override;

	void log(LogLevel lvl, const QString &id, const QString &msg);

	void info(const QString &id, const QString &msg) { log(LogLevel::Info, id, msg); }
	void warning(const QString &id, const QString &msg) { log(LogLevel::Warning, id, msg); }
	void critical(const QString &id, const QString &msg) { log(LogLevel::Critical, id, msg); }

	void setEngine(scopy::acq::AcquisitionEngine *e);
	void setForwardToEngine(bool en) { m_forward.store(en, std::memory_order_relaxed); }
	bool forwardsToEngine() const { return m_forward.load(std::memory_order_relaxed); }

	// Messages below this level are dropped without being emitted. Defaults to
	// Warning, matching AcquisitionEngine::minReportSeverity: a backend logs a
	// few Info lines per decode() call, so at continuous-mode frame rates Info
	// is a flood the GUI thread has to drain, not a diagnostic anyone reads.
	// Raise it to Info deliberately when debugging.
	void setMinLevel(LogLevel lvl) { m_minLevel.store(lvl, std::memory_order_relaxed); }
	LogLevel minLevel() const { return m_minLevel.load(std::memory_order_relaxed); }

Q_SIGNALS:
	void messageLogged(int level, QString id, QString message);

private:
	QPointer<scopy::acq::AcquisitionEngine> m_engine;
	std::atomic<bool> m_forward{true};
	// Read from decoder worker threads, written from the GUI thread.
	std::atomic<LogLevel> m_minLevel{LogLevel::Warning};
};

} // namespace decoder
} // namespace scopy
