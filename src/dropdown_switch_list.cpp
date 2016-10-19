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

#include "dropdown_switch_list.h"
#include "checkbox_delegate.h"

#include <QLineEdit>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QHeaderView>
#include <QFile>

using namespace adiscope;

/*
 * Class DropdownSwitchList: Resembles to a QCombobox by displaying a list of
 *       items. An item has an icon placed on its left side and havs one or more
 *       QCheckBox widgets on its right side. As opposed to a QCombobox the
 *       items of this widget cannot be selected.
 */
DropdownSwitchList::DropdownSwitchList(int switchColCount, QWidget *parent):
	QComboBox(parent),
	m_title("Title"),
	m_rows(0),
	m_columns(1 + switchColCount),
	m_model(NULL),
	m_treeView(NULL)
{
	if (m_columns < 2)
		m_columns = 2;

	// Title of the dropdown
	setEditable(true);
	lineEdit()->setReadOnly(true);
	this->setTitle(m_title);

	// Force combobox to point to no item
	connect(this, SIGNAL(currentIndexChanged(int)),
		SLOT(resetIndex(int)));

	// Data
	m_model = new QStandardItemModel(0, m_columns, this);
	this->setModel(m_model);

	// View
	m_treeView = new QTreeView(this);
	this->setView(m_treeView);

	// Column titles
	for (int col = 0; col < m_columns; col++)
		m_colTitles.push_back(QString("Title %1").arg(col));
	m_model->setHorizontalHeaderLabels(m_colTitles);

	// Use checkboxes instead of the default editor (spinbuttons)
	CheckboxDelegate *delegate = new CheckboxDelegate(m_treeView);
	for (int col = 1; col < m_columns; col++)
		m_treeView->setItemDelegateForColumn(col, delegate);

	// View size and alignment settings
	m_treeView->header()->setSectionsMovable(false);
	m_treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_treeView->header()->setStretchLastSection(false);
	m_treeView->setAllColumnsShowFocus(false);
	m_treeView->setRootIsDecorated(true);
	m_treeView->setUniformRowHeights(true);
	m_treeView->header()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_treeView->header()->setDefaultAlignment(Qt::AlignLeft);

	QFile file(":stylesheets/stylesheets/dropdown_switch_list.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QString::fromLatin1(file.readAll());
	setStyleSheet(styleSheet);
}

QString DropdownSwitchList::title() const
{
	return m_title;
}

void DropdownSwitchList::setTitle(const QString& title)
{
	m_title = title;
	lineEdit()->setPlaceholderText(title);
}

QString DropdownSwitchList::columnTitle(int col) const
{
	if (col < m_columns)
		return m_colTitles[col];
	else
		return "";
}
void DropdownSwitchList::setColumnTitle(int col, const QString& title)
{
	if (col >= 0 && col < m_columns) {
		m_colTitles[col] = title;
		m_model->setHorizontalHeaderLabels(m_colTitles);
	}
}

int DropdownSwitchList::switchColumnCount() const
{
	return m_columns - 1;
}

void DropdownSwitchList::addDropdownElement(const QIcon& icon,
		const QString& name)
{
	QStandardItem *item = new QStandardItem(icon, name);
	m_model->setItem(m_rows, 0, item);
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

	for (int col = 1; col < m_columns; col++) {
		QModelIndex index = m_model->index(m_rows, col, QModelIndex());
		m_model->setData(index, QVariant(1));
		QStandardItem *item = m_model->item(m_rows, col);
		item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

		m_treeView->openPersistentEditor(index);
	}

	m_rows++;
}

void DropdownSwitchList::resetIndex(int)
{
	setCurrentIndex(-1);
}
