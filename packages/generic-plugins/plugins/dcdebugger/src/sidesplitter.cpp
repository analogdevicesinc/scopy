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

#include "sidesplitter.h"

#include <QPointer>
#include <QRect>
#include <QVariantMap>

#include <gui/docking/dockwrapperinterface.h>
#include <gui/mapstackedwidget.h>

#include <algorithm>
#include <cstdlib>

#ifdef USE_KDDOCKWIDGETS
#include <kddockwidgets/core/DockWidget.h>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>
#endif

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {
QVariantMap &layoutValues()
{
	static QVariantMap values;
	return values;
}

// dockPairState keys.
constexpr const char *DOCK_SIDE = "side";		// left / right / top / bottom / tab
constexpr const char *DOCK_SIZE = "size";		// second's width (left, right) or height (top, bottom)
constexpr const char *DOCK_CURRENT = "current";		// tabbed: the shown one, first / second
constexpr const char *FIRST_FLOATING = "firstFloating"; // floating window geometry, if floating
constexpr const char *SECOND_FLOATING = "secondFloating";

QList<QPair<QPointer<QObject>, std::function<void()>>> &collectors()
{
	static QList<QPair<QPointer<QObject>, std::function<void()>>> list;
	return list;
}
} // namespace

QVariant layoutstate::value(const QString &key, const QVariant &defaultValue)
{
	return layoutValues().value(key, defaultValue);
}

void layoutstate::setValue(const QString &key, const QVariant &value) { layoutValues().insert(key, value); }

void layoutstate::addCollector(QObject *owner, std::function<void()> collect)
{
	collectors().append({QPointer<QObject>(owner), std::move(collect)});
}

QVariantMap layoutstate::values()
{
	auto &list = collectors();
	list.removeIf([](const auto &c) { return c.first.isNull(); });
	for(const auto &c : std::as_const(list)) {
		c.second();
	}
	return layoutValues();
}

void layoutstate::setValues(const QVariantMap &values) { layoutValues() = values; }

#ifdef USE_KDDOCKWIDGETS
namespace {
KDDockWidgets::QtWidgets::DockWidget *kdDock(DockWrapperInterface *dock)
{
	return dynamic_cast<KDDockWidgets::QtWidgets::DockWidget *>(dock);
}
} // namespace
#endif

QVariantMap layoutstate::dockPairState(DockWrapperInterface *first, DockWrapperInterface *second)
{
	QVariantMap state;
#ifdef USE_KDDOCKWIDGETS
	auto *firstView = kdDock(first);
	auto *secondView = kdDock(second);
	if(!firstView || !secondView || !firstView->isOpen() || !secondView->isOpen()) {
		return state;
	}
	KDDockWidgets::Core::DockWidget *a = firstView->asDockWidgetController();
	KDDockWidgets::Core::DockWidget *b = secondView->asDockWidgetController();
	if(a->isFloating()) {
		state.insert(FIRST_FLOATING, static_cast<QWidget *>(firstView)->window()->geometry());
	}
	if(b->isFloating()) {
		state.insert(SECOND_FLOATING, static_cast<QWidget *>(secondView)->window()->geometry());
	}
	if(a->isFloating() || b->isFloating()) {
		return state;
	}
	if(a->isTabbed() && b->isTabbed()) {
		state.insert(DOCK_SIDE, "tab");
		state.insert(DOCK_CURRENT, b->isCurrentTab() ? "second" : "first");
		return state;
	}
	const QRect p = a->groupGeometry();
	const QRect t = b->groupGeometry();
	if(p.isEmpty() || t.isEmpty()) {
		return {}; // not laid out yet
	}
	const int dx = t.center().x() - p.center().x();
	const int dy = t.center().y() - p.center().y();
	if(std::abs(dx) > std::abs(dy)) {
		state.insert(DOCK_SIDE, dx > 0 ? "right" : "left");
		state.insert(DOCK_SIZE, t.width());
	} else {
		state.insert(DOCK_SIDE, dy > 0 ? "bottom" : "top");
		state.insert(DOCK_SIZE, t.height());
	}
#else
	Q_UNUSED(first);
	Q_UNUSED(second);
#endif
	return state;
}

