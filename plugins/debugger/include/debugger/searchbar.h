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

#ifndef SCOPY_SEARCHBAR_H
#define SCOPY_SEARCHBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QCompleter>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QSet>

namespace scopy::debugger {
class SearchBar : public QWidget
{
	Q_OBJECT
public:
	explicit SearchBar(QSet<QString> options, QWidget *parent = nullptr);

	QLineEdit *getLineEdit();

private:
	QLabel *m_label;
	QLineEdit *m_lineEdit;
	QCompleter *m_completer;
};
} // namespace scopy::debugger

#endif // SCOPY_SEARCHBAR_H
