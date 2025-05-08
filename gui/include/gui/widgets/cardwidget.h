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

#ifndef CARDWIDGET_H
#define CARDWIDGET_H

#include "scopy-gui_export.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy {
class SCOPY_GUI_EXPORT CardWidget : public QFrame
{
	Q_OBJECT
public:
	CardWidget(QWidget *parent = nullptr);
	~CardWidget();

	QString id() const;
	QLineEdit *title() const;
	QLabel *subtitle() const;
	QLabel *description() const;
	QComboBox *versCb() const;

	void setId(const QString &newId);

protected:
	QVBoxLayout *m_layout;
	QString m_id;

private:
	QLineEdit *m_title;
	QLabel *m_subtitle;
	QComboBox *m_versCb;
	QLabel *m_description;
	QFrame *createTitleW(QWidget *parent);
};
} // namespace scopy

#endif // CARDWIDGET_H
