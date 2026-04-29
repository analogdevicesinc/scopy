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

#ifndef DAQ2TOOL_H
#define DAQ2TOOL_H

#include "scopy-daq2_export.h"
#include <QWidget>
#include <tooltemplate.h>
#include <animatedrefreshbtn.h>
#include <iio-widgets/iiowidgetbuilder.h>

#include <iio.h>

namespace scopy {
class IIOWidgetGroup;

namespace daq2 {

class SCOPY_DAQ2_EXPORT DAQ2 : public QWidget
{
	Q_OBJECT
public:
	explicit DAQ2(iio_context *ctx, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~DAQ2();

Q_SIGNALS:
	void readRequested();

private:
	QWidget *buildAdcSection();
	QWidget *buildDacSection();

	iio_context *m_ctx = nullptr;
	IIOWidgetGroup *m_group = nullptr;
	ToolTemplate *m_tool;
	AnimatedRefreshBtn *m_refreshButton;
};

} // namespace daq2
} // namespace scopy
#endif // DAQ2TOOL_H
