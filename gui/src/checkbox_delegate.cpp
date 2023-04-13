/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "checkbox_delegate.h"

#include <QApplication>
#include <QCheckBox>

using namespace scopy;

CheckboxDelegate::CheckboxDelegate(QObject *parent):
	QStyledItemDelegate(parent)
{
}

QWidget *CheckboxDelegate::createEditor(QWidget *parent,
	const QStyleOptionViewItem &, const QModelIndex &) const
{
	QCheckBox *editor = new QCheckBox(parent);
	editor->setAutoFillBackground(true);

	connect(editor, SIGNAL(toggled(bool)),
		this, SLOT(commitDataToModel()));

	return editor;
}

void CheckboxDelegate::setEditorData(QWidget *editor,
		const QModelIndex &index) const
{
	bool state = index.model()->data(index, Qt::EditRole).toBool();

	QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
	checkBox->setChecked(state);
}

void CheckboxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const
{
	QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
	int val;

	if (checkBox->isChecked())
		val = 1;
	else
		val = 0;

	model->setData(index, val, Qt::EditRole);
}

void CheckboxDelegate::commitDataToModel()
{
	QCheckBox *editor = qobject_cast<QCheckBox *>(sender());

	Q_EMIT commitData(editor);
}
