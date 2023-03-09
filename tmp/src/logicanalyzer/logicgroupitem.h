/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef LOGICGROUPITEM_H
#define LOGICGROUPITEM_H

#include "gui/basemenuitem.h"

#include <QLabel>

namespace adiscope {
class LogicGroupItem : public BaseMenuItem
{
	Q_OBJECT
public:
	LogicGroupItem(const QString &name, QWidget *parent = nullptr);

	QString getName() const;

public Q_SLOTS:
	void setName(const QString &name);

Q_SIGNALS:
	void deleteBtnClicked();

private:
	void buildUi();

private:
	QLabel *m_nameLabel;
};
}

#endif // LOGICGROUPITEM_H
