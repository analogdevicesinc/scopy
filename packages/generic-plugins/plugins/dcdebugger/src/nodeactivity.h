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

#ifndef NODEACTIVITY_H
#define NODEACTIVITY_H

#include "scopy-dcdebugger_export.h"

#include <QHash>
#include <QObject>
#include <QSet>

namespace scopy::dcdebugger {

class AttributeMonitor;
class StreamAcquisition;
class StreamGeneration;

// Which component nodes have something running on them (an attribute monitor, a stream acquisition
// or generation), so the component tree can flag them. The views register what they run here; it
// is forgotten when it or its node is destroyed.
class SCOPY_DCDEBUGGER_EXPORT NodeActivity : public QObject
{
	Q_OBJECT
public:
	static NodeActivity *instance();

	// The node is active while `source` runs.
	void track(QObject *node, AttributeMonitor *source);
	void track(QObject *node, StreamAcquisition *source);
	void track(QObject *node, StreamGeneration *source);
	bool isActive(const QObject *node) const;

Q_SIGNALS:
	// The node got its first running source (true) or lost its last one (false).
	void activeChanged(QObject *node, bool active);

private:
	NodeActivity() = default;
	template <class Source>
	void trackRunning(QObject *node, Source *source);
	void setRunning(QObject *node, QObject *source, bool running);

	QHash<const QObject *, QSet<const QObject *>> m_running;
};

} // namespace scopy::dcdebugger

#endif // NODEACTIVITY_H
