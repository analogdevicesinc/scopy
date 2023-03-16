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

#ifndef RUNSINGLEWIDGET_H
#define RUNSINGLEWIDGET_H

#include <QPushButton>
#include <QWidget>
#include "scopygui_export.h"

namespace Ui {
class RunSingleWidget;
}

namespace adiscope {
class SCOPYGUI_EXPORT RunSingleWidget : public QWidget
{
	Q_OBJECT

public:
	explicit RunSingleWidget(QWidget *parent = nullptr);
	~RunSingleWidget();

	void enableSingleButton(bool enable);
	bool singleButtonEnabled() const;

	void enableRunButton(bool enable);
	bool runButtonEnabled() const;

	bool singleButtonChecked() const;
	bool runButtonChecked() const;

	QPushButton* getRunButton();
	QPushButton* getSingleButton();

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
	bool d_runButtonEnabled;
};
}

#endif // RUNSINGLEWIDGET_H
