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

#ifndef FILEBROWSERWIDGET_H
#define FILEBROWSERWIDGET_H

#include <scopy-gui_export.h>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

namespace scopy {
class SCOPY_GUI_EXPORT FileBrowserWidget : public QFrame
{
	Q_OBJECT
public:
	typedef enum
	{
		OPEN_FILE,
		SAVE_FILE,
		DIRECTORY
	} BrowserDialogType;

	FileBrowserWidget(BrowserDialogType type, QWidget *parent = nullptr);
	~FileBrowserWidget();

	QLineEdit *lineEdit() const;
	QPushButton *btn() const;
	void browseFile();

	void setFilter(const QString &newFilter);

	void setBaseDirectory(const QString &newBaseDirectory);

private:
	QLineEdit *m_lineEdit;
	QPushButton *m_btn;
	BrowserDialogType m_type;
	QString m_filter;
	QString m_baseDirectory;
};

} // namespace scopy

#endif // FILEBROWSERWIDGET_H
