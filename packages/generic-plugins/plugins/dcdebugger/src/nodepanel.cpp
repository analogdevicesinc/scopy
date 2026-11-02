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
#include "attributessection.h"
#include "generation/generatorview.h"

#include <QHeaderView>
#include <QLabel>
#include <QMetaProperty>
#include <QScrollArea>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

#include <component/attribute.h>
#include <component/inputstream.h>
#include <component/outputstream.h>
#include <menusectionwidget.h>
#include <style.h>
#include <widgets/animatedrefreshbtn.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {
QTableWidget *makeTable(const QStringList &headers, QWidget *parent)
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
	m_layout = new QVBoxLayout(this);
	m_layout->setContentsMargins(0, 0, 0, 0);
	m_layout->setSpacing(6);
	QVBoxLayout *lay = m_layout;

	auto *header = new QWidget(this);
	auto *headerLay = new QHBoxLayout(header);
	headerLay->setContentsMargins(6, 6, 6, 0);
	auto *titles = new QVBoxLayout();
	titles->setSpacing(2);
	auto *titleLabel = new QLabel(title, header);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	auto *infoLabel = new QLabel(
		ComponentNode::kindName(kind) +
			(node && kind == NodeKind::Other ? " — " + ComponentNode::className(node) : QString()),
		header);
	Style::setStyle(infoLabel, style::properties::label::subtle);
	auto *pathTitle = new PathTitle(path, header);
	connect(pathTitle, &PathTitle::pathSelected, this, &NodePanel::pathSelected);
	titles->addWidget(pathTitle);
	titles->addWidget(titleLabel);
	titles->addWidget(infoLabel);
	headerLay->addLayout(titles, 1);

	auto *refreshBtn = new AnimatedRefreshBtn(false, header);
	refreshBtn->setToolTip("Refresh");
	headerLay->addWidget(refreshBtn, 0, Qt::AlignTop);
	connect(refreshBtn, &QPushButton::clicked, this, &NodePanel::refresh);
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

void NodePanel::setMainView(const QString &title, QWidget *view)
{
	m_layout->removeWidget(m_sectionsArea);
	auto *tabs = new QTabWidget(this);
	tabs->addTab(view, title);
	tabs->addTab(m_sectionsArea, "Attributes");
	m_layout->addWidget(tabs, 1);
}

void NodePanel::refresh() { Q_EMIT refreshRequested(); }

void NodePanel::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	refresh();
}

PropertiesSection::PropertiesSection(QObject *node, bool collapsed, QWidget *parent)
	: QWidget(parent)
	, m_node(node)
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	auto *section = makeSection("General info", this, collapsed);
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
	auto *props = new PropertiesSection(node, kind != NodeKind::Attribute && !attrs.isEmpty(), panel);
	QObject::connect(panel, &NodePanel::refreshRequested, props, &PropertiesSection::refresh);
	panel->addSection(props);

	if(auto *in = qobject_cast<component::InputStream *>(node)) {
		panel->setMainView("Acquisition", new AcquisitionView(in, panel));
	} else if(auto *out = qobject_cast<component::OutputStream *>(node)) {
		panel->setMainView("Generator", new GeneratorView(out, panel));
	}
	return panel;
}

#include "moc_nodepanel.cpp"
