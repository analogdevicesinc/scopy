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

#include <QMap>
#include <QPushButton>
#include <QScrollArea>
#include <gui/tooltemplate.h>

#include <component/controller.h>

#define NUMBER_OF_CHANNELS 4

namespace scopy {
namespace component {
class Device;
}
namespace swiot {

class SWIOT_API;

class SwiotConfig : public QWidget
{
	Q_OBJECT
	friend class SWIOT_API;

public:
	SwiotConfig(QString uri, QWidget *parent = nullptr);
	~SwiotConfig();

public Q_SLOTS:
	void onConfigBtnPressed();
	void startTutorial();

Q_SIGNALS:
	void writeModeAttribute(QString mode);
	void configBtnPressed();

private:
	QString m_uri;
	component::ContextHandle m_context;
	component::Device *m_swiotDevice;

	QVector<ConfigController *> m_controllers;
	QVector<QStringList *> m_funcAvailable;
	QPushButton *m_applyBtn;

	ToolTemplate *m_tool;
	DrawArea *m_drawArea;
	QScrollArea *m_scrollArea;
	QWidget *m_chnlsGrid;

	void provideDeviceConnection();
	void setupUiElements();
	void buildGridLayout();
	void createPageLayout();
	void initTutorialProperties();
	QWidget *createGridHeader(QWidget *parent);
	QPushButton *createApplyBtn();

	void showEvent(QShowEvent *event) override;
};
} // namespace swiot
} // namespace scopy

#endif // SWIOTCONFIG_H
