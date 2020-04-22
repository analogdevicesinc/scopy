/*
 * Copyright 2017 Analog Devices, Inc.
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

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <iio.h>

/* Qt includes */
#include <QMainWindow>

/* Local includes */
#include "bitfieldwidget.h"
#include "debug.h"
#include "detachedWindow.hpp"
#include "filter.hpp"
#include "registerwidget.h"
#include "tool.hpp"
#include "tool_launcher.hpp"

class QJSEngine;

namespace Ui {
class Debugger;
}

namespace adiscope {

class Debugger : public Tool
{
	Q_OBJECT

public:
	explicit Debugger(struct iio_context* ctx, Filter* filt, ToolMenuItem* toolMenuItem, QJSEngine* engine,
			  ToolLauncher* parent = 0);
	~Debugger();

Q_SIGNALS:
	void newDebuggerInstance();

public Q_SLOTS:
	void updateSources(void);
	void updateChannelComboBox(QString devName);
	void updateAttributeComboBox(QString channel);
	void updateFilename(int index);
	void updateValueWidget(QString attribute);
	void updateRegMap(void);

	void on_ReadButton_clicked();
	void on_WriteButton_clicked();

	void on_readRegPushButton_clicked();
	void on_writeRegPushButton_clicked();

private Q_SLOTS:
	void on_detailedRegMapCheckBox_stateChanged(int arg1);

	void on_newWindowButton_clicked();

	void on_loadButton_clicked();

	void on_runButton_clicked();

private:
	Ui::Debugger* ui;
	QPushButton* menuRunButton;
	Filter* filter;
	QJSEngine* eng;

	Debug debug;
	QFile scriptFile;

	RegisterWidget* reg;
	QVector<BitfieldWidget*> bitfieldsVector;
};
} // namespace adiscope

#endif // DEBUGGER_H
