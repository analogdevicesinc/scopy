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

#ifndef SCOPY_SCOPYSTATUSBAR_H
#define SCOPY_SCOPYSTATUSBAR_H

#include <QObject>
#include <QStatusBar>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QList>
#include <QStackedWidget>
#include <pluginbase/statusbarmanager.h>
#include "pluginbase/statusmessage.h"
#include "utils.h"
#include "menu_anim.hpp"
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT ScopyStatusBar : public MenuVAnim
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	explicit ScopyStatusBar(QWidget *parent = nullptr);
	~ScopyStatusBar();

Q_SIGNALS:
	void requestHistory();

public Q_SLOTS:
	void displayStatusMessage(StatusMessage *statusMessage);
	void clearStatusMessage();

private:
	void initUi();

	void addToLeft(QWidget *widget);
	void removeLastStatusMessage();

	QList<StatusMessage *> m_messages;

	// UI elements
	QStackedWidget *m_stackedWidget;
	StatusBarManager *m_statusManager;
};
} // namespace scopy

#endif // SCOPY_SCOPYSTATUSBAR_H
