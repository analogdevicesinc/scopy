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

#ifndef RAWGENERATORVIEW_H
#define RAWGENERATORVIEW_H

#include "scopy-dcdebugger_export.h"
#include "streamgeneration.h"

#include <QList>
#include <QMap>
#include <QPointer>
#include <QWidget>

class QLabel;
class QPushButton;
class QTableView;
class QTimer;

namespace scopy {
class MenuLineEdit;
class MenuOnOffSwitch;
class PlotChannel;
class PlotWidget;
class ToolTemplate;
namespace gui {
class MenuSpinbox;
}
} // namespace scopy

namespace scopy::component {
class Attribute;
class OutputStream;
} // namespace scopy::component

namespace scopy::dcdebugger {
class SampleTableModel;
class SideSplitter;

// Simplified Generator tab of an OutputStream: the buffer is a table of raw codes (one column per
// member channel, one row per sample), edited in place, copied / pasted as spreadsheet cells or
// loaded from a CSV / text file; it is pushed as it is (clamped to each channel's code range).
// Preview of the buffer against the sample index above the table, with a legend. Run and the
// settings gear in the top bar; the settings menu: Samples (load a file, its path, the row count),
// then Output (sample rate for every member, cyclic).
class SCOPY_DCDEBUGGER_EXPORT RawGeneratorView : public QWidget
{
	Q_OBJECT
public:
	static constexpr int DefaultRows = 1024;

	explicit RawGeneratorView(component::OutputStream *stream, QWidget *parent = nullptr);
	~RawGeneratorView() override;

	StreamGeneration *generation() const { return m_gen; }
	PlotWidget *plot() const { return m_plot; }
	SampleTableModel *samples() const { return m_model; }
	QTableView *table() const { return m_table; }

public Q_SLOTS:
	// Applies the table to the generation now (it is otherwise applied shortly after a change).
	void apply();
	// Column k of the file feeds channel k; false (status set) when it cannot be read.
	bool loadFile(const QString &path);
	// The clipboard (spreadsheet cells: tab separated) written from the current cell on.
	void pasteClipboard();
	// The selected cells (a range, whole rows or whole columns) to the clipboard, tab separated.
	void copySelection();

private:
	QWidget *buildMenu(QWidget *parent);
	QWidget *buildSettingsSection(QWidget *parent);
	QWidget *buildSamplesSection(QWidget *parent);
	void buildPreview();
	void buildTable();
	void placeLegend();
	// The row numbers' header as wide as the largest one.
	void fitRowHeader();
	void scheduleApply();
	void updatePreview();
	void loadFromDialog();
	void onRunningChanged(bool running);
	void setStatus(const QString &text, bool error = false);

	QPointer<component::OutputStream> m_stream;
	StreamGeneration *m_gen;
	QList<component::Attribute *> m_sampleRateAttrs;
	QStringList m_ids; // member channel ids, in table column order

	ToolTemplate *m_tool;
	SideSplitter *m_sides;
	QPushButton *m_runBtn;
	QPushButton *m_loadBtn;
	MenuLineEdit *m_path = nullptr;
	QLabel *m_status;
	gui::MenuSpinbox *m_rows = nullptr;
	MenuOnOffSwitch *m_cyclic = nullptr;
	PlotWidget *m_plot = nullptr;
	QMap<QString, PlotChannel *> m_plotChannels;
	SampleTableModel *m_model = nullptr;
	QTableView *m_table = nullptr;
	QTimer *m_applyTimer;
};

} // namespace scopy::dcdebugger

#endif // RAWGENERATORVIEW_H
