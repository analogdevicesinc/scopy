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

#ifndef DACINSTRUMENT_H
#define DACINSTRUMENT_H
#include "scopy-dac_export.h"

#include <QWidget>
#include <QPushButton>
#include <QList>

#include "verticalchannelmanager.h"
#include <gui/tooltemplate.h>
#include <gui/tutorialbuilder.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/menucontrolbutton.h>
#include <tooltemplate.h>
#include <iioutil/connectionprovider.h>

namespace scopy {
namespace dac {
class DacDataManager;
class SCOPY_DAC_EXPORT DacInstrument : public QWidget
{
	Q_OBJECT
public:
	DacInstrument(const Connection *conn, QWidget *parent = nullptr);
	virtual ~DacInstrument();

public Q_SLOTS:
	void startTutorial();
	void runToggled(bool toggled);
	void dacRunning(bool toggled);
Q_SIGNALS:
	void running(bool toggled);

private:
	void setupDacDataManagers();
	MenuControlButton *addDevice(DacDataManager *dev, QWidget *parent);
	void addDeviceToStack(DacDataManager *dev, MenuControlButton *btn);
	void setupDacDataDeviceButtons();
	void startBufferTutorial();
	void startBufferNonCyclicTutorial();
	void startDdsTutorial();
	void abortTutorial();

	const Connection *m_conn;
	ToolTemplate *tool;
	InfoBtn *infoBtn;
	GearBtn *settingsBtn;
	QPushButton *openLastMenuBtn;
	QButtonGroup *devicesGroup;
	QButtonGroup *rightMenuBtnGrp;
	MenuControlButton *devicesBtn;
	MapStackedWidget *deviceStack;
	MapStackedWidget *dacManagerStack;
	QList<DacDataManager *> m_dacDataManagers;
	VerticalChannelManager *vcm;
	gui::TutorialBuilder *m_dacBufferTutorial;
	gui::TutorialBuilder *m_dacBufferNonCyclicTutorial;
	gui::TutorialBuilder *m_dacDdsTutorial;
	int uuid = 0;

	const QString settingsMenuId = "settings";
	const QString devicesMenuId = "devices";
	const QString verticalChannelManagerId = "vcm";
};
} // namespace dac
} // namespace scopy

#endif // DACINSTRUMENT_H_
