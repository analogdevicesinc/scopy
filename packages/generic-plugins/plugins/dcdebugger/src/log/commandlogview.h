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

#ifndef COMMANDLOGVIEW_H
#define COMMANDLOGVIEW_H

#include "scopy-dcdebugger_export.h"

#include <QPointer>
#include <QString>
#include <QWidget>

class QAction;
class QLabel;
class QLineEdit;
class QModelIndex;
class QPushButton;
class QTreeView;

namespace scopy {
class MenuOnOffSwitch;
class ToolTemplate;
} // namespace scopy

namespace scopy::component {
class CommandMonitor;
}

namespace scopy::dcdebugger {
class CommandLogModel;

// Command log of the debugger: the library calls every command of one connection issued (its
// CommandMonitor), with the "Log commands" switch (drives that monitor's setEnabled both ways;
// per connection, not persisted), pause, clear, filters, follow tail and CSV export. A click on a
// row asks for the node that issued the command. The rows outlive the monitor (disconnect).
// Ctrl+C / context menu copy the selected rows (or all visible ones) as tab-separated text.
class SCOPY_DCDEBUGGER_EXPORT CommandLogView : public QWidget
{
	Q_OBJECT
public:
	static constexpr const char *SIZE_PREF = "dcdebugger_command_log_size";
	static constexpr const char *COLLAPSE_PREF = "dcdebugger_command_log_collapse_after";
	static constexpr int DEFAULT_SIZE = 10000;
	static constexpr int DEFAULT_COLLAPSE_AFTER = 10;

	// Preference defaults (Preferences::init: an existing value is kept).
	static void initPreferences();

	explicit CommandLogView(QWidget *parent = nullptr);
	~CommandLogView() override;

	// Binds the log and the switch to one connection's monitor (nullptr: unbound, switch
	// disabled). Dropped automatically when the monitor is destroyed; the rows are kept.
	void setMonitor(component::CommandMonitor *monitor);
	component::CommandMonitor *monitor() const;

	// Keep only the commands of this connection.
	void setUri(const QString &uri);

	CommandLogModel *model() const { return m_model; }
	QTreeView *tree() const { return m_tree; }
	// "Log commands": its onOffswitch() is checked while the monitor records.
	MenuOnOffSwitch *logSwitch() const { return m_log; }
	QPushButton *pauseButton() const { return m_pauseBtn; }
	QString statusText() const;
	QAction *copyAction() const { return m_copyAction; }
	// The bound monitor records (follows its enabledChanged and destruction).
	bool isRecording() const;

	// Tab-separated lines of the selected (resp. all) rows the table shows: filtered, and children
	// only under expanded rows. Empty when there is nothing to copy.
	QString selectedRowsText() const;
	QString visibleRowsText() const;
	// To the clipboard; nothing to copy leaves it untouched.
	void copySelectedRows();
	void copyVisibleRows();

Q_SIGNALS:
	// The node that issued the clicked row (origin path, backend id, node identity).
	void nodeRequested(const QString &origin, const QString &backendId, quintptr object);
	// isRecording() changed: the monitor was turned, bound, unbound or destroyed.
	void recordingChanged(bool recording);

private:
	QWidget *buildFilters(QWidget *parent);
	void applyFilter();
	void onRowClicked(const QModelIndex &index);
	void onEnabledChanged(bool enabled);
	void onMonitorDestroyed();
	void exportCsv();
	void showContextMenu(const QPoint &pos);
	bool isShown(const QModelIndex &index) const;
	void copyText(const QString &text);
	void updateStatus();

	CommandLogModel *m_model;
	ToolTemplate *m_tool;
	QLabel *m_status;
	QPointer<component::CommandMonitor> m_monitor;
	MenuOnOffSwitch *m_log;
	QPushButton *m_pauseBtn;
	MenuOnOffSwitch *m_follow;
	QPushButton *m_clearBtn;
	QPushButton *m_exportBtn;
	QPushButton *m_generateBtn;
	QLineEdit *m_component;
	QLineEdit *m_library;
	QLineEdit *m_function;
	MenuOnOffSwitch *m_errorsOnly;
	QTreeView *m_tree;
	QAction *m_copyAction;
	QAction *m_copyAllAction;
	bool m_recording = false;
};

} // namespace scopy::dcdebugger

#endif // COMMANDLOGVIEW_H
