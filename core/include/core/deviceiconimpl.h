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

#ifndef DEVICEICONIMPL_H
#define DEVICEICONIMPL_H

#include "device.h"
#include "gui/utils.h"
#include "scopy-core_export.h"

#include <QAbstractButton>
#include <QPaintEvent>
#include <QWidget>

#include <deviceicon.h>

namespace Ui {
class DeviceButton;
};

namespace scopy {
class SCOPY_CORE_EXPORT DeviceIconImpl : public DeviceIcon
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	explicit DeviceIconImpl(Device *d, QWidget *parent);
	~DeviceIconImpl();
public Q_SLOTS:
	void setConnecting(bool) override;
	void setConnected(bool) override;
Q_SIGNALS:
	void displayNameChanged(QString newName);

private Q_SLOTS:
	void onPenBtnPressed();
	void onEditFinished();

private:
	void createPenBtn();

	Ui::DeviceButton *ui;
};
} // namespace scopy

#endif // DEVICEICONIMPL_H
