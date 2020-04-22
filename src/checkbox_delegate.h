/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CHECKBOX_DELEGATE_H
#define CHECKBOX_DELEGATE_H

#include <QStyledItemDelegate>

namespace adiscope {
class CheckboxDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	CheckboxDelegate(QObject* parent = 0);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
			      const QModelIndex& index) const Q_DECL_OVERRIDE;

	void setEditorData(QWidget* editor, const QModelIndex& index) const Q_DECL_OVERRIDE;

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const Q_DECL_OVERRIDE;

private Q_SLOTS:
	void commitDataToModel();
};
} // namespace adiscope

#endif // CHECKBOX_DELEGATE_H
