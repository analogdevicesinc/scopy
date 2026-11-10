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

#include "component/component_export.h"
#include "core/callrecord.h"

#include <QObject>
#include <QString>

#include <memory>

namespace scopy {
class Command;
class TraceConfig;
} // namespace scopy

namespace scopy::component {

class Connection;

// Command call trace of one connection: on/off switch and sink. Each Connection owns exactly
// one (child of the connection) and installs its TraceConfig on the connection's executor. Off by
// default. When on, every command that starts afterwards on that executor records its library
// calls (DC_TRACE) and, once finished, is emitted as one CommandLogEntry through commandFinished —
// from the executor thread, so receivers living on the GUI thread get it queued. Toggling takes
// effect at the next command (no partial entries); other connections are not affected.
class COMPONENT_EXPORT CommandMonitor : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
	Q_PROPERTY(bool captureData READ captureData WRITE setCaptureData NOTIFY captureDataChanged)
	Q_PROPERTY(int previewBytes READ previewBytes WRITE setPreviewBytes)
	Q_PROPERTY(QString uri READ uri CONSTANT)
public:
	// Detaches from the trace config: no commandFinished once destroyed, even for a command
	// still running on the executor.
	~CommandMonitor() override;

	// Monitor of the connection `node` belongs to (any tree node, a Context or the Connection
	// itself); nullptr for a node not attached to a connection.
	static CommandMonitor *of(const QObject *node);

	QString uri() const { return m_uri; }
	Connection *connection() const;
	// The config installed on the connection's executor.
	std::shared_ptr<TraceConfig> config() const { return m_config; }

	bool isEnabled() const;
	void setEnabled(bool enabled);

	// Full payloads (refill / push blocks, attr values) instead of the first previewBytes().
	bool captureData() const;
	void setCaptureData(bool capture);
	int previewBytes() const;
	void setPreviewBytes(int bytes);

	// Component path / uri / view of a tree node: "/"-joined object names from the Context down.
	// An unnamed leaf (stream, register reader, ...) adds its component base name; attribute
	// readers / writers resolve to their Attribute.
	static CommandOrigin originOf(const QObject *node);
	// Called by a leaf before dispatching `cmd`: records where it comes from. One atomic load
	// and nothing else while every monitor is off.
	static void tag(Command *cmd, const QObject *leaf);

Q_SIGNALS:
	void enabledChanged(bool enabled);
	void captureDataChanged(bool capture);
	void commandFinished(const scopy::CommandLogEntry &entry);

private:
	friend class Connection;
	explicit CommandMonitor(Connection *connection);
	void deliver(CommandLogEntry entry);

	QString m_uri;
	std::shared_ptr<TraceConfig> m_config;
};

} // namespace scopy::component
