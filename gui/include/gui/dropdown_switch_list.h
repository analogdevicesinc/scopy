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

#ifndef DROPDOWN_SWITCH_LIST_H
#define DROPDOWN_SWITCH_LIST_H

#include <QComboBox>
#include "ComboBoxLineEdit.h"
#include "scopygui_export.h"

class QStandardItemModel;
class QTreeView;

namespace scopy {
	class SCOPYGUI_EXPORT DropdownSwitchList: public QComboBox
	{
		Q_OBJECT

	public:
		DropdownSwitchList(int switchColCount = 1, QWidget *parent = 0);

		QString title() const;
		void setTitle(const QString& title);

		QString columnTitle(int col) const;
		void setColumnTitle(int col, const QString& title);

		int switchColumnCount() const;

		void addDropdownElement(const QIcon&, const QString&);
		void addDropdownElement(const QIcon&, const QString&,
			const QVariant& user_data);

		void removeItem(int index);
		void showPopup() override;
		void hidePopup() override;

	protected Q_SLOTS:
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);

	private Q_SLOTS:
		void resetIndex(int);

	private:
		QString m_title;
		int m_rows;
		int m_columns;
		QStandardItemModel *m_model;
		QTreeView *m_treeView;
		QStringList m_colTitles;
		bool m_popVisible;
		bool m_mouseInside;
		bool m_mousePressed;
		ComboBoxLineEdit *m_lineEdit;
	};
}

#endif // DROPDOWN_SWITCH_LIST_H
