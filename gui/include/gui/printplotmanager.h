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

#ifndef PRINTPLOTMANAGER_H
#define PRINTPLOTMANAGER_H

#include "scopy-gui_export.h"

#include <QObject>

#include <plotwidget.h>
#include <toolbuttons.h>

namespace scopy {
class SCOPY_GUI_EXPORT PrintPlotManager : public QObject
{
	Q_OBJECT
public:
	explicit PrintPlotManager(QObject *parent = nullptr);

	void printPlots(QList<PlotWidget *> plotList, QString toolName);
	void setPrintWithSymbols(bool printWithSymbols);
signals:

private:
	bool m_printWithSymbols = false;
};
} // namespace scopy
#endif // PRINTPLOTMANAGER_H