void layoutstate::placeDockPair(QWidget *mainWindow, DockWrapperInterface *first, DockWrapperInterface *second,
				const QVariantMap &state, const QString &defaultSide, int defaultSize)
{
#ifdef USE_KDDOCKWIDGETS
	auto *mw = dynamic_cast<KDDockWidgets::QtWidgets::MainWindow *>(mainWindow);
	auto *firstView = kdDock(first);
	auto *secondView = kdDock(second);
	if(!mw || !firstView || !secondView) {
		return;
	}
	secondView->setAffinities({mw->uniqueName()});

	const QString side = state.value(DOCK_SIDE, defaultSide).toString();
	if(side == QLatin1String("tab")) {
		firstView->addDockWidgetAsTab(secondView);
		(state.value(DOCK_CURRENT).toString() == QLatin1String("second") ? secondView : firstView)
			->setAsCurrentTab();
	} else {
		const bool horizontal = side == QLatin1String("left") || side == QLatin1String("right");
		const int size = state.value(DOCK_SIZE, defaultSize).toInt();
		KDDockWidgets::Location location = KDDockWidgets::Location_OnBottom;
		if(side == QLatin1String("left")) {
			location = KDDockWidgets::Location_OnLeft;
		} else if(side == QLatin1String("right")) {
			location = KDDockWidgets::Location_OnRight;
		} else if(side == QLatin1String("top")) {
			location = KDDockWidgets::Location_OnTop;
		}
		// Added directly, not through the area: only this way it gets its initial size.
		mw->addDockWidget(secondView, location, nullptr,
				  KDDockWidgets::InitialOption(horizontal ? QSize(size, 0) : QSize(0, size)));
	}

	// Floated last: docked first, a floating dock knows where to go back to.
	const QList<QPair<KDDockWidgets::QtWidgets::DockWidget *, const char *>> floating = {
		{firstView, FIRST_FLOATING}, {secondView, SECOND_FLOATING}};
	for(const auto &[view, key] : floating) {
		const QRect geometry = state.value(key).toRect();
		if(geometry.isValid()) {
			view->asDockWidgetController()->setFloatingGeometry(geometry);
			view->asDockWidgetController()->setFloating(true);
		}
	}
#else
	Q_UNUSED(mainWindow);
	Q_UNUSED(first);
	Q_UNUSED(second);
	Q_UNUSED(state);
	Q_UNUSED(defaultSide);
	Q_UNUSED(defaultSize);
#endif
}

void layoutstate::fitDockPair(DockWrapperInterface *second, const QVariantMap &state)
{
#ifdef USE_KDDOCKWIDGETS
	auto *view = kdDock(second);
	const QString side = state.value(DOCK_SIDE).toString();
	const int size = state.value(DOCK_SIZE, 0).toInt();
	if(!view || !view->isOpen() || size <= 0) {
		return;
	}
	KDDockWidgets::Core::DockWidget *dock = view->asDockWidgetController();
	if(dock->isFloating()) {
		return;
	}
	const QSize current = dock->sizeInLayout();
	// The edge facing the other dock moves; positive grows.
	if(side == QLatin1String("right")) {
		dock->resizeInLayout(size - current.width(), 0, 0, 0);
	} else if(side == QLatin1String("left")) {
		dock->resizeInLayout(0, 0, size - current.width(), 0);
	} else if(side == QLatin1String("bottom")) {
		dock->resizeInLayout(0, size - current.height(), 0, 0);
	} else if(side == QLatin1String("top")) {
		dock->resizeInLayout(0, 0, 0, size - current.height());
	}
#else
	Q_UNUSED(second);
	Q_UNUSED(state);
#endif
}

