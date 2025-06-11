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

#ifndef DATAMONITORSETTINGS_HPP
#define DATAMONITORSETTINGS_HPP

#include <QTimer>
#include <QWidget>
#include <edittextmenuheader.h>
#include <menucombo.h>
#include <menuonoffswitch.h>
#include <monitorplotcurve.hpp>
#include <monitorplotmanager.h>
#include <plottimeaxiscontroller.hpp>
#include <progresslineedit.h>
#include <spinbox_a.hpp>
#include "scopy-datalogger_export.h"
#include "qloggingcategory.h"
#include "sevensegmentmonitorsettings.hpp"
#include "dataloggingmenu.hpp"

Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR_SETTINGS)

namespace scopy {

class MenuCollapseSection;

class MenuSectionWidget;

class CollapsableMenuControlButton;
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT DataMonitorSettings : public QWidget
{
	friend class DataLogger_API;
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit DataMonitorSettings(MonitorPlotManager *m_plotManager, QWidget *parent = nullptr);
	~DataMonitorSettings();

	void init(QString title, QColor color);

	SevenSegmentMonitorSettings *getSevenSegmentMonitorSettings() const;

	DataLoggingMenu *getDataLoggingMenu() const;

public Q_SLOTS:
	void setEnableAddRemovePlot(bool);
	void setActiveSettings(int idx);

Q_SIGNALS:
	void titleUpdated(QString title);

private:
	bool m_isDeletable;
	QPushButton *deleteMonitor = nullptr;

	EditTextMenuHeader *header;
	QVBoxLayout *layout;

	QWidget *settingsBody;
	QVBoxLayout *mainLayout;

	QStackedWidget *m_activeSettings;

	MonitorPlotManager *m_plotManager;

	SevenSegmentMonitorSettings *sevenSegmentMonitorSettings;
	DataLoggingMenu *dataLoggingMenu;

	QPushButton *m_addPlotBtn;

	PlotTimeAxisController *m_plotTimeAxisController = nullptr;

	bool eventFilter(QObject *watched, QEvent *event) override;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORSETTINGS_HPP
