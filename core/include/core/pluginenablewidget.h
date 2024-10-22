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

#ifndef PLUGINENABLEWIDGET_H
#define PLUGINENABLEWIDGET_H

#include "gui/utils.h"
#include "qcheckbox.h"
#include "qlabel.h"

#include <qwidget.h>

namespace scopy {

class PluginEnableWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	explicit PluginEnableWidget(QWidget *parent = nullptr);
	~PluginEnableWidget();

	void setDescription(QString description);
	QCheckBox *checkBox() const;

private:
	QCheckBox *m_checkBox;
	QLabel *m_descriptionLabel;
};

} // namespace scopy

#endif // PLUGINENABLEWIDGET_H
