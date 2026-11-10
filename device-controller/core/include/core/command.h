/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "core/callrecord.h"
#include "core/result.h"
#include "core/controller_core_export.h"
#include <QObject>
#include <QUuid>

#include <memory>

namespace scopy {

class TraceConfig;

class CONTROLLER_CORE_EXPORT Command : public QObject
{
	Q_OBJECT
public:
	Command(void *resource, QObject *parent = nullptr)
		: QObject(parent)
		, m_id(QUuid::createUuid())
		, m_resource(resource)
	{}
	virtual ~Command() = default;

	virtual void execute()
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			Command *outer = beginRun();
			run();
			endRun(outer);
		}
		Q_EMIT finished(this);
	}

	virtual QString toString() const
	{
		return QString::fromLatin1(metaObject()->className()) + QStringLiteral("(id=") +
			m_id.toString(QUuid::WithoutBraces) + QLatin1Char(')');
	}

	QUuid id() const { return m_id; }
	void *resource() const { return m_resource; }
	void cancel() { m_cancelled = true; }
	bool isCancelled() const { return m_cancelled; }

	// --- call trace (see core/calltrace.h) ---
	// The command running on this thread (inside run()), nullptr elsewhere.
	static Command *current();
	// Recording right now (decided at the start of execute() from the TraceConfig of the executor
	// running it; never when executed directly).
	bool isRecording() const { return m_trace && m_trace->recording; }
	// Config of the executor this command records for; nullptr unless recording.
	const TraceConfig *traceConfig() const { return isRecording() ? m_trace->config : nullptr; }
	// Recorded its last run (the executor publishes it).
	bool isTraced() const { return m_trace && m_trace->traced; }
	// Appends a library call; ignored unless recording.
	void trace(CallRecord record);
	QList<CallRecord> calls() const { return m_trace ? m_trace->calls : QList<CallRecord>{}; }
	qint64 startedNs() const { return m_trace ? m_trace->startedNs : 0; }
	qint64 finishedNs() const { return m_trace ? m_trace->finishedNs : 0; }

	// Component that enqueued the command; set by the leaf before dispatch (only while tracing).
	QString origin() const { return m_trace ? m_trace->origin.path : QString(); }
	CommandOrigin originInfo() const { return m_trace ? m_trace->origin : CommandOrigin{}; }
	void setOrigin(const QString &path);
	void setOrigin(const CommandOrigin &origin);

	// Failure of the last run for the log; ResultCommand reports its result error.
	virtual Error failure() const { return {}; }

Q_SIGNALS:
	void started(scopy::Command *cmd);
	void finished(scopy::Command *cmd);

protected:
	virtual void run() = 0;

	QUuid m_id;
	void *m_resource;
	bool m_cancelled = false;

private:
	struct TraceData
	{
		CommandOrigin origin;
		QList<CallRecord> calls;
		qint64 startedNs = 0;
		qint64 finishedNs = 0;
		qint64 steadyStartNs = 0;
		const TraceConfig *config = nullptr; // while recording
		bool recording = false;
		bool traced = false;
	};

	// Sets current() and, when the running executor's config is on, starts recording; returns the
	// outer current().
	Command *beginRun();
	void endRun(Command *outer);
	TraceData *traceData();

	std::unique_ptr<TraceData> m_trace;
};

} // namespace scopy
