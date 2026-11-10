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

namespace scopy {
class Command;
}

namespace scopy::component {

// Command call trace switch and sink (singleton). Off by default. When on, every command that
// starts afterwards records its library calls (DC_TRACE) and, once finished, is emitted as one
// CommandLogEntry through commandFinished — from the executor thread, so receivers living on the
// GUI thread get it queued. Toggling takes effect at the next command (no partial entries).
class COMPONENT_EXPORT CommandMonitor : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
	Q_PROPERTY(bool captureData READ captureData WRITE setCaptureData NOTIFY captureDataChanged)
	Q_PROPERTY(int previewBytes READ previewBytes WRITE setPreviewBytes)
public:
	static CommandMonitor *GetInstance();

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
	// and nothing else while the monitor is off.
	static void tag(Command *cmd, const QObject *leaf);

Q_SIGNALS:
	void enabledChanged(bool enabled);
	void captureDataChanged(bool capture);
	void commandFinished(const scopy::CommandLogEntry &entry);

private:
	explicit CommandMonitor(QObject *parent = nullptr);
	static void deliver(const CommandLogEntry &entry);
};

} // namespace scopy::component
