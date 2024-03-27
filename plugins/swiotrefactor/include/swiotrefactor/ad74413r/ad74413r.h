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
#include "bufferplothandler.h"
#include "pluginbase/toolmenuentry.h"
#include "buffermenucontroller.h"
#include "readerthread.h"

#include <iio.h>

#include <gui/mapstackedwidget.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/tooltemplate.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/plotaxis.h>

#include <QVector>
#include <QWidget>

#include <iioutil/connection.h>
#define MAX_CURVES_NUMBER 8
#define AD_NAME "ad74413r"
#define SWIOT_DEVICE_NAME "swiot"
#define MAX_SAMPLE_RATE 4800

namespace scopy {

namespace swiotrefactor {
class Ad74413r : public QWidget
{
	Q_OBJECT
public:
	explicit Ad74413r(QString uri = "", ToolMenuEntry *tme = 0, QWidget *parent = nullptr);

	~Ad74413r();

public Q_SLOTS:
	void onChannelWidgetEnabled(int chnWidgetId, bool en);

	void onRunBtnPressed(bool toggled);
	void onSingleBtnPressed(bool toggled);

	void onReaderThreadFinished();
	void onSingleCaptureFinished();

	void externalPowerSupply(bool ps);
	void onDiagnosticFunctionUpdated();

	void onActivateRunBtns(bool activate);

	void handleConnectionDestroyed();

	void onSamplingFrequencyUpdated(int channelId, int sampFreq);

Q_SIGNALS:
	void timespanChanged(double val);
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
	////
	/// \brief Added for the new adinstrument
	void showPlotLabels(bool b);
	void setupChannel(int chnlIdx, QString function);
	void onBufferRefilled(QMap<int, std::vector<double>> bufferData);
	void refreshSampleRate();
	///////////

private:
	void createDevicesMap(iio_context *ctx);
	void createMonitorChannelMenu();

	void setupConnections();
	void verifyChnlsChanges();
	void initTutorialProperties();
	/////
	/// \brief Added for the new adinstrument
	///
	void initPlotData();
	void setupToolTemplate();
	void initPlot();
	PlotAxis *createYChnlAxis(QPen pen, QString unitType = "V", int yMin = -1, int yMax = 1);
	void setupDeviceBtn();
	void setupChannelsMenuBtn(MenuControlButton *btn, QString name);
	void setupChannelMenuControlButtonHelper(MenuControlButton *btn, PlotChannel *ch, QString chnlId);
	QPushButton *createBackBtn();
	QWidget *createSettingsMenu(QWidget *parent);
	///////////////

private:
	int m_enabledChnlsNo = 0;

	QWidget *m_widget;
	QLabel *m_statusLabel;
	QWidget *m_statusContainer;
	ToolMenuEntry *m_tme;

	QVector<BufferMenuController *> m_controllers;

	std::vector<bool> m_enabledChannels;

	PositionSpinButton *m_timespanSpin;

	QString m_uri;

	BufferLogic *m_swiotAdLogic;
	ReaderThread *m_readerThread;
	BufferPlotHandler *m_plotHandler;
	CommandQueue *m_cmdQueue;

	//	PositionSpinButton *m_timespanSpin;
	//	ExportSettings *m_exportSettings;

	struct iio_context *m_ctx;
	Connection *m_conn;

	////
	/// \brief Added for the new adinstrument
	///
	double m_frequency = 1.0;
	double m_sampleRate = 4800;
	ToolTemplate *m_tool;
	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;
	PrintBtn *m_printBtn;
	QPushButton *m_backBtn;
	PlotWidget *m_plot;
	GearBtn *m_settingsBtn;

	QMap<QString, iio_device *> m_iioDevicesMap;
	CollapsableMenuControlButton *m_devBtn;
	QButtonGroup *m_rightMenuBtnGrp;
	QButtonGroup *m_chnlsBtnGroup;

	MapStackedWidget *m_channelStack;

	int m_currentChannelSelected = 0;
	QMap<int, std::vector<double>> m_yValues;
	std::vector<double> m_xTime;

	const QString channelsMenuId = "channels";
	/////////////////
};
} // namespace swiotrefactor
} // namespace scopy
#endif // AD74413R_H
