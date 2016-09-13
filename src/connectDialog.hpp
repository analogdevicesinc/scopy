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

#ifndef CONNECTDIALOG_HPP
#define CONNECTDIALOG_HPP

#include <QWidget>

namespace Ui {
	class Connect;
}

namespace adiscope
{
	class ConnectDialog : public QObject
	{
		Q_OBJECT

	public:
		explicit ConnectDialog(QWidget *widget);
		~ConnectDialog();

	signals:
		void newContext(const QString& uri);

	private slots:
		void btnClicked();
		void validateInput();
		void discardSettings();

	private:
		Ui::Connect *ui;
		QString uri;
		bool connected;
	};
}

#endif /* CONNECTDIALOG_HPP */
