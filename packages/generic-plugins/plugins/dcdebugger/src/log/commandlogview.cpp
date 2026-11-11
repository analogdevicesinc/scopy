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

#include "commandlogview.h"

#include "codegeneratorpanel.h"
#include "commandlogmodel.h"

#include <QAbstractButton>
#include <QAction>
#include <QClipboard>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

#include <component/commandmonitor.h>
#include <dcdebugger/style_properties.h>
#include <filemanager.h>
#include <menulineedit.h>
#include <menuonoffswitch.h>
#include <pluginbase/preferences.h>
#include <style.h>
#include <tooltemplate.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {
int intPreference(const char *key, int fallback)
{
	const QVariant v = Preferences::get(QString::fromLatin1(key));
	bool ok = false;
	const int value = v.toInt(&ok);
	return v.isValid() && ok ? value : fallback;
}

QLineEdit *filterEdit(const QString &placeholder, QWidget *parent, QHBoxLayout *lay)
{
	auto *edit = new MenuLineEdit(parent);
	edit->edit()->setPlaceholderText(placeholder);
	edit->edit()->setClearButtonEnabled(true);
	lay->addWidget(edit, 1);
	return edit->edit();
}
} // namespace

void CommandLogView::initPreferences()
{
	Preferences::init(SIZE_PREF, DEFAULT_SIZE);
	Preferences::init(COLLAPSE_PREF, DEFAULT_COLLAPSE_AFTER);
}

