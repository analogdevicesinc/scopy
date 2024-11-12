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
class RegisterMapValues;
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
				   RegisterMapValues *registerMapValues = nullptr, QWidget *parent = nullptr);
	~DeviceRegisterMap();

	void registerChanged(RegisterModel *regModel);
	void toggleAutoread(bool toggled);
	void applyFilters(QString filter);
	bool hasTemplate();
	bool getAutoread();
	void startTutorial();
	void startSimpleTutorial();

Q_SIGNALS:
	void requestRead(uint32_t address);
	void requestWrite(uint32_t address, uint32_t value);
	void requestRegisterDump(QString path);
	void tutorialFinished();
	void simpleTutorialFinished();
	void tutorialAborted();

private:
	ToolTemplate *tool;
	bool autoread = false;
	QVBoxLayout *layout;
	RegisterMapTemplate *registerMapTemplate;
	RegisterMapValues *registerMapValues;
	RegisterController *registerController = nullptr;

	RegisterMapTable *registerMapTableWidget = nullptr;
	QDockWidget *docRegisterMapTable = nullptr;

	RegisterDetailedWidget *registerDetailedWidget = nullptr;
	void initSettings();
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
