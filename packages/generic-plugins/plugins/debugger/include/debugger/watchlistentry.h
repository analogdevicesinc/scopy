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

#ifndef WATCHLISTENTRY_H
#define WATCHLISTENTRY_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidgetItem>

#include "iiostandarditem.h"

namespace scopy::debugger {
class WatchListEntry : public QObject
{
	Q_OBJECT
public:
	explicit WatchListEntry(IIOStandardItem *item, QObject *parent = nullptr);
	~WatchListEntry();

	QTableWidgetItem *name();
	void setName(QString name);

	QTableWidgetItem *path();
	void setPath(QString path);

	IIOStandardItem *item();

	QTableWidgetItem *type() const;
	void setType(QString type);

	// the value entry can only be lineedit or combo box, other options do not have enough space to look good
	QWidget *valueUi() const;
	void setValueUi(QWidget *newValueUi);

private Q_SLOTS:
	void setNewData(QString data, QString optionalData);

private:
	void setupUi();
	void setupWidget(IIOWidget *widget);

	IIOStandardItem *m_item;
	QTableWidgetItem *m_name;
	QWidget *m_valueUi;
	QTableWidgetItem *m_type;
	QTableWidgetItem *m_path;

	QComboBox *m_combo;
	QLineEdit *m_lineedit;
};
} // namespace scopy::debugger

#endif // WATCHLISTENTRY_H
