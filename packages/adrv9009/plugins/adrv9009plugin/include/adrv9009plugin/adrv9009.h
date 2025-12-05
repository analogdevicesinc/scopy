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
#include <animatedrefreshbtn.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/menusectionwidget.h>
#include "adrv9009widgetfactory.h"

namespace scopy::adrv9009 {

class SCOPY_ADRV9009PLUGIN_EXPORT Adrv9009 : public QWidget
{
	Q_OBJECT
public:
	Adrv9009(iio_context *ctx, QWidget *parent = nullptr);
	~Adrv9009();

Q_SIGNALS:
	void readRequested();

private:
	iio_context *m_ctx = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	QWidget *m_centralWidget;
	AnimatedRefreshBtn *m_refreshButton;
	QPushButton *m_mcsButton = nullptr;

	QMap<QString, iio_device *> m_adrv9009DeviceMap;
	bool m_multiDeviceMode = false;

	void setupUi();
	void detectAndStoreDevices();
	void performMcsSync();

	void loadProfileFromFile(QString filePath);
	QWidget *generateCalibrationWidget(iio_device *device, QWidget *parent);

	// Simple section generators
	QWidget *generateGlobalSettingsWidget(QString title, QWidget *parent);
	QWidget *generateRxChainWidget(QString title, QWidget *parent);
	QWidget *generateTxChainWidget(QString title, QWidget *parent);
	QWidget *generateObsRxChainWidget(QString title, QWidget *parent);
	QWidget *generateFpgaSettingsWidget(QString title, QWidget *parent);

	// Device-specific content creators
	QWidget *createGlobalSettingsContentForDevice(iio_device *device, QWidget *parent);
	QWidget *createRxChainContentForDevice(iio_device *device, QWidget *parent);
	QWidget *createTxChainContentForDevice(iio_device *device, QWidget *parent);
	QWidget *createObsRxChainContentForDevice(iio_device *device, QWidget *parent);
	QWidget *createFpgaSettingsContentForDevice(iio_device *device, QWidget *parent);

	// RX channel helper
	QWidget *createRxChannelWidget(iio_device *dev, QString title, int channelIndex, QWidget *parent);

	// TX channel helper
	QWidget *createTxChannelWidget(iio_device *dev, QString title, int channelIndex, QWidget *parent);

	// OBS channel helper
	QWidget *createObsChannelWidget(iio_device *dev, QString title, int channelIndex, QWidget *parent);

	// FPGA phase rotation channel helper
	QWidget *createFpgaRxChannelWidget(iio_device *dev, QString title, int channelIndex, QWidget *parent);

	// FPGA Widget Creation Helpers (for different device)
	IIOWidget *createFpgaRangeWidget(iio_device *dev, iio_channel *ch, const QString &attr, const QString &range,
					 const QString &title);

	// FPGA Phase Rotation Helpers
	void writePhase(iio_device *fpgaDev, int channelIndex, int degrees);
	void readPhase(iio_device *fpgaDev, int channelIndex, gui::MenuSpinbox *spinBox);
};

} // namespace scopy::adrv9009
#endif // ADRV9009_H