CommandLogView::CommandLogView(QWidget *parent)
	: QWidget(parent)
	, m_model(new CommandLogModel(this))
{
	m_model->setCapacity(intPreference(SIZE_PREF, DEFAULT_SIZE));
	m_model->setCollapseAfter(intPreference(COLLAPSE_PREF, DEFAULT_COLLAPSE_AFTER));
	m_model->setErrorColor(QColor(Style::getAttribute(json::theme::content_error)));

	// Instrument look, like the node panel above: one grey, rounded pane (basicComponent) holding
	// the controls, the filter strip (a lighter shade, logFilterBar) and the table (a darker
	// shade, logTable); the actions are primary buttons (basicButton).
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	auto *pane = new QFrame(this);
	Style::setStyle(pane, style::properties::widget::basicComponent);
	auto *paneLay = new QVBoxLayout(pane);
	paneLay->setContentsMargins(6, 6, 6, 6);
	lay->addWidget(pane);
	m_tool = new ToolTemplate(pane);
	// ToolTemplate fills its plain containers (".QWidget") background_subtle; let the pane show.
	Style::setBackgroundColor(m_tool, QStringLiteral("transparent"));
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	paneLay->addWidget(m_tool);

	// Top left: "Log commands" over "Follow", then the status; the actions on the right.
	m_log = new MenuOnOffSwitch("Log commands", this);
	m_log->setToolTip("Record the library calls of every command of this connection");
	m_log->setEnabled(false); // until a monitor is bound
	m_follow = new MenuOnOffSwitch("Follow", this);
	m_follow->onOffswitch()->setChecked(true);
	m_follow->setToolTip("Scroll to the newest command");
	auto *switches = new QWidget(this);
	auto *switchLay = new QVBoxLayout(switches);
	switchLay->setContentsMargins(0, 0, 0, 0);
	switchLay->setSpacing(4);
	// One column as wide as the wider switch: labels left-aligned, the toggles lined up.
	switchLay->addWidget(m_log);
	switchLay->addWidget(m_follow);
	switches->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_status = new QLabel(this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_pauseBtn = new QPushButton("Pause", this);
	m_pauseBtn->setCheckable(true);
	m_pauseBtn->setToolTip("Freeze the table; recording continues");
	m_clearBtn = new QPushButton("Clear", this);
	m_clearBtn->setToolTip("Forget every recorded command");
	m_exportBtn = new QPushButton("Export...", this);
	m_exportBtn->setToolTip("Save the filtered calls as CSV");
	m_generateBtn = new QPushButton("Generate code...", this);
	m_generateBtn->setCheckable(true);
	m_generateBtn->setToolTip("Show the script repeating the selected rows (none selected: every visible row)");
	for(QPushButton *b : {m_pauseBtn, m_clearBtn, m_exportBtn, m_generateBtn}) {
		Style::setStyle(b, style::properties::button::basicButton);
		b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	}
	m_tool->addWidgetToTopContainerHelper(switches, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_pauseBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_clearBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_exportBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_generateBtn, TTA_RIGHT);

	// Central: filters, then the log.
	auto *central = new QWidget(this);
	auto *clay = new QVBoxLayout(central);
	clay->setContentsMargins(0, 0, 0, 0);
	clay->setSpacing(6);
	clay->addWidget(buildFilters(central));
	m_tree = new QTreeView(central);
	m_tree->setModel(m_model);
	m_tree->setUniformRowHeights(true);
	m_tree->setRootIsDecorated(true);
	m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_tree->header()->setStretchLastSection(true);
	m_tree->header()->setSectionResizeMode(QHeaderView::Interactive);
	m_tree->setColumnWidth(CommandLogModel::TimeColumn, 110);
	m_tree->setColumnWidth(CommandLogModel::ComponentColumn, 220);
	m_tree->setColumnWidth(CommandLogModel::OperationColumn, 220);
	m_tree->setColumnWidth(CommandLogModel::ArgsColumn, 240);
	m_tree->setColumnWidth(CommandLogModel::ValueColumn, 200);
	Style::setStyle(m_tree, style::properties::dcdebugger::logTable);
	// The code generator sits right of the table, hidden until asked for.
	// Like every splitter of the debugger, both sides collapse: a panel dragged fully closed is
	// hidden (its toggle unchecked).
	m_split = new QSplitter(Qt::Horizontal, central);
	m_split->addWidget(m_tree);
	m_codegen = new CodeGeneratorPanel(m_split);
	m_codegen->setMinimumWidth(360);
	m_codegen->setVisible(false);
	m_split->addWidget(m_codegen);
	m_split->setStretchFactor(0, 3);
	m_split->setStretchFactor(1, 2);
	clay->addWidget(m_split, 1);
	m_tool->addWidgetToCentralContainerHelper(central);

	// Copy: Ctrl+C while the table has focus, and the context menu.
	m_copyAction = new QAction("Copy", m_tree);
	m_copyAction->setShortcut(QKeySequence::Copy);
	m_copyAction->setShortcutContext(Qt::WidgetShortcut);
	m_tree->addAction(m_copyAction);
	m_copyAllAction = new QAction("Copy all visible", m_tree);
	m_generateAction = new QAction("Generate code...", m_tree);
	m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_copyAction, &QAction::triggered, this, &CommandLogView::copySelectedRows);
	connect(m_copyAllAction, &QAction::triggered, this, &CommandLogView::copyVisibleRows);
	connect(m_tree, &QWidget::customContextMenuRequested, this, &CommandLogView::showContextMenu);

	// Code generation follows the selection at once and the log (appends, filters) with a delay.
	m_codegenTimer = new QTimer(this);
	m_codegenTimer->setSingleShot(true);
	m_codegenTimer->setInterval(400);
	connect(m_codegenTimer, &QTimer::timeout, this, &CommandLogView::refreshCodeGenerator);
	connect(m_generateBtn, &QPushButton::toggled, this, [this](bool on) { showCodeGenerator(on); });
	connect(m_split, &QSplitter::splitterMoved, this, [this] {
		if(isCodeGeneratorShown() && m_split->sizes().value(1) == 0) {
			showCodeGenerator(false);
		}
	});
	connect(m_generateAction, &QAction::triggered, this, [this] { showCodeGenerator(true); });
	connect(m_tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
		if(isCodeGeneratorShown()) {
			refreshCodeGenerator();
		}
	});
	connect(m_tree, &QTreeView::expanded, this, &CommandLogView::scheduleCodeGenerator);
	connect(m_tree, &QTreeView::collapsed, this, &CommandLogView::scheduleCodeGenerator);
	for(auto sig : {&QAbstractItemModel::rowsInserted, &QAbstractItemModel::rowsRemoved}) {
		connect(m_model, sig, this, &CommandLogView::scheduleCodeGenerator);
	}
	connect(m_model, &QAbstractItemModel::modelReset, this, &CommandLogView::scheduleCodeGenerator);
	connect(m_model, &QAbstractItemModel::dataChanged, this, &CommandLogView::scheduleCodeGenerator);

	connect(m_log->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		// User action: drive this connection's monitor (not persisted). Not a signal blocker on
		// the way back (onEnabledChanged): the switch animates its thumb from toggled.
		if(m_monitor && m_monitor->isEnabled() != on) {
			m_monitor->setEnabled(on);
		}
	});

	connect(m_pauseBtn, &QPushButton::toggled, this, [this](bool paused) {
		m_pauseBtn->setText(paused ? "Resume" : "Pause");
		m_model->setPaused(paused);
		updateStatus();
		if(!paused && m_follow->onOffswitch()->isChecked()) {
			m_tree->scrollToBottom();
		}
	});
	connect(m_clearBtn, &QPushButton::clicked, m_model, &CommandLogModel::clear);
	connect(m_exportBtn, &QPushButton::clicked, this, &CommandLogView::exportCsv);
	connect(m_model, &CommandLogModel::entryCountChanged, this, &CommandLogView::updateStatus);
	connect(m_model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &parent) {
		if(!parent.isValid() && m_follow->onOffswitch()->isChecked()) {
			m_tree->scrollToBottom();
		}
	});
	connect(m_tree, &QTreeView::clicked, this, &CommandLogView::onRowClicked);
	updateStatus();
}

