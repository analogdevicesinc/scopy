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
 *
 */

#ifndef INFOHEADERWIDGET_H
#define INFOHEADERWIDGET_H

#include "toolbuttons.h"
#include <scopy-gui_export.h>
#include <QLineEdit>
#include <QWidget>
#include <baseheader.h>

namespace scopy {
class SCOPY_GUI_EXPORT InfoHeaderWidget : public QWidget, public BaseHeader
{
	Q_OBJECT
public:
	InfoHeaderWidget(QString title, QWidget *parent = nullptr);
	~InfoHeaderWidget();

	void setTitle(QString title);
	QString title();

	void setDescription(const QString &newDescription);

	InfoBtn *infoBtn() const;

	QLineEdit *titleEdit() const;

private:
	QLineEdit *m_titleEdit;
	InfoBtn *m_infoBtn;
	QString m_description;
	void setupDescriptionButton();
};
} // namespace scopy

#endif // INFOHEADERWIDGET_H
