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

#ifndef CONNECTDIALOG_HPP
#define CONNECTDIALOG_HPP

#include <QWidget>
#include <QProcess>

extern "C" {
	struct iio_context;
}

namespace Ui {
class Connect;
}

namespace adiscope {
class ConnectDialog : public QWidget
{
	Q_OBJECT

public:
	explicit ConnectDialog(QWidget *widget = 0);
	~ConnectDialog();


	static QString URIstringParser(QString uri);

Q_SIGNALS:
	void newContext(const QString& uri);
	void finished(struct iio_context *ctx);

public Q_SLOTS:
	void discardSettings();

private Q_SLOTS:
	void btnClicked();
	void enableDemoBtn();
	void validateInput();
	void updatePopUp(struct iio_context *ctx);


private:
	Ui::Connect *ui;
	bool connected;
	bool enableDemo;
	QProcess *process;
	void createContext(const QString& uri);
	bool eventFilter(QObject *watched, QEvent *event);
};
}

#endif /* CONNECTDIALOG_HPP */
