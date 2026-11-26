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

#include "nodeactivity.h"

#include "acquisition/streamacquisition.h"
#include "generation/streamgeneration.h"
#include "monitor/attributemonitor.h"

#include <QPointer>

using namespace scopy::dcdebugger;

NodeActivity *NodeActivity::instance()
{
	static NodeActivity *activity = new NodeActivity();
	return activity;
}

void NodeActivity::track(QObject *node, AttributeMonitor *source) { trackRunning(node, source); }

void NodeActivity::track(QObject *node, StreamAcquisition *source) { trackRunning(node, source); }

void NodeActivity::track(QObject *node, StreamGeneration *source) { trackRunning(node, source); }

template <class Source>
void NodeActivity::trackRunning(QObject *node, Source *source)
{
	if(!node || !source) {
		return;
	}
	QPointer<QObject> guard(node);
	connect(source, &Source::runningChanged, this, [this, guard, source](bool running) {
		if(guard) {
			setRunning(guard, source, running);
		}
	});
	connect(source, &QObject::destroyed, this, [this, guard, source]() {
		if(guard) {
			setRunning(guard, source, false);
		}
	});
	// A destroyed node has no row left to flag: just forget it.
	connect(node, &QObject::destroyed, this, [this, node]() { m_running.remove(node); });
	if(source->isRunning()) {
		setRunning(node, source, true);
	}
}

bool NodeActivity::isActive(const QObject *node) const { return !m_running.value(node).isEmpty(); }

void NodeActivity::setRunning(QObject *node, QObject *source, bool running)
{
	const bool was = isActive(node);
	if(running) {
		m_running[node].insert(source);
	} else if(auto it = m_running.find(node); it != m_running.end()) {
		it->remove(source);
		if(it->isEmpty()) {
			m_running.erase(it);
		}
	}
	if(isActive(node) != was) {
		Q_EMIT activeChanged(node, !was);
	}
}

#include "moc_nodeactivity.cpp"
