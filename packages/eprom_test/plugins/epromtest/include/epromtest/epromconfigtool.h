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

#ifndef EPROMCONFIGTOOL_H
#define EPROMCONFIGTOOL_H

#include "scopy-epromtest_export.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QWidget>

#include <tooltemplate.h>

namespace scopy {

/**
 * @brief The EEPROM configuration tool: Sync, Configure and a read-only view of the synced structure.
 * UI only; the plugin does the work and updates the view.
 */
class SCOPY_EPROMTEST_EXPORT EpromConfigTool : public QWidget
{
	Q_OBJECT
public:
	explicit EpromConfigTool(QWidget *parent = nullptr);

	void setStructure(const QString &text);
	void setStatus(const QString &text);
	void setConfigureEnabled(bool enabled);
	void setBoard(const QString &board);

Q_SIGNALS:
	void syncRequested();
	void configureRequested();
	void switchDeviceRequested();

private:
	ToolTemplate *m_tool;
	QPushButton *m_syncBtn;
	QPushButton *m_configureBtn;
	QPushButton *m_switchDeviceBtn;
	QPlainTextEdit *m_structureView;
	QLabel *m_board;
	QLabel *m_status;
};

} // namespace scopy

#endif // EPROMCONFIGTOOL_H
