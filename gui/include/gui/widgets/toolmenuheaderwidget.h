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

#ifndef TOOLMENUHEADERWIDGET_H
#define TOOLMENUHEADERWIDGET_H

#include <scopy-gui_export.h>
#include <QWidget>
#include <baseheader.h>
#include <QLineEdit>
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <ledbutton.h>
#include <QElapsedTimer>

namespace scopy {
class SCOPY_GUI_EXPORT ToolMenuHeaderWidget : public QWidget, public BaseHeader
{
	Q_OBJECT
public:
	typedef enum
	{
		SINGLE_INTERRUPT,
		STREAM_RUNNING,
		SINGLE_RUNNING,
		IDLE
	} LedState;
	ToolMenuHeaderWidget(QString title, QWidget *parent);
	~ToolMenuHeaderWidget();

	void setTitle(QString title) override;
	QString title() override;
	void setDeviceIcon(QPixmap icon);
	void setUri(QString uri);
	void setDeviceId(QString deviceId);
	QPushButton *deviceBtn() const;

	void setState(QString id, bool state, QWidget *parent);

Q_SIGNALS:
	void connState(QString id, bool isConnected);
	void blinkLed(int retCode);

private Q_SLOTS:
	void onBlinkLed(int retCode);
	void refresh();
	void onTimeout();

private:
	void handleSingle(bool isSuccess);
	void handleStream(bool isSuccess);

	QTimer *m_refreshTimer;
	QTimer *m_timer;
	QString m_id;
	LedButton *m_ledBtn;
	QPushButton *m_deviceBtn;
	QLineEdit *m_title;
	QLabel *m_uriLabel;
	LedState m_ledState;

	const int LED_ON_MSEC = 100;
	const int WAITING_FACTOR = 10;
};
} // namespace scopy

#endif // TOOLMENUHEADERWIDGET_H
