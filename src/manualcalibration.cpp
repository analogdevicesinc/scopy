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

#include "manualcalibration.h"

ManualCalibration::ManualCalibration(struct iio_context *ctx, Filter *filt,
				     QPushButton *runButton, QJSEngine *engine,
				     ToolLauncher *parent) :
	Tool(ctx, runButton, nullptr, "Calibration", parent),
	ui(new Ui::Debugger), filter(filt),
	eng(engine)
{
	ui->setupUi(this);
}
