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

#ifndef SCOPY_VERSIONCHECKMESSAGE_H
#define SCOPY_VERSIONCHECKMESSAGE_H

#include <QPushButton>
#include <QWidget>

#include <popupwidget.h>

namespace scopy {
class VersionCheckMessage : public QWidget
{
	Q_OBJECT

public:
	explicit VersionCheckMessage(QWidget *parent = nullptr);
	~VersionCheckMessage();

Q_SIGNALS:
	void setCheckVersion(bool allowed);

private Q_SLOTS:
	void saveCheckVersion(bool allowed);
};
} // namespace scopy

#endif // SCOPY_VERSIONCHECKMESSAGE_H
