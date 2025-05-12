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

#ifndef SEARCHBARWIDGET_HPP
#define SEARCHBARWIDGET_HPP

#include "scopy-regmap_export.h"

#include <QWidget>

class QHBoxLayout;
class QPushButton;
class QLineEdit;
namespace scopy::regmap {
class SCOPY_REGMAP_EXPORT SearchBarWidget : public QWidget
{
	Q_OBJECT
public:
	explicit SearchBarWidget(QWidget *parent = nullptr);
	~SearchBarWidget();

	void setEnabled(bool enabled);

Q_SIGNALS:
	void requestSearch(QString searchParam);

private:
	QLineEdit *searchBar;
	QPushButton *searchButton;
	QHBoxLayout *layout;
	void applyStyle();
};
} // namespace scopy::regmap
#endif // SEARCHBARWIDGET_HPP
