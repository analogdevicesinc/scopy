/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 */

#ifndef ADALM2000DIGITALIOTOOL_H
#define ADALM2000DIGITALIOTOOL_H

#include "digitaliomath.h"
#include "scopy-adalm2000_export.h"

#include <QWidget>

#include <gui/widgets/toolbuttons.h>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <pluginbase/toolmenuentry.h>
#include <qcoro/qcorotask.h>
#include <tooltemplate.h>

namespace scopy {
namespace component {
class Context;
}

namespace adalm2000 {
class DioGroupWidget;
class DigitalIO_API;
class M2kDigitalIoController;

class SCOPY_ADALM2000_EXPORT Adalm2000DigitalIoTool : public QWidget
{
	Q_OBJECT

public:
	explicit Adalm2000DigitalIoTool(ToolMenuEntry *tme, component::Context *ctx, IIOWidgetGroup *group,
					QWidget *parent = nullptr);
	~Adalm2000DigitalIoTool() override;

protected:
	// Polling is visibility-scoped and independent of Run.
	void showEvent(QShowEvent *event) override;
	void hideEvent(QHideEvent *event) override;

private:
	friend class DigitalIO_API;

	void setupUi();
	QCoro::Task<void> applyOutputEnabled(bool on);

	DioGroupWidget *groupWidget(int group) const;
	bool isRunning() const;
	// Blocks; never call it from a UI slot.
	void setRunningBlocking(bool running);

	ToolMenuEntry *m_tme;
	component::Context *m_ctx;
	IIOWidgetGroup *m_group;
	M2kDigitalIoController *m_controller = nullptr;
	DigitalIO_API *m_api = nullptr;

	ToolTemplate *m_tool = nullptr;
	RunBtn *m_runBtn = nullptr;
	DioGroupWidget *m_groups[dio::GROUP_COUNT] = {};
};

} // namespace adalm2000
} // namespace scopy
#endif // ADALM2000DIGITALIOTOOL_H