SideSplitter::SideSplitter(int leftWidth, int rightWidth, const QString &stateKey, QWidget *parent)
	: QSplitter(Qt::Horizontal, parent)
	, m_leftWidth(leftWidth)
	, m_rightWidth(rightWidth)
	, m_stateKey(stateKey)
{
	const QString key = m_stateKey + QStringLiteral("/");
	m_leftWidth = std::max(1, layoutstate::value(key + "leftWidth", m_leftWidth).toInt());
	m_rightWidth = std::max(1, layoutstate::value(key + "rightWidth", m_rightWidth).toInt());
	m_leftOpen = layoutstate::value(key + "leftOpen", true).toBool();
	m_rightOpen = layoutstate::value(key + "rightOpen", true).toBool();

	setChildrenCollapsible(true);
	if(leftWidth > 0) {
		m_left = new MapStackedWidget(this);
		addWidget(m_left);
	}
	m_right = new MapStackedWidget(this);
	addWidget(m_right);

	connect(this, &QSplitter::splitterMoved, this, &SideSplitter::onMoved);
	connect(m_right, &QStackedWidget::currentChanged, this, [this]() { setRightOpen(true); });
}

void SideSplitter::setCentralWidget(QWidget *central)
{
	m_central = central;
	insertWidget(indexOf(m_right), central);
	// Extra width goes to the central widget, which never collapses.
	QList<int> s;
	for(int i = 0; i < count(); ++i) {
		QWidget *w = widget(i);
		setStretchFactor(i, w == central ? 1 : 0);
		if(w == m_left) {
			s.append(m_leftOpen ? m_leftWidth : 0);
		} else if(w == m_right) {
			s.append(m_rightOpen ? m_rightWidth : 0);
		} else {
			s.append(central->sizeHint().width());
		}
	}
	setCollapsible(indexOf(central), false);
	setSizes(s);
}

bool SideSplitter::isRightOpen() const { return m_rightOpen; }

void SideSplitter::setLeftOpen(bool open)
{
	if(m_left) {
		setOpen(m_left, m_leftWidth, open);
		onMoved();
	}
}

void SideSplitter::setRightOpen(bool open)
{
	setOpen(m_right, m_rightWidth, open);
	onMoved();
}

void SideSplitter::setOpen(QWidget *side, int &width, bool open)
{
	// Before the first layout the sizes are not real yet; the initial ones apply.
	if(!m_central || !isVisible()) {
		return;
	}
	QList<int> s = sizes();
	const int i = indexOf(side);
	const int c = indexOf(m_central);
	if(open == (s[i] > 0)) {
		return;
	}
	if(open) {
		s[i] = width;
		s[c] = std::max(0, s[c] - width);
	} else {
		width = s[i];
		s[c] += s[i];
		s[i] = 0;
	}
	setSizes(s);
}

void SideSplitter::onMoved()
{
	if(!m_central || !isVisible()) {
		return;
	}
	const QList<int> s = sizes();
	// The widths a hidden stack comes back at.
	if(m_left && s[indexOf(m_left)] > 0) {
		m_leftWidth = s[indexOf(m_left)];
	}
	const int right = s[indexOf(m_right)];
	if(right > 0) {
		m_rightWidth = right;
	}
	if(m_left) {
		m_leftOpen = s[indexOf(m_left)] > 0;
	}
	const bool rightChanged = (right > 0) != m_rightOpen;
	m_rightOpen = right > 0;
	storeState();
	if(rightChanged) {
		Q_EMIT rightOpenChanged(m_rightOpen);
	}
}

void SideSplitter::storeState()
{
	if(m_stateKey.isEmpty()) {
		return;
	}
	const QString key = m_stateKey + QStringLiteral("/");
	layoutstate::setValue(key + "leftWidth", m_leftWidth);
	layoutstate::setValue(key + "rightWidth", m_rightWidth);
	layoutstate::setValue(key + "leftOpen", m_leftOpen);
	layoutstate::setValue(key + "rightOpen", m_rightOpen);
}

#include "moc_sidesplitter.cpp"
