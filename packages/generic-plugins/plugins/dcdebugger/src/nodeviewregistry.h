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

#ifndef NODEVIEWREGISTRY_H
#define NODEVIEWREGISTRY_H

#include "componentnode.h"
#include "scopy-dcdebugger_export.h"

#include <QList>
#include <QString>

#include <memory>

class QObject;
class QWidget;

namespace scopy::dcdebugger {
class NodePanel;

// A main view a node panel can show (acquisition, monitor, ...): matched against the node, built
// when its panel is. Implemented by the debugger for its own views, and by any plugin that wants a
// view of its own nodes (register it in the plugin's init()).
class SCOPY_DCDEBUGGER_EXPORT NodeViewFactory
{
public:
	virtual ~NodeViewFactory() = default;

	// Stable, lowercase: the preference "dcdebugger_view_<id>" and the JS name.
	virtual QString id() const = 0;
	// The tab / dock title.
	virtual QString title() const = 0;
	// By tag, metaobject or children; never by backend type.
	virtual bool supports(QObject *node, NodeKind kind) const = 0;
	// Parented to the panel; may connect to its refreshRequested / selected signals.
	virtual QWidget *create(QObject *node, NodeKind kind, NodePanel *panel) = 0;
};

// The views node panels can show, process-wide. The debugger's own views are registered on
// first use; each view can be turned off in the preferences (on by default).
class SCOPY_DCDEBUGGER_EXPORT NodeViewRegistry
{
public:
	static NodeViewRegistry *instance();

	// Takes the factory; one with the same id is replaced. Panels show the views by descending
	// priority, then registration order. Applies to panels built afterwards.
	void registerFactory(std::unique_ptr<NodeViewFactory> factory, int priority = 0);
	void unregisterFactory(const QString &id);

	QList<NodeViewFactory *> factories() const;
	NodeViewFactory *factory(const QString &id) const;
	// The enabled views that support the node, in panel order.
	QList<NodeViewFactory *> factoriesFor(QObject *node, NodeKind kind) const;

	static QString preferenceKey(const QString &id);
	static bool isEnabled(const QString &id);

private:
	NodeViewRegistry();

	struct Entry
	{
		std::unique_ptr<NodeViewFactory> factory;
		int priority = 0;
	};
	std::vector<Entry> m_entries;
};

// The debugger's own views: Acquisition, Generator, Ping, Monitor, Registers.
void registerBuiltinNodeViews(NodeViewRegistry *registry);

} // namespace scopy::dcdebugger

#endif // NODEVIEWREGISTRY_H
