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

#include "commandlogmodel.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
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

	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	lay->addWidget(m_tool);

	// Top: "Log commands" + status on the left, the actions on the right.
	m_log = new QCheckBox("Log commands", this);
	m_log->setToolTip("Record the library calls of every command of this connection");
	m_log->setEnabled(false); // until a monitor is bound
	m_status = new QLabel(this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_pauseBtn = new QPushButton("Pause", this);
	m_pauseBtn->setCheckable(true);
	m_pauseBtn->setToolTip("Freeze the table; recording continues");
	m_follow = new MenuOnOffSwitch("Follow", this);
	m_follow->onOffswitch()->setChecked(true);
	m_follow->setToolTip("Scroll to the newest command");
	m_clearBtn = new QPushButton("Clear", this);
	m_clearBtn->setToolTip("Forget every recorded command");
	m_exportBtn = new QPushButton("Export...", this);
	m_exportBtn->setToolTip("Save the filtered calls as CSV");
	m_generateBtn = new QPushButton("Generate code...", this);
	m_generateBtn->setEnabled(false);
	m_generateBtn->setToolTip("Code generation from the log is not available yet");
	for(QPushButton *b : {m_pauseBtn, m_clearBtn, m_exportBtn, m_generateBtn}) {
		Style::setStyle(b, style::properties::dcdebugger::topButton);
		b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	}
	m_tool->addWidgetToTopContainerHelper(m_log, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_follow, TTA_RIGHT);
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
	m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
	m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_tree->header()->setStretchLastSection(true);
	m_tree->header()->setSectionResizeMode(QHeaderView::Interactive);
	m_tree->setColumnWidth(CommandLogModel::TimeColumn, 110);
	m_tree->setColumnWidth(CommandLogModel::ComponentColumn, 220);
	m_tree->setColumnWidth(CommandLogModel::OperationColumn, 220);
	m_tree->setColumnWidth(CommandLogModel::ArgsColumn, 240);
	m_tree->setColumnWidth(CommandLogModel::ValueColumn, 200);
	Style::setStyle(m_tree, style::properties::dcdebugger::treeView);
	Style::setStyle(m_tree->header(), style::properties::dcdebugger::headerView);
	clay->addWidget(m_tree, 1);
	m_tool->addWidgetToCentralContainerHelper(central);

	connect(m_log, &QAbstractButton::toggled, this, [this](bool on) {
		// User action: drive this connection's monitor (not persisted).
		if(m_monitor) {
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
	auto *w = new QWidget(parent);
	auto *lay = new QHBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);
	m_component = filterEdit("Filter component", w, lay);
	m_library = filterEdit("Filter library", w, lay);
	m_function = filterEdit("Filter function", w, lay);
	m_errorsOnly = new MenuOnOffSwitch("Errors only", w);
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
	QSignalBlocker block(m_log);
	m_log->setChecked(enabled);
	updateStatus();
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
