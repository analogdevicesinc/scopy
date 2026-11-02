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

#include "dcdebuggertool.h"

#include "componenttreemodel.h"
#include "nodepanel.h"

#include <QCursor>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStackedWidget>
#include <QTreeView>
#include <QVBoxLayout>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/connection.h>
#include <dcdebugger/style_properties.h>
#include <style.h>
#include <tooltemplate.h>
#include <widgets/searchbar.h>

using namespace scopy;
using namespace scopy::dcdebugger;

DCDebuggerTool::DCDebuggerTool(component::Connection *conn, QWidget *parent)
	: QWidget(parent)
	, m_conn(conn)
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->topContainer()->setVisible(false);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->bottomContainer()->setVisible(false);
	m_tool->rightContainer()->setVisible(false);
	m_tool->leftContainer()->setVisible(false);
	m_tool->topCentral()->setVisible(false);
	m_tool->centralContainer()->setVisible(true);
	lay->addWidget(m_tool);

	auto *splitter = new QSplitter(Qt::Horizontal, this);
	splitter->setChildrenCollapsible(false);

	auto *treeContainer = new QWidget(splitter);
	Style::setBackgroundColor(treeContainer, json::theme::background_subtle);
	buildTree(treeContainer);

	auto *panelContainer = new QWidget(splitter);
	auto *panelLay = new QVBoxLayout(panelContainer);
	panelLay->setContentsMargins(6, 6, 6, 6);
	m_panels = new QStackedWidget(panelContainer);
	Style::setStyle(m_panels, style::properties::dcdebugger::panelFrame);
	m_emptyPanel = new QLabel("Select a node in the tree", m_panels);
	static_cast<QLabel *>(m_emptyPanel)->setAlignment(Qt::AlignCenter);
	Style::setStyle(m_emptyPanel, style::properties::label::subtle);
	m_panels->addWidget(m_emptyPanel);
	panelLay->addWidget(m_panels);

	splitter->addWidget(treeContainer);
	splitter->addWidget(panelContainer);
	splitter->setStretchFactor(0, 3);
	splitter->setStretchFactor(1, 2);
	m_tool->addWidgetToCentralContainerHelper(splitter);

	m_model->setConnection(m_conn);
}

DCDebuggerTool::~DCDebuggerTool() { clear(); }

void DCDebuggerTool::buildTree(QWidget *parent)
{
	auto *lay = new QVBoxLayout(parent);
	lay->setContentsMargins(6, 6, 6, 6);
	lay->setSpacing(6);

	m_search = new SearchBar({}, parent);
	m_search->getLineEdit()->setPlaceholderText("Filter by name");
	lay->addWidget(m_search);

	m_model = new ComponentTreeModel(this);
	m_proxy = new QSortFilterProxyModel(this);
	m_proxy->setSourceModel(m_model);
	m_proxy->setRecursiveFilteringEnabled(true);
	m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
	m_proxy->setFilterKeyColumn(ComponentTreeModel::NameColumn);

	m_tree = new QTreeView(parent);
	m_tree->setModel(m_proxy);
	m_tree->setUniformRowHeights(true);
	m_tree->setExpandsOnDoubleClick(false);
	m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
	m_tree->header()->setStretchLastSection(true);
	m_tree->header()->setSectionResizeMode(QHeaderView::Interactive);
	m_tree->setColumnWidth(ComponentTreeModel::NameColumn, 320);
	m_tree->setColumnWidth(ComponentTreeModel::ValueColumn, 240);
	Style::setStyle(m_tree, style::properties::dcdebugger::treeView);
	Style::setStyle(m_tree->header(), style::properties::dcdebugger::headerView);
	lay->addWidget(m_tree);

	connect(m_tree->selectionModel(), &QItemSelectionModel::currentChanged, this,
		[this](const QModelIndex &current) { onCurrentChanged(current); });
	connect(m_tree, &QTreeView::clicked, this, &DCDebuggerTool::onClicked);
	connect(m_tree, &QTreeView::expanded, this, &DCDebuggerTool::onExpanded);
	connect(m_search->getLineEdit(), &QLineEdit::textChanged, this, &DCDebuggerTool::onFilterChanged);
	connect(m_model, &ComponentTreeModel::rebuilt, this, &DCDebuggerTool::expandDefault);
}

void DCDebuggerTool::rebuild()
{
	clearPanels();
	m_model->setConnection(m_conn);
}

void DCDebuggerTool::clear()
{
	clearPanels();
	m_model->clearTree();
	m_conn = nullptr;
}

void DCDebuggerTool::clearPanels()
{
	m_panels->setCurrentWidget(m_emptyPanel);
	for(NodePanel *p : std::as_const(m_panelCache)) {
		m_panels->removeWidget(p);
		delete p;
	}
	m_panelCache.clear();
}

