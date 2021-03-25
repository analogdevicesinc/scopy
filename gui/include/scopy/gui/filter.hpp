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

#ifndef FILTER_HPP
#define FILTER_HPP

#include <QJsonObject>
#include <QString>

#include <string>

extern "C"
{
	struct iio_channel;
	struct iio_context;
	struct iio_device;
}

namespace scopy {
namespace gui {

enum tool
{
	TOOL_OSCILLOSCOPE = 0,
	TOOL_SPECTRUM_ANALYZER,
	TOOL_NETWORK_ANALYZER,
	TOOL_SIGNAL_GENERATOR,
	TOOL_LOGIC_ANALYZER,
	TOOL_PATTERN_GENERATOR,
	TOOL_DIGITALIO,
	TOOL_DMM,
	TOOL_POWER_CONTROLLER,
	TOOL_DEBUGGER,
	TOOL_CALIBRATION,
	TOOL_LAUNCHER,
};

class Filter
{
private:
	QJsonObject m_root;
	QString m_hwname;

public:
	Filter(const struct iio_context* ctx);
	~Filter();

	QString& hwName();

	bool compatible(enum tool tool) const;
	bool usable(enum tool tool, const std::string& dev) const;
	const std::string deviceName(enum tool tool, int idx = 0) const;

	struct iio_device* findDevice(const struct iio_context* ctx, enum tool tool, int idx = 0) const;
	struct iio_channel* findChannel(const struct iio_context* ctx, enum tool tool, int idx = 0,
					 bool output = false) const;

	static const std::string& toolName(enum tool tool);
};
} // namespace gui
} // namespace scopy

#endif /* FILTER_HPP */
