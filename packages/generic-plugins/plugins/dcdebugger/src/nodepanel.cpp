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

#include "nodepanel.h"

#include "acquisition/acquisitionview.h"
#include "acquisition/streamacquisition.h"
#include "attributessection.h"
#include "generation/generatorview.h"
#include "generation/streamgeneration.h"
#include "monitor/monitorview.h"
#include "ping/pingview.h"
#include "registers/registerview.h"
#include "splitterpane.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMetaProperty>
#include <QSet>
#include <QScrollArea>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

#include <component/attribute.h>
#include <component/inputstream.h>
#include <component/outputstream.h>
#include <component/ping.h>
#include <gui/docking/dockablearea.h>
#include <gui/docking/dockwrapper.h>
#include <menusectionwidget.h>
#include <style.h>
#include <widgets/animatedrefreshbtn.h>

using namespace scopy;
using namespace scopy::dcdebugger;

QTableWidget *scopy::dcdebugger::makeTable(const QStringList &headers, QWidget *parent)
{
	auto *t = new QTableWidget(parent);
	t->setColumnCount(headers.size());
	t->setHorizontalHeaderLabels(headers);
	t->horizontalHeader()->setStretchLastSection(true);
	t->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	t->verticalHeader()->setVisible(false);
	t->setEditTriggers(QAbstractItemView::NoEditTriggers);
	t->setSelectionBehavior(QAbstractItemView::SelectRows);
	t->setSelectionMode(QAbstractItemView::SingleSelection);
	t->setWordWrap(false);
	Style::setStyle(t, style::properties::widget::tableViewWidget);
	return t;
}

namespace {

void setRow(QTableWidget *t, int row, const QStringList &cells)
{
	for(int c = 0; c < cells.size(); c++) {
		auto *item = new QTableWidgetItem(cells[c]);
		item->setToolTip(cells[c]);
		t->setItem(row, c, item);
	}
}

// Grows the table to its rows so the panel scrolls, not the table.
void fitHeight(QTableWidget *t)
{
	int h = t->horizontalHeader()->height() + 2 * t->frameWidth();
	for(int r = 0; r < t->rowCount(); ++r) {
		h += t->rowHeight(r);
	}
	t->setFixedHeight(h);
}

} // namespace

MenuSectionCollapseWidget *scopy::dcdebugger::makeSection(const QString &title, QWidget *parent, bool collapsed)
{
	auto *section = new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_ARROW,
						      MenuCollapseSection::MHW_BASEWIDGET, parent);
	section->contentLayout()->setSpacing(6);
	Style::setStyle(section, style::properties::widget::border_interactive);
	section->setCollapsed(collapsed);
	return section;
}

QString scopy::dcdebugger::variantToString(const QVariant &v)
{
	if(!v.isValid()) {
		return {};
	}
	const QMetaType t = v.metaType();
	if(t == QMetaType::fromType<QStringList>()) {
		return v.toStringList().join(", ");
	}
	if(t == QMetaType::fromType<QList<double>>()) {
		QStringList out;
		for(double d : v.value<QList<double>>()) {
			out.append(QString::number(d));
		}
		return "[" + out.join(", ") + "]";
	}
	if(t == QMetaType::fromType<QList<int>>()) {
		QStringList out;
		for(int i : v.value<QList<int>>()) {
			out.append(QString::number(i));
		}
		return "[" + out.join(", ") + "]";
	}
	if(t == QMetaType::fromType<QVariantMap>()) {
		QStringList out;
		const QVariantMap m = v.toMap();
		for(auto it = m.cbegin(); it != m.cend(); ++it) {
			out.append(it.key() + "=" + variantToString(it.value()));
		}
		return out.join("; ");
	}
	if(t == QMetaType::fromType<bool>()) {
		return v.toBool() ? "true" : "false";
	}
	if(v.canConvert<QString>()) {
		return v.toString();
	}
	return QStringLiteral("<%1>").arg(QString::fromLatin1(t.name()));
}

