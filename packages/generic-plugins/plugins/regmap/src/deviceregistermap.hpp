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

#ifndef DEVICEREGISTERMAP_HPP
#define DEVICEREGISTERMAP_HPP

#include "scopy-regmap_export.h"

#include <QMap>
#include <QObject>
#include <QWidget>
#include <tooltemplate.h>
#include <tutorialbuilder.h>
#include "scopy-regmap_export.h"

class QVBoxLayout;

class QDockWidget;

namespace scopy::regmap {
class RegisterMapTemplate;
class RegisterModel;
class RegisterMapAccess;
class RegisterMapDataStore;
class RegisterDetailedWidget;
class RegisterController;
class SearchBarWidget;
class RegisterMapTable;

class SCOPY_REGMAP_EXPORT DeviceRegisterMap : public QWidget
{
	friend class RegMap_API;
	Q_OBJECT
public:
	explicit DeviceRegisterMap(RegisterMapTemplate *registerMapTemplate = nullptr,
				   RegisterMapAccess *registerMapAccess = nullptr, QWidget *parent = nullptr);
	~DeviceRegisterMap();

	void registerChanged(RegisterModel *regModel);
	void toggleAutoread(bool toggled);
	void applyFilters(QString filter);
	bool hasTemplate();
	bool hasReadWrite() const;
	bool getAutoread();
	void startTutorial();
	void startSimpleTutorial();

Q_SIGNALS:
	// forwarded from the settings menu to the register map values
	void requestReadInterval(uint32_t start, uint32_t end);
	void requestWriteFromFile(QString path);
	void requestRegisterDump(QString path, bool readFirst, uint32_t start, uint32_t end);
	void tutorialFinished();
	void simpleTutorialFinished();
	void tutorialAborted();

private:
	ToolTemplate *tool;
	bool autoread = false;
	QVBoxLayout *layout;
	RegisterMapTemplate *registerMapTemplate;
	RegisterMapAccess *registerMapAccess;
	RegisterMapDataStore *registerMapStore = nullptr;
	RegisterController *registerController = nullptr;

	RegisterMapTable *registerMapTableWidget = nullptr;
	QDockWidget *docRegisterMapTable = nullptr;

	RegisterDetailedWidget *registerDetailedWidget = nullptr;
	void initSettings();
	// shows the latest read result of a register: the table row, and the value field and bitfields if selected
	void showRegisterData(uint32_t address);
	uint32_t selectedAddress();
	int registerWidth();
	int selectedRegister;

	void initTutorial();
	void initSimpleTutorial();
	void abortTutorial();
	gui::TutorialBuilder *tutorial;
	QMetaObject::Connection controllerTutorialFinish;
	QMetaObject::Connection mapTutorialFinish;
};
} // namespace scopy::regmap
#endif // DEVICEREGISTERMAP_HPP
