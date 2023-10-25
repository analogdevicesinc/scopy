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

#ifndef SCOPY_STATUSMESSAGE_H
#define SCOPY_STATUSMESSAGE_H

#include <QLabel>
#include "scopy-pluginbase_export.h"

#define TIMESTAMP_FORMAT "[hh:mm ap] "

namespace scopy {
class StatusMessage : public QWidget
{
public:
	virtual ~StatusMessage() = default;

	virtual QString getText() = 0;
	virtual QWidget *getWidget() = 0;
	virtual int getDisplayTime() = 0;
};

class SCOPY_PLUGINBASE_EXPORT StatusMessageText : public StatusMessage
{
	Q_OBJECT
public:
	StatusMessageText(QString text, int ms = -1, QWidget *parent = nullptr);
	~StatusMessageText();

	QString getText() override;
	QWidget *getWidget() override;
	int getDisplayTime() override;

private:
	void prependDateTime();

	int m_ms;	   // display time
	QString m_text;	   // display text
	QWidget *m_widget; // widget
};

class SCOPY_PLUGINBASE_EXPORT StatusMessageWidget : public StatusMessage
{
	Q_OBJECT
public:
	StatusMessageWidget(QWidget *widget, QString description, int ms = -1, QWidget *parent = nullptr);
	~StatusMessageWidget();

	QString getText() override;
	QWidget *getWidget() override;
	int getDisplayTime() override;

private:
	int m_ms;	   // display time
	QString m_text;	   // display text
	QWidget *m_widget; // widget
};
} // namespace scopy

#endif // SCOPY_STATUSMESSAGE_H
