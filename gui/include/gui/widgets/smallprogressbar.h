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

#ifndef SCOPY_SMALLPROGRESSBAR_H
#define SCOPY_SMALLPROGRESSBAR_H

#include <QWidget>
#include <QTimer>
#include <QProgressBar>
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT SmallProgressBar : public QProgressBar
{
	Q_OBJECT
public:
	explicit SmallProgressBar(QWidget *parent = nullptr);

public Q_SLOTS:
	void startProgress(int progressDurationMs = 1000, int steps = 10); // time is 1 second
	void setBarColor(QColor color);
	void resetBarColor();

Q_SIGNALS:
	void progressFinished();

private:
	QTimer *m_timer;
	int m_increment;
};
} // namespace scopy

#endif // SCOPY_SMALLPROGRESSBAR_H