void DCDebuggerTool::expandDefault()
{
	// Connection, its views and the top level of every view.
	const QModelIndex root = m_proxy->index(0, 0);
	if(!root.isValid()) {
		return;
	}
	m_tree->expand(root);
	for(int g = 0; g < m_proxy->rowCount(root); ++g) {
		const QModelIndex group = m_proxy->index(g, 0, root);
		m_tree->expand(group);
		for(int c = 0; c < m_proxy->rowCount(group); ++c) {
			if(m_model->kindAt(m_proxy->mapToSource(m_proxy->index(c, 0, group))) == NodeKind::Context) {
				m_tree->expand(m_proxy->index(c, 0, group));
			}
		}
	}
	m_tree->setCurrentIndex(root);
}

QString DCDebuggerTool::panelKey(const QModelIndex &sourceIndex) const
{
	const QModelIndex idx = sourceIndex.siblingAtColumn(ComponentTreeModel::NameColumn);
	if(m_model->isGroup(idx)) {
		return QStringLiteral("group:%1:%2")
			.arg(quintptr(m_model->objectAt(idx.parent())), 0, 16)
			.arg(idx.data(ComponentTreeModel::GroupNameRole).toString());
	}
	return QStringLiteral("node:%1").arg(quintptr(m_model->objectAt(idx)), 0, 16);
}

void DCDebuggerTool::onCurrentChanged(const QModelIndex &proxyIndex)
{
	const QModelIndex src = m_proxy->mapToSource(proxyIndex).siblingAtColumn(ComponentTreeModel::NameColumn);
	if(!src.isValid()) {
		m_panels->setCurrentWidget(m_emptyPanel);
		return;
	}
	const NodeKind kind = m_model->kindAt(src);
	QObject *node = m_model->objectAt(src);
	if(kind == NodeKind::Attribute) {
		readAttributes({node});
	}

	const QString key = panelKey(src);
	NodePanel *panel = m_panelCache.value(key);
	if(!panel) {
		panel = createNodePanel(kind == NodeKind::Group ? nullptr : node, kind, src.data().toString(),
					m_model->pathOf(src), m_model->childObjects(src), m_panels);
		connect(panel, &NodePanel::pathSelected, this, &DCDebuggerTool::selectNode);
		m_panels->addWidget(panel);
		m_panelCache.insert(key, panel);
	}
	m_panels->setCurrentWidget(panel);
}

void DCDebuggerTool::onClicked(const QModelIndex &proxyIndex)
{
	// A click on a group row (not on its branch arrow) collapses / expands it.
	const QModelIndex idx = proxyIndex.siblingAtColumn(ComponentTreeModel::NameColumn);
	if(!m_model->isGroup(m_proxy->mapToSource(idx))) {
		return;
	}
	const QPoint pos = m_tree->viewport()->mapFromGlobal(QCursor::pos());
	if(!m_tree->visualRect(proxyIndex).contains(pos)) {
		return;
	}
	m_tree->setExpanded(idx, !m_tree->isExpanded(idx));
}

void DCDebuggerTool::onExpanded(const QModelIndex &proxyIndex)
{
	// Attribute values are read lazily, when their group is opened.
	const QModelIndex src = m_proxy->mapToSource(proxyIndex);
	if(!m_bulkExpand && m_model->isGroup(src)) {
		readAttributes(m_model->childObjects(src));
	}
}

void DCDebuggerTool::readAttributes(const QList<QObject *> &objects)
{
	for(QObject *obj : objects) {
		auto *attr = qobject_cast<component::Attribute *>(obj);
		if(attr && attr->readCapability()) {
			attr->readCapability()->readAsync();
		}
	}
}

void DCDebuggerTool::onFilterChanged(const QString &text)
{
	// Bulk expansion must not read every attribute of the tree.
	m_bulkExpand = true;
	m_proxy->setFilterFixedString(text);
	if(text.isEmpty()) {
		m_tree->collapseAll();
		expandDefault();
	} else {
		m_tree->expandAll();
	}
	m_bulkExpand = false;
}

void DCDebuggerTool::selectNode(QObject *node)
{
	const QModelIndex src = m_model->indexOf(node);
	if(!src.isValid()) {
		return;
	}
	QModelIndex idx = m_proxy->mapFromSource(src);
	if(!idx.isValid()) {
		// Filtered out: drop the filter so the node is reachable.
		m_search->getLineEdit()->clear();
		idx = m_proxy->mapFromSource(src);
	}
	m_tree->scrollTo(idx);
	m_tree->setCurrentIndex(idx);
}

#include "moc_dcdebuggertool.cpp"
