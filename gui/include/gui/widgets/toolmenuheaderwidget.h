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

namespace scopy {
class SCOPY_GUI_EXPORT ToolMenuHeaderWidget : public QWidget, public BaseHeader
{
	Q_OBJECT
public:
	ToolMenuHeaderWidget(QString title, QWidget *parent);
	~ToolMenuHeaderWidget();

	void setTitle(QString title) override;
	QString title() override;
	void setDeviceIcon(QPixmap icon);
	void setUri(QString uri);

Q_SIGNALS:
	void blinkLed(int retCode);

private Q_SLOTS:
	void onBlinkLed(int retCode);

private:
	QTimer *m_timer;
	LedButton *m_ledBtn;
	QPushButton *m_deviceBtn;
	QLineEdit *m_title;
	QLabel *m_uriLabel;

	const int LED_ON_MSEC = 200;
	const int WAITING_FACTOR = 10;
};
} // namespace scopy

#endif // TOOLMENUHEADERWIDGET_H
