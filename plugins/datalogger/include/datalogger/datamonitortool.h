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

#ifndef DATAMONITORTOOL_H
#define DATAMONITORTOOL_H

#include "dataacquisitionmanager.hpp"
#include "scopy-datalogger_export.h"
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <menucontrolbutton.h>
#include <printplotmanager.h>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include "monitorplot.hpp"
#include "menus/monitorselectionmenu.hpp"
#include "qloggingcategory.h"

Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR);
Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR_TOOL);

namespace scopy::datamonitor {

class SevenSegmentDisplay;

class DataMonitorSettings;
class SCOPY_DATALOGGER_EXPORT DatamonitorTool : public QWidget
{
	friend class DataLogger_API;
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	DatamonitorTool(DataAcquisitionManager *dataAcquisitionManager, bool isDeletable = false,
			QWidget *parent = nullptr);
	~DatamonitorTool();

	RunBtn *getRunButton() const;

Q_SIGNALS:
	void requestNewTool();
	void runToggled(bool toggled);
	void settingsTitleChanged(QString newTitle);
	void requestDeleteTool();

private:
	MenuControlButton *monitorsButton;
	DataAcquisitionManager *m_dataAcquisitionManager;

	ToolTemplate *tool;
	GearBtn *settingsButton;
	QPushButton *openLastMenuBtn;
	QButtonGroup *rightMenuBtnGrp;
	InfoBtn *infoBtn;
	PrintPlotManager *printplotManager;
	RunBtn *runBtn;
	QPushButton *clearBtn;
	AddBtn *addMonitorButton;
	RemoveBtn *removeBtn;

	bool first = true;
	void resetStartTime();

	MonitorPlot *m_monitorPlot;
	QTextEdit *textMonitors;
	SevenSegmentDisplay *sevenSegmetMonitors;

	MenuControlButton *showPlot;
	MenuControlButton *showText;
	MenuControlButton *showSegments;

	DataMonitorSettings *m_dataMonitorSettings;
	MonitorSelectionMenu *m_monitorSelectionMenu;

	void initTutorialProperties();
	void startTutorial();
};
} // namespace scopy::datamonitor
#endif // DATAMONITORTOOL_H
