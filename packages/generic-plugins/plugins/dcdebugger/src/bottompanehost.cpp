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

#include "bottompanehost.h"

#include <QEvent>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <dcdebugger/style_properties.h>
#include <style.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {
constexpr int MAX_SHORTCUTS = 9;

QString shortcutText(int n)
{
	return QKeySequence(Qt::ALT | Qt::Key(Qt::Key_1 + n)).toString(QKeySequence::NativeText);
}
} // namespace

BottomPaneHost::BottomPaneHost(QWidget *mainWidget, QWidget *parent)
	: QWidget(parent)
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(0);

	m_splitter = new QSplitter(Qt::Vertical, this);
	// Either side may be dragged away completely; a collapsed pane closes (see syncFromSplitter).
	m_splitter->setChildrenCollapsible(true);
	m_splitter->addWidget(mainWidget);

	// The pane area: the open pane, inset from the tool edges like the panel above it.
	m_paneArea = new QWidget(m_splitter);
	auto *areaLay = new QVBoxLayout(m_paneArea);
	areaLay->setContentsMargins(6, 0, 6, 6);
	areaLay->setSpacing(0);
	m_stack = new QStackedWidget(m_paneArea);
	areaLay->addWidget(m_stack, 1);
	m_splitter->addWidget(m_paneArea);
	m_splitter->setStretchFactor(0, 1);
	m_splitter->setStretchFactor(1, 0);
	m_paneArea->hide();
	// A pane dragged fully down closes when the handle is released.
	m_splitter->handle(1)->installEventFilter(this);

	// The bar: one checkable tab per pane, always visible.
	m_bar = new QFrame(this);
	Style::setStyle(m_bar, style::properties::dcdebugger::paneBar);
	m_bar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_barLay = new QHBoxLayout(m_bar);
	m_barLay->setContentsMargins(6, 3, 6, 3);
	m_barLay->setSpacing(4);
	m_barLay->addStretch();

	lay->addWidget(m_splitter, 1);
	lay->addWidget(m_bar);

	// Queued: handled after the splitter's own move handling.
	connect(m_splitter, &QSplitter::splitterMoved, this, &BottomPaneHost::syncFromSplitter, Qt::QueuedConnection);
}

BottomPaneHost::~BottomPaneHost() = default;

QAbstractButton *BottomPaneHost::addPane(const QString &id, const QString &title, QWidget *pane)
{
	if(find(id)) {
		return nullptr;
	}
	Pane p;
	p.id = id;
	p.title = title;
	p.index = m_panes.size();
	p.widget = pane;
	p.tab = new QPushButton(m_bar);
	p.tab->setCheckable(true);
	p.tab->setFocusPolicy(Qt::NoFocus);
	p.tab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	p.tab->setIconSize(QSize(8, 8));
	Style::setStyle(p.tab, style::properties::dcdebugger::paneTab);
	m_barLay->insertWidget(m_panes.size(), p.tab);
	p.tab->show(); // also when added to a visible bar
	m_stack->addWidget(pane);
	connect(p.tab, &QPushButton::clicked, this, [this, id]() { togglePane(id); });

	if(p.index < MAX_SHORTCUTS) {
		auto *sc = new QShortcut(QKeySequence(Qt::ALT | Qt::Key(Qt::Key_1 + p.index)), this);
		sc->setContext(Qt::WidgetWithChildrenShortcut);
		connect(sc, &QShortcut::activated, this, [this, id]() { togglePane(id); });
	}
	m_panes.append(p);
	updateTab(m_panes.last());
	return p.tab;
}

const BottomPaneHost::Pane *BottomPaneHost::find(const QString &id) const
{
	for(const Pane &p : m_panes) {
		if(p.id == id) {
			return &p;
		}
	}
	return nullptr;
}

BottomPaneHost::Pane *BottomPaneHost::find(const QString &id)
{
	return const_cast<Pane *>(std::as_const(*this).find(id));
}

void BottomPaneHost::setPaneBadge(const QString &id, const QString &badge)
{
	Pane *p = find(id);
	if(p && p->badge != badge) {
		p->badge = badge;
		updateTab(*p);
	}
}

void BottomPaneHost::setPaneIndicator(const QString &id, bool on, const QString &toolTip)
{
	Pane *p = find(id);
	if(!p) {
		return;
	}
	if(p->indicator != on || p->indicatorTip != toolTip) {
		p->indicator = on;
		p->indicatorTip = toolTip;
		updateTab(*p);
	}
}

