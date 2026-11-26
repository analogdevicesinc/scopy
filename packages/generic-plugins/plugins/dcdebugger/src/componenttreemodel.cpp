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

#include "componenttreemodel.h"

#include "nodeactivity.h"

#include <QApplication>
#include <QIcon>
#include <QMap>
#include <QPainter>
#include <QPixmap>
#include <QStyle>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/commandmonitor.h>
#include <component/connection.h>
#include <component/stream.h>
#include <style.h>

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

ComponentTreeModel::ComponentTreeModel(QObject *parent)
	: QStandardItemModel(parent)
{
	setColumnCount(ColumnCount);
	setHorizontalHeaderLabels({"Name", "Value"});
}

ComponentTreeModel::~ComponentTreeModel() { delete m_watchers; }

void ComponentTreeModel::setConnection(Connection *conn)
{
	m_conn = conn;
	rebuild();
}

void ComponentTreeModel::clearTree()
{
	delete m_watchers;
	m_watchers = nullptr;
	m_objects.clear();
	m_items.clear();
	removeRows(0, rowCount());
}

void ComponentTreeModel::rebuild()
{
	clearTree();
	if(m_conn) {
		m_watchers = new QObject(this);
		connect(NodeActivity::instance(), &NodeActivity::activeChanged, m_watchers,
			[this](QObject *node, bool active) {
				if(QStandardItem *item = m_items.value(node)) {
					setActivityFlag(item, active);
				}
			});
		addNode(invisibleRootItem(), m_conn, NodeKind::Connection);
	}
	Q_EMIT rebuilt();
}

QList<QStandardItem *> ComponentTreeModel::makeRow(const QString &name, const QString &value) const
{
	QList<QStandardItem *> row = {new QStandardItem(name), new QStandardItem(value)};
	for(QStandardItem *it : row) {
		it->setEditable(false);
	}
	row[ValueColumn]->setToolTip(value);
	return row;
}

void ComponentTreeModel::addNode(QStandardItem *parent, QObject *obj, NodeKind kind)
{
	QList<QStandardItem *> row = makeRow(ComponentNode::name(obj, kind), ComponentNode::value(obj, kind));
	row[NameColumn]->setData(int(kind), KindRole);
	parent->appendRow(row);
	m_objects.insert(row[NameColumn], obj);
	m_items.insert(obj, row[NameColumn]);
	if(NodeActivity::instance()->isActive(obj)) {
		setActivityFlag(row[NameColumn], true); // e.g. a cached panel still polling across a rebuild
	}
	watch(obj, kind, row[ValueColumn]);
	addChildren(row[NameColumn], obj);
}

void ComponentTreeModel::setActivityFlag(QStandardItem *item, bool active)
{
	if(!active) {
		item->setData(QVariant(), Qt::DecorationRole);
		item->setToolTip(QString());
		return;
	}
	// A small dot before the name: light enough for every row, unlike a value or a column. Drawn
	// centred on a full icon-sized canvas at the screen's pixel ratio, so no scaling or rounding
	// moves it off the row's middle.
	const qreal dpr = qApp->devicePixelRatio();
	const int canvas = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
	const qreal diameter = 8;
	QPixmap dot(QSize(canvas, canvas) * dpr);
	dot.setDevicePixelRatio(dpr);
	dot.fill(Qt::transparent);
	QPainter p(&dot);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::NoPen);
	p.setBrush(Style::getColor(json::theme::content_success));
	const qreal offset = (canvas - diameter) / 2;
	p.drawEllipse(QRectF(offset, offset, diameter, diameter));
	p.end();
	item->setData(QIcon(dot), Qt::DecorationRole);
	item->setToolTip(QStringLiteral("Running"));
}

void ComponentTreeModel::addChildren(QStandardItem *item, QObject *obj)
{
	struct Group
	{
		QString name;
		QList<QPair<QObject *, NodeKind>> members;
	};
	QList<Group> groups;
	for(QObject *child : obj->children()) {
		const NodeKind kind = ComponentNode::kindOf(child);
		if(kind == NodeKind::Hidden) {
			continue;
		}
		const QString name = ComponentNode::groupOf(child, kind);
		auto it = std::find_if(groups.begin(), groups.end(), [&](const Group &g) { return g.name == name; });
		if(it == groups.end()) {
			groups.append({name, {}});
			it = std::prev(groups.end());
		}
		it->members.append({child, kind});
	}
	std::stable_sort(groups.begin(), groups.end(), [](const Group &a, const Group &b) {
		return ComponentNode::groupOrder(a.name) < ComponentNode::groupOrder(b.name);
	});

	for(const Group &g : std::as_const(groups)) {
		QList<QStandardItem *> row =
			makeRow(QStringLiteral("%1 (%2)").arg(g.name).arg(g.members.size()), QString());
		row[NameColumn]->setData(int(NodeKind::Group), KindRole);
		row[NameColumn]->setData(g.name, GroupNameRole);
		item->appendRow(row);
		for(const auto &m : g.members) {
			addNode(row[NameColumn], m.first, m.second);
		}
	}
}

