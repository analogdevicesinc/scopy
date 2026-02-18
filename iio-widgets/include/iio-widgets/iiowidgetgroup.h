/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef SCOPY_IIOWIDGETGROUP_H
#define SCOPY_IIOWIDGETGROUP_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include "iiowidget.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT IIOWidgetGroup : public QObject
{
	Q_OBJECT
public:
	explicit IIOWidgetGroup(QObject *parent = nullptr);
	~IIOWidgetGroup();

	void add(IIOWidget *widget);
	void add(QList<IIOWidget *> widgets);

	IIOWidget *get(const QString &key) const;
	QMap<QString, IIOWidget *> getAll() const;

	void remove(const QString &key);
	bool contains(const QString &key) const;
	QStringList keys() const;
	void clear();

	static QString generateKey(const IIOWidgetFactoryRecipe &recipe);

private:
	QMap<QString, IIOWidget *> m_widgets;
};
} // namespace scopy

#endif // SCOPY_IIOWIDGETGROUP_H
