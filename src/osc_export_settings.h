/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef OSC_EXPORT_SETTINGS_H
#define OSC_EXPORT_SETTINGS_H

/* Qt includes */
#include <QWidget>
#include <QString>
#include <QStandardItem>
#include <QPushButton>
#include <QMap>

/* Local includes */
#include "dropdown_switch_list.h"

namespace Ui {
class ExportSettings;
}

namespace adiscope {
class ExportSettings : public QWidget
{
	Q_OBJECT

public:
	explicit ExportSettings(QWidget *parent = 0);
	~ExportSettings();

	void clear();

public Q_SLOTS:
	void addChannel(int id, QString name);
	void removeChannel(int id);
	void onExportChannelChanged(QStandardItem*);

	QPushButton *getExportButton();
	QPushButton *getExportAllButton();
	QMap<int, bool> getExportConfig();
	void setExportConfig(QMap<int, bool> config);
	void on_btnExportAll_clicked();
	void enableExportButton(bool on);
	void disableUIMargins();
	void setTitleLabelVisible(bool enabled);
	void setExportAllButtonLabel(const QString &text);

protected:
	Ui::ExportSettings *ui;
	DropdownSwitchList *exportChannels;
	void checkIfAllActivated();
	int nr_channels;
	QMap<int, bool> oldSettings;
};
}

#endif // OSC_EXPORT_SETTINGS_H
