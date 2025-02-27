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

#ifndef INSTALLPKGDIALOG_H
#define INSTALLPKGDIALOG_H

#include <QPushButton>
#include <QWidget>
#include <popupwidget.h>
#include <scopy-gui_export.h>

namespace scopy {
class SCOPY_GUI_EXPORT InstallPkgDialog : public QWidget
{
	Q_OBJECT
public:
	InstallPkgDialog(QWidget *parent = nullptr);
	~InstallPkgDialog();
	void showDialog();
	void setMessage(const QString &msg);

Q_SIGNALS:
	void yesClicked();
	void noClicked();

private:
	QWidget *parent;
	PopupWidget *m_popupWidget;
};
} // namespace scopy

#endif // INSTALLPKGDIALOG_H
