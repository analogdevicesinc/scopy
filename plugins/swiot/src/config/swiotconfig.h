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

#define AD_NAME "ad74413r"
#define MAX_NAME "max14906"

#include "configcontroller.h"
#include "src/config/drawarea.h"

#include "ui_swiotconfig.h"

#include <QMap>
#include <QPushButton>
#include <QScrollArea>

#include <gui/tool_view.hpp>
#include <iioutil/commandqueue.h>

extern "C"
{
	struct iio_context;
	struct iio_device;
	struct iio_channel;
}

namespace scopy::swiot {

class SwiotConfig : public QWidget
{
	Q_OBJECT
public:
	explicit SwiotConfig(struct iio_context *ctx, QWidget *parent = nullptr);
	~SwiotConfig();

public Q_SLOTS:
	void onConfigBtnPressed();
	void externalPowerSupply(bool ps);
	void modeAttributeChanged(std::string mode);

Q_SIGNALS:
	void writeModeAttribute(std::string mode);
	void configBtnPressed();

private:
	QMap<QString, struct iio_device *> m_iioDevices;
	struct iio_context *m_context;
	struct iio_device *m_swiotDevice;

	QVector<ConfigController *> m_controllers;
	QVector<QStringList *> m_funcAvailable;
	QPushButton *m_configBtn;
	scopy::gui::ToolView *m_toolView;
	CommandQueue *m_commandQueue;

	DrawArea *m_drawArea;
	QScrollArea *m_scrollArea;
	QWidget *m_mainView;
	QLabel *m_statusLabel;
	QWidget *m_statusContainer;
	Ui::ConfigMenu *ui;

	void initTutorialProperties();
	void setupToolView(QWidget *parent);
	void init();
	void createPageLayout();
	void setDevices(struct iio_context *ctx);
	static QPushButton *createConfigBtn();
};
} // namespace scopy::swiot

#endif // SWIOTCONFIG_H