NodePanel::NodePanel(QObject *node, NodeKind kind, const QString &title, const QList<PathTitle::Segment> &path,
		     QWidget *parent)
	: QWidget(parent)
	, m_node(node)
	, m_kind(kind)
{
	QStringList names;
	for(const PathTitle::Segment &s : path) {
		names.append(s.first);
	}
	m_path = names.isEmpty() ? title : names.join(QLatin1Char('/'));

	m_layout = new QVBoxLayout(this);
	m_layout->setContentsMargins(0, 0, 0, 0);
	m_layout->setSpacing(6);
	QVBoxLayout *lay = m_layout;

	// Header: [path ... refresh] on the first row, the title and info below.
	auto *header = new QWidget(this);
	auto *headerLay = new QVBoxLayout(header);
	headerLay->setContentsMargins(6, 6, 6, 0);
	headerLay->setSpacing(2);
	auto *pathRow = new QWidget(header);
	pathRow->setObjectName("nodePanelPathRow");
	auto *pathLay = new QHBoxLayout(pathRow);
	pathLay->setContentsMargins(0, 0, 0, 0);
	pathLay->setSpacing(6);
	auto *titleLabel = new QLabel(title, header);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	auto *infoLabel = new QLabel(
		ComponentNode::kindName(kind) +
			(node && kind == NodeKind::Other ? " — " + ComponentNode::className(node) : QString()),
		header);
	Style::setStyle(infoLabel, style::properties::label::subtle);
	auto *pathTitle = new PathTitle(path, pathRow);
	connect(pathTitle, &PathTitle::pathSelected, this, &NodePanel::pathSelected);
	pathLay->addWidget(pathTitle, 1, Qt::AlignVCenter);

	// As tall as the path buttons, on their line.
	auto *refreshBtn = new AnimatedRefreshBtn(true, pathRow);
	refreshBtn->setToolTip("Refresh");
	const int h = PathTitle::buttonHeight();
	refreshBtn->setFixedHeight(h);
	refreshBtn->setIconSize(QSize(h - 6, h - 6));
	pathLay->addWidget(refreshBtn, 0, Qt::AlignVCenter);
	connect(refreshBtn, &QPushButton::clicked, this, &NodePanel::refresh);

	headerLay->addWidget(pathRow);
	headerLay->addWidget(titleLabel);
	headerLay->addWidget(infoLabel);
	lay->addWidget(header);

	auto *scroll = new QScrollArea(this);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	auto *content = new QWidget(scroll);
	m_sections = new QVBoxLayout(content);
	m_sections->setContentsMargins(0, 0, 0, 0);
	m_sections->setSpacing(10);
	m_sections->addStretch();
	scroll->setWidget(content);
	lay->addWidget(scroll, 1);
	m_sectionsArea = scroll;
}

void NodePanel::addSection(QWidget *section) { m_sections->insertWidget(m_sections->count() - 1, section); }

NodePanel::~NodePanel()
{
	// A floating dock is a window of its own, not a child of the panel: it would outlive it.
	for(DockWrapperInterface *dock : std::as_const(m_docks)) {
		delete dynamic_cast<QWidget *>(dock);
	}
}

void NodePanel::setMainView(const QString &title, QWidget *view)
{
	if(!m_tabs && !m_area) {
		m_layout->removeWidget(m_sectionsArea);
		// The docks: KDDockWidgets when available and preferred (their own area: they only dock
		// with each other), else the classic tabs.
		DockableAreaInterface *area = createDockableArea(this);
		if(dynamic_cast<classic::DockableArea *>(area)) {
			delete area->asWidget();
			m_tabs = new QTabWidget(this);
			m_tabs->addTab(m_sectionsArea, "Attributes");
			m_layout->addWidget(m_tabs, 1);
		} else {
			m_area = area;
			m_layout->addWidget(m_area->asWidget(), 1);
			addDock("Attributes", m_sectionsArea);
		}
	}
	// No minimum of its own: a large view (plots, menus) scrolls inside its tab instead of
	// growing the panel, which would scroll even on the attributes tab.
	auto *page = new SplitterPane(view, this);
	if(m_area) {
		addDock(title, page);
		return;
	}
	m_tabs->insertTab(m_tabs->count() - 1, page, title);
	m_tabs->setCurrentWidget(m_sectionsArea);
}

