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

#ifndef DCDEBUGGERTOOL_H
#define DCDEBUGGERTOOL_H

#include "scopy-dcdebugger_export.h"

#include <QHash>
#include <QPointer>
#include <QWidget>

class QModelIndex;
class QSortFilterProxyModel;
class QStackedWidget;
class QTreeView;

namespace scopy {
class SearchBar;
class ToolTemplate;
} // namespace scopy

namespace scopy::component {
class Connection;
}

namespace scopy::dcdebugger {
class ComponentTreeModel;
class NodePanel;

// Tree of every component of the connection (left) + the control panel of the selected node (right).
class SCOPY_DCDEBUGGER_EXPORT DCDebuggerTool : public QWidget
{
	Q_OBJECT
public:
	explicit DCDebuggerTool(component::Connection *conn, QWidget *parent = nullptr);
	~DCDebuggerTool() override;

public Q_SLOTS:
	void rebuild();
	// Drops every reference into the component tree (before the views are released).
	void clear();

private:
	void buildTree(QWidget *parent);
	void expandDefault();
	void onCurrentChanged(const QModelIndex &proxyIndex);
	void onClicked(const QModelIndex &proxyIndex);
	void onExpanded(const QModelIndex &proxyIndex);
	void onFilterChanged(const QString &text);
	void selectNode(QObject *node);
	void readAttributes(const QList<QObject *> &objects);
	void clearPanels();
	QString panelKey(const QModelIndex &sourceIndex) const;

	QPointer<component::Connection> m_conn;
	ToolTemplate *m_tool;
	SearchBar *m_search;
	QTreeView *m_tree;
	ComponentTreeModel *m_model;
	QSortFilterProxyModel *m_proxy;
	QStackedWidget *m_panels;
	QWidget *m_emptyPanel;
	QHash<QString, NodePanel *> m_panelCache;
	bool m_bulkExpand = false;
};

} // namespace scopy::dcdebugger

#endif // DCDEBUGGERTOOL_H
