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

#ifndef AD74413R_H
#define AD74413R_H

#include "bufferlogic.h"
#include "buffermenucontroller.h"
#include "bufferplothandler.h"
#include "pluginbase/toolmenuentry.h"
#include "src/runtime/readerthread.h"

#include <iio.h>

#include <QVector>
#include <qwidget.h>

#include <gui/channel_manager.hpp>
#include <gui/osc_export_settings.h>
#include <gui/tool_view.hpp>

#define MAX_CURVES_NUMBER 8
#define AD_NAME "ad74413r"
#define SWIOT_DEVICE_NAME "swiot"

extern "C"
{
	struct iio_device;
	struct iio_channel;
	struct iio_buffer;
}

namespace scopy {
namespace gui {
class GenericMenu;
}

namespace swiot {
class Ad74413r : public QWidget
{
	Q_OBJECT
public:
	explicit Ad74413r(iio_context *ctx = nullptr, ToolMenuEntry *tme = 0, QWidget *parent = nullptr);

	~Ad74413r();

	void initChannelToolView(unsigned int i, QString function);

	void initExportSettings(QWidget *parent);

	void verifyChnlsChanges();

	void createDevicesMap(iio_context *ctx);

	scopy::gui::GenericMenu *createSettingsMenu(QString title, QColor *color);

public Q_SLOTS:
	void onChannelWidgetEnabled(int chnWidgetId, bool en);
	void onChannelWidgetSelected(int chnWidgetId, bool en);
	void onOffsetHdlSelected(int hdlIdx, bool selected);

	void onRunBtnPressed(bool toggled);
	void onSingleBtnPressed(bool toggled);

	void onReaderThreadFinished();
	void onSingleCaptureFinished();

	void externalPowerSupply(bool ps);
	void onDiagnosticFunctionUpdated();

	void onActivateRunBtns(bool activate);

Q_SIGNALS:
	void broadcastReadThreshold(QString value);
	void thresholdControlEnable(bool enable);
	void channelWidgetEnabled(int curveId, bool en);
	void channelWidgetSelected(int curveId);

	void exportBtnClicked(QMap<int, bool> exportConfig);

	void activateExportButton();
	void activateRunBtns(bool activate);

	void backBtnPressed();
private Q_SLOTS:
	void onBackBtnPressed();

private:
	void setupToolView(gui::GenericMenu *settingsMenu);

	void createMonitorChannelMenu();

	void setupConnections();

	QPushButton *createBackBtn();

	void initTutorialProperties();

private:
	QMap<QString, iio_device *> m_iioDevicesMap;
	int m_enabledChnlsNo = 0;
	int m_currentChannelSelected;

	scopy::gui::ChannelManager *m_monitorChannelManager;
	scopy::gui::ToolView *m_toolView;
	QWidget *m_widget;
	QPushButton *m_backBtn;
	QLabel *m_statusLabel;
	QWidget *m_statusContainer;
	ToolMenuEntry *m_tme;

	QVector<BufferMenuController *> m_controllers;

	std::vector<bool> m_enabledChannels;
	std::vector<ChannelWidget *> m_channelWidgetList;

	BufferLogic *m_swiotAdLogic;
	ReaderThread *m_readerThread;
	BufferPlotHandler *m_plotHandler;
	CommandQueue *m_cmdQueue;

	QComboBox *m_samplingFreqOptions;
	PositionSpinButton *m_timespanSpin;
	ExportSettings *m_exportSettings;

	struct iio_context *m_ctx;
};
} // namespace swiot
} // namespace scopy
#endif // AD74413R_H
