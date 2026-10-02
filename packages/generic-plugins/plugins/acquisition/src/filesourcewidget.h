/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef FILESOURCEWIDGET_H
#define FILESOURCEWIDGET_H

#include "filesourceblock.h"

#include <QList>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QTimer;
class QVBoxLayout;

namespace scopy {

class FileBrowserWidget;

namespace gui {
class MenuSpinbox;
}

namespace adc {

// The panel for FileSourceBlock: one row per file slot, plus the button that adds one.
//
// Built here rather than by the block, exactly as SnapshotSourceWidget is: the rows are a view of
// a list that changes while the instrument is alive, and a block's createSettingsWidget() is
// called once.
class FileSourceWidget : public QWidget
{
	Q_OBJECT
public:
	explicit FileSourceWidget(FileSourceBlock *src, QWidget *parent = nullptr);

private Q_SLOTS:
	void rebuildRows();
	void syncRow(int index);

private:
	struct Row
	{
		FileBrowserWidget *browser{nullptr};
		QComboBox *firstColumn{nullptr};
		QWidget *firstColumnRow{nullptr};
		gui::MenuSpinbox *rate{nullptr};
		QCheckBox *cyclic{nullptr};
		QLabel *readback{nullptr};
		QLabel *playback{nullptr};
	};

	void setupUI();
	QWidget *buildRow(int index, const FileSourceBlock::SlotInfo &info);
	void syncPlayback(int index);

	FileSourceBlock *m_src{nullptr};
	QVBoxLayout *m_rowsLay{nullptr};
	QLabel *m_emptyHint{nullptr};
	QTimer *m_poll{nullptr};
	QList<Row> m_rows;

	// Populating a row fires the same signals a reader's edit does, so a half-built row must
	// not write itself back.
	bool m_building{false};
};

} // namespace adc
} // namespace scopy

#endif // FILESOURCEWIDGET_H