CommandLogView::~CommandLogView() = default;

QWidget *CommandLogView::buildFilters(QWidget *parent)
{
	// A QFrame, not a plain QWidget: the ToolTemplate's ".QWidget" background does not reach it.
	auto *w = new QFrame(parent);
	Style::setStyle(w, style::properties::widget::basicComponent);
	auto *lay = new QHBoxLayout(w);
	lay->setContentsMargins(4, 4, 4, 4);
	lay->setSpacing(10);
	m_component = filterEdit("Filter component", w, lay);
	m_library = filterEdit("Filter library", w, lay);
	m_function = filterEdit("Filter function", w, lay);
	m_errorsOnly = new MenuOnOffSwitch("Errors only", w);
	m_errorsOnly->setToolTip("Show only the commands with a failed call");
	m_errorsOnly->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	lay->addWidget(m_errorsOnly);
	for(QLineEdit *e : {m_component, m_library, m_function}) {
		connect(e, &QLineEdit::textChanged, this, &CommandLogView::applyFilter);
	}
	connect(m_errorsOnly->onOffswitch(), &QAbstractButton::toggled, this, &CommandLogView::applyFilter);
	return w;
}

void CommandLogView::setUri(const QString &uri) { m_model->setUri(uri); }

void CommandLogView::setMonitor(component::CommandMonitor *monitor)
{
	if(m_monitor == monitor) {
		return;
	}
	if(m_monitor) {
		disconnect(m_monitor, nullptr, this, nullptr);
		disconnect(m_monitor, nullptr, m_model, nullptr);
	}
	m_monitor = monitor;
	if(m_monitor) {
		// Emitted on the executor threads: queued to the model here.
		connect(m_monitor, &component::CommandMonitor::commandFinished, m_model, &CommandLogModel::append,
			Qt::QueuedConnection);
		connect(m_monitor, &component::CommandMonitor::enabledChanged, this, &CommandLogView::onEnabledChanged);
		connect(m_monitor, &QObject::destroyed, this, &CommandLogView::onMonitorDestroyed);
	}
	m_log->setEnabled(m_monitor != nullptr);
	onEnabledChanged(m_monitor && m_monitor->isEnabled());
}

component::CommandMonitor *CommandLogView::monitor() const { return m_monitor.data(); }

QString CommandLogView::statusText() const { return m_status->text(); }

bool CommandLogView::isRecording() const { return m_recording; }

void CommandLogView::applyFilter()
{
	CommandLogFilter f;
	f.component = m_component->text().trimmed();
	f.library = m_library->text().trimmed();
	f.function = m_function->text().trimmed();
	f.errorsOnly = m_errorsOnly->onOffswitch()->isChecked();
	m_model->setFilter(f);
}

void CommandLogView::onRowClicked(const QModelIndex &index)
{
	const QString origin = index.data(CommandLogModel::OriginRole).toString();
	const QModelIndex top = index.parent().isValid() ? index.parent() : index;
	const QString path = origin.isEmpty() ? top.data(CommandLogModel::OriginRole).toString() : origin;
	if(path.isEmpty()) {
		return;
	}
	const QModelIndex src = origin.isEmpty() ? top : index;
	Q_EMIT nodeRequested(path, src.data(CommandLogModel::BackendRole).toString(),
			     src.data(CommandLogModel::OriginObjectRole).value<quintptr>());
}

void CommandLogView::onEnabledChanged(bool enabled)
{
	// Follows the monitor whoever turns it (script, disconnect).
	m_log->onOffswitch()->setChecked(enabled); // no echo: the toggled handler skips equal states
	updateStatus();
	if(m_recording != enabled) {
		m_recording = enabled;
		Q_EMIT recordingChanged(enabled);
	}
}

void CommandLogView::onMonitorDestroyed()
{
	// The connection went away: keep the rows (still exportable), nothing left to switch.
	m_monitor = nullptr;
	m_log->setEnabled(false);
	onEnabledChanged(false);
}

