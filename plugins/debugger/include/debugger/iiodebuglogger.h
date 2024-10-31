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

#ifndef IIODEBUGLOGGER_H
#define IIODEBUGLOGGER_H

#include <QWidget>
#include <QDialog>
#include <QTextBrowser>

namespace scopy::debugger {
class IIODebugLogger : public QFrame
{
	Q_OBJECT
public:
	explicit IIODebugLogger(QWidget *parent = nullptr);
	void setupUi();

public Q_SLOTS:
	void appendLog(QString log);

private:
	QTextBrowser *m_textBrowser;
};
} // namespace scopy::debugger

#endif // IIODEBUGLOGGER_H