bool BottomPaneHost::paneIndicator(const QString &id) const
{
	const Pane *p = find(id);
	return p && p->indicator;
}

QWidget *BottomPaneHost::pane(const QString &id) const
{
	const Pane *p = find(id);
	return p ? p->widget : nullptr;
}

QAbstractButton *BottomPaneHost::tabButton(const QString &id) const
{
	const Pane *p = find(id);
	return p ? p->tab : nullptr;
}

QWidget *BottomPaneHost::bar() const { return m_bar; }

void BottomPaneHost::updateTab(const Pane &p)
{
	p.tab->setText(p.badge.isEmpty() ? p.title : QStringLiteral("%1  %2").arg(p.title, p.badge));
	p.tab->setIcon(p.indicator ? QIcon(Style::getPixmap(":/gui/icons/circle_led.svg",
							    QColor(Style::getAttribute(json::global::led_error))))
				   : QIcon());
	QString tip = p.title;
	if(p.index < MAX_SHORTCUTS) {
		tip += QStringLiteral(" (%1)").arg(shortcutText(p.index));
	}
	if(p.indicator && !p.indicatorTip.isEmpty()) {
		tip += QStringLiteral("\n") + p.indicatorTip;
	}
	p.tab->setToolTip(tip);
}

void BottomPaneHost::syncTabs()
{
	for(const Pane &p : std::as_const(m_panes)) {
		p.tab->setChecked(p.id == m_current);
	}
}

void BottomPaneHost::rememberHeight()
{
	// A collapsed pane (0) keeps the previous height.
	const QList<int> sizes = m_splitter->sizes();
	if(isPaneOpen() && sizes.size() > 1 && sizes.at(1) > 0) {
		m_paneHeight = sizes.at(1);
	}
}

void BottomPaneHost::syncFromSplitter()
{
	rememberHeight(); // a collapsed pane keeps the previous height for the next open
	// Hiding the area also hides the handle: not under a mouse still dragging it.
	if(isPaneCollapsed() && !(QGuiApplication::mouseButtons() & Qt::LeftButton)) {
		hidePane();
	}
}

bool BottomPaneHost::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonRelease) {
		// After the splitter's own release handling.
		QMetaObject::invokeMethod(this, &BottomPaneHost::syncFromSplitter, Qt::QueuedConnection);
	}
	return QWidget::eventFilter(watched, event);
}

bool BottomPaneHost::isPaneCollapsed() const
{
	const QList<int> sizes = m_splitter->sizes();
	return isPaneOpen() && sizes.size() > 1 && sizes.at(1) == 0;
}

void BottomPaneHost::restorePaneHeight()
{
	const int total = m_splitter->height() - m_splitter->handleWidth();
	if(total > 0) {
		const int h = qBound(1, m_paneHeight, qMax(1, total - 100));
		m_splitter->setSizes({total - h, h});
	} else {
		// Not laid out yet: the sizes act as preferred sizes, the main widget takes the rest.
		m_splitter->setSizes({m_paneHeight * 2, m_paneHeight});
	}
}

void BottomPaneHost::showPane(const QString &id)
{
	const Pane *p = find(id);
	if(!p) {
		return;
	}
	if(m_current == id) {
		syncTabs();
		return;
	}
	const bool wasOpen = isPaneOpen();
	m_current = id;
	m_stack->setCurrentWidget(p->widget);
	if(!wasOpen) {
		m_paneArea->show();
		restorePaneHeight();
	}
	syncTabs();
	Q_EMIT currentPaneChanged(m_current);
}

void BottomPaneHost::togglePane(const QString &id)
{
	if(m_current == id && isPaneCollapsed()) {
		restorePaneHeight(); // collapsed by setSizes() and not synced yet: the tab brings it back
		syncTabs();
	} else if(m_current == id) {
		hidePane();
	} else {
		showPane(id);
	}
}

void BottomPaneHost::hidePane()
{
	if(!isPaneOpen()) {
		syncTabs();
		return;
	}
	rememberHeight();
	m_current.clear();
	m_paneArea->hide();
	syncTabs();
	Q_EMIT currentPaneChanged(m_current);
}

#include "moc_bottompanehost.cpp"
