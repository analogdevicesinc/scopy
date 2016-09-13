/*
 * Copyright 2016 Analog Devices, Inc.
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

#include "filter.hpp"

#include <cstdio>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include <iio.h>

using namespace adiscope;

static const std::string tool_names[] = {
	"oscilloscope",
	"signal-generator",
	"voltmeter",
	"power-controller",
};

Filter::Filter(const std::string &id)
{
	QString dev_id = QString::fromStdString(id);
	QFile file(":/filter.json");

	file.open(QIODevice::ReadOnly | QIODevice::Text);

	auto doc = QJsonDocument::fromJson(file.readAll());

	if (!doc.object().contains(dev_id)) {
		fprintf(stderr, "Unable to find device in filter file\n");
		no_filter = true;
	} else {
		this->root = doc.object()[dev_id].toObject();
		no_filter = false;
	}
}

Filter::~Filter()
{
}

bool Filter::compatible(enum tool tool) const
{
	if (no_filter)
		return true;

	auto hdl = root["compatible"];
	if (!hdl.isArray())
		return false;
	else
		return hdl.toArray().contains(
				QString::fromStdString((tool_names[tool])));
}

bool Filter::usable(enum tool tool, const std::string &dev) const
{
	if (no_filter)
		return true;

	auto hdl = root[QString::fromStdString(tool_names[tool] + "-devices")];
	if (hdl.isNull())
		return true;
	if (!hdl.isArray())
		return false;

	return hdl.toArray().contains(QString::fromStdString(dev));
}

const std::string Filter::device_name(enum tool tool) const
{
	if (no_filter)
		throw std::runtime_error("No XML filter file");

	auto hdl = root[QString::fromStdString(tool_names[tool] + "-device")];
	if (!hdl.isString())
		throw std::runtime_error("Tool not compatible");

	return hdl.toString().toStdString();
}

struct iio_device * Filter::find_device(const struct iio_context *ctx,
		enum tool tool) const
{
	return iio_context_find_device(ctx, device_name(tool).c_str());
}
