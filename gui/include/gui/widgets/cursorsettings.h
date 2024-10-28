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

#ifndef CURSORSETTINGS_H
#define CURSORSETTINGS_H

#include <QVBoxLayout>
#include <QWidget>

#include <scopy-gui_export.h>
#include <utils.h>
#include <widgets/menuonoffswitch.h>
#include <widgets/menusectionwidget.h>

namespace scopy {

class SCOPY_GUI_EXPORT CursorSettings : public QWidget
{
	Q_OBJECT
public:
	CursorSettings(QWidget *parent = nullptr);
	~CursorSettings();

	QAbstractButton *getXEn();
	QAbstractButton *getXLock();
	QAbstractButton *getXTrack();
	QAbstractButton *getYEn();
	QAbstractButton *getYLock();
	QAbstractButton *getReadoutsDrag();

	void updateSession();

Q_SIGNALS:
	void sessionUpdated();

protected:
	void initSession();

private:
	MenuSectionWidget *xControls;
	MenuOnOffSwitch *xEn;
	MenuOnOffSwitch *xLock;
	MenuSectionWidget *yControls;
	MenuOnOffSwitch *yEn;
	MenuOnOffSwitch *yLock;
	MenuOnOffSwitch *xTrack;
	MenuSectionWidget *readoutsControls;
	MenuOnOffSwitch *readoutsDrag;
	QVBoxLayout *layout;

	void initUI();
	void connectSignals();
};
} // namespace scopy

#endif // CURSORSETTINGS_H
