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

#include "nodeviewregistry.h"

#include <QVariant>

#include <algorithm>

#include <pluginbase/preferences.h>

using namespace scopy;
using namespace scopy::dcdebugger;

NodeViewRegistry::NodeViewRegistry() = default;

NodeViewRegistry *NodeViewRegistry::instance()
{
	static NodeViewRegistry *registry = []() {
		auto *r = new NodeViewRegistry();
		registerBuiltinNodeViews(r);
		return r;
	}();
	return registry;
}

void NodeViewRegistry::registerFactory(std::unique_ptr<NodeViewFactory> factory, int priority)
{
	if(!factory) {
		return;
	}
	const QString id = factory->id();
	Preferences::init(preferenceKey(id), true);
	unregisterFactory(id);
	// Stable: equal priorities keep the registration order.
	auto pos = std::find_if(m_entries.begin(), m_entries.end(),
				[priority](const Entry &e) { return e.priority < priority; });
	m_entries.insert(pos, Entry{std::move(factory), priority});
}

void NodeViewRegistry::unregisterFactory(const QString &id)
{
	m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
				       [&id](const Entry &e) { return e.factory->id() == id; }),
			m_entries.end());
}

QList<NodeViewFactory *> NodeViewRegistry::factories() const
{
	QList<NodeViewFactory *> out;
	for(const Entry &e : m_entries) {
		out.append(e.factory.get());
	}
	return out;
}

NodeViewFactory *NodeViewRegistry::factory(const QString &id) const
{
	for(const Entry &e : m_entries) {
		if(e.factory->id() == id) {
			return e.factory.get();
		}
	}
	return nullptr;
}

QList<NodeViewFactory *> NodeViewRegistry::factoriesFor(QObject *node, NodeKind kind) const
{
	QList<NodeViewFactory *> out;
	if(!node) {
		return out;
	}
	for(const Entry &e : m_entries) {
		if(isEnabled(e.factory->id()) && e.factory->supports(node, kind)) {
			out.append(e.factory.get());
		}
	}
	return out;
}

QString NodeViewRegistry::preferenceKey(const QString &id) { return QStringLiteral("dcdebugger_view_") + id; }

bool NodeViewRegistry::isEnabled(const QString &id)
{
	// Not initialised (e.g. no plugin init()): shown, like the default.
	const QVariant v = Preferences::get(preferenceKey(id));
	return !v.isValid() || v.toBool();
}
