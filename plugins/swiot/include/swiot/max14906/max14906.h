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

#ifndef MAX14906_H
#define MAX14906_H

#include "diocontroller.h"
#include "diodigitalchannelcontroller.h"
#include "diosettingstab.h"
#include <readerthread.h>
#include <QWidget>
#include <QPushButton>
#include <gui/tooltemplate.h>
#include <gui/widgets/toolbuttons.h>
#include <pluginbase/toolmenuentry.h>
#include <iioutil/commandqueue.h>
#include <iioutil/connection.h>

#define MAX_NAME "max14906"
#define MAX14906_POLLING_TIME 1000

namespace scopy::swiot {
class Max14906 : public QWidget
{
	Q_OBJECT
public:
	Max14906(QString uri, ToolMenuEntry *tme, QWidget *parent = nullptr);
	~Max14906();

Q_SIGNALS:
	void configBtnPressed();

public Q_SLOTS:
	void handleConnectionDestroyed();

private Q_SLOTS:
	void runButtonToggled();
	void timerChanged(double value);
	void onConfigBtnPressed();

private:
	ToolTemplate *m_tool;
	QWidget *m_gridWidget;
	QPushButton *m_configBtn;
	RunBtn *m_runBtn;
	GearBtn *m_gearBtn;

	int m_nbDioChannels;
	DioController *m_max14906ToolController;
	DioSettingsTab *m_max14906SettingsTab = nullptr;

	QTimer *m_qTimer;

	CommandQueue *m_cmdQueue;
	ReaderThread *m_readerThread;
	Connection *m_conn;
	struct iio_context *m_ctx;
	QString m_uri;
	QMap<int, DioDigitalChannelController *> m_channelControls;

	ToolMenuEntry *m_tme;

	void initChannels();
	void setupDynamicUi(QWidget *parent);
	void initMonitorToolView();
	void connectSignalsAndSlots();

	static QFrame *createVLine(QWidget *parent);
	static QFrame *createHLine(QWidget *parent);
	static QMainWindow *createDockableMainWindow(const QString &title, DioDigitalChannel *digitalChannel,
						     QWidget *parent);
	QPushButton *createConfigBtn(QWidget *parent = nullptr);
};
} // namespace scopy::swiot
#endif // MAX14906_H
