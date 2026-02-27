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

#ifndef AD9371TOOL_H
#define AD9371TOOL_H

#include "scopy-ad9371_export.h"
#include <QBoxLayout>
#include <QWidget>
#include <tooltemplate.h>

#include <iio-widgets/iiowidgetbuilder.h>

#include <animatedrefreshbtn.h>
#include <ad9371helper.h>

namespace scopy {
class IIOWidgetGroup;
namespace ad9371 {

class SCOPY_AD9371_EXPORT AD9371 : public QWidget
{
	Q_OBJECT
public:
	AD9371(iio_context *ctx, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~AD9371();

Q_SIGNALS:
	void readRequested();

private:
	iio_context *m_ctx = nullptr;
	IIOWidgetGroup *m_group = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	QWidget *m_controlsWidget;
	AnimatedRefreshBtn *m_refreshButton;

	AD9371Helper *m_helper;
};
} // namespace ad9371
} // namespace scopy
#endif // AD9371TOOL_H
