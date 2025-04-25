/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef SCOPY_IIOWIDGETGROUP_H
#define SCOPY_IIOWIDGETGROUP_H

#include "scopy-iio-widgets_export.h"
#include "iiowidget.h"
#include <QObject>

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT IIOWidgetGroup : public QObject
{
	Q_OBJECT

public:
	IIOWidgetGroup(bool singleTrigger = false, QObject *parent = nullptr);
	virtual ~IIOWidgetGroup();

	void add(IIOWidget *widget, bool triggerPoint = false);

private Q_SLOTS:

	void handleStatusChanged(QDateTime date, QString old, QString newD, int ret, bool readop);

private:
	QVector<IIOWidget *> m_widgets;
	bool m_singleTrigger;
	IIOWidget *m_trigger;
};
} // namespace scopy

#endif // SCOPY_IIOWIDGETGROUP_H
