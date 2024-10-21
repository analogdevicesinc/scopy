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

#ifndef DATALOGGINGMENU_HPP
#define DATALOGGINGMENU_HPP

#include <QWidget>
#include <menuonoffswitch.h>
#include <progresslineedit.h>
#include "scopy-datalogger_export.h"

namespace scopy {
namespace datamonitor {

enum ProgressBarState
{
	SUCCESS,
	ERROR,
	BUSY
};

class SCOPY_DATALOGGER_EXPORT DataLoggingMenu : public QWidget
{
	friend class DataLogger_API;
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit DataLoggingMenu(QWidget *parent = nullptr);

	bool liveDataLogging() const;

public Q_SLOTS:
	void updateDataLoggingStatus(ProgressBarState status);

Q_SIGNALS:
	void pathChanged(QString path);
	void requestLiveDataLogging(QString path);
	void requestDataLogging(QString path);
	void requestDataLoading(QString path);

private:
	QString filename;
	ProgressLineEdit *dataLoggingFilePath;
	QPushButton *dataLoggingBrowseBtn;
	QPushButton *dataLoggingBtn;
	QPushButton *dataLoadingBtn;
	MenuOnOffSwitch *liveDataLoggingButton;
	bool m_liveDataLogging = false;
	void chooseFile();
	void toggleButtonsEnabled(bool en);
};
} // namespace datamonitor
} // namespace scopy
#endif // DATALOGGINGMENU_HPP
