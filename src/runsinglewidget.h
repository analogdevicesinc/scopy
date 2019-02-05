/*
 * Copyright 2018 Analog Devices, Inc.
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

#ifndef RUNSINGLEWIDGET_H
#define RUNSINGLEWIDGET_H

#include <QWidget>

namespace Ui {
class RunSingleWidget;
}

namespace adiscope {
class RunSingleWidget : public QWidget
{
	Q_OBJECT

public:
	explicit RunSingleWidget(QWidget *parent = nullptr);
	~RunSingleWidget();

	void enableSingleButton(bool enable);
	bool singleButtonEnabled() const;

	bool singleButtonChecked() const;
	bool runButtonChecked() const;

public Q_SLOTS:
	void toggle(bool);
	void single();

Q_SIGNALS:
	void toggled(bool);

private Q_SLOTS:
	void _toggle(bool checked);

private:
	Ui::RunSingleWidget *d_ui;
	bool d_singleButtonEnabled;
};
}

#endif // RUNSINGLEWIDGET_H
