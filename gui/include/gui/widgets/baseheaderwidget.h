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

#ifndef BASEHEADERWIDGET_H
#define BASEHEADERWIDGET_H

#include <QLineEdit>
#include <baseheader.h>
#include <scopy-gui_export.h>
#include <QVBoxLayout>

namespace scopy {
class SCOPY_GUI_EXPORT BaseHeaderWidget : public QWidget, public BaseHeader
{
	Q_OBJECT
public:
	BaseHeaderWidget(QString title, QWidget *parent = nullptr);
	~BaseHeaderWidget();

	void setTitle(QString title) override;
	QString title() override;

protected:
	QLineEdit *m_label;
	QVBoxLayout *m_lay;
};
} // namespace scopy

#endif // BASEHEADERWIDGET_H
