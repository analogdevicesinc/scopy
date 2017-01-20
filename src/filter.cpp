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
#include <QStringList>

#include <iio.h>

using namespace adiscope;

static const std::string tool_names[] = {
	"oscilloscope",
	"signal-generator",
	"voltmeter",
	"power-controller",
	"logic-analyzer-rx",
	"logic-analyzer-tx"
};

Filter::Filter(const struct iio_context *ctx)
{
	QFile file(":/filter.json");

	file.open(QIODevice::ReadOnly | QIODevice::Text);

	auto doc = QJsonDocument::fromJson(file.readAll());
	auto obj = doc.object();

	auto keys = obj.keys();

	for (auto it = keys.constBegin(); it != keys.constEnd(); ++it) {
		auto child = obj[*it].toObject();

		if (!child.contains("compatible-devices"))
			continue;

		auto compatible_devices = child["compatible-devices"];
		if (!compatible_devices.isArray())
			continue;

		auto dev_list = compatible_devices.toArray();
		bool compatible = true;

		for (auto it2 = dev_list.constBegin(); compatible &&
				it2 != dev_list.constEnd(); ++it2) {
			if (!it2->isString()) {
				compatible = false;
				break;
			}

			auto str = it2->toString().toStdString();
			compatible &= !!iio_context_find_device(
					ctx, str.c_str());
		}

		if (compatible) {
			this->root = child;
			hwname = *it;
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

const std::string Filter::device_name(enum tool tool, unsigned int idx) const
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
		enum tool tool, unsigned int idx) const
{
	return iio_context_find_device(ctx, device_name(tool, idx).c_str());
}

struct iio_channel * Filter::find_channel(const struct iio_context *ctx,
		enum tool tool, unsigned int idx, bool output) const
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
