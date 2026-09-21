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
 *
 */

#ifndef FMCOMMS11_H
#define FMCOMMS11_H

#include "scopy-fmcomms11_export.h"
#include <QWidget>
#include <tooltemplate.h>
#include <animatedrefreshbtn.h>

#include <iio-widgets/iiowidgetbuilder.h>

namespace scopy {
class IIOWidgetGroup;

namespace component {
class Context;
class Device;
} // namespace component

namespace fmcomms11 {

class SCOPY_FMCOMMS11_EXPORT FMCOMMS11 : public QWidget
{
	Q_OBJECT
public:
	FMCOMMS11(component::Context *ctx, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~FMCOMMS11();

Q_SIGNALS:
	void readRequested();

private:
	void setupUi();
	QWidget *generateAdcWidget(QWidget *parent);
	QWidget *generateInputAttenuatorWidget(QWidget *parent);
	QWidget *generateDacWidget(QWidget *parent);
	QWidget *generateOutputVgaWidget(QWidget *parent);

	component::Context *m_ctx = nullptr;
	component::Device *m_adc = nullptr;
	component::Device *m_dac = nullptr;
	component::Device *m_attn = nullptr;
	component::Device *m_vga = nullptr;
	IIOWidgetGroup *m_group = nullptr;
	ToolTemplate *m_tool = nullptr;
	AnimatedRefreshBtn *m_refreshButton = nullptr;
};
} // namespace fmcomms11
} // namespace scopy
#endif // FMCOMMS11_H