void CommandLogView::exportCsv()
{
	if(m_model->filteredEntries().isEmpty()) {
		m_status->setText("Nothing to export");
		return;
	}
	FileManagerHelper::saveDataToFile(this, m_model->toCsv(), "Export command log");
}

bool CommandLogView::isShown(const QModelIndex &index) const
{
	const QModelIndex parent = index.parent();
	return !parent.isValid() || m_tree->isExpanded(parent);
}

QModelIndexList CommandLogView::shownSelectedRows() const
{
	QModelIndexList rows;
	for(const QModelIndex &i : m_tree->selectionModel()->selectedRows()) {
		if(isShown(i)) {
			rows.append(i);
		}
	}
	return rows;
}

QString CommandLogView::selectedRowsText() const { return m_model->rowsText(shownSelectedRows()); }

QList<CommandLogEntry> CommandLogView::generationEntries() const
{
	const QModelIndexList rows = shownSelectedRows();
	return rows.isEmpty() ? m_model->visibleEntries() : m_model->entriesFor(rows);
}

bool CommandLogView::isCodeGeneratorShown() const { return !m_codegen->isHidden(); }

void CommandLogView::showCodeGenerator(bool show)
{
	if(m_generateBtn->isChecked() != show) {
		m_generateBtn->setChecked(show); // toggled → back here
		return;
	}
	m_codegen->setVisible(show);
	if(show) {
		// Reopened after being dragged closed: give it its share back.
		const QList<int> sizes = m_split->sizes();
		if(sizes.size() == 2 && sizes[1] == 0 && sizes[0] > 0) {
			m_split->setSizes({sizes[0] * 3 / 5, sizes[0] * 2 / 5});
		}
		refreshCodeGenerator();
	} else {
		m_codegenTimer->stop();
	}
}

void CommandLogView::scheduleCodeGenerator()
{
	if(isCodeGeneratorShown() && !m_codegenTimer->isActive()) {
		m_codegenTimer->start();
	}
}

void CommandLogView::refreshCodeGenerator()
{
	m_codegenTimer->stop();
	const QModelIndexList rows = shownSelectedRows();
	const QList<CommandLogEntry> entries = rows.isEmpty() ? m_model->visibleEntries() : m_model->entriesFor(rows);
	QString scope;
	if(entries.isEmpty()) {
		scope = QStringLiteral("Nothing to generate: the log is empty");
	} else if(rows.isEmpty()) {
		scope = QStringLiteral("Every visible row: %1 command(s)").arg(entries.size());
	} else {
		scope = QStringLiteral("%1 selected row(s): %2 command(s)").arg(rows.size()).arg(entries.size());
	}
	m_codegen->setEntries(entries, scope);
}

QString CommandLogView::visibleRowsText() const
{
	QModelIndexList rows;
	for(int r = 0; r < m_model->rowCount(); ++r) {
		const QModelIndex top = m_model->index(r, 0);
		rows.append(top);
		if(!m_tree->isExpanded(top)) {
			continue;
		}
		for(int c = 0; c < m_model->rowCount(top); ++c) {
			rows.append(m_model->index(c, 0, top));
		}
	}
	return m_model->rowsText(rows);
}

void CommandLogView::copyText(const QString &text)
{
	if(!text.isEmpty()) {
		QGuiApplication::clipboard()->setText(text);
	}
}

void CommandLogView::copySelectedRows() { copyText(selectedRowsText()); }

void CommandLogView::copyVisibleRows() { copyText(visibleRowsText()); }

void CommandLogView::showContextMenu(const QPoint &pos)
{
	m_copyAction->setEnabled(m_tree->selectionModel()->hasSelection());
	m_copyAllAction->setEnabled(m_model->rowCount() > 0);
	m_generateAction->setEnabled(m_model->rowCount() > 0);
	QMenu menu(this);
	menu.addAction(m_copyAction);
	menu.addAction(m_copyAllAction);
	menu.addSeparator();
	menu.addAction(m_generateAction);
	menu.exec(m_tree->viewport()->mapToGlobal(pos));
	m_copyAction->setEnabled(true); // the shortcut stays live (a no-op without a selection)
}

void CommandLogView::updateStatus()
{
	QString text = (m_monitor && m_monitor->isEnabled())
		? QStringLiteral("Logging · %1 entries").arg(m_model->entryCount())
		: QStringLiteral("Logging off · %1 entries").arg(m_model->entryCount());
	if(m_model->isPaused()) {
		text += QStringLiteral(" · paused");
	}
	m_status->setText(text);
}

#include "moc_commandlogview.cpp"
