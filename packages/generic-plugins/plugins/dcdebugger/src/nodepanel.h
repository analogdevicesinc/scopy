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

#include "scopy-dcdebugger_export.h"

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
class DockableAreaInterface;
class DockWrapperInterface;
class MenuSectionCollapseWidget;
} // namespace scopy

namespace scopy::dcdebugger {
class DCDebugger_API;

// Control widget shown right of the tree for the selected node. Built from sections; the
// per-kind sections are added by createNodePanel().
class NodePanel : public QWidget
{
	Q_OBJECT
	friend class DCDebugger_API;

public:
	NodePanel(QObject *node, NodeKind kind, const QString &title, const QList<PathTitle::Segment> &path,
		  QWidget *parent = nullptr);
	~NodePanel() override;

	QObject *node() const { return m_node; }
	NodeKind kind() const { return m_kind; }
	void addSection(QWidget *section);
	// Puts `view` in a tab before the "Attributes" tab the sections move to; the attributes stay
	// the current tab. May be called once per view. With docking (KDDockWidgets) every
	// view and the attributes are docks instead, tabbed at first: they can be split side by side
	// or floated.
	void setMainView(const QString &title, QWidget *view);
	// Stops the acquisitions / generations of the panel, docked or floating.
	void shutdown();

public Q_SLOTS:
	// The Refresh button: everything re-reads from the device.
	virtual void refresh();

Q_SIGNALS:
	// The Refresh button was used.
	void refreshRequested();
	// The panel was shown (node selected): sections update, reading only what ReadPolicy allows.
	void selected();
	void pathSelected(QObject *node);

protected:
	void showEvent(QShowEvent *event) override;

private:
	QPointer<QObject> m_node;
	NodeKind m_kind;
	QString m_path; // "ip:192.168.2.1/iio/cf-ad9361-lpc", for the dock titles
	QVBoxLayout *m_layout;
	QWidget *m_sectionsArea;
	QTabWidget *m_tabs = nullptr;
	DockableAreaInterface *m_area = nullptr;
	QList<DockWrapperInterface *> m_docks; // the attributes first, then the views
	QVBoxLayout *m_sections;

	void addDock(const QString &title, QWidget *widget);
};

// Q_PROPERTY values (+ class and tags) of one component, read from its metaobject.
class PropertiesSection : public QWidget
{
	Q_OBJECT
public:
	PropertiesSection(QObject *node, QWidget *parent = nullptr);
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
SCOPY_DCDEBUGGER_EXPORT NodePanel *createNodePanel(QObject *node, NodeKind kind, const QString &title,
						   const QList<PathTitle::Segment> &path,
						   const QList<QObject *> &children, QWidget *parent = nullptr);

QString variantToString(const QVariant &v);

MenuSectionCollapseWidget *makeSection(const QString &title, QWidget *parent, bool collapsed = false);
QTableWidget *makeTable(const QStringList &headers, QWidget *parent);

// Instrument views a node panel may show; each one can be turned off in the preferences.
namespace viewprefs {
constexpr const char *ACQUISITION = "dcdebugger_view_acquisition";
constexpr const char *GENERATOR = "dcdebugger_view_generator";
constexpr const char *MONITOR = "dcdebugger_view_monitor";
constexpr const char *REGISTERS = "dcdebugger_view_registers";

void initPreferences();
bool enabled(const char *id);
} // namespace viewprefs

} // namespace scopy::dcdebugger

#endif // NODEPANEL_H
