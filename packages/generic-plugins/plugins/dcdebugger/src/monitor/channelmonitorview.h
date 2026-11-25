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

#ifndef CHANNELMONITORVIEW_H
#define CHANNELMONITORVIEW_H

#include "attributemonitor.h"
#include "scopy-dcdebugger_export.h"

#include <QList>
#include <QPointer>
#include <QStringList>
#include <QWidget>

class QLabel;
class QPushButton;
class QTableWidget;
class QTimer;

namespace scopy {
class MenuCombo;
class ToolTemplate;
namespace gui {
class MenuSpinbox;
}
} // namespace scopy

namespace scopy::component {
class Attribute;
class Channel;
} // namespace scopy::component

namespace scopy::dcdebugger {
class SideSplitter;

// Table-only Monitor of a Device: polls one attribute, chosen by name in the Settings menu, of
// every input channel that has it, and shows its raw value with min / max / mean over the
// history window. No per-channel control and no scaling.
class SCOPY_DCDEBUGGER_EXPORT ChannelMonitorView : public QWidget
{
	Q_OBJECT
public:
	static constexpr const char *DefaultAttribute = "raw";
	static constexpr int DefaultPeriodMs = 1000;

	explicit ChannelMonitorView(QObject *device, QWidget *parent = nullptr);
	~ChannelMonitorView() override;

	AttributeMonitor *monitor() const { return m_monitor; }

	// Input channels of `device` that have attributes; empty for anything but a Device.
	static QList<component::Channel *> inputChannels(QObject *device);
	// Sorted union of the attribute names of `channels`.
	static QStringList attributeNames(const QList<component::Channel *> &channels);

	QString attributeName() const { return m_attribute; }
	// Monitor keys of the channels that have the selected attribute, in table order.
	QStringList polledKeys() const;

public Q_SLOTS:
	// Polls `name` on every channel that has it; the history restarts.
	void setAttributeName(const QString &name);

private:
	struct Entry
	{
		QString key;
		QString channel;
		QPointer<component::Channel> ch;
	};

	QWidget *buildSettings(QWidget *parent);
	void buildTable();
	void applyAttribute();
	void updateTable();
	void updateStatus();

	QPointer<QObject> m_root;
	AttributeMonitor *m_monitor;
	QList<Entry> m_entries;
	QString m_attribute;

	ToolTemplate *m_tool;
	SideSplitter *m_sides;
	QPushButton *m_runBtn;
	QPushButton *m_clearBtn;
	QLabel *m_status;
	gui::MenuSpinbox *m_period = nullptr;
	gui::MenuSpinbox *m_history = nullptr;
	MenuCombo *m_attrCombo = nullptr;
	QTableWidget *m_table = nullptr;
	QTimer *m_redraw;
	bool m_dirty = false;
};

} // namespace scopy::dcdebugger

#endif // CHANNELMONITORVIEW_H
