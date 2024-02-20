/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef SWIOTCONFIG_H
#define SWIOTCONFIG_H

#include "configcontroller.h"
#include "src/config/drawarea.h"

#include "ui_swiotconfig.h"

#include <QMap>
#include <QPushButton>
#include <QScrollArea>
#include <iio.h>
#include <iioutil/commandqueue.h>
#include <gui/tooltemplate.h>

#define NUMBER_OF_CHANNELS 4

namespace scopy::swiot {

class SwiotConfig : public QWidget
{
	Q_OBJECT
public:
	SwiotConfig(QString uri, QWidget *parent = nullptr);
	~SwiotConfig();

public Q_SLOTS:
	void onConfigBtnPressed();
	void modeAttributeChanged(std::string mode);

Q_SIGNALS:
	void writeModeAttribute(std::string mode);
	void configBtnPressed();

private:
	QString m_uri;
	QMap<QString, struct iio_device *> m_iioDevices;
	iio_context *m_context;
	iio_device *m_swiotDevice;

	QVector<ConfigController *> m_controllers;
	QVector<QStringList *> m_funcAvailable;
	QPushButton *m_applyBtn;
	CommandQueue *m_commandQueue;

	ToolTemplate *m_tool;
	DrawArea *m_drawArea;
	QScrollArea *m_scrollArea;
	Ui::ConfigMenu *m_ui;

	void provideDeviceConnection();
	void setupUiElements();
	void buildGridLayout();
	void createPageLayout();
	void initTutorialProperties();
	QPushButton *createApplyBtn();
};
} // namespace scopy::swiot

#endif // SWIOTCONFIG_H