void ComponentTreeModel::watch(QObject *obj, NodeKind kind, QStandardItem *valueItem)
{
	auto refresh = [obj, kind, valueItem]() {
		const QString v = ComponentNode::value(obj, kind);
		valueItem->setText(v);
		valueItem->setToolTip(v);
	};
	switch(kind) {
	case NodeKind::Attribute: {
		auto *attr = static_cast<Attribute *>(obj);
		connect(attr, &Attribute::valueChanged, m_watchers, refresh);
		if(AttributeReader *reader = attr->readCapability()) {
			connect(reader, &AttributeReader::readFailed, m_watchers, [valueItem](const scopy::Error &e) {
				valueItem->setText(QStringLiteral("read failed: ") + e.errorString());
				valueItem->setToolTip(valueItem->text());
			});
		}
		break;
	}
	case NodeKind::InputStream:
	case NodeKind::OutputStream:
		connect(static_cast<Stream *>(obj), &Stream::openedChanged, m_watchers, refresh);
		connect(static_cast<Stream *>(obj), &Stream::enabledChannelsChanged, m_watchers, refresh);
		break;
	default:
		break;
	}
}

QObject *ComponentTreeModel::objectAt(const QModelIndex &index) const
{
	if(!index.isValid()) {
		return nullptr;
	}
	return m_objects.value(itemFromIndex(index.siblingAtColumn(NameColumn))).data();
}

NodeKind ComponentTreeModel::kindAt(const QModelIndex &index) const
{
	if(!index.isValid()) {
		return NodeKind::Hidden;
	}
	return NodeKind(index.siblingAtColumn(NameColumn).data(KindRole).toInt());
}

bool ComponentTreeModel::isGroup(const QModelIndex &index) const { return kindAt(index) == NodeKind::Group; }

QList<QPair<QString, QObject *>> ComponentTreeModel::pathOf(const QModelIndex &index) const
{
	QList<QPair<QString, QObject *>> parts;
	for(QModelIndex i = index.siblingAtColumn(NameColumn); i.isValid(); i = i.parent()) {
		if(!isGroup(i)) {
			parts.prepend({i.data().toString(), objectAt(i)});
		}
	}
	return parts;
}

QList<QObject *> ComponentTreeModel::childObjects(const QModelIndex &index) const
{
	QList<QObject *> out;
	const QModelIndex idx = index.siblingAtColumn(NameColumn);
	for(int r = 0; r < rowCount(idx); ++r) {
		const QModelIndex child = this->index(r, NameColumn, idx);
		if(isGroup(child)) {
			out.append(childObjects(child));
		} else if(QObject *obj = objectAt(child)) {
			out.append(obj);
		}
	}
	return out;
}

QModelIndex ComponentTreeModel::indexOf(const QObject *obj) const
{
	QStandardItem *item = m_items.value(obj);
	return item ? item->index() : QModelIndex();
}

QModelIndex ComponentTreeModel::indexOfOrigin(const QString &path, const QString &backendId, quintptr object) const
{
	auto matches = [&](const QObject *obj) {
		const CommandOrigin o = component::CommandMonitor::originOf(obj);
		return o.path == path && (backendId.isEmpty() || o.backendId == backendId);
	};
	for(auto it = m_objects.cbegin(); it != m_objects.cend(); ++it) {
		const QObject *obj = it.value().data();
		if(obj && quintptr(obj) == object && matches(obj)) {
			return it.key()->index();
		}
	}
	for(auto it = m_objects.cbegin(); it != m_objects.cend(); ++it) {
		const QObject *obj = it.value().data();
		if(obj && matches(obj)) {
			return it.key()->index();
		}
	}
	return {};
}

#include "moc_componenttreemodel.cpp"
