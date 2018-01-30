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

#ifndef MANUALCALIBRATION_H
#define MANUALCALIBRATION_H

#include <iio.h>

/*Qt includes*/
#include <QObject>
#include <QString>

/*Local includes*/
#include "tool.hpp"
#include "detachedWindow.hpp"
#include "tool_launcher.hpp"
#include "filter.hpp"

class QJSEngine;

struct stCalibProcedure {
	QStringList *CalibText;
	struct iio_device * device;
	struct iio_channel * channel;
};

namespace Ui {
class ManualCalibration;
}

class ManualCalibration : public Tool
{
	Q_OBJECT

public:
	explicit ManualCalibration(struct iio_context *ctx, Filter *filt,
				   QPushButton *runButton, QJSEngine *engine,
				   ToolLauncher *parent = 0);

	~ManualCalibration();

	int startCalibration(struct stCalibProcedure calib, int index);

private:
	Ui::ManualCalibration *ui;
	Filter *filter;
	QJSEngine *eng;
};

#endif // MANUALCALIBRATION_H
