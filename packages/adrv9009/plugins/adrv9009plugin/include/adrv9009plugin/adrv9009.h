/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef ADRV9009_H
#define ADRV9009_H

#include "scopy-adrv9009plugin_export.h"
#include <QBoxLayout>
#include <QMap>
#include <QPushButton>
#include <QTabWidget>
#include <QWidget>
#include <tooltemplate.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <animatedrefreshbtn.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/menusectionwidget.h>
#include "adrv9009widgetfactory.h"

#include <qcoro/qcorotask.h>

namespace scopy::component {
class Context;
class Device;
class Channel;
} // namespace scopy::component

namespace scopy::adrv9009 {

class Adrv9009Plugin_API;

class SCOPY_ADRV9009PLUGIN_EXPORT Adrv9009 : public QWidget
{
	Q_OBJECT
	friend class Adrv9009Plugin_API;

public:
	Adrv9009(component::Context *ctx, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~Adrv9009();

Q_SIGNALS:
	void readRequested();

private:
	component::Context *m_ctx = nullptr;
	IIOWidgetGroup *m_widgetGroup = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	QWidget *m_centralWidget;
	AnimatedRefreshBtn *m_refreshButton;
	QPushButton *m_mcsButton = nullptr;

	QMap<QString, component::Device *> m_adrv9009DeviceMap;
	bool m_multiDeviceMode = false;

	// magic number from iio-osc
	const int multichipSyncValue = 424242;

	void setupUi();
	void detectAndStoreDevices();
	QCoro::Task<void> performMcsSync();

	QCoro::Task<void> loadProfileFromFile(QString filePath);
	QWidget *generateCalibrationWidget(component::Device *device, QWidget *parent);

	// Simple section generators
	QWidget *generateGlobalSettingsWidget(QString title, QWidget *parent);
	QWidget *generateRxChainWidget(QString title, QWidget *parent);
	QWidget *generateTxChainWidget(QString title, QWidget *parent);
	QWidget *generateObsRxChainWidget(QString title, QWidget *parent);
	QWidget *generateFpgaSettingsWidget(QString title, QWidget *parent);

	// Device-specific content creators
	QWidget *createGlobalSettingsContentForDevice(component::Device *device, QWidget *parent);
	QWidget *createRxChainContentForDevice(component::Device *device, QWidget *parent);
	QWidget *createTxChainContentForDevice(component::Device *device, QWidget *parent);
	QWidget *createObsRxChainContentForDevice(component::Device *device, QWidget *parent);
	QWidget *createFpgaSettingsContentForDevice(component::Device *device, QWidget *parent);

	// RX channel helper
	QWidget *createRxChannelWidget(component::Device *dev, QString title, int channelIndex, QWidget *parent);

	// TX channel helper
	QWidget *createTxChannelWidget(component::Device *dev, QString title, int channelIndex, QWidget *parent);

	// OBS channel helper
	QWidget *createObsChannelWidget(component::Device *dev, QString title, int channelIndex, QWidget *parent);

	// FPGA phase rotation channel helper
	QWidget *createFpgaRxChannelWidget(component::Device *dev, QString title, int channelIndex, QWidget *parent);

	// FPGA Phase Rotation Helpers
	QCoro::Task<void> writePhase(component::Device *fpgaDev, int channelIndex, int degrees);
	QCoro::Task<void> readPhase(component::Device *fpgaDev, int channelIndex, gui::MenuSpinbox *spinBox);
};

} // namespace scopy::adrv9009
#endif // ADRV9009_H