void NodePanel::addDock(const QString &title, QWidget *widget)
{
	DockWrapperInterface *dock = createDockWrapper(title);
	dock->setTitle(title);
	dock->setInnerWidget(widget);
	if(m_docks.isEmpty()) {
		m_area->addDockWrapper(dock, DockableAreaInterface::Direction_RIGHT);
		m_docks.append(dock);
		return;
	}
	m_docks.append(dock);
#ifdef USE_KDDOCKWIDGETS
	// Tabbed with the attributes, like the classic tabs; the attributes stay the current tab.
	auto *first = dynamic_cast<KDDockWidgets::QtWidgets::DockWidget *>(m_docks.first());
	auto *added = dynamic_cast<KDDockWidgets::QtWidgets::DockWidget *>(dock);
	if(first && added) {
		added->setAffinities(first->affinities());
		first->addDockWidgetAsTab(added);
		first->setAsCurrentTab();
		return;
	}
#endif
	m_area->addDockWrapper(dock, DockableAreaInterface::Direction_RIGHT);
}

void NodePanel::shutdown()
{
	// Floating docks are not children of the panel: look in them too.
	QList<QWidget *> roots{this};
	for(DockWrapperInterface *dock : std::as_const(m_docks)) {
		if(auto *w = dynamic_cast<QWidget *>(dock)) {
			roots.append(w);
		}
	}
	QSet<StreamAcquisition *> acquisitions;
	QSet<StreamGeneration *> generations;
	for(QWidget *root : std::as_const(roots)) {
		for(StreamAcquisition *acq : root->findChildren<StreamAcquisition *>()) {
			acquisitions.insert(acq);
		}
		for(StreamGeneration *gen : root->findChildren<StreamGeneration *>()) {
			generations.insert(gen);
		}
	}
	for(StreamAcquisition *acq : std::as_const(acquisitions)) {
		acq->shutdown();
	}
	for(StreamGeneration *gen : std::as_const(generations)) {
		gen->shutdown();
	}
}

void NodePanel::refresh() { Q_EMIT refreshRequested(); }

void NodePanel::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	Q_EMIT selected();
}

PropertiesSection::PropertiesSection(QObject *node, QWidget *parent)
	: QWidget(parent)
	, m_node(node)
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	auto *section = makeSection("General info", this);
	m_table = makeTable({"Property", "Value"}, section);
	section->contentLayout()->addWidget(m_table);
	lay->addWidget(section);
	refresh();
}

void PropertiesSection::refresh()
{
	if(!m_node) {
		m_table->setRowCount(0);
		return;
	}
	QList<QStringList> rows;
	rows.append({"class", QString::fromLatin1(m_node->metaObject()->className())});
	const QMetaObject *mo = m_node->metaObject();
	for(int i = QObject::staticMetaObject.propertyCount(); i < mo->propertyCount(); ++i) {
		const QMetaProperty p = mo->property(i);
		rows.append({QString::fromLatin1(p.name()), variantToString(p.read(m_node))});
	}
	for(const QByteArray &name : m_node->dynamicPropertyNames()) {
		rows.append({QString::fromLatin1(name), variantToString(m_node->property(name))});
	}
	m_table->setRowCount(rows.size());
	for(int r = 0; r < rows.size(); ++r) {
		setRow(m_table, r, rows[r]);
	}
	fitHeight(m_table);
}

ChildrenSection::ChildrenSection(const QString &title, const QList<QObject *> &children, QWidget *parent)
	: QWidget(parent)
{
	for(QObject *c : children) {
		m_children.append(c);
	}
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	auto *section = makeSection(title, this);
	m_table = makeTable({"Name", "Value", "Info"}, section);
	section->contentLayout()->addWidget(m_table);
	lay->addWidget(section);
	refresh();
}

