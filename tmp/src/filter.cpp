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

#include "filter.hpp"

#include <cstdio>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>

#include <iio.h>

using namespace adiscope;

static const std::string tool_names[] = {
	"osc",
	"spectrum",
	"network",
	"siggen",
	"logic",
	"pattern",
	"dio",
	"dmm",
	"power",
	"debugger",
	"calibration",
	"launcher"
};

Filter::Filter(const struct iio_context *ctx)
{
	QFile file(":/filter.json");

	file.open(QIODevice::ReadOnly | QIODevice::Text);

	auto doc = QJsonDocument::fromJson(file.readAll());
	auto obj = doc.object();

	auto obj_keys = obj.keys();
	for (const auto &key : qAsConst(obj_keys)) {
		const auto child = obj[key].toObject();

		if (!child.contains("compatible-devices"))
			continue;

		const auto compatible_devices = child["compatible-devices"];
		if (!compatible_devices.isArray())
			continue;

		bool compatible = true;

		auto comp_dev = compatible_devices.toArray();
		for (const auto &value : qAsConst(comp_dev)) {
			if (!value.isString()) {
				compatible = false;
				break;
			}

			const auto str = value.toString().toStdString();
			compatible = !!iio_context_find_device(
					ctx, str.c_str());
			if (!compatible)
				break;
		}

		if (compatible) {
			this->root = child;
			hwname = key;
			return;
		}
	}

	fprintf(stderr, "Unable to find device in filter file\n");
	this->root = obj["generic"].toObject();
	hwname = "generic";
}

Filter::~Filter()
{
}

QString& Filter::hw_name()
{
	return hwname;
}

const std::string& Filter::tool_name(enum tool tool)
{
	return tool_names[tool];
}

bool Filter::compatible(enum tool tool) const
{
	auto hdl = root["compatible"];
	if (!hdl.isArray())
		return false;
	else
		return hdl.toArray().contains(
				QString::fromStdString((tool_names[tool])));
}

bool Filter::usable(enum tool tool, const std::string &dev) const
{
	auto hdl = root[QString::fromStdString(tool_names[tool] + "-devices")];
	if (hdl.isNull())
		return true;
	if (!hdl.isArray())
		return false;

	return hdl.toArray().contains(QString::fromStdString(dev));
}

const std::string Filter::device_name(enum tool tool, int idx) const
{
	auto hdl = root[QString::fromStdString(tool_names[tool] + "-devices")];
	if (hdl.isNull() || !hdl.isArray())
		throw std::runtime_error("Tool not compatible");

	auto array = hdl.toArray();
	if (idx >= array.size())
		throw std::runtime_error("Invalid IDX");

	return array[idx].toString().toStdString();
}

struct iio_device * Filter::find_device(const struct iio_context *ctx,
		enum tool tool, int idx) const
{
	return iio_context_find_device(ctx, device_name(tool, idx).c_str());
}

struct iio_channel * Filter::find_channel(const struct iio_context *ctx,
		enum tool tool, int idx, bool output) const
{
	QString name = QString::fromStdString(device_name(tool, idx));

	if (!name.contains(':'))
		throw std::runtime_error("Filter entry not iio_channel");

	struct iio_device *dev = iio_context_find_device(ctx,
			name.section(':', 0, 0).toStdString().c_str());
	if (!dev)
		return nullptr;

	return iio_device_find_channel(dev,
			name.section(':', 1, 1).toStdString().c_str(), output);
}
