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

#ifndef MAPSTACKEDWIDGET_H
#define MAPSTACKEDWIDGET_H

#include "scopy-gui_export.h"

#include <QMap>
#include <QStackedWidget>

namespace scopy {
class SCOPY_GUI_EXPORT MapStackedWidget : public QStackedWidget
{
	Q_OBJECT
public:
	explicit MapStackedWidget(QWidget *parent = nullptr);
	~MapStackedWidget();

	virtual void add(QString key, QWidget *w);
	virtual bool remove(QString key);
	virtual QString getKey(QWidget *w);
	virtual bool contains(QString key);
	virtual QWidget *get(QString key);

	QSize sizeHint() const override;
	QSize minimumSizeHint() const override;

public Q_SLOTS:
	virtual bool show(QString key);

protected:
	QMap<QString, QWidget *> map;
};
} // namespace scopy
#endif // MAPSTACKEDWIDGET_H
