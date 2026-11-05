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

#ifndef NODEPANEL_H
#define NODEPANEL_H

#include "componentnode.h"
#include "pathtitle.h"

#include <QList>
#include <QPointer>
#include <QWidget>

class QLabel;
class QTableWidget;
class QTabWidget;
class QVBoxLayout;

namespace scopy {
class MenuSectionCollapseWidget;
}

namespace scopy::dcdebugger {

// Control widget shown right of the tree for the selected node. Built from sections; the
// per-kind sections are added by createNodePanel().
class NodePanel : public QWidget
{
	Q_OBJECT
public:
	NodePanel(QObject *node, NodeKind kind, const QString &title, const QList<PathTitle::Segment> &path,
		  QWidget *parent = nullptr);

	QObject *node() const { return m_node; }
	NodeKind kind() const { return m_kind; }
	void addSection(QWidget *section);
	// Puts `view` in a first tab; the sections move to a second "Attributes" tab.
	// Tabs before "Attributes"; may be called once per view.
	void setMainView(const QString &title, QWidget *view);

public Q_SLOTS:
	virtual void refresh();

Q_SIGNALS:
	void refreshRequested();
	void pathSelected(QObject *node);

protected:
	void showEvent(QShowEvent *event) override;

private:
	QPointer<QObject> m_node;
	NodeKind m_kind;
	QVBoxLayout *m_layout;
	QWidget *m_sectionsArea;
	QTabWidget *m_tabs = nullptr;
	QVBoxLayout *m_sections;
};

// Q_PROPERTY values (+ class and tags) of one component, read from its metaobject.
class PropertiesSection : public QWidget
{
	Q_OBJECT
public:
	PropertiesSection(QObject *node, bool collapsed, QWidget *parent = nullptr);
	void refresh();

private:
	QPointer<QObject> m_node;
	QTableWidget *m_table;
};

// name / value / info of a list of components (the members of a group).
class ChildrenSection : public QWidget
{
	Q_OBJECT
public:
	ChildrenSection(const QString &title, const QList<QObject *> &children, QWidget *parent = nullptr);
	void refresh();

private:
	QList<QPointer<QObject>> m_children;
	QTableWidget *m_table;
};

// The panel for a component node (node != nullptr) or a group (node == nullptr, children set).
NodePanel *createNodePanel(QObject *node, NodeKind kind, const QString &title, const QList<PathTitle::Segment> &path,
			   const QList<QObject *> &children, QWidget *parent = nullptr);

QString variantToString(const QVariant &v);

MenuSectionCollapseWidget *makeSection(const QString &title, QWidget *parent, bool collapsed = false);
QTableWidget *makeTable(const QStringList &headers, QWidget *parent);

} // namespace scopy::dcdebugger

#endif // NODEPANEL_H