void ChildrenSection::refresh()
{
	m_table->setRowCount(m_children.size());
	for(int r = 0; r < m_children.size(); ++r) {
		QObject *c = m_children[r];
		if(!c) {
			setRow(m_table, r, {"<destroyed>", {}, {}});
			continue;
		}
		const NodeKind kind = ComponentNode::kindOf(c);
		setRow(m_table, r,
		       {ComponentNode::name(c, kind), ComponentNode::value(c, kind), ComponentNode::info(c, kind)});
	}
	fitHeight(m_table);
}

namespace {
QList<component::Attribute *> attributesOf(const QList<QObject *> &objects)
{
	QList<component::Attribute *> out;
	for(QObject *o : objects) {
		if(auto *a = qobject_cast<component::Attribute *>(o)) {
			out.append(a);
		}
	}
	return out;
}

void addAttributes(NodePanel *panel, const QString &title, const QList<component::Attribute *> &attrs)
{
	auto *section = new AttributesSection(title, attrs, panel);
	QObject::connect(panel, &NodePanel::refreshRequested, section, &AttributesSection::refresh);
	QObject::connect(panel, &NodePanel::selected, section, &AttributesSection::autoRefresh);
	panel->addSection(section);
}
} // namespace

NodePanel *scopy::dcdebugger::createNodePanel(QObject *node, NodeKind kind, const QString &title,
					      const QList<PathTitle::Segment> &path, const QList<QObject *> &children,
					      QWidget *parent)
{
	auto *panel = new NodePanel(node, kind, title, path, parent);
	if(kind == NodeKind::Group) {
		// A group of attributes is edited in place; any other group lists its members.
		const QList<component::Attribute *> attrs = attributesOf(children);
		if(!attrs.isEmpty() && attrs.size() == children.size()) {
			addAttributes(panel, QStringLiteral("Attributes (%1)").arg(attrs.size()), attrs);
			return panel;
		}
		auto *members = new ChildrenSection("Members", children, panel);
		QObject::connect(panel, &NodePanel::refreshRequested, members, &ChildrenSection::refresh);
		QObject::connect(panel, &NodePanel::selected, members, &ChildrenSection::refresh);
		panel->addSection(members);
		return panel;
	}

	// The direct attributes of the node (or the attribute itself), then its general info.
	QList<component::Attribute *> attrs;
	if(auto *attr = qobject_cast<component::Attribute *>(node)) {
		attrs.append(attr);
	} else {
		attrs = node->findChildren<component::Attribute *>(QString(), Qt::FindDirectChildrenOnly);
	}
	if(!attrs.isEmpty()) {
		addAttributes(panel,
			      kind == NodeKind::Attribute ? QStringLiteral("Attribute")
							  : QStringLiteral("Attributes (%1)").arg(attrs.size()),
			      attrs);
	}
	auto *props = new PropertiesSection(node, panel);
	QObject::connect(panel, &NodePanel::refreshRequested, props, &PropertiesSection::refresh);
	QObject::connect(panel, &NodePanel::selected, props, &PropertiesSection::refresh);
	panel->addSection(props);

	if(auto *in = qobject_cast<component::InputStream *>(node)) {
		panel->setMainView("Acquisition", new AcquisitionView(in, panel));
	} else if(auto *out = qobject_cast<component::OutputStream *>(node)) {
		panel->setMainView("Generator", new GeneratorView(out, panel));
	} else if(auto *ping = qobject_cast<component::Ping *>(node)) {
		auto *view = new PingView(ping, panel);
		QObject::connect(panel, &NodePanel::refreshRequested, view, &PingView::refresh);
		QObject::connect(panel, &NodePanel::selected, view, &PingView::refresh);
		panel->setMainView("Ping", view);
	} else if((kind == NodeKind::Context || kind == NodeKind::Device) &&
		  !MonitorView::monitorableChannels(node).isEmpty()) {
		panel->setMainView("Monitor", new MonitorView(node, panel));
	}
	if((kind == NodeKind::Context || kind == NodeKind::Device) && !RegisterView::targetsOf(node).isEmpty()) {
		panel->setMainView("Registers", new RegisterView(node, panel));
	}
	return panel;
}

#include "moc_nodepanel.cpp"
