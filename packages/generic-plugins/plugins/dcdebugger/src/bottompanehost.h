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

#ifndef BOTTOMPANEHOST_H
#define BOTTOMPANEHOST_H

#include "scopy-dcdebugger_export.h"

#include <QList>
#include <QString>
#include <QWidget>

class QAbstractButton;
class QFrame;
class QHBoxLayout;
class QPushButton;
class QSplitter;
class QStackedWidget;

namespace scopy::dcdebugger {

// IDE-style output panes: the main widget on top, at most one pane below it (vertical splitter)
// and an always visible bar of checkable tabs at the bottom (no close button: the tab is the
// toggle). A tab click opens its pane, a second click hides it; another tab switches panes.
// Alt+<n> toggles the n-th pane. Every pane starts hidden. Both splitter children collapse: a pane
// dragged fully down is closed (its tab unchecked) once the drag ends; the last non-zero pane
// height is kept while the host lives and reused on the next open.
class SCOPY_DCDEBUGGER_EXPORT BottomPaneHost : public QWidget
{
	Q_OBJECT
public:
	static constexpr int DEFAULT_PANE_HEIGHT = 280;

	explicit BottomPaneHost(QWidget *mainWidget, QWidget *parent = nullptr);
	~BottomPaneHost() override;

	// Adds a tab and its (hidden) pane; the host takes the pane. Returns the tab.
	QAbstractButton *addPane(const QString &id, const QString &title, QWidget *pane);

	// Text after the title on the tab ("" for none), e.g. an entry count.
	void setPaneBadge(const QString &id, const QString &badge);
	// A dot before the title on the tab, e.g. "recording"; the tooltip explains it.
	void setPaneIndicator(const QString &id, bool on, const QString &toolTip = {});
	bool paneIndicator(const QString &id) const;

	// Id of the open pane; empty when none is.
	QString currentPane() const { return m_current; }
	bool isPaneOpen() const { return !m_current.isEmpty(); }
	QWidget *pane(const QString &id) const;
	QAbstractButton *tabButton(const QString &id) const;
	QWidget *paneArea() const { return m_paneArea; }
	QWidget *bar() const;
	QSplitter *splitter() const { return m_splitter; }
	// Height the pane opens with (the last non-zero one it had).
	int paneHeight() const { return m_paneHeight; }

public Q_SLOTS:
	void showPane(const QString &id);
	void togglePane(const QString &id);
	void hidePane();
	// Follows the splitter sizes: any non-zero pane height is remembered; a collapsed pane
	// (height 0) is hidden, unless the splitter handle is still being dragged (then on release).
	// Runs on QSplitter::splitterMoved (drag only); call it after setSizes().
	void syncFromSplitter();

Q_SIGNALS:
	void currentPaneChanged(const QString &id);

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	struct Pane
	{
		QString id;
		QString title;
		QString badge;
		QString indicatorTip;
		int index = 0;
		bool indicator = false;
		QPushButton *tab = nullptr;
		QWidget *widget = nullptr;
	};

	const Pane *find(const QString &id) const;
	Pane *find(const QString &id);
	void updateTab(const Pane &p);
	void syncTabs();
	void rememberHeight();
	bool isPaneCollapsed() const;
	void restorePaneHeight();

	QSplitter *m_splitter;
	QWidget *m_paneArea;
	QStackedWidget *m_stack;
	QFrame *m_bar;
	QHBoxLayout *m_barLay;
	QList<Pane> m_panes;
	QString m_current;
	int m_paneHeight = DEFAULT_PANE_HEIGHT;
};

} // namespace scopy::dcdebugger

#endif // BOTTOMPANEHOST_H
