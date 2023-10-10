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

#ifndef DEBUGGERINSTRUMENT_H
#define DEBUGGERINSTRUMENT_H

#include <iio.h>

#include <gui/tool_view.hpp>

/* Qt includes */
#include <QFile>
#include <QMainWindow>
#include <QPushButton>

/* Local includes */
#include "bitfieldwidget.h"
#include "debuggercontroller.h"
#include "registerwidget.h"

class QJSEngine;

namespace Ui {
class DebuggerInstrument;
}

namespace scopy {
namespace debugger {
class DebuggerInstrument : public QWidget
{
	Q_OBJECT

public:
	explicit DebuggerInstrument(struct iio_context *ctx, QJSEngine *engine, QWidget *parent = 0);
	~DebuggerInstrument();

	void updateReadValue(QString value);
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

private:
	Ui::DebuggerInstrument *ui;
	QPushButton *menuRunButton;
	scopy::gui::ToolView *m_toolView;
	QWidget *m_widget;

	QJSEngine *eng;

	DebuggerController m_debugController;
	QFile scriptFile;

	RegisterWidget *reg;
	QVector<BitfieldWidget *> bitfieldsVector;
};
} // namespace debugger
} // namespace scopy

#endif // DEBUGGERINSTRUMENT_H
