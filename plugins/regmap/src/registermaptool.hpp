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

#ifndef REGISTERMAPTOOL_HPP
#define REGISTERMAPTOOL_HPP

#include <QObject>
#include <QWidget>
#include <toolbuttons.h>
#include "registermaptemplate.hpp"
#include "scopy-regmap_export.h"
#include "gui/tooltemplate.h"
#include <iio.h>

class QComboBox;
namespace scopy {
namespace regmap {

class RegisterMapValues;

class DeviceRegisterMap;

class RegisterMapSettingsMenu;

class SearchBarWidget;

class SCOPY_REGMAP_EXPORT RegisterMapTool : public QWidget
{
	friend class RegMap_API;
	Q_OBJECT
public:
	explicit RegisterMapTool(QWidget *parent = nullptr);
	~RegisterMapTool();

	void addDevice(QString devName, RegisterMapTemplate *registerMapTemplate = nullptr,
		       RegisterMapValues *registerMapValues = nullptr);

signals:

private:
	ToolTemplate *tool;
	GearBtn *settingsMenu;
	QComboBox *registerDeviceList;
	QString activeRegisterMap;
	SearchBarWidget *searchBarWidget;
	RegisterMapSettingsMenu *settings;
	QMap<QString, DeviceRegisterMap *> *deviceList;
	bool first = true;
	void toggleSettingsMenu(QString registerName, bool toggle);

private Q_SLOTS:
	void updateActiveRegisterMap(QString registerName);
	void toggleSearchBarEnabled(bool enabled);
};
} // namespace regmap
} // namespace scopy
#endif // REGISTERMAPTOOL_HPP
