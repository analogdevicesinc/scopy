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

#ifndef SIDESPLITTER_H
#define SIDESPLITTER_H

#include "scopy-dcdebugger_export.h"

#include <QSplitter>
#include <QVariant>

#include <functional>

namespace scopy {
class DockWrapperInterface;
class MapStackedWidget;
} // namespace scopy

namespace scopy::dcdebugger {

// Splitter arrangement kept between sessions, as the "layout" property of DCDebugger_API (saved and
// loaded with the plugin's session settings, only with "Save session" on). Views read it when built
// and update it as the user moves a handle, so it always holds the latest arrangement.
namespace layoutstate {
SCOPY_DCDEBUGGER_EXPORT QVariant value(const QString &key, const QVariant &defaultValue = {});
SCOPY_DCDEBUGGER_EXPORT void setValue(const QString &key, const QVariant &value);
// Runs before every save while `owner` lives: for state that is only read on demand.
SCOPY_DCDEBUGGER_EXPORT void addCollector(QObject *owner, std::function<void()> collect);
// Every value, after running the collectors; and replacing them all (e.g. the API's "layout").
SCOPY_DCDEBUGGER_EXPORT QVariantMap values();
SCOPY_DCDEBUGGER_EXPORT void setValues(const QVariantMap &values);

// Two KDDockWidgets docks of one area: where `second` is relative to `first` (side left / right /
// top / bottom, or tabbed with it), its size along that side, the current tab and the floating
// geometries. Empty without KDDockWidgets, before the docks were laid out or with `second` closed.
SCOPY_DCDEBUGGER_EXPORT QVariantMap dockPairState(DockWrapperInterface *first, DockWrapperInterface *second);
// Adds `second` to `mainWindow`, where `first` already is, as `state` says (defaultSide /
// defaultSize without one), then floats what was floating. No-op without KDDockWidgets.
SCOPY_DCDEBUGGER_EXPORT void placeDockPair(QWidget *mainWindow, DockWrapperInterface *first,
					   DockWrapperInterface *second, const QVariantMap &state,
					   const QString &defaultSide, int defaultSize);
// Resizes the docked `second` within the layout back to its size in `state` (once laid out).
SCOPY_DCDEBUGGER_EXPORT void fitDockPair(DockWrapperInterface *second, const QVariantMap &state);
} // namespace layoutstate

// Instrument body as a splitter: [left stack (channels)] [central] [right stack (menus)], like the
// ADC instrument. The handles resize the side stacks or hide them (dragged to the edge); a hidden
// stack comes back at its last width. Showing a page of the right stack opens it.
class SideSplitter : public QSplitter
{
	Q_OBJECT
public:
	// No left stack for leftWidth == 0. The side widths and open states are kept per stateKey
	// (one per instrument type), see layoutstate.
	SideSplitter(int leftWidth, int rightWidth, const QString &stateKey, QWidget *parent = nullptr);

	// Between the stacks; once.
	void setCentralWidget(QWidget *central);

	MapStackedWidget *leftStack() const { return m_left; }
	MapStackedWidget *rightStack() const { return m_right; }
	bool isRightOpen() const;

public Q_SLOTS:
	void setLeftOpen(bool open);
	void setRightOpen(bool open);

Q_SIGNALS:
	// Also when the right stack is dragged closed / open.
	void rightOpenChanged(bool open);

private:
	void setOpen(QWidget *side, int &width, bool open);
	void onMoved();
	void storeState();

	MapStackedWidget *m_left = nullptr;
	MapStackedWidget *m_right;
	QWidget *m_central = nullptr;
	int m_leftWidth;
	int m_rightWidth;
	bool m_leftOpen = true;
	bool m_rightOpen = true;
	QString m_stateKey;
};

} // namespace scopy::dcdebugger

#endif // SIDESPLITTER_H
