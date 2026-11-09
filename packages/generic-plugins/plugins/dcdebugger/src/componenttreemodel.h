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

#ifndef COMPONENTTREEMODEL_H
#define COMPONENTTREEMODEL_H

#include "componentnode.h"
#include "scopy-dcdebugger_export.h"

#include <QHash>
#include <QPointer>
#include <QStandardItemModel>

namespace scopy::component {
class Connection;
}

namespace scopy::dcdebugger {

// Three-column (name / value / info) view of one connection's component tree. Children of a
// node are listed under group rows of the same kind ("Devices (3)", "Attributes (12)", ...).
// Values follow the components live (Attribute::valueChanged, Stream::openedChanged, ...).
class SCOPY_DCDEBUGGER_EXPORT ComponentTreeModel : public QStandardItemModel
{
	Q_OBJECT
public:
	enum Column
	{
		NameColumn = 0,
		ValueColumn,
		InfoColumn,
		ColumnCount
	};
	enum Role
	{
		KindRole = Qt::UserRole + 1,
		GroupNameRole
	};

	explicit ComponentTreeModel(QObject *parent = nullptr);
	~ComponentTreeModel() override;

	void setConnection(component::Connection *conn);
	void rebuild();
	void clearTree();

	QObject *objectAt(const QModelIndex &index) const;
	NodeKind kindAt(const QModelIndex &index) const;
	bool isGroup(const QModelIndex &index) const;
	// (name, component) from the root down to index, groups skipped.
	QList<QPair<QString, QObject *>> pathOf(const QModelIndex &index) const;
	// Components listed directly under index (through its groups for a component node).
	QList<QObject *> childObjects(const QModelIndex &index) const;
	QModelIndex indexOf(const QObject *obj) const;

Q_SIGNALS:
	void rebuilt();

private:
	void addNode(QStandardItem *parent, QObject *obj, NodeKind kind);
	void addChildren(QStandardItem *item, QObject *obj);
	void watch(QObject *obj, NodeKind kind, QStandardItem *valueItem, QStandardItem *infoItem);
	QList<QStandardItem *> makeRow(const QString &name, const QString &value, const QString &info) const;

	QPointer<component::Connection> m_conn;
	// Context of every live-update connection of the current build; deleted on rebuild.
	QObject *m_watchers = nullptr;
	QHash<const QStandardItem *, QPointer<QObject>> m_objects;
	QHash<const QObject *, QStandardItem *> m_items;
};

} // namespace scopy::dcdebugger

#endif // COMPONENTTREEMODEL_H
