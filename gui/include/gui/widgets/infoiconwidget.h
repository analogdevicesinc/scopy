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

#ifndef INFOICONWIDGET_H
#define INFOICONWIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <scopy-gui_export.h>

namespace scopy {
class SCOPY_GUI_EXPORT InfoIconWidget : public QWidget
{
	Q_OBJECT
public:
	explicit InfoIconWidget(QString infoMessage, QWidget *parent = nullptr);

	void setInfoMessage(QString infoMessage);
	QString getInfoMessage();

private:
	QVBoxLayout *m_layout;
	QLabel *m_infoIcon;
};
} // namespace scopy

#endif // INFOICONWIDGET_H
