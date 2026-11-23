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
class QTimer;
class QTreeView;

namespace scopy {
class SearchBar;
class ToolTemplate;
} // namespace scopy

namespace scopy::component {
class Connection;
}

namespace scopy::dcdebugger {
class DCDebugger_API;
class BottomPaneHost;
class CommandLogView;
class ComponentTreeModel;
class NodePanel;

// Tree of every component of the connection (left) + the control panel of the selected node (right),
// over a bottom bar of output panes (the command log; hidden until its tab is clicked).
class SCOPY_DCDEBUGGER_EXPORT DCDebuggerTool : public QWidget
{
	Q_OBJECT
	friend class DCDebugger_API;

public:
	explicit DCDebuggerTool(component::Connection *conn, QWidget *parent = nullptr);
	~DCDebuggerTool() override;

	static constexpr const char *LOG_PANE = "log";
	// Throttle of the entry count shown on the "Log" tab.
	static constexpr int LOG_TAB_UPDATE_MS = 250;

	CommandLogView *commandLog() const { return m_log; }
	BottomPaneHost *bottomPanes() const { return m_bottomPanes; }

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
	void selectOrigin(const QString &origin, const QString &backendId, quintptr object);
	void selectIndex(const QModelIndex &sourceIndex);
	void readAttributes(const QList<QObject *> &objects);
	void clearPanels();
	void updateLogTab();
	QString panelKey(const QModelIndex &sourceIndex) const;

	QPointer<component::Connection> m_conn;
	ToolTemplate *m_tool;
	SearchBar *m_search;
	QTreeView *m_tree;
	ComponentTreeModel *m_model;
	QSortFilterProxyModel *m_proxy;
	QStackedWidget *m_panels;
	CommandLogView *m_log;
	BottomPaneHost *m_bottomPanes;
	QTimer *m_logTabTimer;
	QWidget *m_emptyPanel;
	QHash<QString, NodePanel *> m_panelCache;
	bool m_bulkExpand = false;
};

} // namespace scopy::dcdebugger

#endif // DCDEBUGGERTOOL_H
